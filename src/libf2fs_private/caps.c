/*
 * helper for parsing file capability data
 */

// see https://github.com/tytso/e2fsprogs/blob/950a0d6/contrib/android/perms.c#L163-L164

#include <stdint.h>

#include "android_filesystem_capability.h"

#include "f2fs_private.h"

uint64_t
f2fs_parse_caps_ (struct vfs_cap_data *cap_data)
{
  uint64_t caps = 0;

  // sanity check
  if (!(le32_to_cpu(cap_data->magic_etc) & VFS_CAP_REVISION_2))
    goto out;

  caps |= le64_to_cpu(cap_data->data[0].permitted);
  caps |= le64_to_cpu(cap_data->data[1].permitted) << 32;

out:
  return caps;
}
