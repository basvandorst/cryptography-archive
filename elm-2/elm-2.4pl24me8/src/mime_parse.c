#include "headers.h"
#include "me.h"

#ifdef MIME
/* As ordered in mime.h */
char *mime_types[] = {
	"unknown",
	"application",
	"audio",
	"image",
	"message",
	"multipart",
	"text",
	"video",
        NULL
};

int
mime_needs_processing (p)
     mime_t *p;
{
  /* A routing which metapager() calls to determine if the message specified
   * by "p" to determine whether or not any special MIME processing needs
   * to be performed.  This information is used to decide if a temp file
   * needs to be used in order to effect backwards paging, or if we can page
   * directly out of the mailbox file.
   */

  if (p->type != MIME_TYPE_TEXT)
    return TRUE;

  /* This is to support those poor unfortunate folk who don't have true
   * MIME support.  Often, you see "Content-Type: text".  We'll just
   * assume that it's normal text.
   */
  if (p->subtype[0] == '\0')
    return FALSE;

  if (istrcmp (p->subtype, "plain") != 0)
    return TRUE;
  if (p->encoding != ENCODING_7BIT && p->encoding != ENCODING_8BIT)
    return TRUE;

  return FALSE;
}

int
mime_notplain (p)
     mime_t *p;
{
  CT_decoder_t d = select_CT_decoder(p);
  char buf[STRING];
  
  if (d == rfc822_decode)
    return FALSE;
  
  /* Notice: if (d == text_unsupported_decode)
   *         we return TRUE, because we want call metamail
   *         if it is available
   */
  
  if (d == text_decode) {
    if (mime_get_charset(buf,p->type_opts,sizeof(buf)))
      return FALSE;
  }
  else if (p->type == MIME_TYPE_MULTIPART && pagemultipart)
    return FALSE;
#ifdef USE_PGP
  else if (d == mime_pgp_decode)
    return FALSE;
#endif
  return TRUE;
}

char *
mime_parse_content_opts (str)
     char *str;
{
  /* A routine for parsing the options in a Content-Type: field.  The
   * important point here is to skip the semi-colon if it appears
   * inside of quotes.  This works sort of like strtok, except that
   * the token is already known.
   */
  static char *ptr;
  char *ret;
  int in_quote = 0;

  /* This is the initialization call */
  if (str)
    ptr = str;
  
  if (*ptr == '\0')
    return NULL;

  ret = ptr;
  while (*ptr) {
    if (*ptr == '\\' && in_quote) {		  
      /* \ escapes next character  
       * (not allowed outside of quotes) */
      ptr++;
      if (*ptr == '\0')
	break;
    } else if (*ptr == '\"') {
      if (in_quote)
	in_quote = 0;
      else
	in_quote = 1;
    }
    else if (! in_quote) {
      if (*ptr == ';') {
	*ptr++ = '\0';
	/* skip leading spaces */
	while (*ptr && isspace ((unsigned char)*ptr))
	  ptr++;
	return (ret);
      }
    } 
    ptr++;
  }
  return (ret);
}

/* Removes comments from string */
void 
rfc822_reap_comments (ptr, comments, size) 
     char *ptr, *comments;
     int size;
{
  char *w_ptr = ptr, *c_ptr = comments;
  int comment_level = 0, saved_level = 0;
  int in_quote = 0;
  
  while (*ptr) {
    if (*ptr == '\\' && (in_quote || comment_level > 0)) {		  
      /* \ escapes next character  
       * (not allowed outside of quotes or comments) */
      ptr++;
      if (*ptr == '\0')
	break;
      if (comment_level > 0 && comments) {
	if (c_ptr < comments + size - saved_level -3) {
	  *c_ptr++ = '\\';
	  *c_ptr++ = *ptr;
	}
      }
      if (comment_level == 0) {
	*w_ptr++ = '\\';
	*w_ptr++ = *ptr;
      }
      ptr++;
      continue;
    } else if (comment_level > 0) {
      if (*ptr == ')')
	comment_level --;
      if (*ptr == '(')
	comment_level ++;
      if (comments && c_ptr < comments + size - saved_level -3) {
	*c_ptr++ = *ptr;
	saved_level = comment_level;
      }
    } else if (*ptr == '\"') {
      if (in_quote)
	in_quote = 0;
      else
	in_quote = 1;
    } else if (!in_quote && *ptr == '(') {
      comment_level ++;
      if (comments && c_ptr < comments + size - saved_level -4) {
	if (c_ptr != comments)
	  *c_ptr++ = ' ';
	*c_ptr++ = *ptr;
	saved_level = comment_level;
      }
      *w_ptr++ = ' ';  /* RFC 822 (STD 11) says that 
			  comments represents one space */
    }
    if (comment_level == 0 && *ptr != ')')
      *w_ptr++ = *ptr;
    ptr++;
  }
  while (comments && saved_level > 0) {
    *c_ptr++ = ')';
    saved_level--;
  }

  if (comments)
    *c_ptr = '\0';
  *w_ptr = '\0';
}

