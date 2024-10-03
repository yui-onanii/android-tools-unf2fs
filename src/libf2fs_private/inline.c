// based on linux/fs/f2fs/inline.c

//#include <stddef.h>

#include "f2fs_private.h"

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
