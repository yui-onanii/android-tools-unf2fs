/*
 * helper routines for inline feature
 *
 * based on linux/fs/f2fs/inline.c
 */

//#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

#include "f2fs_private.h"

/*
 * for small file/directory, f2fs might inline dentry/file data in the inode block directly,
 * so handle it.
 */

#ifdef IN_LIBF2FS_LISTDIR
/* struct f2fs_dir_entry * */ void f2fs_find_in_inline_dir(struct f2fs_node *dir,
								/* const u8 *name, int len, f2fs_hash_t namehash */
								f2fs_ldir_cb cb)
{
	//struct f2fs_dir_entry *de;
	struct f2fs_dentry_ptr d;
	void *inline_dentry;

	inline_dentry = inline_data_addr(dir);

	make_dentry_ptr(&d, dir, inline_dentry, 2);
	/* de = */ find_target_dentry(cb, /* name, len, namehash, NULL, */ &d);

	/* return de; */
}
#endif

#ifndef MIN
# define MIN(a, b)    ((a < b) ? a : b)
#endif

static inline int f2fs_do_read_inline_data(int fd, struct f2fs_node *node_blk)
{
	if (write(fd, inline_data_addr(node_blk), MIN(node_blk->i.i_size, MAX_INLINE_DATA(node_blk))) < 0)
		return -1;

	return 0;
}