void
mime_destroy (ptr)
     mime_t *ptr;
{
  mime_t *tmp;

  while (ptr) {
    tmp = ptr;
    ptr = ptr->next;

    if (tmp->description)
      free (tmp->description);
    if (tmp->type_opts)
      free (tmp->type_opts);
    if (tmp->disposition_opts)
      free (tmp->disposition_opts);

    if (tmp->parts)
      mime_destroy (tmp->parts);

    if (tmp->unlink)
      unlink (tmp->pathname);

    free (tmp);
  }
  return;
}

void
mime_t_clear (mt)
     mime_t *mt;
{
  mt->flags = mt->offset = mt->begin_offset = mt->length = 0;
  mt->encoding = ENCODING_7BIT;
  mt->unlink = 0;
  mt->type = MIME_TYPE_TEXT;
  mt->disposition = DISP_INLINE;
  mt->notplain = 0;

  if (mt->parts)
    mime_destroy (mt->parts);
  if (mt->next)
    mime_destroy (mt->next);

  mt->next = mt->prev = mt->parts = NULL;

  if (mt->description)
    free (mt->description);
  mt->description = NULL;
  if (mt->type_opts)
    free (mt->type_opts);
  mt->type_opts = NULL;
  if (mt->disposition_opts)
    free (mt->disposition_opts);
  mt->disposition_opts = NULL;
  if (mt->pathname)
    free (mt->pathname);
  mt->pathname = NULL;

  strcpy (mt->subtype, "plain");

  return;
}

void
mime_get_disposition (str, mt)
     char *str;
     mime_t *mt;
{
  char *c, tmp[VERY_LONG_STRING];

  dprint (2, (debugfile, "mime_get_disposition(): str=\"%s\"\n", str));
  
  /* Don't harm "str" */
  strfcpy (tmp, str, sizeof(tmp));
  
  rfc822_reap_comments (tmp, NULL, 0);
  
  /* Look for the options field */
  if ((c = strchr (tmp, ';')) != NULL) {
    char *d = c;
    while (d > tmp && whitespace(*(d-1)))
      d--;
    *d = '\0';
    c++;
    while (*c && whitespace(*c))
      c++;
    mt->disposition_opts = strmcpy (mt->disposition_opts, c);
  }
  else {
    char *d = tmp + strlen(tmp);
    while (d > tmp && whitespace(*(d-1)))
      d--;
    *d = '\0';
    if (mt->disposition_opts) {
      free (mt->disposition_opts);
      mt->disposition_opts = NULL;
    }
  }
  
  /* All that's left now is the main disposition */
  c = tmp;
  while (*c && whitespace(*c))
    c++;
  /* No Content-Disposition -header     -> DISP_INLINE
   *    Content-Disposition: inline     -> DISP_INLINE
   *    Content-Disposition: attachment -> DISP_ATTACH
   *    Content-Disposition: {unknown}  -> DISP_ATTACH
   * See RFC 1806 (Experimental protocol) for details.
   */   
  if (istrcmp (c, "inline") != 0)
    mt->disposition = DISP_ATTACH;
  else
    mt->disposition = DISP_INLINE;
     
  dprint (2, (debugfile,
	     "mime_get_disposition(): disposition=\"%s\", disposition_opts=\"%s\"\n",
	     DISPOSITION(mt->disposition), NONULL(mt->disposition_opts)));
}

