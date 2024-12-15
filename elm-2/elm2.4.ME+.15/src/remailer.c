#include <unistd.h>
#include <sys/wait.h>
#include "headers.h"
#include "me.h"

#ifdef USE_REMAILER
extern int errno;
extern int remailing;

#define REMAILER_SITE "remailer-list@kiwi.cs.berkeley.edu"

#define BOL 1
#define STR 2
#define INT 3

typedef struct remailer {
	char name[STRING];
	char path[STRING];
	unsigned int pgp : 1;
	unsigned int eric : 1;
	unsigned int cpunk : 1;
	unsigned int penet : 1;
	unsigned int latent : 1;
	unsigned int hash : 1;
	unsigned int special : 1;
	long time;
	struct remailer *next;
} remailer_t;

typedef struct remail {
  int encrypt; /* Use encryption through the servers? */
  char path[STRING]; /* User-defined route.  Overrides "chain". */
  int chain; /* How many sites to chain this message through? */
  remailer_t *info;
  char address[STRING];
} remail_t;

static remail_t remailer;

#include "menu2.h"
static struct menu_item remail_items[] = {
  { "R)email:", 'r', 3, BOL, (char *) &remailing },
  { "C)hain:", 'c', 5, INT, (char *) &remailer.chain },
  { "remail P)ath:", 'p', 6, STR, (char *) &remailer.path },
  { "E)ncrypt:", 'e', 8, BOL, (char *) &remailer.encrypt }
};
#define MAX_ITEMS 4

void
remailer_menu ()
{
  /* Reset to the defaults... */
  remailer.encrypt = FALSE;
  remailer.path[0] = '\0';
  remailer.chain = 3;
  remailer.info = NULL;

  generic_menu (&remail_items, MAX_ITEMS, "Remailer Configuration", "remailer: ");
}

static void
remailer_print_db (db)
     remailer_t *db;
{
  dprint(3, (debugfile, "remailer_print_db:\n"));
  while (db) {
    dprint(3, (debugfile, "\t%s = %d\n", db->name, db->time));
    db = db->next;
  }
}

static int
remailer_parse_opts (buf, r)
     char *buf;
     remailer_t *r;
			       
{
  short i = 0;
  char *ptr, *ptr2;

  ptr = strchr (buf, '"');
  if (!ptr)
    return 0;

  ptr++;
  while (*ptr && (*ptr != '"'))
    r->name[i++] = *ptr++;
  r->name[i] = '\0';

  ptr = strchr (ptr, '<');
  if (!ptr)
    return 0;

  ptr++;
  i = 0;
  while (*ptr && (*ptr != '>'))
    r->path[i++] = *ptr++;
  r->path[i] = '\0';

  ptr2 = strrchr (ptr, '"');
  if (!ptr2)
    return 0;

  *ptr2 = '\0';
  while ((ptr2 = strtok (ptr, " \t\r\n")) != NULL) {
    if (strcmp (ptr2, "pgp") == 0)
      r->pgp = 1;
    else if (strcmp (ptr2, "hash") == 0)
      r->hash = 1;
    else if (strcmp (ptr2, "eric") == 0)
      r->eric = 1;
    else if (strcmp (ptr2, "penet") == 0)
      r->penet = 1;
    else if (strcmp (ptr2, "cpunk") == 0)
      r->cpunk = 1;
    else if (strcmp (ptr2, "latent") == 0)
      r->latent = 1;
    else if (strcmp (ptr2, "special") == 0)
      r->special = 1;
    ptr = 0;
  }
  return 1;
}

static remailer_t *
remailer_lookup (char *name, remailer_t *db)
{
  while (db) {
    if (strcmp (name, db->name) == 0)
      return (db);
    db = db->next;
  }
  dprint (3, (debugfile, "remailer_lookup(): %s not found.\n", name));
  return (0);
}

static remailer_t *
remailer_sort (remailer_t *p)
{
  /* sorts the remailers in increasing response time. */

  remailer_t *tmp;
  remailer_t *ptr;
  remailer_t *head;
  remailer_t *gopher;

  if (p == NULL)
    return (NULL);

  /* Pop the first element. */
  head = gopher = p;
  ptr = head->next;
  head->next = 0;
  
  /* No go through the rest of the list */
  while (ptr) {
    tmp = ptr;
    ptr = ptr->next;
    tmp->next = 0;
    
    gopher = head;
    while (gopher) {
      if (tmp->time <= gopher->time) {
	/* This case should only occur when gopher==head, so we don't have
	 * to worry about this not being the beginning of the list.
	 */
	tmp->next = gopher;
	head = tmp;
	break;
      }
      else if (gopher->next && (tmp->time < gopher->next->time)) {
	tmp->next = gopher->next;
	gopher->next = tmp;
	break;
      }
      gopher = gopher->next;
    }
  }
  remailer_print_db(head);
  return (head);
}

