
static char rcsid[] = "@(#)$Id: fileio.c,v 5.16 1994/08/30 15:08:09 syd Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 5.16 $   $State: Exp $
 *
 *			Copyright (c) 1988-1992 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Syd Weinstein, Elm Coordinator
 *	elm@DSI.COM			dsinc!elm
 *
 *******************************************************************************
 * $Log: fileio.c,v $
 * Revision 5.16  1994/08/30  15:08:09  syd
 * Add an fflush to detect over quota on file saves.
 * From: Larry Schwimmer <rosebud@cyclone.Stanford.EDU>
 *
 * Revision 5.15  1994/08/30  15:06:37  syd
 * The "copy_message" function in src/fileio.c incorrectly computes the
 * size of the message.  The "bytes_seen" variable currently counts all
 * bytes in the message ***including the message header*** when it should
 * only be counting the bytes in the actual body of the message.
 * From: Thomas Dwyer III <tomiii@mtu.edu>
 *
 * Revision 5.14  1993/09/27  01:51:38  syd
 * Add elm_chown to consolidate for Xenix not allowing -1
 * From: Syd
 *
 * Revision 5.13  1993/09/19  23:15:52  syd
 * Here's some more patch stuff for undersize buffers for header lines.
 * From: Jukka Ukkonen <ukkonen@csc.fi>
 *
 * Revision 5.12  1993/08/23  12:28:23  syd
 * Fix placement of ifdef for PC_CHOWN
 * From: syd
 *
 * Revision 5.11  1993/08/23  03:26:24  syd
 * Try setting group id separate from user id in chown to
 * allow restricted systems to change group id of file
 * From: Syd
 *
 * Revision 5.10  1993/08/10  20:29:52  syd
 * add PC_CHOWN_RESTRICTED where needed
 * From: Syd
 *
 * Revision 5.9  1993/08/03  19:28:39  syd
 * Elm tries to replace the system toupper() and tolower() on current
 * BSD systems, which is unnecessary.  Even worse, the replacements
 * collide during linking with routines in isctype.o.  This patch adds
 * a Configure test to determine whether replacements are really needed
 * (BROKE_CTYPE definition).  The <ctype.h> header file is now included
 * globally through hdrs/defs.h and the BROKE_CTYPE patchup is handled
 * there.  Inclusion of <ctype.h> was removed from *all* the individual
 * files, and the toupper() and tolower() routines in lib/opt_utils.c
 * were dropped.
 * From: chip@chinacat.unicom.com (Chip Rosenthal)
 *
 * Revision 5.8  1993/02/08  18:38:12  syd
 * Fix to copy_file to ignore unescaped from if content_length not yet reached.
 * Fixes to NLS messages match number of newlines between default messages
 * and NLS messages. Also an extra ) was removed.
 * From: Jan Djarv <Jan.Djarv@sa.erisoft.se>
 *
 * Revision 5.7  1993/01/20  03:02:19  syd
 * Move string declarations to defs.h
 * From: Syd
 *
 * Revision 5.6  1992/12/11  01:45:04  syd
 * remove sys/types.h include, it is now included by defs.h
 * and this routine includes defs.h or indirectly includes defs.h
 * From: Syd
 *
 * Revision 5.5  1992/12/07  04:23:23  syd
 * fix typo
 * From: Syd
 *
 * Revision 5.4  1992/11/26  01:46:26  syd
 * add Decode option to copy_message, convert copy_message to
 * use bit or for options.
 * From: Syd and bjoerns@stud.cs.uit.no (Bjoern Stabell)
 *
 * Revision 5.3  1992/11/26  00:48:34  syd
 * Make it do raw(off) before final error message to
 * display error message on proper screen
 * From: Syd
 *
 * Revision 5.2  1992/11/07  20:05:52  syd
 * change to use header_cmp to allow for linear white space around the colon
 * From: Syd
 *
 * Revision 5.1  1992/10/03  22:58:40  syd
 * Initial checkin as of 2.4 Release at PL0
 *
 *
 ******************************************************************************/

