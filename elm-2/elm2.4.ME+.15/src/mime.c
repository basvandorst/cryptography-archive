
static char rcsid[] = "@(#)$Id: mime.c,v 5.15 1993/08/23 02:55:05 syd Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 5.15 $   $State: Exp $
 *
 *			Copyright (c) 1988-1992 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Syd Weinstein, Elm Coordinator
 *	elm@DSI.COM			dsinc!elm
 *
 ******************************************************************************
 * $Log: mime.c,v $
 * Revision 5.15  1993/08/23  02:55:05  syd
 * Add missing parens
 * From: dwolfe@pffft.sps.mot.com (Dave Wolfe)
 *
 * Revision 5.14  1993/08/10  18:53:31  syd
 * I compiled elm 2.4.22 with Purify 2 and fixed some memory leaks and
 * some reads of unitialized memory.
 * From: vogt@isa.de
 *
 * Revision 5.13  1993/08/03  19:28:39  syd
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
 * Revision 5.12  1993/07/20  02:41:24  syd
 * Three changes to expand_env() in src/read_rc.c:  make it non-destructive,
 * have it return an error code instead of bailing out, and add a buffer
 * size argument to avoid overwritting the destination.  The first is to
 * avoid all of the gymnastics Elm needed to go through (and occasionally
 * forgot to go through) to protect the value handed to expand_env().
 * The second is because expand_env() was originally written to support
 * "elmrc" and bailing out was a reasonable thing to do there -- but not
 * in the other places where it has since been used.  The third is just
 * a matter of practicing safe source code.
 *
 * This patch changes all invocations to expand_env() to eliminate making
 * temporary copies (now that the routine is non-destructive) and to pass
 * in a destination length.  Since expand_env() no longer bails out on
 * error, a do_expand_env() routine was added to src/read_rc.c handle
 * this.  Moreover, the error message now gives some indication of what
 * the problem is rather than just saying "can't expand".
 *
 * Gratitous change to src/editmsg.c renaming filename variables to
 * clarify the purpose.
 * From: chip@chinacat.unicom.com (Chip Rosenthal)
 *
 * Revision 5.11  1993/06/10  03:12:10  syd
 * Add missing rcs id lines
 * From: Syd
 *
 * Revision 5.10  1993/05/14  03:56:19  syd
 * A MIME body-part must end with a newline even when there was no newline
 * at the end of the actual body or the body is null. Otherwise the next
 * mime boundary may not be recognized.  The same goes with the closing
 * boundary too.
 * From: Jukka Ukkonen <ukkonen@csc.fi>
 *
 * Revision 5.9  1993/05/08  20:25:33  syd
 * Add sleepmsg to control transient message delays
 * From: Syd
 *
 * Revision 5.8  1992/12/11  01:45:04  syd
 * remove sys/types.h include, it is now included by defs.h
 * and this routine includes defs.h or indirectly includes defs.h
 * From: Syd
 *
 * Revision 5.7  1992/11/26  00:46:13  syd
 * changes to first change screen back (Raw off) and then issue final
 * error message.
 * From: Syd
 *
 * Revision 5.6  1992/11/22  01:22:48  syd
 * According to the MIME BNF, quoted strings are allowed in the value portion
 * of a parameter.
 * From: chk@alias.com (C. Harald Koch)
 *
 * Revision 5.5  1992/11/07  16:21:56  syd
 * There is no need to write out the MIME-Version header in subparts
 * From: Klaus Steinberger <Klaus.Steinberger@Physik.Uni-Muenchen.DE>
 *
 * Revision 5.4  1992/10/30  21:10:39  syd
 * it invokes metamail (the pseudo is because "text" isn't a legal Content-Type).
 * in src/mime.c notplain() tries to check for text but fails because it should
 * look for "text\n" not "text".
 * From: Jan Djarv <Jan.Djarv@sa.erisoft.se>
 *
 * Revision 5.3  1992/10/25  01:47:45  syd
 * fixed a bug were elm didn't call metamail on messages with a characterset,
 * which could be displayed by elm itself, but message is encoded with QP
 * or BASE64
 * From: Klaus Steinberger <Klaus.Steinberger@Physik.Uni-Muenchen.DE>
 *
 * Revision 5.2  1992/10/24  13:44:41  syd
 * There is now an additional elmrc option "displaycharset", which
 * sets the charset supported on your terminal. This is to prevent
 * elm from calling out to metamail too often.
 * Plus a slight documentation update for MIME composition (added examples)
 * From: Klaus Steinberger <Klaus.Steinberger@Physik.Uni-Muenchen.DE>
 *
 * Revision 5.1  1992/10/03  22:58:40  syd
 * Initial checkin as of 2.4 Release at PL0
 *
 *
 ******************************************************************************/


#include "headers.h"
#include "s_elm.h"

#ifdef MIME

#include <errno.h>
#include <sys/stat.h>

/* Notice that ENCODING_ILLEGAL is -1 
 */

char *mime_encode_names[] = {
  "none", /* Not used. */
  "7bit",
  "8bit",
  "binary",
  "quoted-printable",
  "base64",
  "X-???"         /* ENCODING_EXPERIMENTAL */
};

int check_for_multipart(filedesc, mime_info)
     FILE *filedesc;
     mime_send_t *mime_info;
{
  char buffer[SLEN];
  int Multi_Part = FALSE;
  char *ptr;
  char *incptr;
  char Include_Filename[SLEN];
  char Expanded_Filename[SLEN];

  while (mail_gets(buffer, SLEN, filedesc))
    if (buffer[0] == '[') {
      if (strncmp(buffer, MIME_INCLUDE, strlen(MIME_INCLUDE)) == 0) {
      	Multi_Part = TRUE;
	if (Include_Part((FILE *)NULL, buffer, TRUE, mime_info, FALSE) == -1) {
	   return(-1);
	}
      }
    }
    rewind(filedesc);
  return(Multi_Part);
}

int
Include_Part(dest, buffer, check, mime_info, copy)
     FILE *dest;
     char *buffer;
     int  check;
     mime_send_t *mime_info;
     int copy;
{
  char *ptr;
  char *incptr;
  char Include_Filename[SLEN];
  char Expanded_Filename[SLEN];
  char tmp_fn[SLEN];
  char *filename;
  char Primary_Type[SLEN];
  char SubType[SLEN];
  char Params[STRING];
  char Encoding[SLEN];
  char sh_buffer[SLEN];
  char Encode_Flag[3];
  int  Enc_Type;
  FILE *incfile;
  int  line_len;
  int is_text;

  ptr = buffer + strlen(MIME_INCLUDE);
  while ((*ptr != '\0') && (*ptr == ' '))
    ptr++;
  incptr = Include_Filename;
  while ((*ptr != ' ') && (*ptr != ']') && (*ptr != '\0') &&
	 (incptr < Include_Filename + sizeof(Include_Filename) -1))
    *incptr++ = *ptr++;
  *incptr = '\0';

  while ((*ptr != '\0') && (*ptr == ' '))
    ptr++;
  incptr = Primary_Type;
  while ((*ptr != ' ') && (*ptr != ']') && (*ptr != '\0') && (*ptr!='/')
	 && (*ptr != ';') 
	 && (incptr < Primary_Type + sizeof(Primary_Type) -1))
    *incptr++ = *ptr++;
  *incptr = '\0';
  while ((*ptr != '\0') && (*ptr == ' '))
    ptr++;
  incptr = SubType;
  if (*ptr == '/') {
    ptr++;
    while ((*ptr != '\0') && (*ptr == ' '))
      ptr++;
    while ((*ptr != ' ') && (*ptr != ']') && (*ptr != '\0') && (*ptr!=';')
	   && (incptr < SubType + sizeof(SubType) -1))
      *incptr++ = *ptr++;
  }
  *incptr = '\0';
  while ((*ptr != '\0') && (*ptr == ' '))
    ptr++;
  incptr = Params;
  while (*ptr == ';') {
    ptr++;
    if (incptr > Params) {
      *incptr++ = ';';
    } else if (*ptr == ' ')
      ptr++;

    while ((*ptr != '\0') && (*ptr == ' ')
	   && (incptr < Params + sizeof(Params) -1))
      *incptr++ = *ptr++;

    while ((*ptr != ' ') && (*ptr != ']') && (*ptr != '\0') && (*ptr!=';')
	   && (incptr < Params + sizeof(Params) -1))
      *incptr++ = *ptr++;
    while ((*ptr != '\0') && (*ptr == ' '))
      ptr++;
  }
  *incptr = '\0';

  while ((*ptr != '\0') && (*ptr == ' '))
    ptr++;
  incptr = Encoding;
  while ((*ptr != ' ') && (*ptr != ']') && (*ptr != '\0')
	 && (incptr < Encoding + sizeof(Encoding) -1))
    *incptr++ = *ptr++;
  *incptr = '\0';

  if (strlen(Include_Filename) == 0) {
    Write_to_screen(catgets(elm_msg_cat, ElmSet, ElmNoIncludeFilename,
                    "\n\rNo Filename given, include line ignored\n\r"), 0);
    if (sleepmsg > 0)
	    sleep(sleepmsg);
    return(-1);
  }
  (void) expand_env(Expanded_Filename, Include_Filename, sizeof(Expanded_Filename));

  if (strlen(Primary_Type) == 0 || strlen(SubType) == 0 ) {
    Write_to_screen(catgets(elm_msg_cat, ElmSet, ElmNoContentTypeGiven,
                    "\n\rNo Content-type given, include line ignored\n\r"), 0);
    if (sleepmsg > 0)
	    sleep(sleepmsg);
    return(-1);
  }

  Enc_Type = check_encoding(Encoding);

  if (Enc_Type == ENCODING_ILLEGAL) {
        Write_to_screen(catgets(elm_msg_cat, ElmSet, ElmEncodingIsIllegal,
			"\n\rEncoding is illegal\n\r"), 0);
	if (sleepmsg > 0)
		sleep(sleepmsg);
	return(-1);
  }

  if (can_open(Expanded_Filename, "r")) {
        Write_to_screen(catgets(elm_msg_cat, ElmSet, ElmIncludeCannotAccess,
			"\n\rInclude File can't be accessed\n\r"), 0);
	if (sleepmsg > 0)
		sleep(sleepmsg);
	return(-1);
  }

  /* Don't allow 7BIT if 8-bit charcters in any type,    
   * don't allow 8BIT if 'binary' characters       - K E H */
  if (Enc_Type == ENCODING_7BIT || Enc_Type == ENCODING_NONE
      || Enc_Type == ENCODING_8BIT) {
    
    FILE * fp = fopen (Expanded_Filename, "r");
    if (fp) {
      int tmp = needs_encoding (fp);
      if (tmp & HAVE_BINARY) {

	error ("Include file has BINARY data.");
	
        if (sleepmsg > 0)
	  sleep(sleepmsg);
	
	if (Enc_Type == ENCODING_7BIT || Enc_Type == ENCODING_8BIT) {
	  if (check)
	    return -1; /* indicate failure */
	}
	
	Enc_Type = ENCODING_BINARY;

      } else if ((tmp & HAVE_8BIT) && Enc_Type != ENCODING_8BIT) {
	
	error ("Include file has 8BIT data.");
	
        if (sleepmsg > 0)
	  sleep(sleepmsg);
	
	if (Enc_Type == ENCODING_7BIT) {
	  if (check)
	    return -1; /* indicate failure */
	}
	
	Enc_Type = ENCODING_8BIT;
	
      }
      fclose(fp);
    }
  }

  if (Enc_Type == ENCODING_8BIT) {
#ifndef USE_8BITMIME
    if (allow_no_encoding < 1) {
      Enc_Type = ENCODING_QUOTED;
      error ("Mailer (MTA) doesn't support 8BIT encoding.");
      if (sleepmsg > 0)
	sleep(sleepmsg);
      
      if (check)
	return -1; /* indicate failure */
    }
#endif
  }

  if (Enc_Type == ENCODING_BINARY) {
#ifndef USE_BINARYMIME
    if (allow_no_encoding < 2) {
      /* Convert to QUOTED-PRINTABLE */
      Enc_Type = ENCODING_QUOTED;
      error ("Mailer (MTA) doesn't support BINARY encoding!");
      if (sleepmsg > 0)
	sleep (sleepmsg);
      if (check)
	return (-1);
    }
#endif
  }

  is_text = is_text_type(Primary_Type, SubType, Enc_Type);
  /* 1 if is text type (true)
   * 0 if not text type
   * -1 if can't be encoded (ie structured) Message/ or Multipart/ 
   */
    
  if (is_text < 0 && (Enc_Type == ENCODING_QUOTED ||
		      Enc_Type == ENCODING_BASE64)) {
      error ("Content-Type don't allow encoding -- ignoring this part.");
      if (sleepmsg > 0)
	sleep (sleepmsg);
      return (-1);
  }


  (void) update_encoding(&(mime_info->encoding_top),Enc_Type);

  if (check) {
	return(0);
  }

  incfile = fopen (Expanded_Filename, "r");
  if (incfile) {

    dprint (10, (debugfile, 
		 "Include_Part: '%s' C-T=%s/%s Params=%s Enc=%d is_text=%d\n",
		 Expanded_Filename,Primary_Type,SubType, Params, Enc_Type,
		 is_text));

    fprintf(dest, "%s %s/%s", MIME_CONTENTTYPE, Primary_Type, SubType);
    if (Params[0] == '\0') 
      print_EOLN(dest,mime_info->encoding_top);
    else { 
      if (strlen(Primary_Type) + strlen(SubType) + strlen(Params) > 60) {
	fprintf(dest,";");
	print_EOLN(dest,mime_info->encoding_top);
	fputc('\t',dest);
      } else
	fprintf(dest,"; ");
      fprintf(dest,"%s",Params);
      print_EOLN(dest,mime_info->encoding_top);
    }

    {
      char buf[STRING];
      buf [0] = 0;

      add_parameter(buf,"filename",Include_Filename,sizeof(buf),0);
      
      /* When user uses [include ...] it is better use disposition
       * 'inline' instead of 'attachment'
       */
      fprintf(dest,"Content-Disposition: inline; %s",buf);
      print_EOLN(dest,mime_info->encoding_top);
    }

    if (Enc_Type != ENCODING_NONE) {
      if (Enc_Type < ENCODING_EXPERIMENTAL) 
	fprintf(dest, "%s %s", MIME_CONTENTENCOD, 
		ENCODING(Enc_Type));
      else /* For ENCODING_EXPERIMENTAL */
	fprintf(dest, "%s %s", MIME_CONTENTENCOD, Encoding);
      print_EOLN(dest,mime_info->encoding_top);
    }
    print_EOLN(dest,mime_info->encoding_top);

    /* encoding rules are different for text and no text 
     * for text we must do \n -> \r\n before base64 encoding */
    
    /* For ENCODING_EXPERIMENTAL this is already supposed to be encoded */
    (void)write_encoded(incfile,dest,Enc_Type,copy,is_text,mime_info);
    
    fclose(incfile);
  }
  else {
    MoveCursor(LINES, 0);
    Raw(OFF);
    Write_to_screen(catgets(elm_msg_cat, ElmSet, ElmCantOpenIncludedFile,
		    "\nCan't open included File\n"), 0);
    emergency_exit();
  }
  return(0);
}

int check_encoding(Encoding)
char *Encoding;
{
	if (strlen(Encoding) == 0) return (ENCODING_NONE);
	if (strincmp(Encoding, ENC_NAME_7BIT, strlen(ENC_NAME_7BIT)) == 0)
		return (ENCODING_7BIT);
	if (strincmp(Encoding, ENC_NAME_8BIT, strlen(ENC_NAME_8BIT)) == 0)
		return (ENCODING_8BIT);
	if (strincmp(Encoding, ENC_NAME_BINARY, strlen(ENC_NAME_BINARY)) == 0)
		return (ENCODING_BINARY);
	if (strincmp(Encoding, ENC_NAME_QUOTED, strlen(ENC_NAME_QUOTED)) == 0)
		return (ENCODING_QUOTED);
	if (strincmp(Encoding, ENC_NAME_BASE64, strlen(ENC_NAME_BASE64)) == 0)
		return (ENCODING_BASE64);
	if (strincmp(Encoding, "x-", 2) == 0) return (ENCODING_EXPERIMENTAL);
	return(ENCODING_ILLEGAL);
}

#if 0
needs_mmdecode(EncType)
int EncType;
{
	if ((EncType == ENCODING_NONE) ||
	    (EncType == ENCODING_7BIT) ||
	    (EncType == ENCODING_8BIT) ||
	    (EncType == ENCODING_BINARY) ||
	    (EncType == ENCODING_QUOTED) ||
	    (EncType == ENCODING_BASE64)) {
	    /* We don't need to go out to mmdecode, return 0 */
	    return(0);
	} else {
	    return(1);
	}
}
#endif

int charset_ok(s)
char *s;
{
    /* Return true if configured charset could display us-ascii too */
    char buf[SLEN];	/* assumes sizeof(charset_compatlist) <= SLEN */
    char *bp, *chset;

    /* the "charset_compatlist[]" format is: */
    /*   charset charset charset ... */
    bp = strcpy(buf, charset_compatlist);
    while ((chset = strtok(bp, " \t\n")) != NULL) {
	bp = NULL;
	if (istrcmp(chset, s) == 0)
	    break;
    }

    /* see if we reached the end of the list without a match */
    if (chset == NULL) {
	return(FALSE);
    }
    return(TRUE);
}

/* Determine whether or not the data in "fp" needs to be QUOTED-PRINTABLE
    encoded.  Used when sending a message.
       1 == have 8bit data                (HAVE_8BIT)
       4 == have control charcters        (HAVE_CTRL)
       8 == mime requires encoding BINARY (HAVE_BINARY)
*/
int
needs_encoding (fp)
FILE *fp;
{
  int ch;
  int ret = FALSE;
  int len = 0;

  rewind (fp);
  while ((ch = fgetc (fp)) != EOF) {
    /* check for end of line */
    if (ch == 13) {   /* CR */
      ch = fgetc(fp);
      if (ch != 10) {  /* Not CR LF */
	dprint (3, (debugfile, "\nneeds_encoding(): found CR without LF\n"));
	ret |= HAVE_BINARY;
      }
      if (ch == EOF)
	break;
    }
    if (ch == 10) {
      len = 0;
      continue; /* skip newlines and tabs */
    }
    len++;
    if (len > 990) {
      dprint (3, (debugfile,"\nneeds_encoding(): Line over 990 characters\n"));
	ret |= HAVE_BINARY;
    }

    if (ch == 9) /* skip newlines and tabs */
      continue;
    if (ch < 32 || ch > 126) {
      dprint (3, (debugfile, "\nneeds_encoding(): found char decimal=%d\n", ch));
      if (ch == 0) 
	ret |= HAVE_BINARY;
      if (ch < 32 || ch == 127)
	ret |= HAVE_CTRL;
      if (ch > 127)
	ret |= HAVE_8BIT;
    }
    if (ret == (HAVE_8BIT | HAVE_CTRL | HAVE_BINARY))
      break;

  }
  rewind (fp);
  return (ret);
}

char *error_description();

int
have_metamail()
{
  int return_value = 1;
  
  if (strcmp(metamail_path,"none") == 0 || 
      metamail_path[0] == '\0') {
    return_value = 0;
  } else if (metamail_path[0] == '/') {
    if (-1 == access(metamail_path,EXECUTE_ACCESS)) {
      int err = errno;
      error2("Can't execute metamail: %s: %s",metamail_path,
	     error_description(err));
      dprint(5,(debugfile,"have_metamail: no access %s: %s\n",metamail_path,
		error_description(err)));
      if (sleepmsg > 0)
	sleep(sleepmsg);
      return_value = 0;
    }
  }

  dprint(5,(debugfile,"have_metamail=%d\n",return_value));
  return return_value;
}

#endif /* MIME */
