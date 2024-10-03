// modified from f2fs-tools/fsck/dir.c
// also linux/fs/f2fs/dir.c, mainly f2fs_readdir

#include "f2fs_private.h"

static /* struct f2fs_dir_entry * */ inline void find_target_dentry(f2fs_ldir_cb cb, /* const u8 *name,
		unsigned int len, f2fs_hash_t namehash, int *max_slots, */
		struct f2fs_dentry_ptr *d)
{
	struct f2fs_dir_entry *de;
	unsigned long bit_pos = 0;
	int max_len = 0;

	/*if (max_slots)
		*max_slots = 0;*/
	while (bit_pos < (unsigned long)d->max) {
		if (!test_bit_le(bit_pos, d->bitmap)) {
			bit_pos++;
			max_len++;
			continue;
		}

		de = &d->dentry[bit_pos];
		/*if (le16_to_cpu(de->name_len) == len &&
			de->hash_code == namehash &&
			!memcmp(d->filename[bit_pos], name, len)) {
			goto found;
		}*/
                cb((void *)d->filename[bit_pos],
                   le16_to_cpu(de->name_len),
                   de->file_type,
                   le32_to_cpu(de->ino));

		/*if (max_slots && max_len > *max_slots)
			*max_slots = max_len;*/
		max_len = 0;
		bit_pos += GET_DENTRY_SLOTS(le16_to_cpu(de->name_len));
	}
	//de = NULL;
//found:
	/*if (max_slots && max_len > *max_slots)
		*max_slots = max_len;*/
	//return de;
}

static /* struct f2fs_dir_entry * */ inline void find_in_block(void *block,
		//const u8 *name, int len, f2fs_hash_t namehash,
		f2fs_ldir_cb cb
		/* int *max_slots */ )
{
	struct f2fs_dentry_ptr d;

	make_dentry_ptr(&d, NULL, block, 1);
	/* return */ find_target_dentry(cb, /* name, len, namehash, max_slots, */ &d);
}

#include "inline.c"

static /* int */ inline void /* find_in_level */ find_in_dir(struct f2fs_sb_info *sbi, struct f2fs_node *dir,
		/* unsigned int level, struct dentry *de, */ f2fs_ldir_cb cb )
{
	//unsigned int nbucket, nblock;
	unsigned int bidx, end_block;
	//struct f2fs_dir_entry *dentry = NULL;
	struct dnode_of_data dn;
	void *dentry_blk;
	//int max_slots = 214;
	nid_t ino = le32_to_cpu(F2FS_NODE_FOOTER(dir)->ino);
	//f2fs_hash_t namehash;
	//unsigned int dir_level = dir->i.i_dir_level;
	int ret = 0;

	if (f2fs_has_inline_dentry(dir))
	{
		/* dentry = */ f2fs_find_in_inline_dir(dir, /* de->name, de->len, namehash */ cb);
		/*if (dentry) {
			ret = 1;
			de->ino = le32_to_cpu(dentry->ino);
		}*/
		return /* ret */ ;
	}

	/*namehash = f2fs_dentry_hash(get_encoding(sbi), IS_CASEFOLDED(&dir->i),
					de->name, de->len);*/

	/*nbucket = dir_buckets(level, dir_level);
	nblock = bucket_blocks(level);*/

	bidx = /* dir_block_index(level, dir_level, le32_to_cpu(namehash) % nbucket) */ 0;
	end_block = /* bidx + nblock */ dir_blocks(dir);

	dentry_blk = calloc(F2FS_BLKSIZE, 1);
	ASSERT(dentry_blk);

	memset(&dn, 0, sizeof(dn));
	for (; bidx < end_block; bidx++) {

		/* Firstly, we should know direct node of target data blk */
		if (dn.node_blk && dn.node_blk != dn.inode_blk)
			free(dn.node_blk);

		set_new_dnode(&dn, dir, NULL, ino);
		get_dnode_of_data(sbi, &dn, bidx, LOOKUP_NODE);
		if (dn.data_blkaddr == NULL_ADDR)
			continue;

		ret = dev_read_block(dentry_blk, dn.data_blkaddr);
		ASSERT(ret >= 0);

		/* dentry = */ find_in_block(dentry_blk, cb /*, de->name, de->len,
						namehash, &max_slots */ );
		/*if (dentry) {
			ret = 1;
			de->ino = le32_to_cpu(dentry->ino);
			break;
		}*/
	}

	if (dn.node_blk && dn.node_blk != dn.inode_blk)
		free(dn.node_blk);
	free(dentry_blk);

	//return ret;
}

static /* int */ inline void f2fs_find_entry(struct f2fs_sb_info *sbi,
				struct f2fs_node *dir /* , struct dentry *de */ , f2fs_ldir_cb cb )
{
	/*unsigned int max_depth;
	unsigned int level;

	max_depth = le32_to_cpu(dir->i.i_current_depth);
	for (level = 0; level < max_depth; level ++) {*/
		/* if (find_in_level */ find_in_dir(sbi, dir /*, level , de */ , cb ) // )
			/* return 1 */ ;
	//}
	//return 0;
}

/* og behavior: return ino if file exists, otherwise return 0 */
/* nid_t */ void /* f2fs_lookup */ f2fs_listdir_(struct f2fs_sb_info *sbi, struct f2fs_node *dir //,
				/* u8 *name, int len */ , f2fs_ldir_cb cb)
{
	//int err;
	/*struct dentry de = {
		.name = name,
		.len = len,
	};*/

	/* err = */ f2fs_find_entry(sbi, dir /*, &de */ , cb);
	/*if (err == 1)
		return de.ino;
	else
		return 0;*/
}
