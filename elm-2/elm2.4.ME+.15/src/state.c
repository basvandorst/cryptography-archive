#include "headers.h"
#include "me.h"

#ifdef MIME

PUBLIC int NULL_filter(c,st)
     int c;          /* unsigned char assumed */
     struct state * st;
{
  st++;
  return c;
}

PUBLIC void state_clear (s)
     state_t *s;
{
  s->fpin = s->fpout = NULL;
  s->inbuf = s->outbuf = s->inreadp = s->outwritep = s->prefix = NULL;
  s->outbufsize = 0;
  s->displaying = 0;
  s->filter = NULL_filter;
}

PUBLIC void state_buffer (c, s)
     char *c;
     state_t *s;
{
  s->inreadp = s->inbuf = strmcpy (s->inbuf, c);
}

PUBLIC void state_destroy (s)
     state_t *s;
{
  if (s->inbuf)
    free(s->inbuf);
  s->inbuf = NULL;

  state_clear(s);
}


PUBLIC int state_getc (s)
     state_t *s;
{
  if (s->inbuf) {
    char ret;

    if (*s->inreadp)
      ret = *s->inreadp++;
    if (! *s->inreadp) {
      /* No more data left in the buffer, so clean up... */
      s->inreadp = NULL;
      free (s->inbuf);
      s->inbuf = NULL;
    }
    return ret;
  }
  else if (s->fpin)
    return fgetc (s->fpin);
  else {
    dprint(1, (debugfile, "state_getc: inbuf==NULL and fpin==NULL!\n"));
    return EOF;
  }
}

PUBLIC char *state_gets (dest, length, state)
     char *dest;
     int length;
     state_t *state;
{
  int i = 0, ch;
  
  if (state->inbuf) {
    while (length > 1) {
      ch = state_getc (state);
      if (ch == EOF)
	break;
      else if (ch == '\n') {
	dest[i++] = '\n';
	break;
      }
      else
	dest[i++] = ch;
      length--;
    }
    dest[i] = '\0';
    if (i > 0)
      return dest;
    else
      return NULL;
  }
  else
    return (fgets (dest, length, state->fpin));
}

PUBLIC int state_putc (ch, state)
     int ch;                        /* unsigned char assumed */
     state_t *state;
{
  int res = state -> filter(ch,state);

  if (res < 0) /* Destroy char */
    return EOF;

  if (state->fpout)
    return (fputc (res, state->fpout));
  else {
    if (state->outwritep - state->outbuf < state->outbufsize-1) {
      *state->outwritep++ = res;
      *state->outwritep = '\0';
      return res;
    }
    else
      return EOF;
  }
}

PUBLIC int state_put (string, len, state)
     char *string;
     int len;
     state_t *state;
{
  if (state->fpout && state->filter == NULL_filter)
    return fwrite (string, 1, len, state->fpout);
  else {
    int count;
  
    for (count = 0; count < len; count++) {
      if (state_putc ((unsigned char) string[count], state) == EOF)
	return (count > 0 ? count : EOF);
    }
    return count;
  }
}

PUBLIC int state_puts (string, state)
     char *string;
     state_t *state;
{
  return state_put(string,strlen(string),state);
}

#endif