void
remailer_destroy_db ()
{
  remailer_t *p, *db = remailer.info;

  while (db) {
    p = db;
    db = db->next;
    free (p);
  }
}

static remailer_t *
remailer_get_db ()
{
  /* This routine retrives the list of remailers and their properties.
   * It first tries to read ~/.elm/remailers, and failing that, it
   * fingers whatever is defined in REMAILER_SITE for that info.
   */
  FILE *fpin;
  char buf[STRING], *c, buf2[STRING];
  short i = 0;
  int hour, min, sec;
  remailer_t *mailers = 0;
  remailer_t *end = 0;
  remailer_t *tmp;

  sprintf (buf, "%s/.elm/remailers", home);
  fpin = fopen (buf, "r");
  if (fpin == NULL) {
    int fd[2];

    if (pipe (fd) == -1) {
      dprint (1, (debugfile, "remailer_get_db(): ERROR: pipe (errno %d)\n", errno));
      return (NULL);
    }
  
    if (fork () == 0) {
      int tmp;

      setgid(groupid);
      setuid(userid);

      close (fd[0]);
      close (1);
      dup (fd[1]);
      close (fd[1]);
      execl(FINGER_PATH,FINGER_PATH,REMAILER_SITE,(char *)0);
      tmp = errno;
      perror(FINGER_PATH);
      _exit(tmp);
    }
  
    close (fd[1]);
    fpin = fdopen (fd[0], "r");
    if (fpin == NULL) {
      dprint (1, (debugfile, "remailer_get_db(): ERROR: fdopen (errno %d)\n", errno));
      return (NULL);
    }
    error1 ("Fingering %s...", REMAILER_SITE);
  }
  else
    error1 ("Reading %s...", buf);

  while (fgets (buf, STRING, fpin) != NULL) {
    if (strncmp (buf, "$remailer{", 10) == 0) {
      tmp = (remailer_t *) safe_malloc (sizeof (remailer_t));
  
      tmp->name[0] = '\0';
      tmp->path[0] = '\0';
      tmp->pgp     = 0;
      tmp->eric    = 0;
      tmp->cpunk   = 0;
      tmp->penet   = 0;
      tmp->latent  = 0;
      tmp->hash    = 0;
      tmp->special = 0;
      tmp->time    = 0;
      tmp->next    = 0;

      if (remailer_parse_opts (buf, tmp)) {
	if (mailers == 0)
	  mailers = end = tmp;
	else {
	  end->next = tmp;
	  end = end->next;
	}
	tmp->next = 0;
      } else {
	free((void *)tmp);
      }
    }
    else if (strncmp (buf, "-----", 5) == 0)
      break;
  }

  /* Phase 2 : retrieve the rankings (latency). */
  while (fgets (buf, STRING, fpin) != NULL) {
    c = buf;
    i = 0;
    while (*c && !isspace (*c))
      buf2[i++] = *c++;
    buf2[i] = '\0';
    tmp = remailer_lookup (buf2, mailers);
    if (tmp != NULL) {
      c = strchr (buf, ':');
      while (!isspace(*c))
	c--;
      c++;
      i = sscanf (c, "%d:%d:%d", &hour, &min, &sec);
      if (i == 3)
	tmp->time = (hour * 3600) + min * 60 + sec;
      else if (i == 2)
	tmp->time = hour * 60 + min;
      else {
	dprint (1, (debugfile, "remailer_get_db(): error parsing time: %s\n", c));
	error1 ("Could not convert time for remailer %s!", tmp->name);
	if (sleepmsg > 0)
	  sleep(sleepmsg);
      }
    }
  }
  fclose(fpin);
  remailer_print_db (mailers);
  return (remailer_sort (mailers));
}

static int
#ifdef MIME
remailer_write_msg (pdb, to, subj, src, tmpfn, pgp, mime_info)
     mime_send_t *mime_info;
