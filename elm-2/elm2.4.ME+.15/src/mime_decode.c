#include "headers.h"
#include "me.h"

#ifdef MIME

static int index_hex[128] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
     0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

static int index_64[128] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};

#define base64(c) ((((c) > 0) && ((c) < 127)) ? index_64[ (c) ] : -1)
#define hex(c) ((((c) > 0) && ((c) < 127)) ? index_hex[ (c) ] : -1)

static void Xbit_decode		  P_((state_t *, int)); /* Prototype */

static void Xbit_decode (s, len)
     state_t *s;
     int len;
{
  char buf[VERY_LONG_STRING];
  int offset = 0, length;

  if (s->prefix) {
    strcpy (buf, s->prefix);
    offset = strlen (buf);
  }
  while (len > 0) {
    if (0 ==
	(length = mail_gets (buf + offset, VERY_LONG_STRING - offset, s->fpin)))
      break;
    len -= length;

    /* Take care of CRLF => LF conversion */
    if (length > 1 &&
	buf[offset + length - 1] == '\n' &&
        buf[offset + length - 2] == '\r') {
      buf[offset + length - 2] = '\n';
      buf[offset + length - 1] = '\0';
      length--;
    }

    state_put(buf,offset+length,s);
  }
}

PUBLIC void base64_decode (s, length, astext)
     state_t *s;
     int length;
     int astext;
{
  /* Notes:  this routine assumes that strlen(line length) % 4 == 0.
     Most intelligent software will do this, so I haven't worried
     about being *completely* compliant with the standard.  I've
     not run into any encoded data where this was not true.  */
  
  char buf[STRING], *p;
  int
    bytes = 0, /* the total number of bytes read so far */
    terminate = FALSE,
    c1, c2, c3, c4;
  unsigned char
    ch,
    store_ch = 0; /* for astext */
  int corrupted = FALSE;

  dprint(10,(debugfile,
           "base64_decode: length=%d, prefix=%s, astext=%d\n",
           length,
           s->prefix ? s->prefix : "<NULL>",
           astext));

  /* Make sure to quote the first line! */
  state_add_prefix(s);

  while (! terminate) {
    if (bytes >= length)
      break;

    if (state_gets (buf, VERY_LONG_STRING, s) == NULL)
      break;

    bytes += strlen (buf);

    p = buf;
    while (*p) {
      c1 = *p++;

      if (base64(c1) == -1) {
        p--;
        /* partial fix -- K E H   <hurtta@dionysos.FMi.FI>  */

        while (*p == ' ' || *p == '\t' || *p == '\r')
          p++;

        if (*p != '\n')
          corrupted = TRUE;  /* Perhaps corrupted */

        if (*p == 0)
          break;

        c1 = *p++;
      }

      /* End of the line. */
      if (c1 == '\n')
	continue;

      c2 = *p++;
      
      c1 = base64(c1);
      if (c1 == -1)
        corrupted = TRUE;
      c2 = base64(c2);
      if (c2 == -1)
        corrupted = TRUE;
      ch = (c1 << 2) | (c2 >> 4);
      
      if (store_ch && ch != '\n') /* for astext */
	state_putc(store_ch,s);

      store_ch = 0;
      if (astext && ch == '\r')
        store_ch = ch;
      else {
	state_putc(ch,s);
	if (ch == '\n')
	  state_add_prefix(s);
      }      
    
      c3 = *p++;
      if (c3 == '=') {
	terminate = TRUE;
	break;
      }
      c3 = base64(c3);
      if (c3 == -1)
	corrupted = TRUE;
      ch = ((c2 & 0xf) << 4) | (c3 >> 2);

      if (store_ch && ch != '\n')
	state_putc(store_ch,s);

      store_ch = 0;

      if (astext && ch == '\r')
	store_ch = ch;
      else {
	state_putc(ch,s);
	if (ch == '\n')
	  state_add_prefix(s);
      }
  
      c4 = *p++;
      if (c4 == '=') {
	terminate = TRUE;
	break;
      }
      
      c4 = base64(c4);
      if (c4 == -1)
	corrupted = TRUE;
      ch = ((c3 & 0x3) << 6) | c4;
      
      if (store_ch && ch != '\n')
	state_putc(store_ch,s);

      store_ch = 0;

      if (astext && ch == '\r') {
	store_ch = ch;
      }
      else {
	state_putc(ch,s);
	if (ch=='\n')
	  state_add_prefix(s);
      }
    }
  }

  /* Make sure to flush anything left in the internal buffer. */
  if (store_ch)  /* for astext */
    state_putc(store_ch,s);

  if (corrupted) {
    error ("BASE64 data was corrupt!");
    if (sleepmsg > 0)
      sleep (sleepmsg);
  }

  dprint(10,(debugfile,"base64_decode: readed=%d bytes, corrupted=%d.\n",
	     bytes,corrupted));   

  return;
}

