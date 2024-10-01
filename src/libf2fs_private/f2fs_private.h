#ifndef LIBF2FS_PRIVATE_H
#define LIBF2FS_PRIVATE_H

#include "fsck.h"
#include "node.h"

typedef void (*f2fs_ldir_cb)(nid_t ent_ino);

void
f2fs_listdir_ (struct f2fs_sb_info *sbi,
               struct f2fs_node *dir,
               f2fs_ldir_cb cb);

#endif /* LIBF2FS_PRIVATE_H */
