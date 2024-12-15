#ifndef _pipes_h_
#  define _pipes_h_
#include <stdio.h>

FILE *open_pgp_pipe (const char **pgp_argv, const char *mode);
int close_pgp_pipe (FILE *stream);

#endif /* _pipes_h_ */