PUBLIC void quoted_printable_decode (s, length, astext)
     state_t *s;
     int length;
     int astext;
{
  int bytes = 0; /* number of bytes read */
  int nl = TRUE; /* flag to indicate hitting a new line */
  char *p;
  int c1, c2;
  unsigned char ch, store_ch = 0;
  char buf[VERY_LONG_STRING];
  int corrupted = 0;

  dprint(10,(debugfile,
	     "quoted_printable_decode: length=%d, prefix=%s, astext=%d\n",
	     length,
	     s->prefix ? s->prefix : "<NULL>",
	     astext));

  for (;;) {
    if (length && length - bytes <= 0)
      break;

    if (state_gets (buf, VERY_LONG_STRING, s) == NULL)
      break;
    bytes += strlen (buf);

    p = buf;
    while (*p) {

      /* If there is a prefix and this is the beginning of a new line... */
      if (nl) {
	state_add_prefix(s);
	nl = FALSE;
      }

      if (store_ch)
	state_putc(store_ch,s);
      
      if (*p == '=') {
	p++;
	/* Ignore spaces in end of line   -- see MIME */
	if (*p == '\r' || *p == ' ' || *p == '\t') {
	  char *t = p;
	  while (*t && (*t == '\r' || *t == ' ' || *t == '\t'))
	    t++;
	  if (*t && *t == '\n')
	    p = t;
	}

	if (*p == '\n') { /* soft linebreak */
	  if (length == 0)
	    break;
	  p++;
	}
	else {
	  c1 = hex(*p);
	  if (c1 == -1)
	    corrupted = TRUE;
	  p++;
	  c2 = hex(*p);
	  if (c2 == -1)
	    corrupted = TRUE;
	  p++;
	  ch = (c1 << 4) | c2;

	  /* We not need here CR LF -> LF removing, because
	   * CRLF's which presents end of line should NOT be encoded.
	   *                             - K E H <hurtta@dionysos.FMI.FI> */

	  state_putc(ch,s);
	}
      }
      else {
       if (astext && *p == '\r')
         store_ch = *p;
       else
	 state_putc(*p,s);
	 
       if (*p == '\n') {
	 nl = TRUE;
	 if (length == 0)
	   break;
       }

       p++;
      }  
    }
  }

  /* Flush anything left in the buffer */
  if (store_ch) /* for astext */
    state_putc(store_ch,s);

  if (corrupted) {
    error ("Seems that QUOTED-PRINTABLE data was corrupted.");
    if (sleepmsg > 0)
      sleep(sleepmsg);
  }
  
  dprint(10,(debugfile,
	     "quoted_printable_decode: readed=%d bytes, corrupted=%d.\n",
	     bytes,corrupted));

  return;
}

static void multipart_decode  P_((mime_t *, state_t *)); /* Prototype */

static void multipart_decode (att, state)
     mime_t *att; /* The list of attachments for this MULTIPART data */
     state_t *state;
{
  int nattach = 0;
  char buf[VERY_LONG_STRING];

  while (att) {
    fseek (state->fpin, att->offset, 0);
    nattach++;
    if (state->displaying) {
      char *Encoding = "???";
       
      Encoding = ENCODING(att->encoding);
      
      /* First print out a nice display line about this attachment */
      sprintf (buf,
	       "%s[Attach #%d: \"%.60s\"]\n[Type: %.15s/%.30s, Encoding: %s, Size: %d]\n",
               nattach > 1 ? "\n" : "",
	       nattach, NONULL(att->description),
	       mime_types[att->type], att->subtype,
	       Encoding, att->length);
    }
    else {
      buf[0] = '\0';
      if (nattach > 1)
        strcpy (buf, "\n");
      if (att->description && (strlen(att->description) < sizeof(buf) -30)) {
	strcat (buf, "Content-Description: ");
	strcat (buf, att->description);
	strcat (buf, "\n\n");
      }
    }
      
    state_puts(buf,state);
    if (state->displaying)
      state_putc('\n', state);
    mime_decode (att, state);
    att = att->next;
  }
}

