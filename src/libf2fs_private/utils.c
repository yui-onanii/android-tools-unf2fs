/*
 * miscellaneous routines for convenience
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "android_filesystem_capability.h"

#include "f2fs_private.h"

/*
 * f2fs has multiple type of nodes
 * but with a inode number it should always give us an inode
 */
struct f2fs_node *
f2fs_read_inode_ (struct f2fs_sb_info *sbi,
                  nid_t tgt_ino)
{
  struct node_info ni;
  struct f2fs_node *res;

  // yes, node takes a full block, we dont want sizeof
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

// for f2fs_do_read_inline_data
#include "inline.c"

/*
 * one can also kang these from f2fs-tools/fsck/dump.c
 * i didn't knew until i finished writing ;_;
 */
int
f2fs_sendfile_ (struct f2fs_sb_info *sbi,
                struct f2fs_node *file_node,
                int out_fd)
{
  int ret = -1;
  struct stat fs;
  off_t off;
  __u64 size;
  void *ptr;
  nid_t inum;

  if ((ret = fstat (out_fd, &fs)) < 0)
    goto out;

  if ((off = lseek (out_fd, 0, SEEK_CUR)) == (off_t)-1)
    goto out;

  size = le64_to_cpu(file_node->i.i_size);
  if (off + size > fs.st_size
      && (ret = ftruncate (out_fd,
                           off + size)) < 0)
    goto out;

  if (f2fs_has_inline_data (file_node))
  {
    // file is small
    // f2fs will de-inline if the file become bigger
    // so just read these and we're done
    if ((ret = f2fs_do_read_inline_data (out_fd,
                                         file_node)) < 0)
      goto out;
  }
  else
  {
    if (le32_to_cpu(file_node->i.i_flags) & F2FS_COMPR_FL)
    {
      // TODO: support compressed files
      printf ("Compressed File! Support Coming Soon!\n");
      ret = -1;
      goto out;
    }

    // f2fs_read want a buffer instead of a fd

    ptr = mmap (NULL, off + size, PROT_WRITE,
                MAP_SHARED, out_fd, 0);
    if (ptr == MAP_FAILED)
    {
      ret = -1;
      goto out;
    }

    madvise (ptr + off, size, MADV_SEQUENTIAL);

    inum = le32_to_cpu(F2FS_NODE_FOOTER(file_node)->ino);
    // f2fs_read is already buffered based on block size
    if (f2fs_read (sbi, inum, ptr + off, size, 0) != size)
    {
      ret = -1;
      goto unmap;
    }

    ret = 0;

unmap:
    munmap (ptr, off + size);
  }

out:
  return ret;
}

uint64_t
f2fs_parse_caps_ (struct vfs_cap_data *cap_data);

uint64_t
f2fs_getcaps_ (struct f2fs_sb_info *sbi,
               struct f2fs_node *ent_node)
{
  nid_t inum;
  struct vfs_cap_data cap_data;
  int err;

  inum = le32_to_cpu(F2FS_NODE_FOOTER(ent_node)->ino);
  // security.capability: this key stores a vfs_cap_data struct
  err = f2fs_getxattr_ (sbi, inum,
                        F2FS_XATTR_INDEX_SECURITY,
                        XATTR_CAPS_SUFFIX,
                        &cap_data, sizeof (cap_data));
  if (err == -ENODATA ||
      err == -EUCLEAN ||
      err == -ENOMEM)
    return 0;

  if (err < 0)
    abort ();

  return f2fs_parse_caps_ (&cap_data);
}

const char *
f2fs_getcon_ (struct f2fs_sb_info *sbi,
              struct f2fs_node *ent_node)
{
  nid_t inum;
  static char buff[8192];  // FIXME
  int err;

  inum = le32_to_cpu(F2FS_NODE_FOOTER(ent_node)->ino);
  memset (buff, 0, sizeof (buff));
  // security.selinux: this key stores the selabel string
  err = f2fs_getxattr_ (sbi, inum,
                        F2FS_XATTR_INDEX_SECURITY,
                        XATTR_SELINUX_SUFFIX, buff,
                        sizeof (buff) - 1  /* NUL */ );
  if (err == -ENODATA ||
      err == -EUCLEAN ||
      err == -ENOMEM)
    return NULL;

  if (err < 0)
    abort ();

  return buff;
}
