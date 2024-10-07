/*
 * file path construction
 */

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
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

// Ok, now tell me how u still overflow this
static char path_buf[8192] = {};  // current file path
static char *path_end;  // NUL byte or current file name

// see extract.c

int
extract_one_file (struct f2fs_sb_info *sbi,
                  const char *name,
                  const char *path,
                  struct f2fs_node *file_node);

int
extract_enter_dir (const char *name,
                   const char *path,
                   struct f2fs_node *dir);

void
extract_leave_dir (void);

static void
handle_entry (const char *name,
              __u16 name_len,
              __u8 file_type,
              nid_t ent_ino)
{
  struct f2fs_node *ent_node;
  char *old_end;

  // sanity check
  if (!name_len)
    return;

  if (is_dot_dotdot ((void *)name, name_len))
    return;  // skip specials

  // append file/dir name
  memcpy (path_end, name, name_len);
  path_end[name_len] = '\0';

  if (!(ent_node = f2fs_read_inode_ (gsbi, ent_ino)))
  {
    // should this ever happen?
    err("can't read inode %u (%s)\n", ent_ino, path_buf);
    goto quit;
  }

  if (file_type == F2FS_FT_DIR)
  {
    if (extract_enter_dir (path_end, path_buf, ent_node) < 0)
      goto out;

    // enter dir
    old_end = path_end;
    path_end += name_len;
    path_end = stpcpy (path_end, "/");
    // XXX: let's just hope this wont overflow call stack
    f2fs_listdir_ (gsbi, ent_node, &handle_entry);

    extract_leave_dir ();

    // leave dir
    path_end = old_end;
  }
  else
    extract_one_file (gsbi, path_end, path_buf, ent_node);

out:
  free (ent_node);

quit:
  // pop file/dir name
  *path_end = '\0';
}

// see config.c

void
fscfg_append (const char *path,
              struct f2fs_node *ent_node,
              uint64_t caps, int root);

void
fsctx_append (const char *path,
              const char *selabel,
              int root);

static inline void
do_traverse (struct f2fs_sb_info *sbi,
             struct f2fs_node *root)
{
  uint64_t root_caps;
  const char *root_selabel;

  // mimic f2fsUnpack
  root_caps = f2fs_getcaps_ (root);
  fscfg_append ("/", root, root_caps, 1);
  root_selabel = f2fs_getcon_ (root);
  fsctx_append ("/", root_selabel, 1);

  gsbi = sbi;
  path_end = stpcpy (path_buf, "/");
  f2fs_listdir_ (gsbi, root, &handle_entry);
}

#define interval(start, end)                            \
({                                                      \
  struct timespec diff;                                 \
                                                        \
  if (end.tv_nsec < start.tv_nsec  /* need borrow */)   \
  {                                                     \
    end.tv_sec--;                                       \
    end.tv_nsec += 1e9;  /* 1s */                       \
  }                                                     \
  diff.tv_sec = end.tv_sec - start.tv_sec;              \
  diff.tv_nsec = end.tv_nsec - start.tv_nsec;           \
                                                        \
  diff;                                                 \
})

static inline const char *
elapsed (struct timespec start, struct timespec end)
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
  struct timespec start, end;

  measure(start);
  printf ("scan all files...\n");

  do_traverse (sbi, root);

  measure(end);
  printf ("Done. %s\n", elapsed (start, end));
}
