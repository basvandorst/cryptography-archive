/* pipes.c -- open and close pipes to a PGP process.
   Adapted from the GNU C library's "pipestream.c".  */
#include <errno.h>
/*#include <stddef.h>*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "config.h"
#include "debug.h"

#ifdef HAVE_VFORK
#define FORK vfork
#else
#define FORK fork
#endif

/* Structure describing a child process.  */
struct child {
  int fd;			/* File descriptor of pipe */
  pid_t pid;			/* Process ID of child */
  struct child *next;		/* Next child in list */
};

/* Head of children list.  */
static struct child *head = NULL;

/* Open a new stream that is a one-way pipe to a
   child process running the given PGP command.  */
FILE *
open_pgp_pipe (const char **pgp_argv, const char *mode)
{
  pid_t pid;
  int pipedes[2];
  FILE *stream;
  struct child *child;

  /* Check arguments.  */
  if (pgp_argv == NULL || mode == NULL || (*mode != 'r' && *mode != 'w')) {
    errno = EINVAL;
    return NULL;
  }

  /* Create the pipe.  */
  if (pipe (pipedes) < 0)
    return NULL;

  /* Fork off the child.  */
  pid = FORK ();
  if (pid == (pid_t) -1) {
    /* Fork failed.  Save errno because it might be set by close().  */
    int errno_save = errno;
    
    (void) close (pipedes[0]);
    (void) close (pipedes[1]);
    errno = errno_save;
    return NULL;
  } else if (pid == (pid_t) 0) {
    /* We are the child side.  First, remove PATH from the
       environment, then make the write side of the pipe be stdin or
       the read side be stdout.  */
    putenv ("PATH=");
    
    if ((*mode == 'w' ? dup2 (pipedes[STDIN_FILENO], STDIN_FILENO) :
	 dup2 (pipedes[STDOUT_FILENO], STDOUT_FILENO)) < 0)
      _exit (127);

    /* Close the pipe descriptors.  */
    (void) close (pipedes[STDIN_FILENO]);
    (void) close (pipedes[STDOUT_FILENO]);

    /* Exec the PGP process. */
    (void) execv (PGPPROG, pgp_argv);

    /* Die if exec failed.  */
    _exit (127);
  }

  /* We are the parent side.  */

  /* Close the irrelevant side of the pipe and open the relevant side
     as a new stream.  Mark our side of the pipe to close on exec, so
     new children won't see it.  */
  if (*mode == 'r') {
    (void) close (pipedes[STDOUT_FILENO]);
    (void) fcntl (pipedes[STDIN_FILENO], F_SETFD, FD_CLOEXEC);
    stream = fdopen (pipedes[STDIN_FILENO], mode);
  } else {
    (void) close (pipedes[STDIN_FILENO]);
    (void) fcntl (pipedes[STDOUT_FILENO], F_SETFD, FD_CLOEXEC);
    stream = fdopen (pipedes[STDOUT_FILENO], mode);
  }

  if (stream == NULL)
    goto error;

  child = (struct child *) malloc (sizeof (struct child));
  if (child == NULL)
    goto error;

  child->fd = fileno (stream);
  child->pid = pid;
  child->next = head;
  head = child;
  return stream;

error:;
  {
    /* The stream couldn't be opened or the child structure couldn't
       be allocated.  Kill the child and close the other side of the
       pipe.  */
    int errno_save = errno;
    (void) kill (pid, SIGKILL);
    if (stream == NULL)
      (void) close (pipedes[*mode == 'r' ? STDOUT_FILENO : STDIN_FILENO]);
    else
      (void) fclose (stream);
#ifdef HAVE_WAITPID
    (void) waitpid (pid, (int *) NULL, 0);
#else
    {
      pid_t dead;
      do
	dead = wait ((int *) NULL);
      while (dead > 0 && dead != pid);
    }
#endif
    errno = errno_save;
    return NULL;
  }
}


/* Close a stream opened py open_pgp_pipe and return its status.
   Returns -1 if the stream wasn't opened by open_pgp_pipe().  */
int
close_pgp_pipe (FILE *stream)
{
  pid_t pid, dead;
  int status;
  struct child *child;
  struct child *p_child;

  for (child = head;
       child != 0 && child->fd != fileno (stream);
       child = child->next)
    ;

  if (child == 0) {
    errno = EINVAL;
    return -1;
  }

  pid = child->pid;
  if (child == head)
    head = child->next;
  else {
    for (p_child = head; p_child->next != child; p_child = p_child->next)
      ;
    p_child->next = child->next;
  }

  free (child);

  if (fclose (stream) != 0)
    return -1;

#ifdef HAVE_WAITPID
  dead = waitpid (pid, &status, 0);
#else
  do
    dead = wait (&status);
  while (dead > 0 && dead != pid);
#endif

  if (dead != pid)
    status = -1;

  return status;
}
