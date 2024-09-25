#include <alloca.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#ifndef BUILD_DATE
# define BUILD_DATE     "yymmdd"
#endif
#define UNF2FS_BANNER   "unf2fs 1.0." BUILD_DATE

#define err(s, ...)     printf ("Error: " s, ##__VA_ARGS__)

#define die(s, ...)         \
  err(s, ##__VA_ARGS__);    \
  exit (1)

#ifndef MIN
# define MIN(a, b)      ((a < b) ? a : b)
#endif

static inline void
pr_usage_short (void)
{
  printf ("Usage: command [OPTIONS] INPUT\n"
          "\n");
}

static inline void
pr_usage_full (void)
{
  pr_usage_short ();
  printf ("Options:\n"
          "  -o, --out-path TEXT  output path\n"
          "  -h, --help           Show this message and exit\n"
          "\n"
          "Arguments:\n"
          "  INPUT  input file\n");
}

__attribute__((noreturn)) static inline void
die_usage (void)
{
  pr_usage_full ();
  exit (0);
}

static inline void
assert_unexpected_args (size_t n, char *args[])
{
  size_t i;

  if (!n)
    return;

  pr_usage_short ();
  err("Got unexpected extra arguments (%s", args[0]);
  for (i = 1; i < MIN(n, 3); i++)
    printf (" %s", args[i]);

  n -= i;
  if (n)
    printf (" ...");

  printf (")\n");
  exit (1);
}

static struct option long_opts[] = {
  { "help",     no_argument,       NULL, 'h' },
  { "out-path", required_argument, NULL, 'o' },
  { NULL,       0,                 NULL,  0  }
};

static inline void
assert_unexpected_arg (const char *opt)
{
  struct option *option;
  const char *pos;
  size_t len;
  char *buff;

  for (option = long_opts; option->name; option++)
  {
    if (option->val == optopt)
    {
      if (option->has_arg == no_argument)
      {
        pos = strchr (opt, '=');
        if (!pos)
          abort ();

        len = pos - opt;
        buff = alloca (len + 1);

        memcpy (buff, opt, len);
        buff[len] = '\0';

        die("option %s does not take a value\n", buff);
      }

      abort ();
    }
  }
}

const char *
calc_suggestion(const char *dir[],
                size_t dir_size,
                const char *name);

__attribute__((noreturn)) static inline void
abort_unk_long_opt (const char *opt)
{
  size_t n_opts;
  const char **names;
  size_t i;
  const char *suggest;

  for (n_opts = 0; long_opts[n_opts].name; n_opts++);
  names = alloca (n_opts * sizeof (const char *));
  for (i = 0; i < n_opts; i++)
    names[i] = long_opts[i].name;

  suggest = calc_suggestion (names, n_opts, opt + 2);
  if (suggest)
  {
    die("no such option: \"%s\". "
        "Did you mean \"--%s\"?\n", opt, suggest);
  }
  else
  {
    die("no such option: \"%s\"\n", opt);
  }
}

void
unf2fs_main (const char *input,
             const char *out_path);

int
main (int argc, char *argv[])
{
  int opt;
  char *input;
  char *out_path = ".";

  while ((opt = getopt_long (argc, argv,
                             ":ho:", long_opts,
                             NULL)) != EOF)
  {
    switch (opt)
    {
      case ':':
        die("option %s requires a value\n", argv[optind - 1]);
      case 'h':
        die_usage ();
      case 'o':
        out_path = optarg;
        break;
      case '?':
        if (optopt)
        {
          assert_unexpected_arg (argv[optind - 1]);
          die("no such option: \"-%c\"\n", optopt);
        }
        else
        {
          abort_unk_long_opt (argv[optind - 1]);
        }
      default:
        abort ();
    }
  }

  if (optind >= argc)
  {
    pr_usage_short ();
    die("Missing argument \"INPUT\"\n");
  }
  input = argv[optind++];

  assert_unexpected_args (argc - optind, &argv[optind]);

  printf (UNF2FS_BANNER "\n");
  unf2fs_main (input, out_path);

  return 0;
}
