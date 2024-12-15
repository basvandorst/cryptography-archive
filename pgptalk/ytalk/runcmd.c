/* runcmd.c - run an external command via /bin/sh.  The difference to
   system(3) is that 1. /bin/sh is used always, and 2. that PATH is
   removed from the environment via putenv().
   
   Written 14 May 1993 by Stephan Neuhaus <neuhaus@informatik.uni-kl.de>
   and placed in the public domain.   NOTE: I know I should use pid_t
   instead of into for the fork result, but I was writing this for
   good transportability, and many systems do not know about pid_t. */
#ifdef HAVE_VFORK
#define FORK vfork
#else
#define FORK fork
#endif

#include <stdio.h>

int
run_shell_command (command_line)
     char *command_line;
{
  int child;
  int status;
  char *shell_argv[4];

  shell_argv[0] = "sh";
  shell_argv[1] = "-c";
  shell_argv[2] = command_line;
  shell_argv[3] = 0;
  
  child = FORK ();
  if (child == 0) {
    putenv ("PATH=");
    execv ("/bin/sh", shell_argv);
    return -1;
  } else if (child > 0) {
    if (waitpid (child, &status, 0) != child)
      return -1;
    else
      return status;
  } else
    return -1;
}

static int popen_child;

FILE *
open_shell_pipe (command, type)
     char *command;
     char *type;
{
  char *shell_argv[4];
  FILE *res;
  int fd;
  int pipe_fd[2];
    
  shell_argv[0] = "sh";
  shell_argv[1] = "-c";
  shell_argv[2] = command;
  shell_argv[3] = 0;

  if (pipe (pipe_fd) == -1)
    return 0;
  
  popen_child = FORK ();
  if (popen_child == 0) {
    putenv ("PATH=");
    if (type[0] == 'r') {
      close (1);
      close (pipe_fd[0]);
      dup (pipe_fd[1]);
    } else {
      close (0);
      close (pipe_fd[1]);
      dup (pipe_fd[0]);
    }
    execv ("/bin/sh", shell_argv);
    return 0;
  } else if (popen_child > 0) {
    if (type[0] == 'r') {
      close (0);
      close (pipe_fd[1]);
      fd = dup (pipe_fd[0]);
    } else {
      close (1);
      close (pipe_fd[0]);
      fd = dup (pipe_fd[1]);
    }
    res = fdopen (fd, type);
    return res;
  }
}

int
close_shell_pipe (fp)
     FILE *fp;
{
  int status;
  
  if (waitpid (popen_child, &status, 0) != popen_child)
    return -1;
  if (fclose (fp) == -1)
    return -1;
  return status;
}
