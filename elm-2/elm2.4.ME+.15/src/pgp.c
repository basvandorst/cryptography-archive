#include "headers.h"
#include "me.h"

#ifdef USE_PGP
#include <sys/time.h>
#include <errno.h>
#ifdef BSD
#  include <sys/wait.h>
#endif
#include "menu.h"

/* these are defined in mailmsg1.c */
extern char expanded_to[VERY_LONG_STRING];
extern char expanded_cc[VERY_LONG_STRING];
extern char expanded_bcc[VERY_LONG_STRING];
extern char subject[VERY_LONG_STRING];

extern int errno;

int pgp_keeppassfor = 300; /* 5 minutes */
static char pgp_passphrase[STRING];
static int pgp_expires;

/* the column in which the userid begins in the 'pgp -kv' command output */
#define PGP_USERID_OFFSET 30

void
pgp_void_passphrase ()
{
  int i;

  for (i = 0 ; i < STRING ; i++)
    pgp_passphrase[i] = '\0';
  pgp_expires = 0;
  return;
}

static int
QueryExpirePassphrase()
{
  struct timeval now;

  /* negative implies never expire */
  if (pgp_keeppassfor < 0)
    return(0);
  gettimeofday(&now, 0);
  if (now.tv_sec < pgp_expires) {
    pgp_expires = now.tv_sec + pgp_keeppassfor;
    return(0);
  }
  pgp_void_passphrase ();
  return(1);
}

/* 'n' is the key we are looking for.  'k' is the list of possible matches
   which contains 'len' entries.  prompt the user for which key s/he really
   wants to use. */
static char *
PGPSelectKey (n, k, len)
	char *n;
	char **k;
	int len;
{
  int i;
  char buf[STRING];
  menu_t menu;

  sprintf(buf, "Multiple keys match '%s':", n);
  MenuInit (&menu, buf, "Select key or 'q' to quit: ", 0);

  for (i = 0; i < len; i++)
    MenuAdd(&menu, k[i]);

  for (;;) {
    switch (MenuLoop(&menu)) {
    case 'q':
      MenuDestroy(&menu);
      return(0);
    case '\n':
    case '\r':
      MenuDestroy(&menu);
      return(k[MenuCurrent(menu)]);
    }
  }
  /* not reached */
}

static int
GetPGPKey (name, target)
	char *name;
	char *target;
{
  /* given "name", return the string to use during the pgp call to specify
     the key which the user means.  return -1 on error. */
  char buf[STRING], address[STRING], *c, **keys=0, *pc, userpart[STRING];
  int i=0, keys_len=0, keys_max=0, return_val=0, start=0;
  FILE *p;

  if (!name || !target)
    return(-1);

  if (index(name, '@') == NULL && index(name, '(') == NULL && index(name, '<') == NULL) {
    /* this is either just a username, or someone's real name.  in either
       case it only needs to be checked once. */

    strcpy(address, name);
    i = 2;
  }
  else {
    get_address_from (name, address);
  
    i=0;
    while (address[i] && address[i] != '@') {
      userpart[i] = address[i];
      i++;
    }
    userpart[i] = '\0';

    i = 0;
  }
  c = address;

  /* the following loop first checks to see if any keys with the full
     address, or real name, or finally the username exist */

  for (;;) {
    while (*c && isspace (*c)) c++; /* move past any leading space! */
    sprintf(buf, "%s +verbose=0 -kv '%s'", PGP_PATH, c);
    p = popen(buf, "r");
    if (!p)
      return(-1);
    while (fgets(buf, STRING, p) != NULL) {
      /* see if we've reached the beginning of the key listings... */
      if (!start && strncmp(buf, "pub", 3)==0)
	  start=1;

      if (start) {

	/* if we've read all the keys, stop here */
	if (buf[0] != 'p' && buf[0] != ' ')
	  break;

	if (keys_len == keys_max)
	  keys = (char**)DynamicArray(keys, sizeof(char*), &keys_max, 5);
	
	pc = rindex(buf, '\n');
	if (!pc) /* this shouldn't happen! */
	  continue;
	*pc = '\0';
	
	pc = buf + PGP_USERID_OFFSET;
	keys[keys_len] = (char*)safe_malloc(sizeof(char)*(strlen(pc)+1));
	strcpy(keys[keys_len], pc);
	++keys_len;
      }
    }
    pclose(p);
    if (keys_len > 0 || i > 1)
      break;
    else {
      if (i == 0) {
	get_real_name(name, address, sizeof (address));

	/* if there was no real name, go on to the userpart check */
	if (strcmp(name, address) == 0) {
	  c = userpart;
	  i++;
	}
      } else if (i == 1)
	c = userpart;
      i++;
    }
  }

  if (keys_len == 1) /* perfect match! */
    get_address_from(keys[0], target);
    /*
    ** strcpy(target,keys[0]); 
    **  changed back to get_address_from() to remove leading blanks! -VPa
    */
  else if (keys_len > 1) { /* ask the user which, if any, s/he meant */
    c = PGPSelectKey(name, keys, keys_len);
    if (c)
      get_address_from(c, target);
    else {
      target[0] = '\0';
      return_val = -1;
    }
  } else
    return_val = -1;

  DestroyDynamicArray(keys);

  return(return_val);
}

