/*
 * copied from f2fs-tools/fsck/segment.c
 */

#include "f2fs_private.h"

u64 f2fs_read(struct f2fs_sb_info *sbi, nid_t ino, u8 *buffer,
					u64 count, pgoff_t offset)
{
	struct dnode_of_data dn;
	struct node_info ni;
	struct f2fs_node *inode;
	char *blk_buffer;
	u64 filesize;
	u64 off_in_blk;
	u64 len_in_blk;
	u64 read_count;
	u64 remained_blkentries;
	block_t blkaddr;
	void *index_node = NULL;

	memset(&dn, 0, sizeof(dn));

	/* Memory allocation for block buffer and inode. */
	blk_buffer = calloc(F2FS_BLKSIZE, 2);
	ASSERT(blk_buffer);
	inode = (struct f2fs_node*)(blk_buffer + F2FS_BLKSIZE);

	/* Read inode */
	get_node_info(sbi, ino, &ni);
	ASSERT(dev_read_block(inode, ni.blk_addr) >= 0);
	ASSERT(!S_ISDIR(le16_to_cpu(inode->i.i_mode)));
	ASSERT(!S_ISLNK(le16_to_cpu(inode->i.i_mode)));

	/* Adjust count with file length. */
	filesize = le64_to_cpu(inode->i.i_size);
	if (offset > filesize)
		count = 0;
	else if (count + offset > filesize)
		count = filesize - offset;

	/* Main loop for file blocks */
	read_count = remained_blkentries = 0;
	while (count > 0) {
		if (remained_blkentries == 0) {
			set_new_dnode(&dn, inode, NULL, ino);
			get_dnode_of_data(sbi, &dn, F2FS_BYTES_TO_BLK(offset),
					LOOKUP_NODE);
			if (index_node)
				free(index_node);
			index_node = (dn.node_blk == dn.inode_blk) ?
							NULL : dn.node_blk;
			remained_blkentries = ADDRS_PER_PAGE(sbi,
						dn.node_blk, dn.inode_blk);
		}
		ASSERT(remained_blkentries > 0);

		blkaddr = datablock_addr(dn.node_blk, dn.ofs_in_node);
		if (blkaddr == NULL_ADDR || blkaddr == NEW_ADDR)
			break;

		off_in_blk = offset % F2FS_BLKSIZE;
		len_in_blk = F2FS_BLKSIZE - off_in_blk;
		if (len_in_blk > count)
			len_in_blk = count;

		/* Read data from single block. */
		if (len_in_blk < F2FS_BLKSIZE) {
			ASSERT(dev_read_block(blk_buffer, blkaddr) >= 0);
			memcpy(buffer, blk_buffer + off_in_blk, len_in_blk);
		} else {
			/* Direct read */
			ASSERT(dev_read_block(buffer, blkaddr) >= 0);
		}

		offset += len_in_blk;
		count -= len_in_blk;
		buffer += len_in_blk;
		read_count += len_in_blk;

		dn.ofs_in_node++;
		remained_blkentries--;
	}
	if (index_node)
		free(index_node);
	free(blk_buffer);

	return read_count;
}
