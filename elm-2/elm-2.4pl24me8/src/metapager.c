#include <errno.h>
#include "headers.h"
#include "s_elm.h"
#include "me.h"

#ifdef MIME
extern int errno;
#endif /* MIME */

void
PressAnyKeyToContinue()
{
  Raw(ON | NO_TITE);
  PutLine0(LINES, 0, "Press any key to continue...");
  ReadCh();
  Raw(OFF | NO_TITE);
  printf("\n\r");
}

metapager (fp, hdr, do_headers)
     FILE *fp;
     struct header_rec *hdr;
     int do_headers;
{

  int p[2], builtin = 0, matched = 0, status, len, line_len;
  long start_offset = hdr->offset, end_offset;
  char ch, buf[VERY_LONG_STRING];
  FILE *fpout = NULL;
  int wait_ret;                         /* wait return value */
  int fork_ret;				/* fork return value */
  int err;				/* place holder for errno */
  char **text = 0;
  int text_idx = 0, text_len = 0;
  char tempfile[STRING];
#ifdef MIME
  int istext;
#endif
#ifdef USE_PGP
  int pgp = 0;
  FILE *fpin, *tfpout;

  /* must ask about key up here since the external pager could be forked
     off and steal stdin */

  if (hdr->pgp & (PGP_MESSAGE|PGP_SIGNED_MESSAGE)) {
    if ((hdr->pgp & PGP_MESSAGE) && pgp_keeppass)
      if (! pgp_goodPassphrase())
        return (0);
#ifdef MIME
    if (hdr->mime_rec.type == MIME_TYPE_APPLICATION &&
	(istrcmp(hdr->mime_rec.subtype, "pgp") == 0))
#endif /* MIME */
    {
      if (hdr->pgp & PGP_MESSAGE)
        error("Decrypting message...");
      else
        error("Checking signature...");
      pgp = hdr->pgp; /* handle PGP at the top level */
    }
  }
#endif

  /* check to see if we want the internal pager */
  if ((strncmp(pager, "builtin", 7) == 0 && (pager[7]=='\0' || pager[7]=='+'))
    || (strncmp(pager, "internal", 8) == 0 && (pager[8]=='\0' || pager[8]=='+'))
    || (builtin_lines < 0 ? hdr->lines < LINES + builtin_lines :
	    hdr->lines < builtin_lines)) {

    builtin++;

#ifdef MIME
# ifdef USE_PGP
    if ((pgp & (PGP_MESSAGE | PGP_SIGNED_MESSAGE)) ||
	mime_needs_processing (&hdr->mime_rec))
# else
    if (mime_needs_processing (&hdr->mime_rec))
# endif
#else
# ifdef USE_PGP
    if (pgp & (PGP_MESSAGE | PGP_SIGNED_MESSAGE))
# endif
#endif
      
#if defined (MIME) || defined (USE_PGP)
    {
      sprintf (tempfile, "%s/elm.%d", temp_dir, getpid());
      fpout = fopen (tempfile, "w");
      (void) elm_chown (tempfile, userid, groupid);
      chmod (tempfile, 0600);
      dprint (1, (debugfile, "metapager(): [experimental] using tempfile %s\n", tempfile));
    }
    else {
      dprint (1, (debugfile, "metapager(): [experimental] look ma!  no tempfile!\n"));
#endif
      builtin++;
#if defined (MIME) || defined (USE_PGP)
    }
#endif
  }
  else {
    /* put terminal out of raw mode so external pager has normal env */
    Raw (OFF);

    /* create pipe for external pager and fork */

    if (pipe (p) == -1) {
      err = errno;
      dprint(1, (debugfile, "Error: pipe failed, errno %s (metapager)\n",
        error_description(err)));
      error1(catgets(elm_msg_cat, ElmSet, ElmPreparePagerPipe,
        "Could not prepare for external pager(pipe()-%s)."),
        error_description(err));	
      PressAnyKeyToContinue();
      Raw (ON);
      return (0);
    }

    if ((fork_ret = fork()) == -1) {
      err = errno;
      close (p[0]);
      close (p[1]);
      dprint(1, (debugfile, "Error: fork failed, errno %s (metapager)\n",
        error_description(err)));
      error1(catgets(elm_msg_cat, ElmSet, ElmPreparePagerFork,
        "Could not prepare for external pager(fork()-%s)."),
        error_description(err));	
      PressAnyKeyToContinue();
      Raw (ON);
      return (0);
    } else if (fork_ret == 0) {
      /* child fork */

      /* close write-only pipe fd and fit read-only pipe fd to stdin */

      close (p[1]);
      close (0);
      if (dup (p[0]) == -1) {
	err = errno;
	dprint(1, (debugfile, "Error: dup failed, errno %s (metapager)\n",
	  error_description(err)));
	error1(catgets(elm_msg_cat, ElmSet, ElmPreparePagerDup,
	  "Could not prepare for external pager(dup()-%s)."),
	  error_description(err));	
	_exit(err);
      }

      /* now execute pager and exit */
	    
      /* system_call() will return user to user's normal permissions.
       * This is what makes this pipe secure - user won't have elm's
       * special setgid permissions (if so configured) and will only
       * be able to execute a pager that user normally has permission
       * to execute */

      _exit(system_call(pager, SY_ENAB_SIGINT));
    }

    close (p[0]);
    fpout = fdopen (p[1], "w");
    if (fpout == NULL) {
      err = errno;
      close (p[1]);
      dprint(1, (debugfile, "Error: parent fdopen failed, errno %s (metapager)\n",
        error_description(err)));
      error1(catgets(elm_msg_cat, ElmSet, ElmPreparePagerParentFdopen,
        "Could not prepare for external pager(parent fdopen()-%s)."),
        error_description(err));	
      PressAnyKeyToContinue();
      Raw (ON);
      return (0);
    }

    ClearScreen();
  }

  if (fseek (fp, hdr->offset, 0) == -1) {
    err = errno;
    dprint(1, (debugfile,
	  "Error: seek %d bytes into file, errno %s (show_message)\n",
	  hdr->offset, error_description(err)));
    error2(catgets(elm_msg_cat, ElmSet, ElmSeekFailedFile,
	  "ELM [seek] couldn't read %d bytes into file (%s)."),
	  hdr->offset, error_description(err));	
    return (0);
  }
      
  /* this is only true if metapager() is not called from ViewAttachment() */
  if (do_headers) {
    if (title_messages) {
      char buf2[STRING], buf3[STRING];

      /* first print a title line */

      elm_date_str(buf2, hdr->time_sent + hdr->tz_offset);
      strcat(buf2, " ");
      strcat(buf2, hdr->time_zone);
      sprintf(buf, "%s %d/%d ",
	      hdr->status & DELETED ? "[Deleted]" : "Message",
	      current,
	      message_count);
      len = COLUMNS - 2 - strlen(buf) - strlen(buf2),
      sprintf(buf3, "%s%-*.*s %s",
	      buf,
	      len,
	      len,
	      hdr->from,
	      buf2);
      
      if (builtin > 1) {
	if (text_idx == text_len)
	  text = (char **) DynamicArray (text, sizeof (char *), &text_len, 5);
	text[text_idx] = strmcpy (text[text_idx], buf3);
	text_idx++;
      }
      else {
	fputs (buf3, fpout);
	fputs ("\n", fpout);
      }

      /** Print the subject line centered if there is enough room **/
      if ((len = strlen(hdr->subject)) > 0 && matches_weedlist("subject")) {
        len = (COLUMNS-len)/2;
        if (len < 0)
          len = 0;
        sprintf(buf,"%*.*s%s",len,len,"",hdr->subject);
        if (builtin > 1)
          text[text_idx++] = strmcpy(text[text_idx], buf);
        else
          fprintf(fpout, "%s\n", buf);
      }
      else {
        if (builtin > 1) {
          text[text_idx] = (char *) safe_malloc(1);
          *text[text_idx++] = '\0';
        }
        else
          fputc('\n', fpout);
      }
      
      /* now some information about this message */
      
      buf[0]='\0';
#ifdef USE_PGP
      if (pgp & PGP_MESSAGE)
	strcpy(buf, "(** This message is PGP encrypted");
      else if (pgp & PGP_SIGNED_MESSAGE)
	strcpy(buf, "(** This message is PGP signed");
#endif
      if (hdr->status & EXPIRED) {
	if (buf[0] == '\0')
	  strcpy(buf, "(** This message has EXPIRED");
	else
	  strcat(buf, ", and has EXPIRED");
      }
      if (hdr->status & CONFIDENTIAL) {
	if (buf[0] == '\0')
	  strcpy(buf, "(** This message is tagged CONFIDENTIAL");
	else
	  strcat(buf, ", and is tagged CONFIDENTIAL");
      }      
      if (hdr->status & URGENT) {
	if (buf[0] == '\0')
	  strcpy(buf, "(** This message is tagged URGENT");
	else if (hdr->status & CONFIDENTIAL)
	  strcat(buf, ", and URGENT");
	else
	  strcat(buf, ", and is tagged URGENT");
      }
      
      if (buf[0] != '\0') {
	strcat(buf, " **)");
	if (builtin > 1) {
	  text[text_idx] = strmcpy (text[text_idx], buf);
	  text_idx++;
	  text[text_idx] = (char *) safe_malloc (1);
	  *text[text_idx] = '\0';
	  text_idx++;
	}
	else	  {
	  fputs (buf, fpout);
	  fputs ("\n\n", fpout);
	}
      }
    }

#ifdef MIME
    istext = is_text_type(TYPE(hdr->mime_rec.type),hdr->mime_rec.subtype,
			  hdr->mime_rec.encoding);
#endif /* MIME */

    /** Now do the headers. **/
    
    while (0 < (line_len = mail_gets (buf, VERY_LONG_STRING, fp))) {
      int is_end = 0;
#ifdef MMDF
      if (strcmp (buf, MSG_SEPARATOR) == 0)
        continue;
#endif
      if (filter) {
	if (matches_weedlist (buf) || (whitespace (buf[0]) && matched)) {
	  matched = 1;
	  continue;
	}
	else
	  matched = 0;
      }
      
      if (buf[0] == '\n') is_end = 1;
      else if (buf[0] == '\r' && buf[1] == '\n') is_end = 1;

      if (builtin > 1) {
	if (buf[line_len-1] == '\n') {
	  buf[line_len-1] = '\0';
	  if (line_len > 1 && buf[line_len-2] == '\r') 
	    buf[line_len-2] = '\0';
	}

	if (text_idx >= text_len - 1)
	  text = (char **) DynamicArray (text, sizeof (char *), &text_len, 5);
	text[text_idx] = strmcpy (text[text_idx], buf);
	text_idx++;
      }
      else 
	fputs (buf, fpout);
      
      if (is_end) {
#ifndef MIME
	start_offset = ftell(fp);
#endif
	break;
      }
    }
  }

#ifdef MIME
  start_offset = hdr->mime_rec.offset;
#endif
  
  /* finally the body */

  if (builtin > 1) {
#ifdef MIME
    if (hdr->status & MIME_MESSAGE)
      mime_warnings(hdr);
#endif

    /* No special processing needs to take place, so just start the pager! */
    
    ch = builtinplusplus (fp, start_offset, hdr->content_length, text, text_idx);
    DestroyDynamicArray (text);
    return ch;
  }

  end_offset = ftell(fp) + hdr->content_length;

#ifdef USE_PGP
  if (pgp & (PGP_MESSAGE | PGP_SIGNED_MESSAGE)) {
    if (pgp_decrypt_init(&fpin, &tfpout, pgp & (PGP_MESSAGE | PGP_SIGNED_MESSAGE)) == -1) {
       Raw(ON);
       return(0);
    }
    pgp=TRUE;
  }
#endif

#ifdef MIME
  if ((hdr->status & MIME_MESSAGE)
#ifdef USE_PGP    
	   && !pgp
#endif
      ) {
    state_t state;

    state.fpin = fp;
    state.fpout =  fpout;
    state.displaying = TRUE;
    state.prefix = NULL;

    attach_parse(hdr, fp);
    mime_decode(&hdr->mime_rec, &state);
  }
  else {
#endif /* MIME */
#ifdef USE_PGP
    if (pgp) {
      /* for a PGP message, just pipe everything to the child process */
      
      while (ftell(fp) < end_offset) {
	if (fgets (buf, VERY_LONG_STRING, fp) == NULL)
	  break;
	
	fputs(buf, tfpout);
	fflush (tfpout);
	
      }
      fclose (tfpout);
      while (fgets(buf,VERY_LONG_STRING,fpin)!=NULL)
	fputs(buf,fpout);
      fclose(fpin);
      PressAnyKeyToContinue();
      if (builtin)
	Raw(ON);
    }
    else
#endif
      while (ftell(fp) < end_offset) {
	if (fgets (buf, VERY_LONG_STRING, fp) == NULL)
	  break;
	fputs(buf, fpout);
      }
#ifdef MIME
  }
#endif

  /** Now run the pager! **/

  if (builtin) {
    /* This is a trick.  I unlink the temp file while it is still open so the
     * file disappears, but we can still access the data so long as the stream
     * is still open.
     */
    fpout = freopen (tempfile, "r", fpout);
    unlink (tempfile);
    fseek (fpout, (long)0, 2);
    ch = builtinplusplus (fpout, (long) 0, ftell (fpout), NULL, 0);
    fclose (fpout);
    return (ch);
  }

  /** The rest of the code is for an external pager. **/

  fclose (fpout);

  while ((wait_ret = wait (&status)) != fork_ret && wait_ret != -1)
    ;
  /* turn raw on **after** child terminates in case child
   * doesn't put us back to cooked mode after we return ourselves to
   * raw.
   */
  Raw(ON);

  if (prompt_after_pager) {
    StartBold ();
    PutLine0 (LINES, 0, catgets(elm_msg_cat, ElmSet, ElmCommandIToReturn,
				" Command ('i' to return to index): "));
    EndBold ();
    fflush (stdout);
    ch = ReadCh();
  }
  else
    ch = 0;

  return (ch == 'i' || ch == 'q' ? 0 : ch);
}
