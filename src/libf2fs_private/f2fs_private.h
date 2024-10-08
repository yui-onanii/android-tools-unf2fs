/*
 * handy header for our use
 */

#ifndef LIBF2FS_PRIVATE_H
#define LIBF2FS_PRIVATE_H

#include <stddef.h>
#include <stdint.h>

// private headers, since lots of things are not exported in f2fs_fs.h
#include "fsck.h"
#include "node.h"
#include "xattr.h"

#ifndef XATTR_CAPS_SUFFIX
#define XATTR_CAPS_SUFFIX   "capability"
#endif

typedef void (*f2fs_ldir_cb)(const char *name,
                             __u16 name_len,
                             __u8 file_type,
                             nid_t ent_ino);

static inline int
f2fs_has_inline_dentry (struct f2fs_node *dir)
{
  return !!(dir->i.i_inline & F2FS_INLINE_DENTRY);
}

static inline int
f2fs_has_inline_data (struct f2fs_node *file_node)
{
  return !!(file_node->i.i_inline & F2FS_INLINE_DATA);
}

/*
 * for directory, inode stored total size of dentry blocks
 */
static inline unsigned int
dir_blocks (struct f2fs_node *dir)
{
  // fast round-up
  return ((unsigned long)(le32_to_cpu(dir->i.i_size) + F2FS_BLKSIZE - 1))
                                                     / F2FS_BLKSIZE;
}

struct f2fs_node *
f2fs_read_inode_ (struct f2fs_sb_info *sbi,
                  nid_t tgt_ino);

int
f2fs_sendfile_ (struct f2fs_sb_info *sbi,
                struct f2fs_node *file_node,
                int out_fd);

void
f2fs_listdir_ (struct f2fs_sb_info *sbi,
               struct f2fs_node *dir,
               f2fs_ldir_cb cb);

int
f2fs_getxattr_ (struct f2fs_sb_info *sbi,
                nid_t ino, int index,
                const char *name,
                void *buff,
                size_t size);

uint64_t
f2fs_getcaps_ (struct f2fs_sb_info *sbi,
               struct f2fs_node *ent_node);

const char *
f2fs_getcon_ (struct f2fs_sb_info *sbi,
              struct f2fs_node *ent_node);

#endif /* LIBF2FS_PRIVATE_H */
