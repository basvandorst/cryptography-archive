#include <stdio.h>
#include "mod_limit.h"

extern char *optarg;
extern int optind, opterr;

void setuser(GDBM_FILE dbf, unsigned long newval, char *user)
{
  datum key, content;
  struct user_limit *info;

  key.dptr = user;
  key.dsize = strlen(user) + 1;
  content = gdbm_fetch(dbf, key);
  if(content.dptr == NULL)
    {
      printf("no user %s, creating...\n", user);
      info = (struct user_limit *) malloc(1 * sizeof(struct user_limit));
      strncpy(info->account, user, 15);
      info->bytes_sent = 0;
    }
  else
    {
      info = (struct user_limit *) content.dptr;
    }
  info->bytes_max = newval * 1024;
  content.dptr = (char *) info;
  content.dsize = sizeof(struct user_limit);
  if(gdbm_store(dbf, key, content, GDBM_REPLACE) != 0)
    {
      fprintf(stderr, "store failed: %s\n", gdbm_strerror(gdbm_errno));
    }
}

void dumpuser(GDBM_FILE dbf, char *user, int print_zeroes)
{
  datum key, content;
  struct user_limit *info;
  key.dptr = user;
  key.dsize = strlen(user) + 1;
  content = gdbm_fetch(dbf, key);
  if(content.dptr == NULL)
    {
      fprintf(stderr, "no data on %s: %s\n",
	      user, gdbm_strerror(gdbm_errno));
      return;
    }
  info = (struct user_limit *) content.dptr;

  if(print_zeroes || info->bytes_sent)
    printf("%16s: %15.4f %15.4f\n",
	   info->account, ((float) info->bytes_sent) / (float) 1024,
	   ((float) info->bytes_max) / (float) 1024);
}

void cleartallies(GDBM_FILE dbf)
{
  datum key, content;

  key = gdbm_firstkey(dbf);
  while(key.dptr)
    {
      /* Clear the tally here */
      content = gdbm_fetch(dbf, key);
      ((struct user_limit *) content.dptr)->bytes_sent = 0;
      gdbm_store(dbf, key, content, GDBM_REPLACE);
      key = gdbm_nextkey(dbf, key);
    }
}

void dumpdb(GDBM_FILE dbf)
{
  datum key;

  key = gdbm_firstkey(dbf);
  while(key.dptr)
    {
      dumpuser(dbf, key.dptr, 0);
      key = gdbm_nextkey(dbf, key);
    }
}

void usage(char *program)
{
  fprintf(stderr, "yousuck\n");
}

int main(int argc, char **argv)
{
  char *user = NULL;
  char *filename = NULL;
  unsigned long newval = 0;
  GDBM_FILE dbf;
  int dump = 0;
  int clear = 0;
  int opt;

  /* -u user */
  /* -d dump */
  /* -s set */
  /* -f file */
  /* -c clear */
  while((opt = getopt(argc, argv, "u:ds:f:c")) != -1)
    {
      switch(opt)
	{
	  case 'u':
	     user = optarg;
	     break;
          case 'f':
	     filename = optarg;
	     break;
          case 'd':
	     dump = 1;
	     break;
          case 'c':
	     clear = 1;
	     break;
	  case 's':
	     newval = atol(optarg);
	     break;
          default:
	     usage(argv[0]);
	     exit(1);
	}
    }
  if(filename == NULL)
    {
      usage(argv[0]);
      exit(1);
    }
  dbf = gdbm_open(filename, 512, GDBM_WRCREAT, 0600, 0);
  if(!dbf)
    {
      fprintf(stderr, "can't open db: %s\n",
	      gdbm_strerror(gdbm_errno));
      exit(gdbm_errno);
    }
  if(dump && user == NULL)
    {
      /* dump everything */
      printf ("dumping\n");
      dumpdb(dbf);
    }
  if(clear)
    {
      cleartallies(dbf);
    }
  if(user != NULL)
    {
      /* Deal with a user */
      if(newval)
	{
	  setuser(dbf, newval, user);
	}
      dumpuser(dbf, user, 1);
    }
  gdbm_close(dbf);
  return(0);
}