#else
remailer_write_msg (pdb, to, subj, src, tmpfn, pgp)
#endif
     remailer_t *pdb;
     char *to, *subj, *src, *tmpfn;
{
  FILE *fpin, *fpout;
  char buf[VERY_LONG_STRING];

  if (pdb == NULL) {
    dprint (1, (debugfile, "remailer_write_msg(): ERROR!  pdb==NULL\n"));
    error ("Could not retrieve list of remailers!");
    if (sleepmsg > 0)
      sleep(sleepmsg);
    return(-1);
  }

  fpin = fopen (src, "r");
  if (!fpin) {
    dprint(1,(debugfile,"remailer_write_msg: %s: could not open for reading!\n",src));
    error("Error opening temp file!");
    if (sleepmsg > 0)
      sleep(sleepmsg);
    return(-1);
  }
    
  fpout = safeopen (tmpfn);
  if (!fpout) {
    dprint(1,(debugfile,"remailer_write_msg: %s: could not open for writing!\n",tmpfn));
    error("Error opening temp file!");
    if (sleepmsg > 0)
      sleep(sleepmsg);
    return(-1);
  }

  fputs ("::\n", fpout);
  if (pdb->cpunk)
    fputs ("Request-Remailing-To: ", fpout);
  else if (pdb->eric)
    fputs ("Anon-Send-To: ", fpout);
  else if (pdb->penet)
    fputs ("X-Anon-To: ", fpout);
  fprintf (fpout, "%s\n\n", to);
  /* Some remailers require the subject line to be in the "hash" area, so
   * use it if it's availible.
   */
  if (pdb->hash) {
    fprintf (fpout, "##\nSubject: %s\n", subj);
#ifdef MIME
    if (mime_info) {
      /* If this is a MIME message, write out a header in the "hash" area
       * so that the information will be preserved when the remailer
       * rewrites the message.
       */
      mime_write_header(fpout, mime_info);
      print_EOLN(fpout, mime_info->encoding_top);
    }
    else
#endif
      fputc('\n', fpout);
  }  
  while (fgets (buf, VERY_LONG_STRING, fpin) != NULL)
    fputs (buf, fpout);
  fclose (fpin);
  fclose (fpout);
#ifdef USE_PGP
  if (pgp && pdb->pgp) {
    pgp_encrypt (tmpfn, pdb->path, PGP_MESSAGE, FALSE);
    fpin = fopen (tmpfn, "r");
    fpout = fopen (src, "w");
    fputs ("::\nEncrypted: PGP\n\n", fpout);
    while (fgets (buf, VERY_LONG_STRING, fpin) != NULL)
      fputs (buf, fpout);
    fclose (fpin);
    fclose (fpout);
    unlink (tmpfn);
  }
  else
#endif
  {
#ifdef RENAME
    if (rename (tmpfn, src) < 0) {
      error ("Could not rename temp file!");
      dprint (1, (debugfile, "remailer_write_msg: ERROR!  rename(%s,%s); errno=%d\n", src, tmpfn, errno));
      return(-1);
    }
#else
    link (tmpfn, src);
    unlink (tmpfn);
#endif
  }
  dprint (2, (debugfile, "remailer_write_msg: routing through %s.\n", 
	      pdb->path));
  return 0;
}

int remailer_proc ()
{
  remailer_t *pdb;
  int chain = remailer.chain;
  char *c;

  /* See if there is anything to do... */
  if (chain > 0 || (remailer.path[0] != '\0')) {
    extern char expanded_to[];
      
    /* Save this information for later use... */
    pdb = remailer.info = remailer_get_db ();

    if (remailer.path[0] != '\0') {
      /* The user has specified a path to use. */
      c = remailer.path;
      while ((c = strtok (c, ";")) != NULL) {
	while (*c && isspace (*c))
	  c++;
	pdb = remailer_lookup (c, remailer.info);
	if (! pdb)
	  return(-1);
	c = NULL;
      }
    }
    else {
      remailer_t *savedb = 0;

      while (chain > 0) {
	if (remailer.encrypt) {
	  /* Find the next remailer which supports pgp. */
	  while (pdb && ((! pdb->pgp) || (pdb->time == 0)))
	    pdb = pdb->next;
	  if (! pdb)
	    return (-1);
	}
	else {
	  /* Some servers will only accept PGP encoded messages, so if PGP was
	   * not requested, skip all servers marked "special".  Also, we want
	   * to skip any servers with a time of zero, since that means there 
	   * wasn't any information availible.
	   */
	  while (pdb && (pdb->special || (pdb->time == 0)))
	    pdb = pdb->next;
	  if (! pdb)
	    return (-1);
	}
	savedb = pdb;
	pdb = pdb->next;
	chain--;
      }
      pdb = savedb;
    }

    /* "pdb" should now be set to the last entry.  We should set Elm's
     * concept of the "To:" address to this remailer.
     */
    strcpy (remailer.address, expanded_to);
    strcpy (expanded_to, pdb->path);
  }
  return(0);
}

static int
#ifdef MIME
remailer_genmsg (filename, to, subj, chain, pgp, mime_info)
     mime_send_t *mime_info;
