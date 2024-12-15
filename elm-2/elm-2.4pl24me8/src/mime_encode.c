#include "headers.h"
#include "s_elm.h"
#include "me.h"

#ifdef MIME
#include <sys/time.h>

extern short mime_count;

static char base64chars[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b',
	'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};
#define to64(c) (((c) >= 0) && ((c) < 64)) ? base64chars[(c)] : -1

static char hexchars[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
	'E', 'F',
};
#define hexchar(c) (((c) >= 0) && ((c) < 16)) ? hexchars[(c)] : -1

int
update_encoding(top_encoding,encoding)
int *top_encoding;
int encoding;
{
  if (encoding == ENCODING_8BIT &&
      (*top_encoding) != ENCODING_BINARY)
    (*top_encoding) = ENCODING_8BIT;
  if (encoding == ENCODING_BINARY)
    (*top_encoding) = ENCODING_BINARY;
  
  return (*top_encoding);
}

void
print_EOLN(fp,top_encoding) 
     FILE *fp;
     int top_encoding;
{  /* Write canoniocal end of line of we are sending binary mail */
  if (top_encoding == ENCODING_BINARY)
    fputc('\r',fp);
  fputc('\n',fp);
}

char *
mime_generate_boundary (str)
     char *str;
{
  time_t t = time (NULL);
	
  sprintf (str, "ELM%d-%d-%d_", t, getpid(), mime_count++);
  return str;
}

void 
add_parameter(opts,name,value,size,quoted) 
     char *opts, *name, *value;
     int size, quoted;
{
  int len = strlen(opts);
  int ln = strlen(name);
  char * ptr = opts + len, *c;
  int need_quotation = 0;
  
  /* Following characters require quotation: ( ) < > @ , ; : \ " / [ ] ? =
   * Also whitespace requires quotation. See Mime Draft Standard
   */
  if (!quoted && (NULL != strpbrk(value,"()<>@,;:\\\"/[]?= \t")
		  || value[0] == '\0'))
    need_quotation = 1;

  if (len + strlen(value) + ln + 4 + 2 * need_quotation > size)
    return; /* Don't fit anyway */

  if (ptr != opts) {
    *ptr++ = ';'; *ptr++ = ' ';
  }
  strcpy(ptr,name);  ptr += ln;
  *ptr++ = '=';

  if (need_quotation) 
    *ptr++ = '"';

  for (c = value; *c != '\0'; c++) {
    if (need_quotation && ('\\' == *c || '"' == *c)) {
      if (ptr < opts + size - 3) {
	*ptr++ = '\\';
	*ptr++ = *c;
      } else
	break;
    } else {
      if (ptr < opts + size - need_quotation - 1) 
	*ptr++ = *c;
      else
	break;
    }
  }

  if (need_quotation) 
    *ptr++ = '"';
  *ptr = '\0';
  return;
}

void write_encoded();

void
attach_generate_message (gopher, fpout, copy, mime_info)
     mime_t *gopher;
     FILE *fpout;
     int copy;
     mime_send_t *mime_info;
{
/* given a list of attachments, write them in MIME format to the specified
 * output stream */

  FILE *srcfp;
  int is_text;

  while (gopher) {
    if (can_open(gopher->pathname,"r") != 0) {
      gopher = gopher -> next;
      continue;
    }
    srcfp = fopen (gopher->pathname, "r");

    if (srcfp) {

        /* 1 if is text type (true)
         * 0 if not text type
         * -1 if can't be encoded (ie structured) Message/ or Multpart/
         */
        is_text = is_text_type (mime_types[gopher->type], gopher->subtype, 
				gopher->encoding);

        /* write the attachment header */
	print_EOLN(fpout,mime_info->encoding_top);
        fprintf (fpout, "--%s", mime_info->mime_boundary);
	print_EOLN(fpout,mime_info->encoding_top);
        fprintf (fpout, "Content-Type: %s/%s", mime_types[gopher->type], gopher->subtype);
        if (gopher->type_opts) {
          fputc (';', fpout);
          if (strlen (gopher->type_opts) > 45) {
            print_EOLN (fpout, mime_info->encoding_top);
            fputc ('\t', fpout);
          }
          else
            fputc (' ', fpout);
          fputs (gopher->type_opts, fpout);
	}
        print_EOLN (fpout, mime_info->encoding_top);

        /* Now add the Content-Disposition header */
	fprintf (fpout, "Content-Disposition: %s",
		 DISPOSITION(gopher->disposition));
        if (gopher->disposition_opts) {
          fputc (';', fpout);
          if (strlen (gopher->disposition_opts) > 45) {
            print_EOLN (fpout, mime_info->encoding_top);
            fputc ('\t', fpout);
          }
          else
            fputc (' ', fpout);
          fputs (gopher->disposition_opts, fpout);
        }
        print_EOLN (fpout, mime_info->encoding_top);

	fprintf (fpout, "Content-Description: ");
        if (gopher->description)
          fputs (gopher->description, fpout);
        else
          fputs (gopher->pathname, fpout);
	print_EOLN(fpout,mime_info->encoding_top);
	fprintf (fpout, "Content-Transfer-Encoding: %s",
                 ENCODING(gopher->encoding));
	print_EOLN(fpout,mime_info->encoding_top);
	print_EOLN(fpout,mime_info->encoding_top);

	(void) write_encoded (srcfp, fpout, gopher->encoding, copy, is_text,
			      mime_info);

	fclose (srcfp);
    }
    else {
      error1 ("Error opening %s!", gopher->pathname);
      dprint (1, (debugfile, "mime_gen_msg(): could not open %s\n", gopher->pathname));
    }

    gopher = gopher->next;
  }
  return;
}

void
base64_encode (srcfp, fpout, istext, mime_info)
     FILE *srcfp, *fpout;
     int istext;
     mime_send_t * mime_info;
{
  int c1, c2, c3;
  char ch1, ch2, ch3, ch4;
  int chars = 0;
  int last_c = 0;

  for (;;) {
    c1 = fgetc (srcfp);
    if (c1 == -1)
      break;
    if (istext && last_c != '\r' && c1 == '\n') {
      /* In text end of line must be coded as CR LF */
      c1 = '\r';
      c2 = '\n';
    }
    else
      c2 = fgetc (srcfp);

    if (istext && c1 != '\r' && c2 == '\n') {
      /* In text end of line must be coded as CR LF */
      c2 = '\r';
      c3 = '\n';
    }
    else if (c2 != -1)
      c3 = fgetc (srcfp);
 
    if (istext && c2 != '\r' && c3 == '\n') {
      /* In text end of line must be coded as CR LF */
      ungetc(c3,srcfp);
      c3 = '\r';
    }
      
    last_c = c3;
 
    ch1 = c1 >> 2;
    ch1 = to64(ch1);
    
    if (c2 != -1) {
      ch2 = ((c1 & 0x3) << 4) | (c2 >> 4);
      ch2 = to64(ch2);
      
      if (c3 != -1) {
	ch3 = ((c2 & 0xf) << 2) | (c3 >> 6);
	ch3 = to64(ch3);
	ch4 = c3 & 0x3f;
	ch4 = to64(ch4);
      }
      else {
	ch3 = (c2 & 0xf) << 2;
	ch3 = to64(ch3);
	ch4 = '=';
      }
    }
    else {
      ch2 = (c1 & 0x3) << 4;
      ch2 = to64(ch2);
      ch3 = '=';
      ch4 = '=';
    }
    
    fputc (ch1, fpout);
    fputc (ch2, fpout);
    fputc (ch3, fpout);
    fputc (ch4, fpout);
    chars += 4;
    
    if (chars >= 76) {
      print_EOLN(fpout,mime_info->encoding_top);
      chars = 0;
    }	
  }
  print_EOLN(fpout,mime_info->encoding_top);
  return;
}

void
line_quoted_printable_encode (input,fpout,copy,len,istext,mime_info) 
     char *input;
     FILE *fpout;
     int copy; /* whether or not this is a copy of the message */
     int len; /* length of data -- be binary clean */
     int istext; /* if binary (not text) also CRLF need to be encoded */
     mime_send_t *mime_info;
{
  int chars = 0;
  char buffer[STRING];
  unsigned char c1, c2, c3;
  unsigned char lastchar = 0;
  unsigned char lastchar2 = 0;
  
  dprint (10, (debugfile, 
	       "line_quoted_printable_encode: copy=%d, len=%d, istext=%d\n",
	       copy,len,istext));

  if (istext) {  
    if (len > 0 && input[len-1] == '\n') {
      lastchar = input[len-1];
      input[len-1] = '\0';
      len--;
      if (len > 0 && input[len-1] == '\r')  {   /* Was CR LF */
        lastchar2 = input[len-1];
        input[len-1] = '\0';
        len--;
      } else if (mime_info->encoding_top == ENCODING_BINARY)
	lastchar2 = '\r';     /* Simulate it */
    }
  }

  /* I don't test agaist macros bacause these encodings are recommended
   * according MIME Draft Standard anyway and MIME encodings are
   * reversible.
   *
   * DONT_ESCAPE_MESSAGES refers '>' escaping -- not reversible
   * MIME encoding -- '>' escaping was not reversible -- this is.
   *
   * We also want do these encodings when sending (copy == 0)
   * not only when copying to another folder 
   *            -- K E H <hurtta@dionysos.FMI.FI>                     */

  while (len > 0)  {
    /* Assume that buffer don't have newlines (or they have binary data) ...
       this routine encodes one line */
    c1 = (unsigned char) *input++;
    len--;

    if (c1 == '=') {
      if (chars > 72) {	    
	buffer[chars++] = '=';
	buffer[chars++] = '\0';
	chars = 0;

	fputs (buffer, fpout);
	print_EOLN(fpout,mime_info->encoding_top);
      }
      buffer[chars++] = '=';
      buffer[chars++] = '3';
      buffer[chars++] = 'D';
    }    
    /*  printable characters -- EXCEPT:   Encode "From " in beginning */
    else if ((c1 > 31 && c1 < 127) && ((c1 != ' ' || chars != 4 || 
					strncmp(buffer,"From",4) != 0)
				       /* Encode "." if only in line alone */
				       && (c1 != '.' || chars != 0 || len > 0)
				       /* Last space must encode also */
				       && (c1 != ' ' || len > 0)) 
	     ||
	     (c1 == 9 && len > 0 && istext)) { 
      /* Don't make sense wrap before last character, when last character
       * is not encoded -- wrapping and character use equal number of columns.
       * But left space for '=\n' if we will print it in end of function
       * instead of '\n'. */
      if (chars > 74 && (len > 0 || !lastchar)) {
	buffer[chars++] = '=';
	buffer[chars++] = '\0';
	chars = 0;

	fputs (buffer, fpout);
	print_EOLN(fpout,mime_info->encoding_top);
      }
      buffer[chars++] = c1;
    }
    else {
      if (chars > 72) {
	buffer[chars++] = '=';
	buffer[chars++] = '\0';
	chars = 0;

	fputs (buffer, fpout);
	print_EOLN(fpout,mime_info->encoding_top);
      }
      c2 = (c1 >> 4) & 0xf;
      c3 = c1 & 0xf;
      buffer[chars++] = '=';
      buffer[chars++] = hexchar(c2);
      buffer[chars++] = hexchar(c3);
    }
  }

  /* Make sure to flush the buffer.  */
  if (chars > 0) {
    buffer[chars] = '\0';
    fputs (buffer, fpout);
  }
  if (lastchar2)
    fputc (lastchar2, fpout);
  if (lastchar) {
    fputc(lastchar,fpout);
  } else { /* If input line don't terminate NL then print shoft wrap to end
	    * instead of hard NL */
    fputs ("=", fpout);
    print_EOLN(fpout,mime_info->encoding_top);
  }
}

void
quoted_printable_encode (srcfp, fpout, copy, istext, mime_info)
     FILE *srcfp, *fpout;
     int copy;    /* whether or not this is a copy of the message */
     int istext;  /* if binary (not text) also CRLF need to be encoded */
     mime_send_t *mime_info;
{
  char buffer[VERY_LONG_STRING];
  int len;

  if (istext) {
    /* mail_gets is in ../lib -- it handles also NUL characters */
    while ((len = mail_gets (buffer, VERY_LONG_STRING, srcfp)) > 0)
      line_quoted_printable_encode (buffer, fpout, copy, len, istext,
				    mime_info);
  } else {
    /* mail_gets may add LF to end of file if file don't end with LF
     * So it is not good for binary data */
    while ((len = fread(buffer, 1, sizeof(buffer), srcfp)) > 0)
      line_quoted_printable_encode (buffer, fpout, copy, len, istext,
				    mime_info);
  }

  return;
}

int
is_text_type (primary_type, subtype, enc)
     char *primary_type, *subtype;
     int enc;
{
 /* encoding rules are different for text and no text 
  * for text we must do \n -> \r\n before base64 encoding */

 /* Current MIME Draft Standard says that part is text
  * (ie. line oriented) when it is Text/ -type -- otherwise no.
  *
  * new draft for Mime standards allows also other parts ot be text.
  * It says that content-transfer-encodings: 7bit and 8bit are only
  * allowed for line orienteed types */

  /* Actually Message/ and Multipart/ can't be newer encoded directly */
  if ( istrcmp(primary_type,"Message") == 0 ||
       istrcmp(primary_type,"Multipart") == 0)
    return -1;

  if (istrcmp(primary_type,"Text") == 0) 
    return 1;               /* Subtypes of text are always text (ie.
                             * line oriented). */
    
  if (enc == ENCODING_NONE || enc == ENCODING_7BIT || enc == ENCODING_8BIT)
    return 1;             /* It is text */

  if (enc == ENCODING_BINARY)
    return 0;            /* It is probably binary (or very long lines) */

  if (istrcmp(primary_type,"Application") == 0 &&
      istrcmp(subtype,"Postscript") == 0)
    return 1;            /*  Postscript is often text */
  
  return 0;              /* Default: It is binary */
}

void
write_encoded (srcfp, fpout, encoding, copy, is_text, mime_info)
     FILE *srcfp, *fpout;
     int encoding, copy, is_text;
     mime_send_t *mime_info;
{
  char buffer[VERY_LONG_STRING];
  int line_len;

  if (encoding == ENCODING_BASE64)
    base64_encode (srcfp, fpout, is_text, mime_info);
  else if (encoding == ENCODING_QUOTED)
    quoted_printable_encode (srcfp, fpout, copy, is_text, mime_info);
  else if (mime_info-> encoding_top == ENCODING_BINARY && is_text > 0) {
    /* It is better perhaps use canonical eol (CRLF) when mail have
     * content transfer encoding BINARY somewhere (see notes about BINARYMIME)
     */
    while ((line_len = mail_gets(buffer, sizeof(buffer)-1, srcfp)) > 0) {
      if (buffer[line_len-1] == '\n') {
        int add = 1;
        if (line_len >1 && buffer[line_len-2] == '\r')
          add = 0;
        if (add) {
          buffer[line_len-1] = '\r';
          buffer[line_len] = '\n';
          line_len++;
        }
      }
      if (fwrite(buffer, 1, line_len, fpout) != line_len) {
        MoveCursor(LINES, 0);
        Raw(OFF);
        Write_to_screen("\nWrite failed in write_encoded\n", 0);
        emergency_exit();
      }
    }
  }
  else {
    /* BUG: This may add extra \n in end of file if file does not end with \n
     * but we can't read that with fread when we to need look for escaping of
     * "From " or lone "."
     */
    while (1) {
#ifndef DONT_ESCAPE_MESSAGES
      if (mime_info->encoding_top != ENCODING_BINARY) {
	if ((line_len = mail_gets(buffer, sizeof(buffer)-1, srcfp)) <= 0)
	  break;
      }
      else
#endif
	if ((line_len = fread(buffer, 1,sizeof(buffer)-1, srcfp)) <= 0)
	  break;
      
      if (mime_info-> encoding_top != ENCODING_BINARY) {
#ifndef DONT_ESCAPE_MESSAGES
	if (copy && (strncmp(buffer, "From ", 5) == 0)) {
	  /* Add in the > to a From on our copy */
	  fprintf(fpout, ">");
	  if (fwrite(buffer, 1, line_len, fpout) != line_len) {
	    MoveCursor(LINES, 0);
	    Raw(OFF);
	    Write_to_screen("\nWrite failed in write_encoded()\n", 0);
	    emergency_exit();
	  }
	}
#ifdef NEED_LONE_PERIOD_ESCAPE
	else if (!copy && strcmp(buffer, ".\n") == 0)
	  /* Because some mail transport agents take a lone period to
	   * mean EOF, we add a blank space on outbound message.
	   */
	  fputs(". \n", fpout);
#endif /* NEED_LONE_PERIOD_ESCAPE */
	else
#endif /* DONT_ESCAPE_MESSAGES */
	  if (fwrite(buffer, 1, line_len, fpout) != line_len) {
	    MoveCursor(LINES, 0);
	    Raw(OFF);
	    Write_to_screen("\nWrite failed in write_encoded()\n", 0);
	    emergency_exit();
	  }
      }
      else if (fwrite(buffer, 1, line_len, fpout) != line_len) {
	MoveCursor(LINES, 0);
	Raw(OFF);
	Write_to_screen("\nWrite failed in write_encoded()\n", 0);
	emergency_exit();
      }
    }
  }
  return;
}

void
mime_write_header(fp, ptr)
     FILE *fp;
     mime_send_t *ptr;
{
  fputs(MIME_HEADER, fp);
  print_EOLN(fp, ptr->encoding_top);  
  fprintf(fp, "%s %s/%s", MIME_CONTENTTYPE, TYPE(ptr->type), ptr->subtype);
  if (ptr->type_opts[0])
    fprintf(fp, "; %s", ptr->type_opts);
  print_EOLN(fp, ptr->encoding_top);

  if (ptr->encoding_top != ENCODING_NONE) {
    fprintf(fp, "%s %s", MIME_CONTENTENCOD, ENCODING(ptr->encoding_top));
    print_EOLN(fp, ptr->encoding_top);
  }
}
#endif /* MIME */
