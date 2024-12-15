#include "headers.h"
#include "me.h"

/* Removes comments from string */
PUBLIC void rfc822_reap_comments (ptr, comments, size) 
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
    if (comment_level == 0 && (in_quote || *ptr != ')'))
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


PUBLIC int read_header_line (fp, buf, size,flag) 
     FILE *fp;
     char *buf;
     int size;
     int flag;
{
  /* Read and unfold header line -- stores maximum size-1 bytes to buffer
   * (plus \0). Also handle case when headers are eneded either CR LF or LF.
   * Returns number of bytes stored. Always _read_ end of header
   * (even when buffer fills). Returns 0 when reads empty line (only CR LF).
   * That indicates end of headers.
   *
   * If flag & 1 then folding is marked with '\n' (instead of ' ')
   * And buffer ended with '\n' before '\0'
   * If flag & 2 then check that this was header line...
   */

  int len = 0,c;
  int col_seen = 0;
  long pos = ftell(fp);

  dprint(12,(debugfile,"read_header_line: size=%d, flag=%d\n",size,flag));
  size--; /* Place for \0 */

  if (pos < 0 && (flag & 2)) {
    dprint(12,(debugfile,"read_header_line=0; ftell failed!"));
    buf[0] = '\0';
    return 0;
  }
#define PUTC(c) { if (len < size) buf[len++] = (c); }
  
  while (EOF != (c = fgetc(fp))) {
    if ('\r' == c) {                /* Is this CR LF sequence ? */
      if (EOF == (c = fgetc(fp)))
	break;
      if (c != '\n') 
	PUTC('\r');
    }
    if (c == '\n') {                /* Readed CR LF or LF, check folding */
      if (!col_seen && len > 0 && (flag & 2)) {
	dprint(12,(debugfile,
		   "read_header_line: Not ':' seen. Not a header line!\n"));
	if (0 != fseek(fp,pos,SEEK_SET)) {
	  dprint(5,(debugfile,
		    "read_header_line: seek failed!\n"));
	}
	len = 0;
      }
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
      if (flag & 1)
	c = '\n';
    }
    if (':' == c)
      col_seen = 1;
    PUTC(c);
  }

  if (flag & 1) {
    PUTC('\n');
  }

#undef PUTC
  buf[len] = 0;

  dprint(12,(debugfile,"read_header_line: len=%d, buf=%s\n",len,buf));

  return len;
}

static struct header_info {
  char * header;
  int flag;
} header_types[] = {
  /* From STD 11 (RFC 822): */
  { "Subject",    HDR_TEXT },
  { "Comments",   HDR_TEXT },
  { "Return-path",HDR_STRUCTURED },
  { "Received",   HDR_STRUCTURED },
  { "Reply-To",   HDR_STRUCTURED|HDR_PHRASE },    /* Not exactly correct .. */
  { "From",       HDR_STRUCTURED|HDR_PHRASE },    /* Not exactly correct .. */
  { "Sender",     HDR_STRUCTURED|HDR_PHRASE },    /* Not exactly correct .. */
  { "Resent-Reply-To", HDR_STRUCTURED|HDR_PHRASE },/* Not exactly correct .. */
  { "Resent-From",     HDR_STRUCTURED|HDR_PHRASE },/* Not exactly correct .. */
  { "Resent-Sender",   HDR_STRUCTURED|HDR_PHRASE },/* Not exactly correct .. */
  { "Date",        HDR_STRUCTURED },
  { "Resent-Date", HDR_STRUCTURED },
  { "To",          HDR_STRUCTURED|HDR_PHRASE }, /* Not exactly correct .. */
  { "Resent-To",   HDR_STRUCTURED|HDR_PHRASE }, /* Not exactly correct .. */
  { "cc",          HDR_STRUCTURED|HDR_PHRASE }, /* Not exactly correct .. */
  { "Resent-cc",   HDR_STRUCTURED|HDR_PHRASE }, /* Not exactly correct .. */
  { "bcc",         HDR_STRUCTURED|HDR_PHRASE }, /* Not exactly correct .. */
  { "Resent-bcc",  HDR_STRUCTURED|HDR_PHRASE }, /* Not exactly correct .. */
  { "Message-ID",  HDR_STRUCTURED },
  { "Resent-Message-ID",  HDR_STRUCTURED },
  { "In-Reply-To", HDR_STRUCTURED|HDR_PHRASE }, /* Is correct .. */
  { "References",  HDR_STRUCTURED|HDR_PHRASE }, /* Is correct .. */
  { "Keywords",  HDR_STRUCTURED|HDR_PHRASE },   /* Is correct :-) .. */
  { "Encrypted", HDR_STRUCTURED },              /* Well ... */
  /* From MIME (RFC 1521) */
  { "MIME-Version",              HDR_STRUCTURED },     
  { "Content-Type",              HDR_STRUCTURED },     
  { "Content-Transfer-Encoding", HDR_STRUCTURED },
  { "Content-ID",                HDR_STRUCTURED },     
  { "Content-Description",       HDR_TEXT },
  /* From RFC 1806 */
  { "Content-Disposition",       HDR_STRUCTURED },
  /* From RFC 1864 */
  { "Content-MD5",               0 },

  /* Tailer */
  { NULL, 0 }
};

PUBLIC int classify_header(name)
     char *name;
{
  int i;

  for (i = 0; header_types[i].header; i++) 
    if (strincmp(name,header_types[i].header,STRING) == 0)
      return header_types[i].flag;
  
  if (strincmp(name,"X-",2) == 0)
    return HDR_TEXT;              /* Default for user defined headers */

  return 0;
}