/** File I/O routines, including deletion from the folder! 

**/

#include "headers.h"
#include "s_elm.h"
#include <sys/stat.h>
#include <errno.h>
#ifdef BSD
#  include <sys/wait.h>
#endif
#include "me.h"

extern int errno;

char *error_description();

static void copy_write_error_exit()
{
	MoveCursor(LINES, 0);
	Raw(OFF);
	printf(catgets(elm_msg_cat, ElmSet, ElmWriteCopyMessageFailed,
		"\nWrite in copy_message failed\n"));
	dprint(1, (debugfile,"\n*** Fprint failed on copy_message;\n"));
	rm_temps_exit();
}

#ifdef USE_PGP
/* Prototype */
static void copy_pgp P_((char *,FILE *,int, struct header_rec *, FILE *));

static void copy_pgp(prefix,dest_file,cm_options,current_header, infile)
     char *prefix;
     FILE *dest_file;
     int cm_options;
     struct header_rec *current_header;
     FILE * infile;
{
  int body_bytes = 0;
  FILE *fpin = NULL, *fpout = NULL;
  char buffer[VERY_LONG_STRING];
  int buf_len,err,child;
#if defined(BSD) && !defined(WEXITSTATUS)
  union wait status;
#else
  int status;
#endif

  dprint(5,(debugfile,
	    "copy_pgp called: Need read %d bytes\n",
	    current_header->content_length));

  if (current_header->pgp & PGP_MESSAGE)
    error("Decrypting message...");
  else
    error("Checking signature...");

  if ((current_header->pgp & PGP_MESSAGE) && pgp_keeppass) {
    if (!pgp_goodPassphrase()) {
      error("Decrypting message... Bad PGP passphrase.");
      fprintf(dest_file,"[Decrypting message... Bad PGP passphrase]\n");
      return;
    }
  }
  
  Raw(OFF);
  if ((child=pgp_decrypt_init(&fpin, &fpout, current_header->pgp & 
			      (PGP_MESSAGE|PGP_SIGNED_MESSAGE))) == -1) {
    Raw(ON);
    error("Decrypting message... Failed to init PGP.");
    fprintf(dest_file,"[Decrypting message... Failed to init PGP.]\n");   
    return;
  }

  while (body_bytes < current_header->content_length) {      
    if (! (buf_len = mail_gets(buffer, VERY_LONG_STRING, infile)))
      break;
    
    body_bytes += buf_len;
    if (EOF == fputs(buffer, fpout))
      copy_write_error_exit();
  }
  if (fclose(fpout) == EOF)
    copy_write_error_exit();
  
  dprint(5,(debugfile,
	    "copy_pgp: Passed %d bytes to PGP.\n",
	    body_bytes));
  body_bytes = 0;

  fprintf(dest_file,"-- Start of PGP%s%s section.\n",
	  current_header->pgp & PGP_SIGNED_MESSAGE ? " signed" : "",
	  current_header->pgp & PGP_MESSAGE ? " encoded": "");

  while (0 < (buf_len = mail_gets(buffer, VERY_LONG_STRING, fpin))) {
    body_bytes += buf_len;
    
    err = fprintf(dest_file, "%s", prefix);
    if (err != EOF)
      err = fwrite(buffer, 1, buf_len, dest_file);
    if (err != buf_len) {
      copy_write_error_exit();
    }
  }
  fclose(fpin);

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
    fprintf(dest_file,"-- End of PGP%s%s section%s\n",
	  current_header->pgp & PGP_SIGNED_MESSAGE ? " signed" : "",
	  current_header->pgp & PGP_MESSAGE ? " encoded" : "",
	  stat ? ", PGP failed!" : ".");

  }

  PressAnyKeyToContinue();
  Raw(ON);
  dprint(5,(debugfile,
	    "copy_pgp: Readed %d bytes from PGP.\n",
	    body_bytes));
}
#endif /* USE_PGP */

