#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
  int null_fd;

  if (enabled)
  {
    if (saved_fd < 0)
      abort ();

    if (dup2 (saved_fd, STDOUT_FILENO) < 0)
      abort ();

    close (saved_fd);
    saved_fd = -1;
  }
  else
  {
    if (saved_fd >= 0)
      abort ();

    saved_fd = dup (STDOUT_FILENO);
    if (saved_fd < 0)
      abort ();

    null_fd = open ("/dev/null", O_WRONLY);
    if (null_fd < 0)
      abort ();

    if (dup2 (null_fd, STDOUT_FILENO) < 0)
      abort ();
    close (null_fd);
  }
#endif
}

static inline void
do_unfs (struct f2fs_sb_info *sbi)
{
  struct node_info ni;
  int ret;
  struct f2fs_node *root;

  root = malloc (sizeof (struct f2fs_node));
  if (!root)
  {
    err("can't malloc root node");
    return;
  }

  BEGIN_LIBF2FS_CALL();
    get_node_info (sbi, F2FS_ROOT_INO(sbi), &ni);
    ret = dev_read_block (root, ni.blk_addr);
  END_LIBF2FS_CALL();
  if (ret < 0)
  {
    err("can't read root node");
    goto out;
  }

out:
  free (root);
}

static inline int
test_file (const char *path)
{
  int fd;

  fd = open (path, O_RDWR);
  if (fd < 0)
    return fd;

  close (fd);
  return 0;
}

void
unf2fs_main (const char *input,
             const char *out_path)
{
  int ret;
  struct f2fs_sb_info *sbi;

  if (test_file (input) < 0)
  {
    err("Cannot open file %s\n", input);
    return;
  }

  BEGIN_LIBF2FS_CALL();
    f2fs_init_configuration ();
  END_LIBF2FS_CALL();
  c.devices[0].path = strdup (input);

  BEGIN_LIBF2FS_CALL();
    check_block_struct_sizes ();
    ret = f2fs_get_device_info ();
  END_LIBF2FS_CALL();
  if (ret < 0)
    return;

  BEGIN_LIBF2FS_CALL();
    ret = f2fs_get_f2fs_info ();
  END_LIBF2FS_CALL();
  if (ret < 0)
    return;

  memset (&gfsck, 0, sizeof (gfsck));
  gfsck.sbi.fsck = &gfsck;
  sbi = &gfsck.sbi;

  BEGIN_LIBF2FS_CALL();
    ret = f2fs_do_mount (sbi);
  END_LIBF2FS_CALL();
  if (ret)
    goto out_err;

  do_unfs (sbi);

  BEGIN_LIBF2FS_CALL();
    f2fs_do_umount (sbi);
    ret = f2fs_finalize_device ();
  END_LIBF2FS_CALL();
  if (ret)
    return;

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
