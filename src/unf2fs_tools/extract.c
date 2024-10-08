/*
 * dump file/directory
 */

#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "f2fs_private.h"

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

// see config.c
int
config_setup_ (const char *part_name,
               const char *out_path);

static char g_buf[8192];
static char *g_part_name;

int
extract_setup (const char *input,
               const char *out_path)
{
  char *input_;
  char *dot_pos;
  int ret;

  mkdir (out_path, 0755);
  if ((ret = chdir (out_path)) < 0)
  {
    err("Cannot open directory %s\n", out_path);
    return ret;
  }

  // remove dir components and extensions
  input_ = strncpy (g_buf, input, sizeof (g_buf));
  g_part_name = basename (input_);
  if ((dot_pos = strchr (g_part_name, '.')))
    *dot_pos = '\0';

  if ((ret = config_setup_ (g_part_name,
                            out_path)) < 0)
    return ret;

  mkdir (g_part_name, 0755);
  if ((ret = chdir (g_part_name)) < 0)
    err("Cannot open directory %s/%s\n",
        out_path, g_part_name);

  return ret;
}

// see config.c

void
fscfg_append (const char *path,
              struct f2fs_node *ent_node,
              uint64_t caps, int root);

void
fsctx_append (const char *path,
              const char *selabel,
              int root);

int
extract_one_file (struct f2fs_sb_info *sbi,
                  const char *name,
                  const char *path,
                  struct f2fs_node *file_node)
{
  int fd = -1;
  uint64_t caps;
  const char *selabel;
  __u16 mode;
  const char *lnk_tgt;

  mode = le16_to_cpu(file_node->i.i_mode);
  if (LINUX_S_ISLNK(mode))
  {
    lnk_tgt = f2fs_readlink_ (sbi, file_node);
    if (symlink (lnk_tgt, name) < 0)
    {
      err("failed to create symlink %s (to %s)\n",
          path, lnk_tgt);
      return -1;
    }
    goto write_config;
  }

  if ((fd = open (name,
                  O_RDWR | O_CREAT | O_TRUNC,
                  0644)) < 0)
  {
    err("Cannot open file %s\n", path);
    return fd;
  }

  if (f2fs_sendfile_ (sbi, file_node, fd) < 0)
  {
    err("failed to dump %s\n", path);
    unlink (name);
    goto out;
  }

write_config:
  caps = f2fs_getcaps_ (sbi, file_node);
  fscfg_append (path, file_node, caps, 0);
  selabel = f2fs_getcon_ (sbi, file_node);
  fsctx_append (path, selabel, 0);

out:
  close (fd);

  return 0;
}

int
extract_enter_dir (struct f2fs_sb_info *sbi,
                   const char *name,
                   const char *path,
                   struct f2fs_node *dir)
{
  int ret;
  uint64_t caps;
  const char *selabel;

  mkdir (name, 0755);
  if ((ret = chdir (name)) < 0)
    err("Cannot open directory %s\n", path);

  caps = f2fs_getcaps_ (sbi, dir);
  fscfg_append (path, dir, caps, 0);
  selabel = f2fs_getcon_ (sbi, dir);
  fsctx_append (path, selabel, 0);

  return ret;
}

void
extract_leave_dir (void)
{
  if (chdir ("..") < 0)
    abort ();
}