int
mime_check_type (str)
	char *str;
{
  if (istrcmp (str, "text") == 0)
    return MIME_TYPE_TEXT;
  if (istrcmp (str, "multipart") == 0)
    return MIME_TYPE_MULTIPART;
  if (istrcmp (str, "message") == 0)
    return MIME_TYPE_MESSAGE;
  if (istrcmp (str, "application") == 0)
    return MIME_TYPE_APPLICATION;
  if (istrcmp (str, "image") == 0)
    return MIME_TYPE_IMAGE;
  if (istrcmp (str, "audio") == 0)
    return MIME_TYPE_AUDIO;
  if (istrcmp (str, "video") == 0)
    return MIME_TYPE_VIDEO;
  return MIME_TYPE_UNKNOWN;
}

void
mime_get_content (str, mt)
     char *str;
     mime_t *mt;
{
  char *c, tmp[VERY_LONG_STRING];

  dprint (2, (debugfile, "mime_get_content(): str=\"%s\"\n", str));

  /* Don't harm "str" */
  strfcpy (tmp, str, sizeof(tmp));

  rfc822_reap_comments (tmp, NULL, 0);

  /* Look for the options field */
  if ((c = strchr (tmp, ';')) != NULL) {
    char *d = c;
    while (d > tmp && whitespace(*(d-1)))
      d--;
    *d = '\0';
    c++;
    while (*c && whitespace(*c))
      c++;
    mt->type_opts = strmcpy (mt->type_opts, c);
  }
  else {
    char *d = tmp + strlen(tmp);
    while (d > tmp && whitespace(*(d-1)))
      d--;
    *d = '\0';
    if (mt->type_opts) {
      free (mt->type_opts);
      mt->type_opts = NULL;
    }
  }
  
  mt->subtype[0] = '\0';
  /* Get the subtype */
  if ((c = strchr (tmp, '/')) != NULL) {
    char *d = c;
    while (d > tmp && whitespace(*(d-1)))
      d--;
    *d = '\0';

    c++;
    while (*c && whitespace(*c))
      c++;
    strfcpy (mt->subtype, c, sizeof(mt->subtype));
  }
  
  /* All that's left now is the main type */
  c = tmp;
  while (*c && whitespace(*c))
    c++;
  mt->type = mime_check_type (c);

  /* Mark MESSAGE/RFC822 so we can do special handling later */
  if (mt->type == MIME_TYPE_MESSAGE && istrcmp (mt->subtype, "rfc822") == 0)
    mt->flags |= MIME_RFC822;

  dprint (2,(debugfile,
	     "mime_get_content(): type=\"%s\", subtype=\"%s\", opts=\"%s\"\n",
	     mime_types[mt->type], mt->subtype, NONULL(mt->type_opts)));

  return;
}

int
mime_get_param (name, value, opts, size)
     char *name, *opts, *value;
     int size;
{
  char *c, tmp[VERY_LONG_STRING];
  int i = 0, quoted = FALSE, found = FALSE;

  value[0] = '\0';

  if (!opts) {
    dprint(11, (debugfile,"mime_get_param: name=\"%s\", opts=NULL\n",name));
    return 0;
  }

  dprint(11, (debugfile,"mime_get_param: name=\"%s\", opts=\"%s\"\n",
	      name,opts));

  /* Make sure not to harm opts */
  strfcpy (tmp, opts, sizeof (tmp));
  
  c = tmp;
  while ((c = mime_parse_content_opts (c)) != NULL && !found) {
    char * d = strchr(c,'=');
    if (!d) {
      c = NULL;
      continue;    /* bad paramater */
    }
    while (d > c && (whitespace (*(d-1))))
      d--;
    *d = '\0';
    
    while (*c && whitespace(*c))
      c++;
    
    if (istrcmp (c, name) == 0) {
      found = TRUE;
      
      c = d+1;
      while (*c && whitespace(*c))
	c++;
      if (*c == '"') {
	c++;
        quoted = TRUE;
      }
      /* Either look for a trailing quoted (if quoted==TRUE) or a SPACE */
      while (*c && ((quoted && *c != '"') || (!quoted && *c != ' '))) {
	if (*c == '\\' && quoted) {
	  /* \ escapes next character */
	  c++;
	  if (!*c)
	    break;
	}
	if (i >= size-1)
	  break;    /* Avoid buffer overflow */
	value[i++] = *c++;
      }
      value[i] = '\0';
      break;
    }
    c = NULL;
  }

  dprint(11,(debugfile,"mime_get_param: found=%d, value=%s\n",found,value));

  return found;
}

