#include <stddef.h>
#include <stdlib.h>

#include "f2fs_private.h"

struct f2fs_node *
f2fs_read_inode_ (struct f2fs_sb_info *sbi,
                  nid_t tgt_ino)
{
  struct node_info ni;
  struct f2fs_node *res;

  res = malloc (F2FS_BLKSIZE);
  if (!res)
    abort ();

  get_node_info (sbi, tgt_ino, &ni);
  if (dev_read_block (res, ni.blk_addr) < 0)
  {
    free (res);
    return NULL;
  }

  return res;
}

#include "inline.c"

int
f2fs_sendfile_ (struct f2fs_sb_info *sbi,
                struct f2fs_node *file_node,
                int out_fd)
{
  int ret = -1;

  if (f2fs_has_inline_data (file_node))
  {
    if ((ret = f2fs_do_read_inline_data (out_fd,
                                         file_node)) < 0)
      goto out;
  }
  else
  {
    goto out;  // TBD
  }

  if ((ret = ftruncate (out_fd,
                        file_node->i.i_size)) < 0)
    goto out;

out:
  return ret;
}

uint64_t
f2fs_getcaps_ (struct f2fs_node *ent_node)
{
  return 0;  // TODO
}
