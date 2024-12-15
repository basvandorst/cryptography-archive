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

  int wait_ret, fork_ret, builtin = 0, matched = 0, status, len, line_len;
  long start_offset = hdr->offset, end_offset;
  char ch, buf[VERY_LONG_STRING];
  FILE *fpout = NULL;
  copy_decoder_t decoder;
  int err;				/* place holder for errno */
  char **text = 0;
  int text_idx = 0, text_len = 0;
  char tempfile[STRING];
  FILE *fpin, *tfpout;
  int wanna_tempfile = 0;

  /* check to see if we want the internal pager */
  if (strincmp(pager, "builtin", 7) == 0 ||
      strincmp(pager, "internal", 8) == 0 ||
      (builtin_lines < 0 ? hdr->lines < LINES + builtin_lines : hdr->lines < builtin_lines)) {

    builtin++;

    decoder = select_copy_decoder(hdr);

    /* Also [encode] [clear] requires tempfile ... */
    if (decoder != copy_plain 
#ifdef MIME
	&&
	(decoder != copy_mime || mime_needs_processing (&hdr->mime_rec))
#endif
	) 
      wanna_tempfile = 1;
  } else {
    /* FOr simplify we use tempfile always with external pager.
     * That way we don't need take care if pager is taken terminal
     * before routines called from copy_message want ask decode key
     * or passhrase from user ...
     */
    wanna_tempfile = 1;
  }

  if (wanna_tempfile) {
    sprintf (tempfile, "%selm.%d", temp_dir, getpid());
    
    unlink(tempfile);
    fpout = safeopen_rdwr(tempfile);
    unlink(tempfile); /* We can now unlink tempfile ... So nobody
		       * sees data easily (and O_EXCL should prevent
		       * evil playings with symlinks) ...
		       */
    if (!fpout) {
      error("Failed to create temporary file!");
      return 0;
    }
    
    dprint (1, (debugfile, "metapager(): using tempfile %s\n", tempfile));
  } else {
    dprint (1, (debugfile, "metapager(): [experimental] look ma!  no tempfile!\n"));
    builtin++;
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
#ifdef MIME
      char buf4[STRING];
      
      strfcpy (buf4, hdr->from, sizeof (buf4));
      if (is_rfc1522 (hdr->from))
	rfc1522_decode (buf4, sizeof (buf4));
#endif
      
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
#ifdef MIME
	      buf4,
#else
	      hdr->from,
#endif
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
      if ((len = strlen(hdr->subject)) > 0 && matches_weedlist("subject:")) {
        len = (COLUMNS-len)/2;
        if (len < 0)
          len = 0;
        sprintf(buf,"%*.*s%s",len,len,"",hdr->subject);
        if (builtin > 1)
          text[text_idx++] = strmcpy(text[text_idx], buf);
        else
          fprintf(fpout, "%s\n", buf);
      }
      
      if (builtin > 1) {
        text[text_idx] = (char *) safe_malloc(1);
        *text[text_idx++] = '\0';
      }
      else
        fputc('\n', fpout);
      
      /* now some information about this message */
      
      buf[0]='\0';
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

    /** Now do the headers. **/

    /* read_header_line with flag = 1 terminates header with \n and marks
     * folding with \n
     */
    while (0 < (line_len = read_header_line (fp, buf, VERY_LONG_STRING,1))) {
      char *ptr;
#ifdef MMDF
      if (strcmp (buf, MSG_SEPARATOR) == 0)
        continue;
#endif
      if (filter) {
	if (matches_weedlist (buf)) {
	  matched = 1;
	  continue;
	}
	else
	  matched = 0;
      }
      
      if (buf[0] == '\n') {
#ifndef MIME
	start_offset = ftell(fp);
#endif
	/* strtok -hack don't print last empty line anyway ... */

	break;       
      }
#ifdef MIME
      else {
	ptr = strchr(buf,':');
	if (ptr && (hdr->status & MIME_MESSAGE)) {
	  char *ptr2 = ptr +1;
	  int class;
	  *ptr = '\0';

	  class = classify_header(buf);
	  dprint(12,(debugfile,"metapager: header=%s,class=%d,rest=%s\n",
		     buf,class,ptr2));

	  if (class & HDR_TEXT) {
	    if (is_rfc1522 (ptr2))
	      rfc1522_decode (ptr2, sizeof (buf) - (ptr2 - buf) -1);
 	  } else if (class & HDR_STRUCTURED) {
	    rfc1522_decode_structured(class,
				      ptr2, sizeof (buf) - (ptr2 - buf) -1);
	  }

	  dprint(12,(debugfile,"metapager: decoded rest=%s\n",ptr2));
	  *ptr = ':';
	}
      }
#endif

      for (ptr = strtok(buf,"\n"); ptr; ptr = strtok(NULL,"\n")) { 
	if (ptr > buf) { /* Do folding */
	  --ptr;
	  if (*(ptr+1) == ' ')
	    *ptr = ' ';
	  else
	    *ptr = '\t';
	}
	if (builtin > 1) {
	  
	  if (text_idx >= text_len - 1)
	    text = (char **) DynamicArray (text, sizeof (char *), &text_len, 5);
	  text[text_idx] = strmcpy (text[text_idx], ptr);
	  text_idx++;
	}
	else {
	  fputs (ptr, fpout);
	  fputc ('\n',fpout);
	}
      }
    }
  }

  if (builtin > 1) {    
    if (text_idx >= text_len - 1)
      text = (char **) DynamicArray (text, sizeof (char *), &text_len, 5);
    text[text_idx] = strmcpy (text[text_idx], "");
    text_idx++;
  }
  else {
    fputc ('\n',fpout);
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
    
#ifdef MMDF
    /* Shorten the size of the message such that it does not include the
     * trailing separator.
     */
    ch = builtinplusplus (fp, start_offset, hdr->content_length - 5,
                          text, text_idx);
#else
    ch = builtinplusplus (fp, start_offset, hdr->content_length, text,
                          text_idx);
#endif
    DestroyDynamicArray (text);
    return ch;
  }

  /* Now this does decoding of MIME and PGP */
  copy_message(fp,hdr,
	       "",fpout,CM_REMOVE_HEADER|CM_DECODE|CM_DISPLAYING);
  rewind(fpout);
  clear_error();

  /** Now run the pager! **/

  if (builtin) {
    fseek (fpout, (long)0, 2);
    ch = builtinplusplus (fpout, (long) 0, ftell (fpout), NULL, 0);
    fclose (fpout);
    return (ch);
  }

  /** The rest of the code is for an external pager. **/

  if ((fork_ret = fork()) == -1) {
    err = errno;
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
    
    /* Direct our temporary file to standard input of child.
     * Because we immediately unlinked it (for security reasons) 
     * after creating we don't have name for it and
     * we can't use < in system_call
     */

    if (dup2 (fileno(fpout),0) == -1) {
      err = errno;
      dprint(1, (debugfile, "Error: dup failed, errno %s (metapager)\n",
		 error_description(err)));	
      error1(catgets(elm_msg_cat, ElmSet, ElmPreparePagerDup,
	  "Could not prepare for external pager(dup()-%s)."),
	     error_description(err));	
      _exit(err);	
    }
    
    /* now execute pager and exit */
    
    /* system_call() will return user to user's normal permissions. */
    _exit(system_call(pager, SY_ENAB_SIGINT));
  }

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