static void
mime_get_boundary (boundary, opts, size)
     char *opts, *boundary;
     int size;
{
  if (!mime_get_param ("boundary", boundary, opts, size))
    error ("'boundary' paramater is missing from Multipart -type!");

  return;
}

/* There should better handling in metapager (I have one idea)
 * Perhaps in next version ...    -K E H    <hurtta@dionysos.fmi.fi>
 */
int 
mime_get_charset (charset_value, opts, size)
     char *opts, *charset_value;
     int size;
{  /* Returns 1 if charset is displayable with charset of text
    *     (suitable for replying)
    * Return 2 if cahrset is displayable with display_charset
    *           (when replying charset should be changed to display_charset)
    */
  int ret = 0;

  if (!mime_get_param("charset",charset_value,opts,size)) 
    strcpy(charset_value,"US-ASCII"); 
    /* Default charset if nothing specified */

  /* Check agaist charset of text */
  if (0 == istrcmp(charset_value,"US-ASCII") && charset_ok(charset))
    ret = 1; /* If charset of text can show us-ascii? */
  else if (0 == istrcmp(charset_value,charset))
    ret = 1;
  /* Check agaist display charset */
  else if (0 == istrcmp(charset_value,"US-ASCII") && 
	   charset_ok(display_charset))
    ret = 2; /* If display_charset can show us-ascii? */
  else if (0 == istrcmp(charset_value,display_charset))
    ret = 2;

  if (!opts) {
    dprint(9,(debugfile,"mime_get_charset: opts=NULL\n",opts));
  } else {
    dprint(9,(debugfile,"mime_get_charset: opts=\"%s\"\n",opts));
  }
  dprint(9,(debugfile,"                : charset_value=\"%s\"\n",
	    charset_value));
  dprint(9,(debugfile,
	    "                : (text)charset=\"%s\",display_charset=\"%s\"\n",
	    charset,display_charset));
  dprint(9,(debugfile,"mime_get_charset=%d\n",ret));

  return ret;
}

void
mime_t_zero (ptr)
     mime_t *ptr;
{
  /* This routine should be called whenever a new "mime_t" is created.  It
   * makes sure that the pointers inside are correctly initialized to NULL
   * so that we don't end up calling free() on an uninitialized pointer.
   */
  ptr->next = ptr->parts = NULL;
  ptr->description = ptr->type_opts = ptr->disposition_opts = 
    ptr->pathname = NULL;
}

mime_t *
mime_t_alloc ()
{
  mime_t *ptr;

  ptr = (mime_t *) safe_malloc (sizeof (mime_t));
  /* Make sure to clear the pointers initially so that later we know when
   * to reclaim memory in mime_t_clear().
   */
  mime_t_zero (ptr);
  mime_t_clear (ptr);
  return ptr;
}

int
read_header_line (fp, buf, size) 
     FILE *fp;
     char *buf;
     int size;
{
  /* Read and unfold header line -- stores maximum size-1 bytes to buffer
   * (plus \0). Also handle case when headers are eneded either CR LF or LF.
   * Returns number of bytes stored. Always _read_ end of header
   * (even when buffer fills). Returns 0 when reads empty line (only CR LF).
   * That indicates end of headers.
   */
  int len = 0,c;
  size--; /* Place for \0 */

#define PUTC(c) { if (len < size) buf[len++] = (c); }
  
  while (EOF != (c = fgetc(fp))) {
    if ('\r' == c) {                /* Is this CR LF sequence ? */
      if (EOF == (c = fgetc(fp)))
	break;
      if (c != '\n') 
	PUTC('\r');
    }
    if (c == '\n') {                /* Readed CR LF or LF, check folding */
      if (len == 0)
	break;                      /* End of headers ! */
      if (EOF == (c = fgetc(fp)))
	break;
      if (c != ' ' && c != '\t') {   /* Not a continuation line */
	ungetc(c,fp);
	break;
      }
      /* CRLF LWSP sequence should be replaced with ' ' */
      c = ' ';
    }
    PUTC(c);
  }
#undef PUTC
  buf[len] = 0;

  dprint(12,(debugfile,"read_header_line: len=%d, buf=%s\n",len,buf));

  return len;
}