#ifdef MIME
/* Prototype */
void copy_mime P_((char *,FILE *,int, struct header_rec *, FILE *));

void copy_mime(prefix,dest_file,cm_options,current_header, infile)
     char *prefix;
     FILE *dest_file;
     int cm_options;
     struct header_rec *current_header;
     FILE *infile;
{
  state_t state;
  state_clear(&state);

  dprint(5,(debugfile,
	    "copy_mime called: Need read %d bytes\n",
	    current_header->content_length));

  attach_parse(current_header, infile);
  state.fpin = infile;
  state.fpout = dest_file;
  state.prefix = prefix;
  state.displaying = (cm_options & CM_DISPLAYING) ? 1 : 0;
  mime_decode(&(current_header->mime_rec), &state);
  
  dprint(5,(debugfile,"copy_mime: mail decoded\n"));


  state_destroy(&state);  
}
#endif /* MIME */

/* Prototype */
static void copy_encrypted P_((char *,FILE *,int, struct header_rec *, 
			       FILE *));

static void copy_encrypted(prefix,dest_file,cm_options,current_header,infile)
     char *prefix;
     FILE *dest_file;
     int cm_options;
     struct header_rec *current_header;
     FILE *infile;
{
  char buffer[VERY_LONG_STRING];
  long body_bytes = 0;
  int crypted = OFF;
  int buf_len, err;

  dprint(5,(debugfile,
	    "copy_encrypted called: Need read %d bytes\n",
	    current_header->content_length));

  getkey(OFF);

  while (body_bytes < current_header->content_length) {      
    if (! (buf_len = mail_gets(buffer, VERY_LONG_STRING, infile)))
      break;
    
    body_bytes += buf_len;
    
    if (!strncmp(buffer, START_ENCODE, strlen(START_ENCODE))) {
      crypted = ON;
      fputs("-- Start of (Elm) encoded section.\n",dest_file);
      continue;
    } else if (!strncmp(buffer, END_ENCODE, strlen(END_ENCODE))) {
      crypted = OFF;
      fputs("-- End of (Elm) encoded section.\n",dest_file);
      continue;
    } else if (crypted) {
      no_ret(buffer);
      encode(buffer);      
      strcat(buffer, "\n");
    }
    if (fprintf(dest_file, "%s%s", prefix, buffer) == EOF) {
      copy_write_error_exit();
    }
  }
  dprint(5,(debugfile,
	      "copy_encrypted: Readed %d bytes from body\n",body_bytes));
}

/* Prototype */
void copy_plain P_((char *,FILE *,int, struct header_rec *, FILE *));

void copy_plain(prefix,dest_file,cm_options,current_header, infile) 
     char *prefix;
     FILE *dest_file;
     int cm_options;
     struct header_rec *current_header;
     FILE *infile;
{
  char buffer[VERY_LONG_STRING];
  int body_bytes = 0;
  int buf_len, err;
 
  dprint(5,(debugfile,
	    "copy_plain called: Need read %d bytes\n",
	    current_header->content_length));;


  while (body_bytes < current_header->content_length) {      
    if (! (buf_len = mail_gets(buffer, VERY_LONG_STRING, infile)))
      break;
    
    body_bytes += buf_len;
    
    err = fprintf(dest_file, "%s", prefix);
    if (err != EOF)
      err = fwrite(buffer, 1, buf_len, dest_file);
    if (err != buf_len) {
      copy_write_error_exit();
    }
  }
  dprint(5,(debugfile,
	      "copy_plain: Readed %d bytes from body\n",body_bytes));
}


