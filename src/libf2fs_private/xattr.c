// modified from f2fs-tools/fsck/xattr.c
//
// removed xattr setting logics
// added code for dumping xattr value
// tweaked a bit for our need

#include "f2fs_private.h"

static struct f2fs_xattr_entry *__find_xattr(void *base_addr,
				void *last_base_addr, int index,
				size_t len, const char *name)
{
	struct f2fs_xattr_entry *entry;

	list_for_each_xattr(entry, base_addr) {
		if ((void *)(entry) + sizeof(__u32) > last_base_addr ||
			(void *)XATTR_NEXT_ENTRY(entry) > last_base_addr) {
			MSG(0, "xattr entry crosses the end of xattr space\n");
			return NULL;
		}

		if (entry->e_name_index != index)
			continue;
		if (entry->e_name_len != len)
			continue;
		if (!memcmp(entry->e_name, name, len))
			break;
	}
	return entry;
}

int /* f2fs_setxattr */ f2fs_getxattr_(struct f2fs_sb_info *sbi, nid_t ino, int index, const char *name,
		/* const */ void *value /* , size_t size, int flags */)
{
	struct f2fs_node *inode;
	void *base_addr;
	void *last_base_addr;
	struct f2fs_xattr_entry *here /*, *last */;
	struct node_info ni;
	int error = 0;
	int len;
	int found /* , newsize;
	__u32 new_hsize */ ;
	int ret;

	if (name == NULL)
		return -EINVAL;

	if (value == NULL)
		return -EINVAL;

	len = strlen(name);

	if (len > F2FS_NAME_LEN /* || size > MAX_VALUE_LEN */ )
		return -ERANGE;

	if (ino < 3)
		return -EINVAL;

	/* Now We just support selinux */
	ASSERT(index == F2FS_XATTR_INDEX_SECURITY);

	get_node_info(sbi, ino, &ni);
	inode = calloc(F2FS_BLKSIZE, 1);
	ASSERT(inode);
	ret = dev_read_block(inode, ni.blk_addr);
	ASSERT(ret >= 0);

	base_addr = read_all_xattrs(sbi, inode, true);
	ASSERT(base_addr);

	last_base_addr = (void *)base_addr + XATTR_SIZE(&inode->i);

	here = __find_xattr(base_addr, last_base_addr, index, len, name);
	if (!here) {
		//MSG(0, "Need to run fsck due to corrupted xattr.\n");
		error = /* -EINVAL */ -EUCLEAN;
		goto exit;
	}

	found = IS_XATTR_LAST_ENTRY(here) ? 0 : 1;

	if (/* (flags & XATTR_REPLACE) && */ !found) {
		error = -ENODATA;
		goto exit;
	} /*else if ((flags & XATTR_CREATE) && found) {
		error = -EEXIST;
		goto exit;
	}*/

	/*last = here;
	while (!IS_XATTR_LAST_ENTRY(last))
		last = XATTR_NEXT_ENTRY(last);

	newsize = XATTR_ALIGN(sizeof(struct f2fs_xattr_entry) + len + size);*/

	/* 1. Check space */
	/*if (value) {
		int free;*/
		/*
		 * If value is NULL, it is remove operation.
		 * In case of update operation, we calculate free.
		 */
		/*free = MIN_OFFSET - ((char *)last - (char *)base_addr);
		if (found)
			free = free + ENTRY_SIZE(here);
		if (free < newsize) {
			error = -ENOSPC;
			goto exit;
		}
	}*/

	/* 2. Remove old entry */
	//if (found) {
		/*
		 * If entry if sound, remove old entry.
		 * If not found, remove operation is not needed
		 */
		/*struct f2fs_xattr_entry *next = XATTR_NEXT_ENTRY(here);
		int oldsize = ENTRY_SIZE(here);

		memmove(here, next, (char *)last - (char *)next);
		last = (struct f2fs_xattr_entry *)((char *)last - oldsize);
		memset(last, 0, oldsize);

	}

	new_hsize = (char *)last - (char *)base_addr;*/

	/* 3. Write new entry */
	//if (value) {
		char *pval;
		/*
		 * Before we come here, old entry is removed.
		 * We just write new entry.
		 */
		/*memset(last, 0, newsize);
		last->e_name_index = index;
		last->e_name_len = len;
		memcpy(last->e_name, name, len);*/
		pval = /* last */ here->e_name + len;
		/*memcpy(pval, value, size);
		last->e_value_size = cpu_to_le16(size);
		new_hsize += newsize;
	}

	write_all_xattrs(sbi, inode, new_hsize, base_addr);*/

	/* inode need update */
	//ASSERT(update_inode(sbi, inode, &ni.blk_addr) >= 0);

	// ADDED BY UNF2FS: copy out xattr value
	memcpy (value, pval, here->e_value_size);
exit:
	free(inode);
	free(base_addr);
	return error;
}