mime_t *
mime_read_header (fp, opts)
     FILE *fp;
     int opts;
{
  int len;
  char buf [VERY_LONG_STRING], *c;
  mime_t *ptr;

  ptr = mime_t_alloc ();

  /* set some defaults */
  ptr->encoding = ENCODING_7BIT;
  if (opts & MIME_DIGEST) {
    ptr->type = MIME_TYPE_MESSAGE;
    strcpy (ptr->subtype, "rfc822");
  }
  else {
    ptr->type = MIME_TYPE_TEXT;
    strcpy (ptr->subtype, "plain");
  }
  ptr->disposition = DISP_INLINE;
  ptr->description = NULL;

  ptr->begin_offset = ftell (fp);
  ptr->offset = ptr->begin_offset;
  ptr->length = 0;

  while ((len = read_header_line (fp, buf, VERY_LONG_STRING)) > 0) {
    char * value;
    /* read_header_line is already done unfolding of header line 
     * and removing of CRLF on end of header */
    if (NULL != (value = strchr(buf,':'))) {
      *value = '\0';
      value++;
    }
    else { /* Not a header line!! */
      error ("Error when reading mime headers!");
      fseek (fp, ptr->offset, SEEK_SET);     /* rewind last read .... */
      break;
    }
    /* For rewind position in case of error ... */
    ptr->offset = ftell (fp);

    if (istrcmp (buf, "content-type") == 0)
      mime_get_content (value, ptr);
    else if (istrcmp (buf, "content-disposition") == 0)
      mime_get_disposition (value, ptr);
#if 0
    else if (istrcmp (buf, "content-length") == 0)
      ptr->length = atoi (value);
#endif
    else if (istrcmp (buf, "content-transfer-encoding") == 0) {
      rfc822_reap_comments (value, NULL, 0);
      c = value;
      while (*c && isspace((unsigned char) *c))
	c++;
      ptr->encoding = check_encoding (c);
    }
    else if (istrcmp (buf, "content-description") == 0) {
      c = buf + 20;
      while (*c && whitespace(*c))
	c++;
      ptr->description = NULL;
      ptr->description = strmcpy (ptr->description, c);
    }
  }

  /* where the real data starts */
  ptr->offset = ftell (fp);

  if (ptr->type == MIME_TYPE_MULTIPART) {
    if (istrcmp (ptr->subtype, "mixed") == 0)
      ptr->flags |= MIME_MIXED;
    else if (istrcmp (ptr->subtype, "digest") == 0)
      ptr->flags |= MIME_DIGEST;
  }
  else if (ptr->type == MIME_TYPE_MESSAGE &&
	   istrcmp (ptr->subtype, "rfc822") == 0)
    ptr->flags |= MIME_RFC822;
#if 0
  if (ptr->length) {
    /* Check validity of Content-Length: -header. It's value can be 
     * incorrect due the CRLF-> LF and  LF -> CRLF conversions...
     */
    int ok = 0, c;
    char buffer[LONG_STRING];

    fseek (fp, (long) (ptr->length),SEEK_CUR);   /* Seeks forward */
    c = fgetc (fp);
    /* There should be one newline before next mail or boundary */
    if ('\r' == c) /* OK */
      c = fgetc(fp);
    if ('\n' == c) /* OK */
      c = fgetc(fp);
    if (c != EOF) {
      int len = 0;
      /* So now we must be in either end of multipart
       * or end of mail .... */      
      buffer[0] = c;
      if (0 != (len = mail_gets(buffer+1,sizeof(buffer)-1,fp))) {
	if (0 == strncmp(buffer,"--",2))   /* End of multipart */
	  ok = 1;
	/* It is ok also if next mail starts here */
#ifdef MMDF
	if (0 == strcmp(buffer,MSG_SEPARATOR))
	  ok = 1;
#else  
	if (0 == strncmp(buffer,"From ",5))
	  ok = 1;
#endif  /* MMDF */
	dprint(5, (debugfile, 
		   "mime_read_header: checking content-length, ok = %d, (%d bytes) buffer=%s\n",
		   ok,len+1,buffer));
      }
    } else { 
      if (feof(fp)) {
	ok = 1;
	dprint(5, (debugfile, 
		   "mime_read_header: checking content-length, ok = %d, got EOF.\n",
		   ok));
      }
    }
    if (!ok) {
      dprint(5, (debugfile, 
		 "mime_read_header: content-length=%ld was incorrect!\n",
		 ptr->length));
      ptr->length = 0;     /* Reset it */
    }
    
    fseek(fp,ptr->offset,SEEK_SET);   /* return to start of data */
  }
#endif

  return ptr;
}    