static void multipart_0_decode P_((mime_t *, state_t *)); /* Prototype */

static void multipart_0_decode (ptr, state)
     mime_t *ptr;
     state_t *state;
{
  if (ptr->parts)
    multipart_decode (ptr->parts, state);
  else
    state_puts ("[Content-Type: multipart/*, internal error]\n", state);
}

static void rfc822_decode P_((mime_t *, state_t *)); /* Prototype */

static void rfc822_decode (mt, state)
     mime_t *mt;
     state_t *state;
{
  /* This looks a lot like multipart_decode() except that we want to print
   * the headers for the message.  NOTE: "mt" should be a pointer to the
   * RFC822 attachment, NOT its subpart, since we need the offset in order
   * to print out the headers.
   */

  int matched = FALSE;
  char buf[VERY_LONG_STRING];
  
  fseek (state->fpin, mt->offset, 0);
  
  /** Print out the headers. **/
  while (1) {
    int len;
    char *ptr;
    
   /* read_header_line with flag = 1 terminates header with \n and marks
     * folding with \n
     */

    if (0 == (len = read_header_line (state->fpin, buf, VERY_LONG_STRING,1)))
      break;
    
    if (filter) {
      if (matches_weedlist (buf)) {
	matched = 1;
	continue;
      }
      else
	matched = 0;
    }

    if (buf[0] == '\n') {
      /* strtok -hack don't print last empty line anyway ... */ 
      break;
    }
    else {
      ptr = strchr(buf,':');
      if (ptr) {
	char *ptr2 = ptr +1;
	int class;
	*ptr = '\0';

	class = classify_header(buf);
	dprint(12,(debugfile,"rfc822_decode: header=%s,class=%d,rest=%s",
		   buf,class,ptr2));

	if (class & HDR_TEXT) {
	  if (is_rfc1522 (ptr2))
	    rfc1522_decode (ptr2, sizeof (buf) - (ptr2 - buf) -1);
	} else if (class & HDR_STRUCTURED) {
	  rfc1522_decode_structured(class,
				    ptr2, sizeof (buf) - (ptr2 - buf) -1);
	}

	dprint(12,(debugfile,"rfc822_decode: decoded rest=%s",ptr2));
	*ptr = ':';
      }
    }

    for (ptr = strtok(buf,"\n"); ptr; ptr = strtok(NULL,"\n")) { 
      if (ptr > buf) { /* Do folding */
	--ptr;
	if (*(ptr+1) == ' ')
	  *ptr = ' ';
	else
	  *ptr = '\t';
      }
      state_add_prefix(state);
      state_puts(ptr,state);
      state_putc('\n',state);
    }
  }

  state_putc('\n',state);

  mime_decode (mt->parts, state);
  return;
}

static int ASCII_filter(c,st)
     char c;
     struct state * st;
{
  int res = (unsigned char) c;
  st++;

  if (res & 128)
    res = '_';
  return res;
}

static int run_decoder P_((mime_t *, state_t *)); /* Prototype */

