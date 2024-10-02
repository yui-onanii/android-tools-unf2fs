#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "f2fs_private.h"

#define measure(t)                                \
{                                                 \
  if (clock_gettime (CLOCK_MONOTONIC, &t) < 0)    \
    abort ();                                     \
}

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

static struct f2fs_sb_info *gsbi;

static char path_buf[PATH_MAX] = {};
static char *path_end;

static void
handle_entry (const char *name,
              __u16 name_len,
              __u8 file_type,
              nid_t ent_ino)
{
  char name_[PATH_MAX];
  struct node_info ent_ni;
  struct f2fs_node *ent_node;
  char *old_end;

  if (!name_len)
    return;

  if (is_dot_dotdot ((void *)name, name_len))
    return;

  // make a NUL-terminated name for our use
  if (name_len > sizeof (name_) - 1)
    name_len = sizeof (name_) - 1;
  memcpy (name_, name, name_len);
  name_[name_len] = '\0';

  if (file_type == F2FS_FT_DIR)
  {
    ent_node = malloc (F2FS_BLKSIZE);
    if (!ent_node)
      abort ();

    get_node_info (gsbi, ent_ino, &ent_ni);
    if (dev_read_block (ent_node, ent_ni.blk_addr) < 0)
    {
      // should this ever happen?
      err("can't read inode %u\n", ent_ino);
      goto skip;
    }

    // enter dir
    old_end = path_end;
    path_end += snprintf (old_end,
                          sizeof (path_buf) - (old_end - path_buf),
                          "%s/", name_);



    f2fs_listdir_ (gsbi, ent_node, &handle_entry);

    // leave dir
    path_end = old_end;
    *old_end = '\0';

skip:
    free (ent_node);
  }
  else
  {
    // dont touch path_end
    strncpy (path_end, name_,
             sizeof (path_buf) - (path_end - path_buf));



    *path_end = '\0';
  }
}

static inline void
do_traverse (struct f2fs_sb_info *sbi,
             struct f2fs_node *root)
{
  gsbi = sbi;
  path_end = stpcpy (path_buf, "/");
  f2fs_listdir_ (gsbi, root, &handle_entry);
}

static inline struct timespec
interval (struct timespec start,
          struct timespec end)
{
  struct timespec diff;

  if (end.tv_nsec < start.tv_nsec  /* need borrow */)
  {
    end.tv_sec--;
    end.tv_nsec += 1e9;  // 1s
  }
  diff.tv_sec = end.tv_sec - start.tv_sec;
  diff.tv_nsec = end.tv_nsec - start.tv_nsec;
  return diff;
}

static inline const char *
elapsed (struct timespec start,
         struct timespec end)
{
  struct timespec diff;
  static char buff[20];

  diff = interval (start, end);
  snprintf (buff, sizeof (buff),
            "%" PRIu64 ".%09" PRIu64 "s",
            diff.tv_sec, diff.tv_nsec);
  return buff;
}

void
traverse_main (struct f2fs_sb_info *sbi,
               struct f2fs_node *root)
{
  struct timespec start;
  struct timespec end;

  measure(start);
  printf ("scan all files...\n");

  do_traverse (sbi, root);

  measure(end);
  printf ("Done. %s\n", elapsed (start, end));
}