static mime_t *
multipart_parse (fp, length, boundary, opts)
     FILE *fp;
     int length, opts;
     char *boundary;
{
  int blen, len,last_pos;
  long end_offset;
  char buf[VERY_LONG_STRING], subbound[STRING];
  mime_t *ptr = NULL, *tmp, *ret = NULL;

  dprint(9,(debugfile,"multipart_parse --> length=%d, boundary=%s\n",
	    length, boundary));

  blen = strlen (boundary);
  end_offset = ftell (fp) + length;
  last_pos = ftell(fp);

  while (ftell (fp) < end_offset) {
    if ((len = mail_gets (buf, VERY_LONG_STRING, fp)) == 0)
      break;

    if (buf[0] == '-' && buf[1] == '-' &&
        strncmp (buf + 2, boundary, blen) == 0) {

      /* Save the length of the previous part */
      if (ptr) {
        ptr->length = last_pos - ptr->offset;
	dprint(9,(debugfile,"multipart_parse: fixing length=%d\n",
		  ptr->length));
      }
      /* Check for the end boundary. */
      if (buf[blen+2] == '-' && buf[blen+3] == '-')
        break;
      
      tmp = mime_read_header (fp, opts);

      dprint(9,(debugfile,"multipart_parse: (reading) content-type=%s/%s; flags=%d\n",
		mime_types[tmp->type], tmp->subtype, tmp->flags));

      
      if (ret == NULL)
        ptr = ret = tmp;
      else {
        ptr->next = tmp;
        ptr = ptr->next;
      }
#if 0
      if (ptr->length > 0) {
	/* If the length of this part is known, skip ahead to the next
	 * part.  If the length is not known, we don't have to worry
	 * about it because the algorithm will search for the next
	 * boundary...
	 */
        fseek (fp, (long) (ptr->length), SEEK_CUR);
	continue;
      }
#endif
    }
    if (ptr && 0 == ptr->length) { /* mark position before CR LF */
      int pos = ftell(fp);
      if (len > 1 && buf[len-2] == '\r' && buf[len-1] == '\n')
	last_pos = pos -2;
      else if (len > 0 && buf[len-1] == '\n')
	last_pos = pos -1;
    }
  }

  /* Now that we know what this message consists of, see if any of the
   * parts contain data that needs to be parsed. */

  for (tmp = ret; tmp != NULL; tmp = tmp->next) {
    dprint(9,(debugfile,"multipart_parse: (parsing) content-type=%s/%s; flags=%d\n",
	      mime_types[tmp->type],tmp->subtype,tmp->flags));

    if (tmp->flags & MIME_RFC822) {
      fseek (fp, tmp->offset, SEEK_SET);
      tmp->parts = rfc822_parse (fp, tmp->length);
    }
    else if (tmp->type == MIME_TYPE_MULTIPART) {
      fseek (fp, tmp->offset, SEEK_SET);
      mime_get_boundary (subbound, tmp->type_opts, sizeof (subbound));
      tmp->parts = multipart_parse (fp, tmp->length, subbound, tmp->flags);
    }
  }

  /* Make sure to leave the stream at the end of the data since the
   * calling function might be assuming this.  */
  fseek (fp, end_offset, SEEK_SET);

  dprint(9,(debugfile,"multipart_parse <-- DONE\n"));

  return ret;
}

