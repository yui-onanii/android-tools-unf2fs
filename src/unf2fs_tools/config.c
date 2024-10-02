#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "f2fs_private.h"

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

static FILE *fscfg_fp = NULL;
static FILE *fsctx_fp = NULL;

int
config_setup_ (const char *part_name,
               const char *out_path)
{
  int ret;
  char fscfg_name[PATH_MAX];
  char fsctx_name[PATH_MAX];

  mkdir ("config", 0755);
  if ((ret = chdir ("config")) < 0)
  {
    err("Cannot open directory %s/%s\n",
        out_path, part_name);
    return ret;
  }

  snprintf (fscfg_name, sizeof (fscfg_name),
            "%s_fs_config", part_name);
  snprintf (fsctx_name, sizeof (fsctx_name),
            "%s_fs_contexts", part_name);

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
fscfg_append (const char *part_name,
              const char *path,
              struct f2fs_node *node)
{
  fprintf (fscfg_fp, "%s%s %u %u %#o\n",
           part_name, path,
           le32_to_cpu(node->i.i_uid),
           le32_to_cpu(node->i.i_gid),
           le16_to_cpu(node->i.i_mode) & 07777);
}

__attribute__((destructor)) static void
config_finalize_ (void)
{
  if (fscfg_fp)
    fclose (fscfg_fp);
  if (fsctx_fp)
    fclose (fsctx_fp);
}
