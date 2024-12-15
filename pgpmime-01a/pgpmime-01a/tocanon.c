/* A small utility to make sure that text is in canonical form.
 * The end of a line of text should be denoted with CRLF, but on Unix
 * machines, this is generally not the local convention.
 */

#include <stdio.h>

#define STRING 1024

main ()
{
  char buffer[STRING];
  int len;

  while (fgets (buffer, STRING, stdin) != NULL) {
    len = strlen (buffer);
    if (buffer[len - 2] != '\r') {
      buffer[len - 1] = '\0';
      fputs (buffer, stdout);
      fputs ("\r\n", stdout);
    }
    else
      fputs (buffer, stdout);
  }
  exit (0);
}
