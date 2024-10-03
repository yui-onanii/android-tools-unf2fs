#ifndef LIBF2FS_PRIVATE_H
#define LIBF2FS_PRIVATE_H

#include "fsck.h"
#include "node.h"

typedef void (*f2fs_ldir_cb)(const char *name,
                             __u16 name_len,
                             __u8 file_type,
                             nid_t ent_ino);

static inline int
f2fs_has_inline_dentry (struct f2fs_node *dir)
{
  return !!(dir->i.i_inline & F2FS_INLINE_DENTRY);
}

static inline unsigned int
dir_blocks (struct f2fs_node *dir)
{
  return ((unsigned long)(le32_to_cpu(dir->i.i_size) + F2FS_BLKSIZE - 1))
                                                     / F2FS_BLKSIZE;
}

struct f2fs_node *
f2fs_read_node_ (struct f2fs_sb_info *sbi,
                 nid_t tgt_ino);

int
f2fs_sendfile_ (struct f2fs_sb_info *sbi,
                struct f2fs_node *file_node,
                int out_fd);

void
f2fs_listdir_ (struct f2fs_sb_info *sbi,
               struct f2fs_node *dir,
               f2fs_ldir_cb cb);

#endif /* LIBF2FS_PRIVATE_H */