static int
pgp_call (filename, opts)
	char *filename;
	int opts;
{
  char tobuf[VERY_LONG_STRING] = {0};

  if (opts & PGP_MESSAGE) {
    /* build up the list of recipients */
    strcpy(tobuf, expanded_to);
    if (expanded_cc[0] != '\0') {
      strcat(tobuf, ", ");
      strcat(tobuf, expanded_cc);
    }
    if (expanded_bcc[0] != '\0') {
      strcat(tobuf, ", ");
      strcat(tobuf, expanded_bcc);
    }
    /* If the message is to be encrypted, give the user a chance to edit
     * the list of ids to encrypt to since the given address may not always
     * be correct.
     */
    PutLine0 (LINES-2, 0, "To: ");
    if (optionally_enter (tobuf, LINES-2, 4, TRUE, FALSE) < 0 || tobuf[0] == '\0')
      return FALSE;
  }
  return (pgp_encrypt (filename, tobuf, opts, auto_cc));
}

int
pgp_encrypt (filename, ids, opts, metoo)
	char *filename, *ids;
	int opts, metoo;
{
  int i, id_len=0, id_max=0, st, usepgppass=FALSE, fd[2];
  char
    keyid[STRING], buf[VERY_LONG_STRING], buf2[STRING], **id_array=0, *c,
    *p;
  
  dprint (2, (debugfile, "pgp_encrypt(): ids=\"%s\", encrypt=%s, sign=%s\n", ids, opts & PGP_MESSAGE ? "TRUE" : "FALSE", opts & PGP_SIGNED_MESSAGE ? "TRUE" : "FALSE"));

  p = ids;
  /* If this message is to be encrypted, look up the keys of all the
     recipients */
  if (opts & PGP_MESSAGE) {
    i=0;
    while ((c = strtok(p, ",")) != NULL) {
      if (GetPGPKey(c, keyid) == -1) {
	error1("Couldn't find key matching '%s'!",c);
	return FALSE;
      }
      if (id_len == id_max)
	id_array = (char**)DynamicArray(id_array, sizeof(char*), &id_max, 25);
      id_array[id_len] = 
	(char*)safe_malloc((strlen(keyid) + 1) * sizeof(char));
      strcpy(id_array[id_len], keyid);
      id_len++;
      p=NULL;
    }
  }

  if ((opts & PGP_SIGNED_MESSAGE) && pgp_keeppass && pgp_goodPassphrase()) {
    usepgppass = TRUE;
    pipe(fd);
  }

  /* build up the command */
  buf[0] = '\0';
  if (usepgppass) {
    sprintf(buf2, "PGPPASSFD=%d; export PGPPASSFD; ", fd[0]);
    strcat(buf, buf2);
  }
  strcat(buf, PGP_PATH);
  if (metoo)
    strcat(buf, " +encrypttoself=on");
  if (usepgppass)
    strcat(buf, " +batchmode");
  if (opts & PGP_SIGNED_MESSAGE)
    strcat(buf," +clearsig=on");

  strcat(buf, " +verbose=0 -atw");

  if (opts & PGP_SIGNED_MESSAGE)
    strcat(buf, "s");
  if (opts & PGP_MESSAGE)
    strcat(buf, "e");

  strcat(buf, " ");
  strcat(buf, filename);

  /* add all of the userid's protected inside quotes to prevent shell
     no-no's */
  if (opts & PGP_MESSAGE)
    for (i=0; i<id_len; i++) {
      strcat(buf, " '");
      strcat(buf, id_array[i]);
      strcat(buf, "'");
    }
  DestroyDynamicArray(id_array); /* don't need this any more... */

  if (usepgppass) {
/*    strcat(buf, " > /dev/null 2>&1");/**/
    Raw(OFF);
    ClearScreen();/**/

    write(fd[1], pgp_passphrase, strlen(pgp_passphrase));
    write(fd[1], "\n", 1); /* pgp expects this as a line terminator! */
    close(fd[1]);
  }
  else {
    Raw(OFF);
    ClearScreen();
    printf("Executing: %s\n\n", buf);
  }
  st = system_call(buf, 0);
/*  if (!usepgppass)/**/
    Raw(ON);
  
  if (st == 0) { /* pgp returns zero upon success */
    /* copy the file into it's final destination */
    sprintf(buf, "%s.asc", filename);
#ifdef RENAME
    if (rename(buf, filename) < 0) {
      error ("Could not rename temporary file!");
      return FALSE;
    }
#else
    if (link(buf, filename) < 0) {
      Centerline(LINES, "Could not create link to temporary file!");
      return FALSE;
    }
    /* this is not fatal, but a warning should be given */
    if (unlink(buf) < 0)
      error ("Could not unlink temporary file!");
#endif
  }
  else {
    error ("pgp returned a non-zero value!");
    if (pgp_keeppass)
      pgp_void_passphrase ();
    return FALSE;
  }
  return opts;
}

