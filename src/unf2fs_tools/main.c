#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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

void
unf2fs_main (const char *input,
             const char *out_path);

int
main (int argc, char *argv[])
{
  int opt;
  struct option long_opts[] = {
    { "help",     no_argument,       NULL, 'h' },
    { "out-path", required_argument, NULL, 'o' },
    { NULL,       0,                 NULL, 0   }
  };
  char *input;
  char *out_path = ".";

  while ((opt = getopt_long (argc, argv,
                             ":ho:", long_opts,
                             NULL)) != EOF)
  {
    switch (opt)
    {
      case ':':
        die("option -%c requires a value\n", optopt);
      case 'h':
        die_usage ();
      case 'o':
        out_path = optarg;
        break;
      case '?':
        if (optopt)
        {
          die("no such option: \"-%c\"\n", optopt);
        }
        else
        {
          die("no such option: \"%s\"\n", argv[optind - 1]);
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