copy_decoder_t select_copy_decoder (current_header) 
     struct header_rec * current_header;
{
    if (0)
      /* Do nothing -- filler */;
#ifdef MIME
    else if (current_header->status & MIME_MESSAGE) 
      return copy_mime; 
#endif
#ifdef USE_PGP
    else if (current_header->pgp & (PGP_MESSAGE|PGP_SIGNED_MESSAGE))
      return copy_pgp;
#endif
    else if (current_header -> encrypted)
      return copy_encrypted;
    else
      return copy_plain;
}

void
copy_message(infile,
	     current_header,
	     prefix, 
	     dest_file, 
	     cm_options)
     FILE *infile;
     struct header_rec *current_header;
     char *prefix;
     FILE *dest_file;
     int cm_options;
{
  /** Copy current message to destination file, with optional 'prefix' 
    as the prefix for each line.  If remove_header is true, it will 
    skip lines in the message until it finds the end of header line...
    then it will start copying into the file... If remote is true
    then it will append "remote from <hostname>" at the end of the
    very first line of the file (for remailing) 
    
    If "filter_header" is true then it'll do some nice things to
    ensure that the forwarded message looks pleasant; e.g. remove
    stuff like ">From " lines and "Received:" lines.
    
    If "update_status" is true then it will write a new Status:
    line at the end of the headers.  It never copies an existing one.
    
    If "decode" decode MIME, PGP and elm's (unsafe) own decoding.
    **/
  
  char buffer[32*1024+1]; /* Allow one header line to be 32 KB
			   * after unfolding 
			   */
  int in_header = 1, first_line = TRUE;
  int remove_header = cm_options & CM_REMOVE_HEADER;
  int remote = cm_options & CM_REMOTE;
  int update_status = cm_options & CM_UPDATE_STATUS;
  int remail = cm_options & CM_REMAIL;
  int decode = cm_options & CM_DECODE;
  int filter_headers = cm_options & CM_FILT_HDR;
  int bytes_seen = 0, body_bytes = 0;
  int content_length_seen = FALSE;
  long CL_pos = -1L, BODY_pos = -1L;
  int was_binary = current_header -> binary && !decode;
  
  int buf_len, err;
  int i;
  
  
  /** get to the first line of the message desired **/
  
  if (fseek(infile, current_header->offset, 0) == -1) {
    dprint(1, (debugfile, 
	       "ERROR: Attempt to seek %d bytes into file failed (%s)",
	       current_header->offset, "copy_message"));
    error1(catgets(elm_msg_cat, ElmSet, ElmSeekFailed,
		   "ELM [seek] failed trying to read %d bytes into file."),
	   current_header->offset);
    return;
  }
  
  /* now while not EOF & still in message... copy it! */
  
  dprint(5,(debugfile,"copy_message: start mailbox separator section\n"));
  
#ifdef MMDF
  if (!(CM_MMDF_HEAD & cm_options) && !remove_header) {
    dprint(5,(debugfile,"copy_message: Write MMDF message separator\n"));
    if (fprintf(dest_file, "%s", MSG_SEPARATOR) == EOF)
      copy_write_error_exit();
  }   
#endif
  
  while (in_header) {      
    long last_pos = ftell(infile);
    if (last_pos < 0) {
      dprint(5,(debugfile,"copy_message: ftell(infile) failed!\n"));
      break;
    }
    
    if (! (buf_len = mail_gets(buffer, sizeof(buffer), infile)))
      break;
    
#ifdef MMDF
    if (strcmp(buffer, MSG_SEPARATOR) == 0)
      continue; /* MSG SEPRATOR is already written */
#endif /* MMDF */
    
    if(buffer[buf_len - 1] == '\n') {
      no_ret(buffer);
      if (first_word(buffer, "From ")) {
	if (first_line && remote && !remove_header) {
	  if (fprintf(dest_file, "%s%s remote from %s\n",
		      prefix, buffer, hostname) == EOF) {
	    copy_write_error_exit();
	  }
	  first_line = FALSE;
	  continue;
	}
	if (!remove_header) {
	  if (fprintf(dest_file, "%s%s\n", prefix, buffer) == EOF) {
	    copy_write_error_exit();
	  }
	}
	first_line = FALSE;
	continue;
      }
      if (!first_line && first_word_nc(buffer, ">From")) {
	if (!filter_headers && !remove_header) {
	  if (fprintf(dest_file, "%s%s\n", prefix, buffer) == EOF) {
	    copy_write_error_exit();
	  }
	}
	continue;	
      }
      /* fall thru */
    }
    dprint(5,(debugfile,
	      "copy_message: Not a mailbox line -- seeking back!\n"));
    dprint(5,(debugfile,
	      "copy_message- Line was: %s\n",buffer));
    
    if (0 != fseek(infile,last_pos,SEEK_SET)) {
      dprint(5,(debugfile,
		"copy_message: seek failed!\n"));
      copy_write_error_exit();
    }
    break; /* Go out of loop */
  }
  if (first_line) {
    dprint(5,(debugfile,
	      "copy_message: No 'From ' line seen...\n"));
    if (!remove_header) {
      long thetime = time((long *) 0);
      dprint(5,(debugfile,
		"copy_message: Create it!\n"));
      if (fprintf(dest_file,
		  "%sFrom %s %s", prefix,username, ctime(&thetime)) == EOF)
	copy_write_error_exit();
    }
  }
  
  dprint(5,(debugfile,"copy_message: start header section\n"));
  
  while (in_header) {      
    char * colptr,*hdr,*body,*ptr;
    int header_class;

    if ( (buf_len = read_header_line(infile,buffer,sizeof(buffer),3))
	< 2) {
      dprint(5,(debugfile,"copy_message: End of headers: buf_len=%d\n",
		buf_len));
      break;
    }
    
    bytes_seen += buf_len;
    
    colptr = strchr(buffer, ':');
    if (NULL == colptr) {
      in_header = 0;
      dprint(1,(debugfile,"copy_message: Software error; No ':' !!\n"));
      break;
    }
    *colptr = '\0';
    hdr = buffer;
    body = colptr +1;

    if (remove_header)
      continue;
    
    header_class = classify_header(hdr);
    dprint(12,(debugfile,"copy_message: header=%s,class=%d,rest=%s\n",
	       hdr,header_class,body));;
    
#ifdef MIME
    if (decode && (current_header->status & MIME_MESSAGE)) {
      
      if (header_class & HDR_TEXT) {
	if (is_rfc1522 (body))
	  rfc1522_decode (body, sizeof (buffer) - (body - buffer) -1);
      } else if (header_class & HDR_STRUCTURED) {
	rfc1522_decode_structured(header_class,
				  body, 
				  sizeof (buffer) -(body - buffer) -1);
      }
      
      dprint(12,(debugfile,"copy_message: decoded rest=%s\n",body));
    }
#endif
    if ((remote || remail) && 0 == istrcmp("Sender",hdr)) {
	continue;
    }
#ifdef MIME
    /* These headers are incorrect after decoding ... */
    if (decode && (current_header->status & MIME_MESSAGE) &&
	(0 == istrcmp(buffer, "MIME-Version") ||
	 0 == istrcmp(buffer, "Content-Type") ||
	 0 == istrcmp(buffer, "Content-Transfer-Encoding")))
      continue;
#endif
    
    if(0 == istrcmp(buffer, "Content-Length")) {
      /* make correct Content-Length later */
      content_length_seen = TRUE;
      continue;
    }
    
    if (!filter_headers) {
      if (0 == istrcmp(buffer,"Status"))
	continue;   /* we will output a new Status: line later, if desired. */
    }
    else { /* filter_headers */
      if (0 == istrcmp(buffer, "Received") ||
	  0 == istrcmp(buffer, "Status") ||
	  0 == istrcmp(buffer, "Return-Path"))
	continue;
      if (remail && 0 == istrcmp(buffer, "To"))
	hdr = "Orig-To";
    }
    dprint(5,(debugfile,"copy_message- hdr=%s body=%s\n",hdr,body));
    
    for (ptr = strtok(body,"\n"); ptr; ptr = strtok(NULL,"\n")) { 
      if (fprintf(dest_file,"%s",prefix) == EOF)
	copy_write_error_exit ();
      
      if (ptr > body) { /* Do folding */
	--ptr;
	if (*(ptr+1) == ' ')
	  *ptr = ' ';
	else
	  *ptr = '\t';
      } else {
	if (fprintf(dest_file,"%s:",hdr) == EOF)
	  copy_write_error_exit ();
      }
      if (fprintf(dest_file,"%s%s",ptr,was_binary ? "\r\n" : "\n") == EOF)
	copy_write_error_exit ();
    }       
  }
  
  dprint(5,(debugfile,"copy_message: end of headers. Readed ~ %d bytes.\n",
	    bytes_seen));

  if (!remove_header) {  
    if (!filter_headers && content_length_seen) {
	if (fprintf (dest_file, "Content-Length: ") == EOF)
	  copy_write_error_exit ();
	CL_pos = ftell(dest_file);
	if (fprintf (dest_file, "%5d%s",
		     current_header->content_length,
		     was_binary ? "\r\n" : "\n") == EOF)
	  copy_write_error_exit ();
    }
    if (remail) {
      if (fprintf (dest_file, "%sSender: %s%s",
		   prefix,username,
		   was_binary ? "\r\n" : "\n") == EOF)
	copy_write_error_exit ();
    }
    if (update_status) {
      if (isoff(current_header->status, NEW)) {
	if (fprintf (dest_file, "%sStatus: ", prefix) == EOF)
	  copy_write_error_exit ();
	
	if (ison(current_header->status, UNREAD)) {
	  if (fprintf(dest_file, "O") == EOF)
	    copy_write_error_exit();
	} else {	/* read */
	if (fprintf(dest_file, "RO") == EOF)
	  copy_write_error_exit();
	
	if (ison(current_header->status, REPLIED)) {
	  if (fprintf (dest_file, "r") == EOF)
	    copy_write_error_exit ();
	}
      }
	
	/* save all the status flags that ELM doesn't understand */
	for (i=0; current_header->mailx_status[i] != '\0'; i++)
	  switch (current_header->mailx_status[i]) {
	  case 'R':
	  case 'O':
	  case 'r':
	    break;
	  default:
	    if (fprintf (dest_file, "%c", current_header->mailx_status[i]) == EOF)
	      copy_write_error_exit ();
	  }
	if (fprintf(dest_file, was_binary ? "\r\n" : "\n") == EOF) {
	  copy_write_error_exit();
	}      
      }	/* else if NEW - indicate NEW with no Status: line. This is
	 * important if we resync a mailfile - we don't want
	 * NEW status lost when we copy each message out.
	 * It is the responsibility of the function that calls
	 * this function to unset NEW as appropriate to its
	 * reason for using this function to copy a message
	 */
    }
    
    /*
     * Add empty line between headers and body (that was not copied
     *  in above)
     */

    if (fprintf(dest_file, was_binary ? "\r\n" : "\n") == EOF) {
      copy_write_error_exit();
    }
  }
   
  dprint(5,(debugfile,
	    "copy_message: Starting reading of body: %d bytes expected\n",
	    current_header->content_length));
     
  BODY_pos = ftell(dest_file);

  if (decode) {
    copy_decoder_t decoder = select_copy_decoder(current_header);
    decoder(prefix,dest_file,cm_options,current_header,infile);
  } else    
    copy_plain(prefix,dest_file,cm_options,current_header,infile);

  if (CL_pos > 0 && BODY_pos > 0) {
    long cur_pos;

    /* Actually written content length is good if conversions are
     * not done and there was no errors ... 
     */
    clearerr(dest_file);
    cur_pos = ftell(dest_file);

    /* Notice that these tests indicates failure */
    if (cur_pos < BODY_pos || 
	0   != fseek(dest_file,CL_pos,SEEK_SET) ||
	ftell(dest_file) != CL_pos ||
	EOF == fprintf (dest_file, "%5d", (int) (cur_pos - BODY_pos)) ||
	0   != fseek(dest_file,cur_pos,SEEK_SET))
      copy_write_error_exit();
    else
      dprint(5,(debugfile,
		"copy_message: Content-length fixed: %d bytes.\n",
		(int) (cur_pos - CL_pos)));
      
  }
  
  if (0 == fseek(dest_file,-2L,SEEK_CUR)) {
    char buffer[2];
    buffer[0] = '\0';
    buffer[1] = '\0';
    if (2 != fread(buffer,sizeof (char),2,dest_file) ||
	0 != fseek(dest_file,0L,SEEK_CUR)) {
      dprint(5,(debugfile,
		"copy_message: Read or seek (0) error!\n")); 
    }
    dprint(5,(debugfile,
	      "copy_message: last two bytes %d %d\n",
	      buffer[0], buffer[1])); 
    if (buffer[1] != '\n') {
      /* No \n in end ? */ 	
      dprint(5,(debugfile,
		"copy_message: NL missing from end of mail\n")); 
      if (fprintf(dest_file, "\n") == EOF) 
	copy_write_error_exit();
    }
#ifndef MMDF
    if (buffer[0] != '\n') {
      dprint(5,(debugfile,
		"copy_message: NL NL missing from end of mail\n")); 
      /* blank line to keep mailx happy *sigh* */      
      if (fprintf(dest_file, "\n") == EOF) 
	copy_write_error_exit();
    }
#endif

  }
  else {
    dprint(5,(debugfile,
	      "copy_message: Failed to seek backward!\n")); 
    copy_write_error_exit();   /* Flushing failed? */
  }
  
  /* Since fprintf is buffered, its return value is only useful for
   * writes which exceed the blocksize.  Do a fflush to ensure that
   * the message has, in fact, been written.
   */
  if (fflush(dest_file) == EOF) {
    copy_write_error_exit();
  }
}