mime_t *
rfc822_parse (fp, len)
     FILE *fp;
     int len;
{
  /* Called to read MESSAGE/RFC822 data.  First reads the header of the
   * message for MIME information, then (when necessary) calls other
   * functions to determine the content of MULTIPART or MESSAGE/RFC822
   * data contained.
   */
  mime_t *ret;
  long end_offset = ftell (fp) + len;
  char boundary[STRING];

  dprint(9,(debugfile,"rfc822_parse --> len=%d\n",len));

  ret = mime_read_header (fp, MIME_MIXED);
  
  dprint(9,(debugfile,"rfc822_parse: content-type=%s/%s; flags=%d\n",
	    mime_types[ret->type], ret->subtype,ret->flags));

  if (ret->length == 0) { 
    int pos = ftell(fp);
    
    ret->length = end_offset - pos;
    dprint(9,(debugfile,"rfc822_parse: fixing length=%d\n",ret->length));
  }

  if (ret->type == MIME_TYPE_MULTIPART) {
    mime_get_boundary (boundary, ret->type_opts, STRING);
    ret->parts = multipart_parse (fp, ret->length, boundary, ret->flags);
  }
  else if (ret->flags & MIME_RFC822)
    ret->parts = rfc822_parse (fp, ret->length);
  
  if (ret->length == 0) 
    ret->length = len - (ret->offset - ret->begin_offset);

  /* Make sure the leave the stream at the end of the data! */
  fseek (fp, end_offset, SEEK_SET);

  dprint(9,(debugfile,"rfc822_parse <-- DONE\n"));

  return ret;
}

void
mime_warnings(hdr) 
     struct header_rec *hdr;
{

  if (hdr->status & PRE_MIME_CONTENT) {
    error("Error: MIME-message has pre-MIME content-type!");
    if (sleepmsg > 0)
      sleep (sleepmsg);
  }
  if (hdr->status & MIME_UNSUPPORTED) {
    error("Warning: Unsupported MIME-Version!");
    if (sleepmsg > 0)
      sleep (sleepmsg);
  }
}

void
attach_parse (hdr, fp)
     struct header_rec *hdr;
     FILE *fp;
{
  /* This routine checks to see if the multipart messages specified by
   * "hdr" has been parsed for its subparts, and if not, calls the routine
   * to do so.
   */

  char boundary[STRING];

  mime_warnings(hdr);

  /* Copy value */
  if (hdr -> content_length >= 0)
    hdr->mime_rec.length = hdr -> content_length;

  if (hdr->mime_rec.begin_offset == 0) {
    char buf[STRING];
    int tmp;

    fseek(fp,hdr->offset,SEEK_SET);
    hdr->mime_rec.begin_offset = hdr->offset;
    /* Skip mailbox's separator lines ... */

    dprint(9,(debugfile,"attach_parse: scanning begin_offset: %d\n",
	      hdr->mime_rec.begin_offset));
    while (0 < (tmp = mail_gets(buf,sizeof(buf),fp))) {
      dprint(9,(debugfile,"attach_parse: len=%d, got: %s\n",tmp,buf));
#ifdef MMDF
      if (0 == strcmp(buf,MSG_SEPARATOR))
	continue;
#endif
      if (0 == strncmp(buf,"From ",5))
	continue;
      break;
    }
    hdr->mime_rec.begin_offset = ftell(fp) - tmp;
    dprint(9,(debugfile,"attach_parse: begin_offset=%d\n",
	      hdr->mime_rec.begin_offset));
  }

  if (hdr->mime_rec.type == MIME_TYPE_MULTIPART) {
    if (hdr->mime_rec.parts == NULL) {
      mime_get_boundary (boundary, hdr->mime_rec.type_opts, STRING);
      fseek (fp, hdr->mime_rec.offset, SEEK_SET);
      hdr->mime_rec.parts = multipart_parse (fp, hdr->content_length, boundary, hdr->mime_rec.flags);
    }
  }
  else if (hdr->mime_rec.flags & MIME_RFC822) {
    if (hdr->mime_rec.parts == NULL) {
      fseek (fp, hdr->mime_rec.offset, SEEK_SET);
      hdr->mime_rec.parts = rfc822_parse (fp, hdr->content_length);
    }
  }
  return;
}
#endif /* MIME */
