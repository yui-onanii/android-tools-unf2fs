/*
 * for writing fsconfig and fscontexts
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "f2fs_private.h"

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

static const char *g_part_name;

static FILE *fscfg_fp = NULL;
static FILE *fsctx_fp = NULL;

int
config_setup_ (const char *part_name,
               const char *out_path)
{
  int ret;
  char fscfg_name[PATH_MAX];
  char fsctx_name[PATH_MAX];

  g_part_name = part_name;

  mkdir ("config", 0755);
  if ((ret = chdir ("config")) < 0)
  {
    err("Cannot open directory %s/%s\n",
        out_path, g_part_name);
    return ret;
  }

  snprintf (fscfg_name, sizeof (fscfg_name),
            "%s_fs_config", g_part_name);
  snprintf (fsctx_name, sizeof (fsctx_name),
            "%s_fs_contexts", g_part_name);

  fscfg_fp = fopen (fscfg_name, "w");
  if (!fscfg_fp)
  {
    err("Cannot open file %s/config/%s\n",
        out_path, fscfg_name);
    return -1;
  }

  fsctx_fp = fopen(fsctx_name, "w");
  if (!fsctx_fp)
  {
    err("Cannot open file %s/config/%s\n",
        out_path, fsctx_name);
    return -1;
  }

  if (chdir ("..") < 0)
    abort ();

  return ret;
}

void
fscfg_append (const char *path,
              struct f2fs_node *ent_node,
              uint64_t caps, int root)
{
  if (!root)
    fwrite (g_part_name, strlen (g_part_name), 1, fscfg_fp);

  fprintf (fscfg_fp, "%s %u %u %#o",
           path,
           le32_to_cpu(ent_node->i.i_uid),
           le32_to_cpu(ent_node->i.i_gid),
           le16_to_cpu(ent_node->i.i_mode) & 07777);

  // optional
  if (caps)
    fprintf (fscfg_fp, " capabilities=%#lx", caps);

  fwrite ("\n", 1, 1, fscfg_fp);

  if (root)
    fscfg_append (path, ent_node, caps, 0);
}

// see regex.c
const char *
re_esc_str_ (const char *s);

void
fsctx_append (const char *path,
              const char *selabel,
              int root)
{
  if (!root)
  {
    path = re_esc_str_ (path);
rerun_no_esc:
    fprintf (fsctx_fp, "/%s", g_part_name);
  }

  fwrite (path, strlen (path), 1, fsctx_fp);

  if (selabel)
    fprintf (fsctx_fp, " %s", selabel);

  fwrite ("\n", 1, 1, fsctx_fp);

  if (root)
  {
    path = "(/.*)?";
    root = 0;
    goto rerun_no_esc;
  }
}

__attribute__((destructor)) static void
config_finalize_ (void)
{
  if (fscfg_fp)
    fclose (fscfg_fp);
  if (fsctx_fp)
    fclose (fsctx_fp);
}