int
pgp_menu (filename)
	char *filename;
{
  int update = TRUE;

  for (;;) {
    if (update) {
      MoveCursor (LINES-3, 0);
      CleartoEOS ();
      Centerline (LINES-2, "e)ncrypt, s)ign, b)oth encrypt and sign");
      PutLine0 (LINES-3, 0, "pgp: ");
      update = FALSE;
    }
    switch(ReadCh()) {
    case 'e':
      Write_to_screen("Encrypt", 0);
      return (pgp_call (filename, PGP_MESSAGE));
    case 's':
      Write_to_screen("Sign", 0);
      return (pgp_call (filename, PGP_SIGNED_MESSAGE));
    case 'b':
      Write_to_screen("Sign and Encrypt", 0);
      return (pgp_call (filename, PGP_MESSAGE | PGP_SIGNED_MESSAGE));
    case '\n':
    case '\r':
      return FALSE;
    case ctrl('L'):
      update = 1;
    }
  }
  /* not reached */
}

int
pgp_mail_public_key ()
{
  char userid[SLEN], pgpkey[SLEN], tmpfil[STRING], cmd[STRING], subj[STRING];

  userid[0] = '\0';
  pgpkey[0] = '\0';
  PutLine0(LINES-2, 0, "Enter userid of public key: ");
  CleartoEOS ();
  if (optionally_enter(userid, LINES-2, 28, FALSE, FALSE) != 0)
    return(0);
  if (GetPGPKey(userid, pgpkey) < 0) {
    Centerline(LINES, "Sorry, couldn't find that userid.");
    ClearLine(LINES-2);
    return(0);
  }
  sprintf(tmpfil, "%selm.%d", temp_dir, getpid());
  sprintf(cmd, "%s +verbose=0 -kxa '%s' %s > /dev/null 2>&1", PGP_PATH, pgpkey, tmpfil);
  if (system_call(cmd, 0) == 0) 
    sprintf(included_file, "%s.asc", tmpfil);
  else {
    ClearLine(LINES);
    Centerline(LINES, "pgp returned a non-zero value!");
    return(TRUE);
  }

  /* set the default subject for this message */
  sprintf(subj, "PGP public key for %s", pgpkey);

  pgp_status = PGP_PUBLIC_KEY;

  /* Now send the message off! */
  send_msg ("", "", subj, FALSE, allow_forms, FALSE);

  unlink (included_file); /* make sure to clean up. */
  included_file[0] = '\0';
  pgp_status = 0; /* reset */
  return(TRUE);
}

