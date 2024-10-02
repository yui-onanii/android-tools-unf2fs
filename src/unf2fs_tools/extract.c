#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define err(s, ...)   printf ("Error: " s, ##__VA_ARGS__)

int
config_setup_ (const char *part_name,
               const char *out_path);

int
extract_setup (const char *input,
               const char *out_path)
{
  char *input_;
  char *part_name;
  char *dot_pos;
  int ret;

  mkdir (out_path, 0755);
  if ((ret = chdir (out_path)) < 0)
  {
    err("Cannot open directory %s\n", out_path);
    return ret;
  }

  // remove dir components and extensions
  input_ = strdup (input);
  part_name = basename (input_);
  if ((dot_pos = strchr (part_name, '.')))
    *dot_pos = '\0';

  if ((ret = config_setup_ (part_name,
                            out_path)) < 0)
    goto out;

  mkdir (part_name, 0755);
  if ((ret = chdir (part_name)) < 0)
    err("Cannot open directory %s/%s\n",
        out_path, part_name);

out:
  free (input_);
  return ret;
}

int
extract_one_file (const char *name,
                  const char *path)
{
  int fd;

  if ((fd = open (name,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0644)) < 0)
  {
    err("Cannot open file %s\n", path);
    return fd;
  }

  close (fd);
  return 0;
}

int
extract_enter_dir (const char *name,
                   const char *path)
{
  int ret;

  mkdir (name, 0755);
  if ((ret = chdir (name)) < 0)
    err("Cannot open directory %s\n", path);

  return ret;
}

void
extract_leave_dir (void)
{
  if (chdir ("..") < 0)
    abort ();
}