static struct stat saved_buf;

/*
 *  Don't take chances that a file name is really longer than SLEN.
 *  You'll just pollute the memory right after the allocated space
 *  if you have MAXPATHLEN of 1024 (_PATH_MAX in POSIX).
 */

static char saved_fname[VERY_LONG_STRING];

int
save_file_stats(fname)
char *fname;
{
	/* if fname exists, save the owner, group, mode and filename.
	 * otherwise flag nothing saved. Return 0 if saved, else -1.
	 */

	if(stat(fname, &saved_buf) != -1) {
	  (void)strcpy(saved_fname, fname);
	  dprint(2, (debugfile,
	    "** saved stats for file owner = %d group = %d mode = %o %s **\n",
	    saved_buf.st_uid, saved_buf.st_gid, saved_buf.st_mode, fname));
	  return(0);
	}
	dprint(2, (debugfile,
	  "** couldn't save stats for file %s [errno=%d] **\n",
	  fname, errno));
	return(-1);

}

restore_file_stats(fname)
char *fname;
{
	/* if fname matches the saved file name, set the owner and group
	 * of fname to the saved owner, group and mode,
	 * else to the userid and groupid of the user and to 700.
	 * Return	-1 if the  either mode or owner/group not set
	 *		0 if the default values were used
	 *		1 if the saved values were used
	 */

	int old_umask, i, new_mode, new_owner, new_group, ret_code;
	struct stat testbuf;
	new_mode = 0600;
	new_owner = userid;
	new_group = groupid;
	ret_code = 0;

	if(strcmp(fname, saved_fname) == 0) {
	  new_mode = saved_buf.st_mode & 0777;
	  
	  /* Restore also special bits if file is NOT executable */
	  if (!(saved_buf.st_mode & 0111)) {
	    new_mode |= saved_buf.st_mode & 07000;
	    dprint(2, (debugfile, 
		       "** not excutable -- restore also special modes\n"));
	  }

	  new_owner = saved_buf.st_uid;
	  new_group = saved_buf.st_gid;
	  ret_code = 1;
	}

	dprint(2, (debugfile, "** %s file stats for %s **\n",
		   (ret_code ? "restoring" : "setting"), fname));

	old_umask = umask(0);
	if((i = chmod(fname, new_mode)) == -1)
	  ret_code = -1;

	dprint(2, (debugfile, "** chmod(%s, %05o) returns %d [errno=%d] **\n",
		   fname, new_mode, i, errno));

	(void) umask(old_umask);

	if(stat(fname, &testbuf) != -1) {
	  dprint(2, (debugfile,
	    "** stats for file owner = %d group = %d mode = %o %s **\n",
	    testbuf.st_uid, testbuf.st_gid, testbuf.st_mode, fname));

	  /* Don't do chown because it resets special modes, if not
	   * neccessary 
	   */
	  
	  if (new_owner == testbuf.st_uid && new_group == testbuf.st_gid) {
	    dprint(2, (debugfile,
		       "** Owner correct -- chown not needed.\n"));

	  } else {

#ifdef	BSD
	    /*
	     * Chown is restricted to root on BSD unix
	     */
	    (void) elm_chown(fname, new_owner, new_group);
#else
#  ifdef _PC_CHOWN_RESTRICTED
	    /*
	     * Chown may or may not be restricted to root in SVR4, if it is,
	     *	then need to copy must be true, and no restore of permissions
	     *	should be performed.
	     */
	    if (!pathconf(fname, _PC_CHOWN_RESTRICTED)) {
#  endif
	      if((i = elm_chown(fname, new_owner, new_group)) == -1)
		ret_code = -1;
	      
	      dprint(2, (debugfile, "** elm_chown(%s, %d, %d) returns %d [errno=%d] **\n",
			 fname, new_owner, new_group, i, errno));
#  ifdef _PC_CHOWN_RESTRICTED
	    } else {
	      (void) elm_chown(fname, new_owner, new_group);
	    }
#  endif /* _PC_CHOWN_RESTRICTED */
#endif /* BSD */
	  }
	}
	
	if(stat(fname, &testbuf) != -1) {
	  dprint(2, (debugfile,
	    "** stats for file owner = %d group = %d mode = %o %s **\n",
	    testbuf.st_uid, testbuf.st_gid, testbuf.st_mode, fname));
	}

	return(ret_code);

}

/** and finally, here's something for that evil trick: site hiding **/

#ifdef SITE_HIDING

int
is_a_hidden_user(specific_username)
char *specific_username;
{
	/** Returns true iff the username is present in the list of
	   'hidden users' on the system.
	**/
	
    FILE *hidden_users;
    char  buffer[VERY_LONG_STRING];

    /* 
	this line is deliberately inserted to ensure that you THINK
	about what you're doing, and perhaps even contact the author
	of Elm before you USE this option...
     */

	if ((hidden_users = fopen (HIDDEN_SITE_USERS,"r")) == NULL) {
	  dprint(1, (debugfile,
		  "Couldn't open hidden site file %s [%s]\n",
		  HIDDEN_SITE_USERS, error_description(errno)));
	  return(FALSE);
	}

	while (fscanf(hidden_users, "%s", buffer) != EOF)
	  if (strcmp(buffer, specific_username) == 0) {
	    dprint(3, (debugfile, "** Found user '%s' in hidden site file!\n",
		    specific_username));
	    fclose(hidden_users);
	    return(TRUE);
	  }

	fclose(hidden_users);
	dprint(3, (debugfile, 
		"** Couldn't find user '%s' in hidden site file!\n",
		specific_username));

	return(FALSE);
}

#endif