static int
GetPassphrase ()
{
  struct timeval now;

  PutLine0(LINES-2, 0, "Please enter your passphrase: ");
  CleartoEOS();
  if (optionally_enter(pgp_passphrase, LINES-2, 30, FALSE, TRUE) != 0)
    return(0);
  gettimeofday(&now, 0);
  if (pgp_keeppassfor > 0)
    pgp_expires = now.tv_sec + pgp_keeppassfor;
  return(pgp_passphrase[0] != '\0');
}

int
pgp_goodPassphrase()
{
  if (pgp_passphrase[0] == '\0' || QueryExpirePassphrase())
    return(GetPassphrase());
  else
    return(1);
}

/* opens up a PGP process as a child and returns its stdin and stdout */
int
pgp_decrypt_init (fpin, fpout, opts)
FILE **fpin, **fpout;
int opts; /* PGP_MESSAGE or PGP_SIGNED_MESSAGE */
{
  int pgp_child_in[2];
  int pgp_child_out[2];
  int passpipe[2];
  int usepass=FALSE;
  int fork_ret;
  char cmd[STRING];

  dprint (2, (debugfile, "pgp_descrypt_init() called with opts=%d\n", opts));

  if (pipe(pgp_child_in) == -1)
    return(-1);
  if (pipe(pgp_child_out) == -1)
    return(-1);
  
  if ((opts & PGP_MESSAGE) && pgp_keeppass) {
    if (pipe(passpipe) == -1) {
      close(pgp_child_in[0]);
      close(pgp_child_out[0]);
      close(pgp_child_in[1]);
      close(pgp_child_out[1]);
      return(-1);
    }
    usepass = TRUE;
  }
  dprint (3, (debugfile, "usepass = %d.\n", usepass));

  Raw(OFF);
  ClearScreen();
  if ((fork_ret = fork()) == 0) {
    close (pgp_child_in[1]);
    close (0);
    dup (pgp_child_in[0]);
    close (pgp_child_in[0]);
    
    close (pgp_child_out[0]);
    close (1);
    dup (pgp_child_out[1]);
    close (pgp_child_out[1]);
    
    /* build up the command */
    if (usepass) {
      close (passpipe[1]);
      sprintf (cmd, "PGPPASSFD=%d; export PGPPASSFD; ",
	       passpipe[0]);
    }
    else
      cmd[0] = '\0';
    strcat(cmd,PGP_PATH);
    strcat(cmd," -f +verbose=0");
    if (usepass || opts == PGP_SIGNED_MESSAGE)
      strcat(cmd, " +batchmode");
    _exit (system_call(cmd,0));	
  }
  
  close (pgp_child_in[0]);
  close (pgp_child_out[1]);
  
  /* now send the passphrase if needed */
  if (usepass) {
    dprint(3,(debugfile,"pgp_decrypt_init: sending pgp passphrase.\n"));
    close (passpipe[0]);
    write (passpipe[1], pgp_passphrase, strlen(pgp_passphrase));
    write (passpipe[1], "\n", 1);
    close (passpipe[1]);
  }
  
  if ((*fpin = fdopen(pgp_child_out[0], "r")) == 0) {
    Raw(ON);
    return(-1);
  }
  if ((*fpout = fdopen(pgp_child_in[1], "w")) == 0) {
    Raw(ON);
    return(-1);
  }
  
  return(fork_ret);
}

