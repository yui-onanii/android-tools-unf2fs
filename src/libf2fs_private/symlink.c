#include <stdlib.h>
#include <string.h>

#include "f2fs_private.h"

const char *
f2fs_readlink_ (struct f2fs_sb_info *sbi,
                struct f2fs_node *link_file)
{
  __u64 file_size;
  static char buff[8192];
  nid_t inum;
  struct dnode_of_data dn;
  char *data_blk;
  int ret;

  // for symlinks, this is the length of destination path
  file_size = le64_to_cpu(link_file->i.i_size);
  if (f2fs_has_inline_data (link_file))
    memcpy (buff,
            inline_data_addr (link_file),
            file_size);
  else
  {
    // the first data block should store link destination,
    // read from it.

    data_blk = calloc (F2FS_BLKSIZE, 1);
    if (!data_blk)
      abort ();

    inum = le32_to_cpu(F2FS_NODE_FOOTER(link_file)->ino);
    set_new_dnode (&dn, link_file, NULL, inum);
    get_dnode_of_data (sbi, &dn, 0, LOOKUP_NODE);

    ret = dev_read_block (data_blk, dn.data_blkaddr);
    ASSERT(ret >= 0);

    memcpy (buff, data_blk, file_size);

    // clean up
    if (dn.node_blk && dn.node_blk != dn.inode_blk)
      free(dn.node_blk);
    free (data_blk);
  }

  buff[file_size] = '\0';
  return buff;
}
