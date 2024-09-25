#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <f2fs_fs.h>

#include "fsck.h"

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

struct f2fs_fsck gfsck;

INIT_FEATURE_TABLE;

static inline void
do_unfs (struct f2fs_sb_info *sbi)
{
  struct node_info ni;
  struct f2fs_node *root;

  root = malloc (sizeof (struct f2fs_node));
  if (!root)
  {
    err("can't malloc root node");
    return;
  }

  get_node_info (sbi, F2FS_ROOT_INO(sbi), &ni);
  if (dev_read_block (root, ni.blk_addr) < 0)
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
  struct f2fs_sb_info *sbi;

  if (test_file (input) < 0)
  {
    err("Cannot open file %s\n", input);
    return;
  }

  f2fs_init_configuration ();
  c.devices[0].path = strdup (input);
  check_block_struct_sizes ();

  if (f2fs_get_device_info () < 0)
    return;
  if (f2fs_get_f2fs_info () < 0)
    return;

  memset (&gfsck, 0, sizeof (gfsck));
  gfsck.sbi.fsck = &gfsck;
  sbi = &gfsck.sbi;

  if (f2fs_do_mount (sbi))
    goto out_err;

  do_unfs (sbi);

  f2fs_do_umount (sbi);
  if (f2fs_finalize_device ())
    return;

  return;

out_err:
  if (sbi->ckpt)
    free (sbi->ckpt);
  if (sbi->raw_super)
    free (sbi->raw_super);
  f2fs_release_sparse_resource ();
}
