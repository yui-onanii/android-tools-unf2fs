#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "f2fs_private.h"

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

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

static const char *
get_basename (const char *path)
{
  const char *pos;

  pos = strrchr (path, '/');
  if (pos)
    return pos + 1;

  return path;
}

void
fscfg_append (const char *path,
              struct f2fs_node *node);

int
extract_one_file (struct f2fs_sb_info *sbi,
                  const char *path,
                  struct f2fs_node *file_node)
{
  const char *name;
  int fd;

  name = get_basename (path);

  if ((fd = open (name,
                  O_WRONLY | O_CREAT | O_TRUNC,
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

  fscfg_append (path, file_node);

out:
  close (fd);

  return 0;
}

int
extract_enter_dir (const char *path,
                   struct f2fs_node *dir)
{
  const char *name;
  int ret;

  name = get_basename (path);

  mkdir (name, 0755);
  if ((ret = chdir (name)) < 0)
    err("Cannot open directory %s\n", path);

  fscfg_append (path, dir);

  return ret;
}

void
extract_leave_dir (void)
{
  if (chdir ("..") < 0)
    abort ();
}