int
pgp_extract_public_key ()
{
  char tempfile[STRING], buf[STRING];
  FILE *fpout;
  int err;

  sprintf(tempfile,"%selm.%d",temp_dir,getpid());
  fpout=safeopen_rdwr(tempfile);
  if (!fpout) {
    error1("Could not open temp file %s for writing!", tempfile);
    return FALSE;
  }
  copy_message(mailfile,headers[current-1],
	       "",fpout,0);
  fclose(fpout);
  ClearScreen();
  Raw(OFF);
  sprintf(buf,"%s +verbose=0 %s", PGP_PATH, tempfile);
  printf("Executing: %s\n\n", buf);
  system_call(buf, 0);
  PressAnyKeyToContinue();
  Raw(ON);
  unlink(tempfile);
  return TRUE;
}

#ifdef MIME
int
pgp_mime_opts (s)
     char *s;
{
  char value[STRING];

  if (s) {
    if (mime_get_param ("format", value, s, sizeof(value))) {
      if (istrcmp (value, "keys-only") ==  0)
	return PGP_PUBLIC_KEY;
    }
    if (mime_get_param ("x-action", value, s, sizeof (value))) {
      if (istrcmp (value, "encryptsign") == 0)
	return (PGP_MESSAGE | PGP_SIGNED_MESSAGE);
      else if (istrcmp (value, "encrypt") == 0)
	return PGP_MESSAGE;
      else if (istrcmp (value, "sign") == 0)
	return PGP_SIGNED_MESSAGE;
    }
  }
  return PGP_MESSAGE;
}

