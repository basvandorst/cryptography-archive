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

void
Xbit_decode (s, len)
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

void mime_decode ();

void
base64_decode (s, length, astext)
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

    if (fgets (buf, VERY_LONG_STRING, s->fpin) == NULL)
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

void
quoted_printable_decode (s, length, astext)
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

    if (fgets (buf, VERY_LONG_STRING, s->fpin) == NULL)
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

void
multipart_decode (att, state)
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

void
multipart_0_decode (ptr, state)
     mime_t *ptr;
     state_t *state;
{
  if (ptr->parts)
    multipart_decode (ptr->parts, state);
  else
    state_puts ("[Content-Type: multipart/*, internal error]\n", state);
}

void
rfc822_decode (mt, state)
     mime_t *mt;
     state_t *state;
{
  /* This looks a lot like multipart_decode() except that we want to print
   * the headers for the message.  NOTE: "mt" should be a pointer to the
   * RFC822 attachment, NOT its subpart, since we need the offset in order
   * to print out the headers.
   */

  int done = FALSE, matched = FALSE;
  char buf[VERY_LONG_STRING];
  
  fseek (state->fpin, mt->offset, 0);
  
  /** Print out the headers. **/
  while (! done) {
    int len;
    
    if (0 == (len = mail_gets (buf, VERY_LONG_STRING, state->fpin)))
      break;
    
    if (filter) {
      if (matches_weedlist (buf) || (whitespace (buf[0]) && matched)) {
        matched = TRUE;
        continue;
      }
      else
        matched = FALSE;
    }

    if (buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
      done = TRUE;

    state_puts(buf,state);
  }
  mime_decode (mt->parts, state);
  return;
}

void 
text_decode(ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  char buf[STRING];
  char tmp[SLEN];


  dprint(12,(debugfile,
	     "text_decode -> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));

  if (!mime_get_charset (tmp, ptr->type_opts, sizeof(tmp))) {
    
    /* Don't show this part (or copy to reply buffer) ... */
	  
    sprintf (buf, "[Charset: %.15s, skipping]\n", tmp);
    state_puts(buf,state);
  }
  else {
    if (ptr->encoding == ENCODING_BASE64)
      base64_decode (state, ptr->length, TRUE);
    else if (ptr->encoding == ENCODING_QUOTED)
      quoted_printable_decode (state, ptr->length, TRUE);
    else if (ptr->encoding != ENCODING_NONE && 
	     ptr->encoding != ENCODING_7BIT &&
	     ptr->encoding != ENCODING_8BIT &&
	     ptr->encoding != ENCODING_BINARY) {
	  
      state_puts("[Unsupported encoding, skipping]\n",state);
    }
    else
      Xbit_decode (state, ptr->length);
  }


  dprint(12,(debugfile,
	     "text_decode <- END; ftell=%ld\n",
	     ftell(state->fpin)));

}

void 
text_unsupported_decode(ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  char buf[STRING];

  dprint(12,(debugfile,
	     "text_unsupported_decode -> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));


  sprintf (buf, "[%s/%.30s is unsupported, treating like TEXT/PLAIN]\n\n", 
	   TYPE(ptr->type), ptr->subtype);
  state_puts(buf,state);
  text_decode(ptr,state);

  dprint(12,(debugfile,
	     "text_unsupported_decode <- END; ftell=%ld\n",
	     ftell(state->fpin)));
}

void 
null_decode(ptr, state) 
     mime_t *ptr;
     state_t *state;
{
  char buf[STRING];


  dprint(12,(debugfile,
	     "null_decode <-> state: offset=%d, length=%d; ftell=%ld\n",
	     ptr -> offset, ptr -> length, ftell(state->fpin)));

  sprintf (buf, "[%.15s/%.30s is not supported, skipping...]\n",
	   TYPE(ptr->type),ptr->subtype);
  state_puts(buf,state);
}

CT_decoder_t 
select_CT_decoder (ptr) 
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
      return mime_pgp_decode;
#endif

    return null_decode;
}


void
mime_decode (ptr, state)
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
  else
    state_puts("[Attachment, skipping...  Use 'v' to view this part.]\n",
               state);

  dprint(12,(debugfile,"mime_decode <- END\n"));

}
#endif /* MIME */
