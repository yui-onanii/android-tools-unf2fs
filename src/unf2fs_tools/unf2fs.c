#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <f2fs_fs.h>

#include "fsck.h"

#define BEGIN_LIBF2FS_CALL()    \
{                               \
  set_stdout (0)

#define END_LIBF2FS_CALL()      \
  set_stdout (1);               \
}

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

struct f2fs_fsck gfsck;

INIT_FEATURE_TABLE;

static inline void
set_stdout (int enabled)
{
#ifdef NDEBUG
  static int saved_fd = -1;
  int nulfd;

  if (enabled)
  {
    if (saved_fd < 0 ||
        dup2 (saved_fd, STDOUT_FILENO) < 0)
      abort ();

    close (saved_fd);
    saved_fd = -1;
    return;
  }

  if (saved_fd >= 0 ||
      (saved_fd = dup (STDOUT_FILENO)) < 0 ||
      (nulfd = open ("/dev/null", O_WRONLY)) < 0
      || dup2 (nulfd, STDOUT_FILENO) < 0)
    abort ();
  close (nulfd);
#endif
}

void
traverse_main (struct f2fs_sb_info *sbi,
               struct f2fs_node *root,
               const char *out_path);

static inline void
do_unfs (struct f2fs_sb_info *sbi,
         const char *out_path)
{
  struct node_info ni;
  int ret;
  struct f2fs_node *root;

  root = malloc (sizeof (struct f2fs_node));
  if (!root)
    abort ();

  BEGIN_LIBF2FS_CALL();
    get_node_info (sbi, F2FS_ROOT_INO(sbi), &ni);
    ret = dev_read_block (root, ni.blk_addr);
  END_LIBF2FS_CALL();
  if (ret < 0)
  {
    err("can't read root node\n");
    goto out;
  }

  traverse_main (sbi, root, out_path);

out:
  free (root);
}

static inline int
assert_image (const char *path)
{
  int fd;
  struct stat fs;
  __le32 buff;
  __le32 magic = 0;
  int res = -1;

  if ((fd = open (path, O_RDWR)) < 0
      || fstat (fd, &fs) < 0)
  {
    err("Cannot open file %s\n", path);
    goto out;
  }

  if (fs.st_size < 4096 ||
      lseek (fd, F2FS_SUPER_OFFSET, SEEK_SET) != 1024
      || read (fd, &buff, sizeof (buff)) != 4 ||
      (magic = le32_to_cpu(buff)) != F2FS_SUPER_MAGIC)
  {
    err("bad magic %x != 0xf2f52010u\n", magic);
    goto out;
  }

  res = 0;

out:
  if (fd >= 0)
    close (fd);
  return res;
}

void
unf2fs_main (const char *input,
             const char *out_path)
{
  int ret;
  struct f2fs_sb_info *sbi;

  if (assert_image (input) < 0)
    return;

  BEGIN_LIBF2FS_CALL();
    f2fs_init_configuration ();
  END_LIBF2FS_CALL();
  c.devices[0].path = strdup (input);

  BEGIN_LIBF2FS_CALL();
    check_block_struct_sizes ();
    ret = f2fs_get_device_info ();
  END_LIBF2FS_CALL();
  if (ret < 0)
  {
    err("can't get image info\n");
    return;
  }

  BEGIN_LIBF2FS_CALL();
    ret = f2fs_get_f2fs_info ();
  END_LIBF2FS_CALL();
  if (ret < 0)
  {
    err("can't get f2fs info\n");
    return;
  }

  memset (&gfsck, 0, sizeof (gfsck));
  gfsck.sbi.fsck = &gfsck;
  sbi = &gfsck.sbi;

  BEGIN_LIBF2FS_CALL();
    ret = f2fs_do_mount (sbi);
  END_LIBF2FS_CALL();
  if (ret)
  {
    err("can't mount image\n");
    goto out_err;
  }

  do_unfs (sbi, out_path);

  BEGIN_LIBF2FS_CALL();
    f2fs_do_umount (sbi);
    ret = f2fs_finalize_device ();
  END_LIBF2FS_CALL();
  if (ret)
  {
#ifndef NDEBUG
    err("can't finalize libf2fs\n");
#endif
    return;
  }

  return;

out_err:
  if (sbi->ckpt)
    free (sbi->ckpt);
  if (sbi->raw_super)
    free (sbi->raw_super);
  BEGIN_LIBF2FS_CALL();
    f2fs_release_sparse_resource ();
  END_LIBF2FS_CALL();
}