void
pgp_decode (m, s)
     mime_t *m;
     state_t *s;
{
  /* This procedure implements the de-facto standard for using PGP with MIME.
   * Content-Type: application/pgp
   * Required-Parameters: none
   * Optional parameters: format, x-action
   *     format = mime | text | keys-only
   *         mime : indicates that the signed/encrypted body contains a MIME
   *                compliant body and should be parsed recursively.
   *         text : [DEFAULT if there is no format option].  This option
   *                means that the encrypted/signed data should be presented
   *                to the user after processing, no additional processing
   *                needed.
   *         keys-only:
   *                The data in the body represents public key data only
   *     x-action = encryptsign | encrypt | sign
   *         This keyword is meant to be helpful to the application, but is
   *         not required, and may not even be necessary to look at.
   *
   *         encryptsign : the application/pgp data is both signed and
   *                       encrypted.
   *         encrypt     : the data is encrypted only
   *         sign        : the data is signed only
   */

  char tempfile[STRING], buffer[STRING];
  FILE *pgpout, *pgpin, *tmpfp, *savefp;
  int
    child,
    inbody = FALSE,
    opts,
    len,
    raw,
    bytes = 0,
    nested = FALSE; /* PGP output should be parsed as a MIME body */
  mime_t *tmpmt;
#if defined(BSD) && !defined(WEXITSTATUS)
  union wait status;
#else
  int status;
#endif

  raw = RawState ();

  if (mime_get_param ("format", buffer, m->type_opts, sizeof (buffer)) &&
      (istrcmp (buffer, "mime") == 0)) {
    nested = TRUE;
    dprint (3, (debugfile, "pgp_decode: format=mime\n"));
    
    sprintf (tempfile, "%selmPT%d", temp_dir, getpid ());
    
    if (NULL == (tmpfp = safeopen_rdwr(tempfile))) {
      error("Failed to create file for decoding.");
      state_puts("[Failed to create file for decoding.]\n",s);      
      return;
    }

    unlink (tempfile); /* Filename is no longer needed ... */
  }
  else
    tmpfp = s->fpout;

  opts = pgp_mime_opts (m->type_opts);

  /* Now metapager does not ask passphrase -- this is called indirectly
   * via copy_message    -KEH
   */
  if ((opts & PGP_MESSAGE) && pgp_keeppass) {
    if (!pgp_goodPassphrase()) {
      error("Decrypting message... Bad PGP passphrase.");
      state_puts("[Decrypting message... Bad PGP passphrase]\n",s);
      return;
    }
  }

  /* Tell the user why they are waiting */
  if (opts & PGP_MESSAGE) {
    if (raw)
      error("Decrypting message...");
    else
      Write_to_screen("\nDecrypting message...\n", 0);
  }
  else if (opts & PGP_SIGNED_MESSAGE) {
    if (raw)
      error("Checking signature...");
    else
      Write_to_screen("\nChecking signature...\n", 0);
  }

  buffer[0] = '\0';
  if (opts & PGP_PUBLIC_KEY) {
    strcpy (buffer, "(** This message contains PGP public key(s)");
    strcat (buffer, " **)\n\n");
    state_puts (buffer, s);
  }
  
  if ((child = pgp_decrypt_init (&pgpout, &pgpin, opts)) == -1) {
    state_puts ("[internal error while calling pgp, skipping...]\n", s);
    if (raw)
      Raw (ON);
    return;
  }
  
  while (bytes < m->length) {
    len = mail_gets (buffer, sizeof (buffer), s->fpin);
    if (len < 1)
      break;
    fputs (buffer, pgpin);
    bytes += len;
  }
  fclose (pgpin);

  sprintf(buffer,"-- Start of PGP%s%s section.\n",
	  opts & PGP_SIGNED_MESSAGE ? " signed" : "",
	  opts & PGP_MESSAGE ? " encoded": "");
  state_puts(buffer,s);

  bytes = 0;
  while ((len = mail_gets (buffer, sizeof (buffer), pgpout)) > 0) {
    if (nested) {
      if (buffer[0] == '\n' || (buffer[0] == '\r' && buffer[1] == '\n'))
	inbody = TRUE;
      fputs (buffer, tmpfp);
      if (inbody)
	bytes += len;
    } else {
      state_add_prefix(s);
      state_puts(buffer,s);
    }
  }
  fclose (pgpout);

  if (nested) {
    dprint (3, (debugfile, "pgp_decode: parsing decrypted data as MIME\n"));

    if (EOF == fflush(tmpfp)) {
      error("Error when flushing temporary file.");
      state_puts("[Error when flushing temporary file.]\n",s);
    }
    rewind(tmpfp); /* Rewind it for reading */
    tmpmt = mime_read_header (tmpfp, 0);
    tmpmt->length = bytes;
    savefp = s->fpin;
    s->fpin = tmpfp;
    mime_decode (tmpmt, s);
    mime_destroy (tmpmt);
    fclose (s->fpin);
    s->fpin = savefp;
  }

  {
    int w,stat=-1;
    while ((w = wait(&status)) != child)
      if (w == -1 && errno != EINTR)
	break;
    if (w == child) {
#ifdef	WEXITSTATUS
      stat = WEXITSTATUS(status);
#else
# ifdef	BSD
      stat = status.w_retcode;
# else
      stat = status;
# endif
#endif
    }
    sprintf(buffer,"-- End of PGP%s%s section%s\n",
	    opts & PGP_SIGNED_MESSAGE ? " signed" : "",
	    opts & PGP_MESSAGE ? " encoded" : "",
	  stat ? ", PGP failed!" : ".");
    state_puts(buffer,s);
  }
  PressAnyKeyToContinue();

  if (raw)
    Raw (ON);
}
#endif /* MIME */
#endif /* USE_PGP */
