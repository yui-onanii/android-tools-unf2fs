#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "f2fs_private.h"

#define measure(t)                                \
{                                                 \
  if (clock_gettime (CLOCK_MONOTONIC, &t) < 0)    \
    abort ();                                     \
}

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

static struct f2fs_sb_info *gsbi;

static void
handle_entry (const char *name,
              __u16 name_len,
              __u8 file_type,
              nid_t ent_ino)
{
  struct node_info ent_ni;
  struct f2fs_node *ent_node;

  if (is_dot_dotdot ((void *)name, name_len))
    return;

  if (file_type == F2FS_FT_DIR)
  {
    ent_node = malloc (F2FS_BLKSIZE);
    if (!ent_node)
      abort ();

    get_node_info (gsbi, ent_ino, &ent_ni);
    if (dev_read_block (ent_node, ent_ni.blk_addr) < 0)
    {
      err("can't read inode %u\n", ent_ino);
      goto skip;
    }

    f2fs_listdir_ (gsbi, ent_node, &handle_entry);

skip:
    free (ent_node);
  }
}

static inline void
do_traverse (struct f2fs_sb_info *sbi,
             struct f2fs_node *root,
             const char *out_path)
{
  gsbi = sbi;
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
               struct f2fs_node *root,
               const char *out_path)
{
  struct timespec start;
  struct timespec end;

  measure(start);
  printf ("scan all files...\n");

  do_traverse (sbi, root, out_path);

  measure(end);
  printf ("Done. %s\n", elapsed (start, end));
}
