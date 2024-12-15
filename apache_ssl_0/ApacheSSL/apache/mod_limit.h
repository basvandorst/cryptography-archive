#include <gdbm.h>

extern gdbm_error gdbm_errno;

struct user_limit
{
  char account[16];
  unsigned long bytes_sent;
  unsigned long bytes_max;
};