static int run_decoder (ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  dprint(12,(debugfile,
	     "run_decoder -> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));

  if (ptr->encoding == ENCODING_BASE64)
    base64_decode (state, ptr->length, TRUE);
  else if (ptr->encoding == ENCODING_QUOTED)
    quoted_printable_decode (state, ptr->length, TRUE);
  else if (ptr->encoding != ENCODING_NONE && 
	   ptr->encoding != ENCODING_7BIT &&
	   ptr->encoding != ENCODING_8BIT &&
	   ptr->encoding != ENCODING_BINARY) {

    dprint(12,(debugfile,
	       "run_decoder=0 <- END; ftell=%ld\n",
	       ftell(state->fpin)));

    return 0;
  }
  else
    Xbit_decode (state, ptr->length);

  dprint(12,(debugfile,
	     "run_decoder=1 <- END; ftell=%ld\n",
	     ftell(state->fpin)));
  return 1;
}

static int set_filter	P_((mime_t *, state_t *)); /* Prototype */
static int set_filter (ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  char tmp[SLEN];
  char buf[STRING];
  int code;

  dprint(12,(debugfile,
	     "set_filter -> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));

  state -> filter = NULL_filter;

  dprint(12,(debugfile,
	     "set_filter: filter=%p (NULL_filter)",(void *)(state -> filter)));

  if (0 == (code = mime_get_charset (tmp, ptr->type_opts, sizeof(tmp)))) {
    
    /* Don't show this part (or copy to reply buffer) ... */
	  
    sprintf (buf, "[Charset %.15s unsupported, skipping...]\n", tmp);
    state_puts(buf,state);
    if (state->displaying)
      state_puts(   "[Use 'v' to view or save this part.]\n",state);

    dprint(12,(debugfile,
	       "set_filter=0 <- END\n"));
    return 0;
  } else {
    if (code < 0) {
      sprintf (buf, "[Charset %.15s unsupported, filtering to ASCII...]\n",
	       tmp);
      state_puts(buf,state);
      if (state->displaying)	
	state_puts(   "[You can also use 'v' to view or save this part.]\n\n",
		   state);
      
      state -> filter = ASCII_filter;

      dprint(12,(debugfile,
		 "set_filter: filter=%p (ASCII_filter)",
		 (void *)(state -> filter)));
    }
    dprint(12,(debugfile,
	       "set_filter=1 <- END\n"));
    return 1;
  }
}

static void text_decode	P_((mime_t *, state_t *)); /* Prototype */

static void text_decode (ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  dprint(12,(debugfile,
	     "text_decode -> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));

  if (set_filter(ptr,state)) {
    if (!run_decoder(ptr,state)) {
      state_puts("[Unsupported encoding, skipping...]\n",state);
      if (state->displaying)	
	state_puts("[Use 'v' to save this part in encoded form.]\n",state);
    }
  }

  state -> filter = NULL_filter;

  dprint(12,(debugfile,
	     "text_decode <- END; ftell=%ld\n",
	     ftell(state->fpin)));

}

static void elm_decode	P_((mime_t *, state_t *)); /* Prototype */

static void elm_decode (ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  state_t newstate,newstate2;
  
  char fname[STRING];
  FILE *tmpfp;
  char buffer[LONG_STRING];
  
  state_clear(&newstate);
  state_clear(&newstate2);
  
  dprint(12,(debugfile,
	     "elm_decode -> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));
  
  sprintf (fname, "%selmdecode.%d", temp_dir, getpid ());
  if (NULL == (tmpfp = safeopen_rdwr(fname))) {
    error("Failed to create file for decoding.");
    state_puts("[Failed to create file for decoding.]\n",state);
  } else { /* Tempfile opened */
    int crypted = OFF;
    int bytes = 0;
    unlink(fname);  /* We can unlink it now ... */
    
    newstate.displaying = state->displaying;
    
    newstate.fpin       = state->fpin;
    newstate.inbuf      = state->inbuf;
    newstate.inreadp    = state->inreadp;   /* Danger of dangling pointers!! */
    
    newstate.fpout      = tmpfp;
    newstate.prefix     = NULL;             /* No prefix in that pass */
    newstate.filter     = state->filter;
    
    if (run_decoder(ptr,&newstate)) {
      if (EOF == fflush(tmpfp)) {
	error("Error when flushing temporary file.");
	state_puts("[Error when flushing temporary file.]\n",state);
      }
      rewind(tmpfp); /* Rewind it for reading */
      
      newstate2.displaying = state->displaying;
      newstate2.prefix     = state->prefix;
      newstate2.fpin       = tmpfp;
      
      newstate2.fpout      = state->fpout;
      newstate2.outbuf     = state->outbuf;
      newstate2.outbufsize = state->outbufsize;
      newstate2.outwritep  = state->outwritep;   /* Must copy back! */
      
      if (set_filter(ptr,&newstate2)) {
	int count = 0;
	getkey(OFF);
	
	while(state_gets(buffer,sizeof(buffer),&newstate2)) {
	  count += strlen(buffer);
	  
	  if (!strncmp(buffer, START_ENCODE, strlen(START_ENCODE))) {
	    state_puts("-- Start of (Elm) encoded section.\n",&newstate2);
	    crypted = ON;
	    continue;
	  } else if (!strncmp(buffer, END_ENCODE, strlen(END_ENCODE))) {
	    crypted = OFF;
	    state_puts("-- End of (Elm) encoded section.\n",&newstate2);
	    continue;
	  } else if (crypted) {
	    no_ret(buffer);
	    encode(buffer);      
	    strcat(buffer, "\n");
	  }
	  state_add_prefix(&newstate2);
	  state_puts(buffer,&newstate2);
	}

	dprint(12,(debugfile,
		   "elm_decode: Readed %d bytes from temp file\n",count));
      }
      newstate2.fpout      = NULL;
      newstate2.fpin       = NULL;
      state->outwritep     = newstate2.outwritep; /* Copy back */
      newstate2.fpout      = NULL;
      newstate2.outbuf     = NULL;
      newstate2.outbufsize = NULL;
      newstate2.outwritep  = NULL;
	
    } else { /* Run decoder failed */
      state_puts("[Unsupported content-transfer-encoding, skipping...]\n",
		 state);
      if (state->displaying)	
	state_puts("[Use 'v' to save this part in encoded form.]\n",state);
    }

    /* Notice that buffer pointed by these objects was shared ...*/
    state->inbuf        = newstate.inbuf; 
    newstate.inbuf      = NULL;
    state->inreadp      = newstate.inreadp; 
    newstate.inreadp    = NULL;
    
    newstate.fpin       = NULL;
    newstate.fpout      = NULL;
 
    fclose(tmpfp);
  }

  dprint(12,(debugfile,
	     "elm_decode <- END; ftell=%ld\n",
	     ftell(state->fpin)));
  state_destroy(&newstate2);
  state_destroy(&newstate);
}

static void text_unsupported_decode P_((mime_t *, state_t *)); /* Prototype */

static void text_unsupported_decode (ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  char buf[STRING];

  dprint(12,(debugfile,
	     "text_unsupported_decode -> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));


  sprintf (buf, "[%s/%.30s is unsupported, treating like TEXT/PLAIN]\n\n", 
	   TYPE(ptr->type), ptr->subtype);
  state_puts (buf,state);
  text_decode (ptr,state);

  dprint(12,(debugfile,
	     "text_unsupported_decode <- END; ftell=%ld\n",
	     ftell(state->fpin)));
}

PUBLIC void null_decode (ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  char buf[STRING];


  dprint(12,(debugfile,
	     "null_decode <-> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));

  sprintf (buf, "[%.15s/%.30s is not supported, skipping...]\n",
	   TYPE(ptr->type),ptr->subtype);
  state_puts (buf,state);
  if (state->displaying)
    state_puts(   "[Use 'v' to view or save this part.]\n",state);
}

PUBLIC CT_decoder_t select_CT_decoder (ptr) 
     mime_t *ptr;
{
    if (ptr->type == MIME_TYPE_MULTIPART)
      return multipart_0_decode;
    else if (ptr->flags & MIME_RFC822)    
      return rfc822_decode;
    else if (ptr->type == MIME_TYPE_MESSAGE &&
	     (0 == istrcmp(ptr->subtype, "delivery-status") ||
	      0 == strincmp(ptr->subtype,"X-delivery-status-",18)))
      return text_decode;
    else if (ptr->type == MIME_TYPE_TEXT) {
      if (0 == istrcmp(ptr->subtype, "rfc822-headers") ||
	  0 == istrcmp(ptr->subtype, "plain"))
	return text_decode;
      return text_unsupported_decode; /* For that type we try use
					 metamail if available! */
    }
#ifdef USE_PGP
    else if (ptr->type == MIME_TYPE_APPLICATION &&
             (istrcmp(ptr->subtype, "pgp") == 0))
      return pgp_decode;
#endif
    else if (ptr->type == MIME_TYPE_APPLICATION &&
             (istrcmp(ptr->subtype, "X-ELM-encode") == 0))
      return elm_decode;

    return null_decode;
}


PUBLIC void mime_decode (ptr, state)
     mime_t *ptr;
     state_t *state;
{
  /* This routine calls the appropriate routine to decode the data
   * described in "ptr".
   */

  dprint(12,(debugfile,"mime_decode -> state: offset=%d, length=%d\n",
	    ptr -> offset, ptr -> length));

  if (ptr->disposition == DISP_INLINE) {
    CT_decoder_t decoder = select_CT_decoder(ptr);

    fseek (state->fpin, ptr->offset, 0);
    decoder (ptr, state);    
  }
  else { 
    if (state->displaying)      
      state_puts("[Attachment, skipping...  Use 'v' to view this part.]\n",
		 state);
    else
      state_puts("[Attachment, skipping...]\n",state);
  }

  dprint(12,(debugfile,"mime_decode <- END\n"));

}

static int rfc1522_decode_word (ptr, state)
     char *ptr;
     state_t *state;
{
  char *charset = NULL;
  char *encoded = NULL;
  char *buffer = NULL;
  char *c;
  int encoding = ENCODING_ILLEGAL;
  int count;
  int result = -1; /* Default to ERROR */
 
  count = 0;

  /* Make a copy so the original is not destroyed */
  c = buffer = strmcpy (buffer, ptr+2);

  while ((c = strtok (c, "?")) != NULL) {
    if (count == 0)
      /* get the charset */
      charset = c;
    else if (count == 1) {
      /* Check the encoding */
      if (toupper (*c) == 'Q')
	encoding = ENCODING_QUOTED;
      else if (toupper (*c) == 'B')
	encoding = ENCODING_BASE64;
    }
    else if (count == 2) 
      encoded = c;
    count++;
    c = NULL;
  }

  if (charset && encoding != ENCODING_ILLEGAL && encoded) {
    int code = class_charset(charset);
    result = 0; /* OK */

    dprint (1, (debugfile, 
		"15522_decode_word: OK: ptr=%s, code=%d\n",ptr,code));

    if (code != 0) {

      state->filter = NULL_filter;

      if (code < 0) {
	dprint (1,(debugfile, 
		   "15522_decode_word: unsupported charset: %s -- filtering\n",
		   charset));
	state->filter = ASCII_filter;
	state_puts("[",state);
      }


      if (encoding == ENCODING_QUOTED) { /* Hack */
	char * ptr;
	for (ptr = encoded; *ptr != '\0'; ptr++)
	  if (*ptr == '_')
	    *ptr = ' ';
      }

      state_buffer (encoded, state);
      
      if (encoding == ENCODING_QUOTED)
	quoted_printable_decode (state, strlen (encoded), TRUE);
      else if (encoding == ENCODING_BASE64)
	base64_decode (state, strlen (encoded), TRUE);
      if (code < 0) 
	state_puts("]",state);

      state->filter = NULL_filter;
    }
    else {
      dprint (1, (debugfile, 
		  "15522_decode_word: unsupported charset: %s -- skipping\n",
		  charset));
      state_puts("[?",state);
      state_puts(charset,state);
      state_puts("?]",state); 
    }
  }

  if (result == -1) {

    dprint (1, (debugfile, 
		"15522_decode_word: FAILED: ptr=%s\n",ptr));
    dprint (1, (debugfile, 
		"15522_decode_word: -> charset=%s, encoding=%d, encoded=%s\n",
		NONULL(charset),encoding,NONULL(encoded)));

  }

  free (buffer);
  buffer = NULL;

  return result;
}

static int rfc1522_decode_real (p, state)
     char *p;
     state_t *state;
{
  /* This routine decodes RFC1522 compliant headers */

  char *buffer = strmcpy(NULL,p);
  char *ptr = buffer;
  unsigned char last_char = 0;
  int result = 0;

  while (ptr && *ptr) {
    char *ptr2 = strpbrk(ptr," \t\r\n");
    unsigned char last_char2 = 0;
    int len;
    int is_encoded = 0;

    if (ptr2) {
      last_char2 = *ptr2;
      len = ptr2 - ptr;
      *ptr2 = '\0';
    } else {
      len = strlen(ptr);
    }

    /* Check if current word is valid encoded word */
    if (len > 9 && ptr[0] == '=' && ptr[1] == '?' &&
	ptr[len-1] == '=' && ptr[len-2] == '?') {
      char *ptr4;
      int count = 0;
      for (ptr4 = ptr; ptr4 - ptr < len; ptr4++)
	if (*ptr4 == '?')
	  count++;
      if (4 == count)
	is_encoded = 1;
    }

    dprint (3, (debugfile, 
		"15522_decode_real: word = %s, len=%d, is_encoded=%d\n",
		ptr,len,is_encoded));

    /* Space between encoded chars must delete! --
     * This allows splitting more than 75 character long
     * encoded words to two encoded word and using of
     * continuation lines!
     */
    if (!is_encoded && last_char) {
      state_putc (last_char, state);
    }

    if (is_encoded) {
      if (-1 == rfc1522_decode_word(ptr,state)) {
	dprint (3, (debugfile, 
		    "15522_decode_real: Error in rfc1522_decode_word ...\n"));
	result = -1;
	break;
      }
	
    } else {
      state_put(ptr,len,state);
    }

    if (!is_encoded && last_char2) {
      state_putc (last_char2, state);
      last_char2 = 0;
    }

    ptr = ptr2; /* Next word if any */
    if (ptr)
      ptr++;
    last_char = last_char2;
  }

  free(buffer);
  return 0;
}

PUBLIC int is_rfc1522 (s)
     char *s;
{
  /* Returns TRUE if the string contains RFC 1522 format encoded data */

  while ((s = strchr (s, '=')) != NULL) {
    s++;
    if (*s == '?')
      return TRUE;
  }
  return FALSE;
}

PUBLIC void rfc1522_decode (ptr, len)
     char *ptr;
     int len; /* size of "ptr" */
{
  /* Support for RFC 1522, which allows for encoding of non-ASCII characters
   * in RFC 822 headers.
   */
  char *tmp = safe_malloc (len);
  state_t state;

  /* Make a working copy of the header */
  strfcpy (tmp, ptr, len);

  /* The output from the decoding should be put back into the memory area
   * which was passed into this routine.
   */
  state_clear (&state);  
  state.outwritep = state.outbuf = ptr;
  state.outbufsize = len;

  if (rfc1522_decode_real (tmp, &state) == -1) {
    /* An error occured.  Restore the original value. */    

    strfcpy (ptr, tmp, len);
  }

  state_destroy (&state);  

  free (tmp);
}

PUBLIC void rfc1522_decode_structured (class, ptr, len)
     int class;
     char *ptr;
     int len; /* size of "ptr" */
{
  /* Decodes comments from structured fields 
   * if class & HDR_PHRASE decodes also prases (or all words outside of < >)
   *   -- not exactly correct for address lines, but otherwise we need full
   *      STD 11 (RFC 822) parser ....
   */
  char *walk, *walk2;
  char *result = safe_malloc(len);
  int tlen;
  int space = 0; /* Seen space or character after what there can begin
		    phrase (in address line) -- this alcorithm can result
		    incorrect output if addresses are not canonical for
		    -- for example 'some @ some . domain' */

  dprint (9, (debugfile, 
	      "rfc1522_decode_structured -> class=%d,len=%d,ptr=%s\n",
	      class,len,ptr));

  for (walk = ptr, walk2 = result; walk - ptr < len && *walk; walk += tlen) {
    int decoded = 0;
    int rlen;
    int c = *walk;

    tlen = rfc822_toklen(walk);
    strfcpy(walk2,walk,tlen+1);  /* Make initial working copy */

    /* Decode possible phrases */
    if ((class & HDR_PHRASE) && space && 
	NULL == strchr("()[]<>.:@,;\\\" \t\r\n",c) &&
	NULL == strchr("[].@,;",walk[tlen])) {
      char * walk3 = walk + tlen;

       while (walk3 < ptr + len -1 && *walk3 && 
	     NULL != strchr(" \t\r\n",*walk3)) {
	char *tmp = walk3;
	int nlen;

	walk3 += rfc822_toklen(walk3);

	nlen = rfc822_toklen(walk3);

 	if (!*walk3 || NULL != strchr("()[]<>.:@,;\\\" \t\r\n",*walk3)
	    || NULL != strchr("[].@,;",walk3[nlen])) {
	  walk3 = tmp; /* back before space */
	  break;
	}

	walk3 += nlen;
      }

      tlen = walk3 - walk;
      strfcpy(walk2,walk,tlen+1);  /* Make initial working copy */

      dprint (9, (debugfile, 
		  "rfc1522_decode_structured: Possible phrase '%s'\n",walk2));

      if (is_rfc1522(walk2)) {	
	rfc1522_decode(walk2,tlen+1);
	decoded = 1;
      } else {
	dprint (9, (debugfile, "rfc1522_decode_structured: '%s' NOT RFC1522\n",
		    walk2));
      }
    }

    space = 0;
    /* Decode comments */
    if (c == '(' && tlen > 9) {
      char *tmp = walk2 + 1;

      if (walk2[tlen-1] == ')')
	walk2[tlen-1] = 0;

      if (is_rfc1522(tmp)) {
	rfc1522_decode(tmp,tlen-1);
	decoded = 1;
      } else {
	dprint (9, (debugfile, "rfc1522_decode_structured: '%s' NOT RFC1522\n",
		    tmp));
      }

      strcat(tmp,")");

      space = 1;
    }

    /* Skip address */
    if (c == '<' && tlen == 1) {
      char *tmp = walk;
      int t1len = tlen;
      tlen = 0;

      while (tmp < ptr + len && *tmp) {
	t1len = rfc822_toklen(tmp);
	tlen += t1len;
	if (*tmp == '>' && t1len == 1)
	  break;
	tmp += t1len;
      }
      dprint (9, (debugfile, "rfc1522_decode_structured: Address '%.*s'\n",
		  tlen,walk));
    }
    if (c == ',' && tlen == 1)
	 space = 1;
	 
    if (strchr(" \t\n\r",c) != NULL)
	 space = 1;

    if (decoded) {
      rlen = strlen(walk2);
      dprint (9, (debugfile, "rfc1522_decode_structured: '%.*s' -> '%s'\n",
		  tlen,walk,walk2));
      walk2 += rlen;
    } else {
      strfcpy(walk2,walk,tlen+1);  /* Make working copy */
      dprint (9, (debugfile, "rfc1522_decode_structured: Skip '%s'\n",walk2));
      walk2 += tlen;
    }
  }

  strcpy(ptr,result);
  free(result);

  dprint (9, (debugfile, "rfc1522_decode_structured <- ptr=%s\n",ptr));
}

static int can_handle(att)     
     mime_t *att;
{
  while (att) {
    if (att->disposition == DISP_INLINE) {      
      if (mime_notplain(att)) {
	dprint(9,(debugfile,"can_handle=NO -- Failed: %s/%s\n",
		  mime_types[att->type], att->subtype));
	return FALSE;
      }
      dprint(9,(debugfile,  "can_handle:     Succeed: %s/%s\n",
		mime_types[att->type], att->subtype));
    } else {
      dprint(9,(debugfile,  "can_handle:  Attachment: %s/%s\n",
		mime_types[att->type], att->subtype));
    }
    att = att->next;
  }
  dprint(9,(debugfile,"can_handle=TRUE\n"));
  return TRUE;
}

PUBLIC int mime_notplain (p)
     mime_t *p;
{
  CT_decoder_t decoder = select_CT_decoder(p);
  char buf[STRING];
 
  if (p->encoding != ENCODING_BASE64 &&
      p->encoding != ENCODING_QUOTED &&
      p->encoding != ENCODING_NONE && 
      p->encoding != ENCODING_7BIT &&
      p->encoding != ENCODING_8BIT &&
      p->encoding != ENCODING_BINARY) {
    dprint(9,(debugfile,
	      "mime_notplain=TRUE -- type: %s/%s; encoding=%s (%d)\n",
	      mime_types[p->type], p->subtype,
	      ENCODING(p->encoding),p->encoding));
    return TRUE;      
  }

  if (decoder == rfc822_decode && p->parts && can_handle(p->parts)) {
    dprint(9,(debugfile,"mime_notplain=FALSE -- type: %s/%s\n",
	      mime_types[p->type], p->subtype));
    return FALSE;      
  }
  if (decoder == rfc822_decode) {
    dprint(9,(debugfile,"mime_notplain=TRUE -- type: %s/%s\n",
	      mime_types[p->type], p->subtype));
    return TRUE;
  }
 
  /* Notice: if (decoder == text_unsupported_decode)
   *         we return TRUE, because we want call metamail
   *         if it is available
   */
 
  if (decoder == text_decode) {
    /* If mime_get_charsets < 0, we want call metamail
     * (instead of filtering to US-ASCII)
     */
    if (mime_get_charset(buf,p->type_opts,sizeof(buf)) > 0) {
      dprint(9,(debugfile,"mime_notplain=FALSE -- type: %s/%s; charset=%s\n",
		mime_types[p->type], p->subtype,buf));
      return FALSE;
    }
  }
  else if (p->type == MIME_TYPE_MULTIPART && 
	   (0 == strincmp(p->subtype,"mixed",STRING) || 
	    0 == strincmp(p->subtype,"digest",STRING) || 
	    0 == strincmp(p->subtype,"report",STRING)) &&
	    p->parts && can_handle(p->parts)) {
    dprint(9,(debugfile,"mime_notplain=FALSE -- type: %s/%s\n",
	      mime_types[p->type], p->subtype));
    return FALSE;	   
  }
  else if (p->type == MIME_TYPE_MULTIPART && pagemultipart) {
    dprint(9,(debugfile,"mime_notplain=TRUE -- type: %s/%s\n",
	      mime_types[p->type], p->subtype));
    return FALSE;
  }
#ifdef USE_PGP
  else if (decoder == pgp_decode) {
    dprint(9,(debugfile,"mime_notplain=FALSE -- type: %s/%s\n",
	      mime_types[p->type], p->subtype));
    return FALSE;
  }
#endif
  dprint(9,(debugfile,"mime_notplain=TRUE -- type: %s/%s\n",
	    mime_types[p->type], p->subtype));
  return TRUE;
}

#endif /* MIME */
