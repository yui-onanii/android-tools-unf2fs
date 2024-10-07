#include <stdio.h>
#include <string.h>

const char *
re_esc_str_ (const char *s)
{
  static char buff[8192];
  char *cur;
  char c;

  // prepend a backslash for every found special char
  for (cur = buff; ; )
  {
    c = *(s++);
    if (!c)
      break;
    if (strchr ("\\|()[]{}^$*+?.", c))
      cur += sprintf (cur, "\\%c", c);
    else
      *(cur++) = c;
  }

  *cur = '\0';
  return buff;
}