#else
remailer_genmsg (filename, to, subj, chain, pgp)
#endif
     char *to, *subj, *filename;
     int chain, pgp;
{
  char tmpfn[STRING], *c;
  char remailto[STRING];
  remailer_t *db, *pdb;

  /* Set the initial address */
  strcpy(remailto, to);

  if (chain < 1 && (remailer.path[0] == '\0'))
    return (-1); /* nothing to do */

  pdb = db = remailer.info;
  if (db == NULL) {
    dprint(1, (debugfile, "remailer_genmsg: no database found!\n"));
    error("Could not find remailer database!");
    if (sleepmsg>0)
      sleep(sleepmsg);
    return(-1);
  }

  sprintf (tmpfn, "%selmRT%d", temp_dir, getpid());

  if (remailer.path[0] != '\0') {
    /* The user has specified a path to use. */
    c = remailer.path;
    while ((c = strtok (c, ";")) != NULL) {
      while (*c && isspace (*c))
	c++;
      pdb = remailer_lookup (c, db);
      if (! pdb) {
	dprint(1,(debugfile, "remailer_genmsg: could not find remailer '%s'\n",
		  c));
	error1("Could not find remailer '%s'.", c);
	if (sleepmsg > 0)
	  sleep(sleepmsg);
        return (-1);
      }
#ifdef MIME
      remailer_write_msg (pdb, remailto, subj, filename, tmpfn, pgp, mime_info);
#else
      remailer_write_msg (pdb, remailto, subj, filename, tmpfn, pgp);
#endif
      strcpy(remailto, pdb->path);
      c = NULL;
    }
  }
  else
    while (chain > 0) {
      if (pgp) {
	/* Find the next remailer which supports pgp. */
	while (pdb && ((! pdb->pgp) || (pdb->time == 0)))
	  pdb = pdb->next;
	if (! pdb)
	  return (-1);
      }
      else {
	/* Some servers will only accept PGP encoded messages, so if PGP was
	 * not requested, skip all servers marked "special".  Also, we want
	 * to skip any servers with a time of zero, since that means there 
	 * wasn't any information availible.
	 */
	while (pdb && (pdb->special || (pdb->time == 0)))
	  pdb = pdb->next;
        if (! pdb)
          return (-1);
      }
#ifdef MIME
      remailer_write_msg (pdb, remailto, subj, filename, tmpfn, pgp, mime_info);
#else
      remailer_write_msg (pdb, remailto, subj, filename, tmpfn, pgp);
#endif
      strcpy(remailto, pdb->path);
      pdb = pdb->next;
      chain--;
    }
  return (0);
}

#ifdef MIME
remailer_copy_message_across (reply, real_reply, copy, mime_info)
     mime_send_t *mime_info;
#else
remailer_copy_message_across (reply, real_reply, copy)
#endif
     FILE *reply, *real_reply;
     int copy;
{
  FILE *tmpfp;
  char tempfile[STRING], buf[VERY_LONG_STRING];
  extern char subject[];
  int ret;

  sprintf(tempfile, "%selmR%d", temp_dir, getpid());

  tmpfp = safeopen(tempfile);
  if (!tmpfp) {
    dprint(1, (debugfile, "remailer_copy_message_across: %s: could not open for writing!\n", tempfile));
    error("Could not open temp file for writing!");
    if (sleepmsg > 0)
      sleep(sleepmsg);
    return(-1);
  }

#ifdef MIME
  copy_message_across(reply, tmpfp, copy, mime_info);
#else
  copy_message_across(reply, tmpfp, copy);
#endif
  fclose(tmpfp);
#ifdef MIME
  ret = remailer_genmsg (tempfile, remailer.address, subject, remailer.chain,
			 remailer.encrypt, mime_info);
#else
  ret = remailer_genmsg (tempfile, remailer.address, subject, remailer.chain,
			 remailer.encrypt);
#endif
  if (ret == -1) {
    error("Error while formatting for remailer!");
    if (sleepmsg > 0)
      sleep(sleepmsg);
    unlink(tempfile);
    return(-1);
  }
  tmpfp = fopen(tempfile, "r");
  if (!tmpfp) {
    dprint(1, (debugfile, "remailer_copy_message_across: %s: could not open for reading!\n", tempfile));
    error("Could not read temp file!");
    if (sleepmsg>0)
      sleep(sleepmsg);
    unlink(tempfile);
    return(-1);
  }
  while (mail_gets(buf, sizeof(buf), tmpfp) > 0)
    fputs(buf, real_reply);
  unlink(tempfile);
  return 0;
}
#endif
