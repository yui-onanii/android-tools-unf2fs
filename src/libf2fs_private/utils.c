#include <stddef.h>
#include <stdlib.h>

#include "f2fs_private.h"

struct f2fs_node *
f2fs_read_node_ (struct f2fs_sb_info *sbi,
                 nid_t tgt_ino)
{
  struct node_info ni;
  struct f2fs_node *node;

  node = malloc (F2FS_BLKSIZE);
  if (!node)
    abort ();

  get_node_info (sbi, tgt_ino, &ni);
  if (dev_read_block (node, ni.blk_addr) < 0)
  {
    free (node);
    return NULL;
  }

  return node;
}

int
f2fs_sendfile_ (struct f2fs_sb_info *sbi,
                struct f2fs_node *file_node,
                int out_fd)
{
  return -1;
}
