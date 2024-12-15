#include "headers.h"
#include "me.h"

extern int errno;
extern int tabspacing;

static void
builtin_help () {
  /* A help screen for the pager below. */

  ClearScreen ();
  StartInverse();
  Write_to_screen ("Help for builtin++", 0);
  EndInverse();
  NewLine ();
  NewLine ();
  Write_to_screen ("Key\t\tAction", 0);
  NewLine ();
  Write_to_screen ("---\t\t------", 0);
  NewLine ();
  Write_to_screen ("<SPACE>, +\tNext page.", 0);
  NewLine ();
  Write_to_screen ("-\t\tPrevious page.", 0);
  NewLine ();
  Write_to_screen ("<RETURN>\tNext line.", 0);
  NewLine ();
  Write_to_screen ("q, x, i\t\tReturn to the index menu.", 0);
  NewLine ();
  Write_to_screen ("/\t\tSearch for pattern in message.", 0);
  NewLine ();
  Write_to_screen ("^\t\tFirst page.", 0);
  NewLine ();
  Write_to_screen ("G\t\tLast page", 0);
  NewLine ();
  Write_to_screen ("^L\t\tRefresh display.", 0);
  NewLine ();
  Write_to_screen ("^P\t\tUp one line.", 0);
  NewLine ();
  Write_to_screen ("^D\t\tDown one-half page.", 0);
  NewLine ();
  Write_to_screen ("^U\t\tUp one-half page.", 0);
  NewLine ();
  Write_to_screen ("?\t\tThis help screen.", 0);
  NewLine ();
  PutLine0 (LINES, 0, "Press any key to return...");
  ReadCh ();
  return;
}

int
builtinplusplus (fp, begin, length, text, textlen)
     FILE *fp;
     long begin;
     int length, textlen;
     char **text;
{
  char buffer[LONG_STRING], searchword[STRING], *pending = NULL;
  long **offsets = NULL;
  int offset_len = 0, offset_max = 0;
  int lines = 0, saveidx, ch, len, chars;
  int slurping = FALSE; /* used to find the end of the file */
  int searching = FALSE;
  long end_offset = begin + length;
  int idx = -textlen, cur_line_len;
  int is_end = 0;

  if (fseek (fp, begin, 0) == -1) {
    dprint (1, (debugfile, "pager2(): fseek() returned errno %d!\n", errno));
    error1 ("Failed to seek %d bytes into file!", begin);
    if (sleepmsg > 0)
      sleep (sleepmsg);
    return (0);
  }

  clear_error();
  ClearScreen();

  for (;;) {
    while (((ftell (fp) < end_offset) || (idx < 0)) &&
	   (lines < LINES || searching || slurping)) {

      if (pending == NULL) {
	/* There is no pending data to output, so we need to grab another
	 * line.
	 */
	if (idx < 0) {
	  /* Negative index numbers refer to the text block passed to pager2.
	   * this is necessary for doings things like adding a title and
	   * weeding unwanted headers.
	   */
	  pending = text[textlen+idx];
	  idx++;
	}
	else {
	  if (offset_len >= offset_max - 1)
	    offsets = (long **) DynamicArray (offsets, sizeof (long *), &offset_max, LINES + 1);
	  if (idx == offset_len) {
	    if (! offsets[idx]) {
	      offsets[idx] = (long *) safe_malloc (sizeof (long));
	      offset_len++;
	    }
	    *offsets[idx] = ftell (fp);
	  }
	
	  if ((cur_line_len= mail_gets (buffer, LONG_STRING, fp)) <= 0) {
	    if (slurping) {
	      slurping = FALSE;
	      idx -= LINES;
	      if (idx < -textlen)
		idx = -textlen;
	      fseek (fp, *offsets[idx > -1 ? idx : 0], 0);
	      continue;
	    }
	    if (searching) {
	      searching = FALSE;
	      ClearLine (LINES);
	      PutLine0 (LINES, 0, "NOT FOUND!");
	      if (sleepmsg > 0)
		sleep (sleepmsg);	 
	      ClearLine (LINES);
	      /* Reposition the file at the appropriate place */
	      idx = saveidx;
	      fseek (fp, *offsets[idx > 0 ? idx : 0], 0);
	    }
	    break;
	  }
	  
	  idx++;

	  if (slurping)
	    continue;
	  
	  if (searching) {
	    if (in_string (buffer, searchword)) {
	      searching = FALSE;
	      idx -= 3;
	      if (idx < -textlen)
		idx = -textlen;
	      if (idx >= 0) fseek (fp, *offsets[idx], 0);
	      else fseek (fp, begin, 0);
	      ClearScreen ();
	      lines = 0;
	    }
	    continue;
	  }

	  if (cur_line_len > 0 && buffer[cur_line_len-1] == '\n') {
	    buffer[cur_line_len-1] = '\0';
	    if (cur_line_len > 1 && buffer[cur_line_len-2] == '\r') 
	      buffer[cur_line_len-2] = '\0';
	  }

	  pending = buffer;
	}
      }

      /* This is the part of the code that actually displays on the screen */
      chars = 0;
      while (*pending) {
	if (chars >= COLUMNS)
	  break;

	if (*pending == '\t') {
	  Writechar('\t');
	  /* 'chars += tabspacing' is INCORRENT! - K E H */ 
	  chars = ((chars / tabspacing ) +1) * tabspacing; 
	}
#ifdef ASCII_CTYPE
        else if (!isascii((unsigned char)*pending)) {
          Writechar('?');
          chars++;
        }
#endif
	else if (!isprint((unsigned char)*pending)) {
	  if (*pending >= 0 && *pending <= 31) {    
            /* This won't fit on the line, so just skip it for now */
            if (chars == COLUMNS - 1)
              break;

            Writechar('^');
            Writechar(*pending + 64);
            chars += 2;

            /* Honor the formfeed character */
            if (*pending == ctrl('L')) {
              break;
            }
          }
          else {
            Writechar('?');
            chars++;
          }
	}
	else {
	  Writechar(*pending);
	  chars++;
	}
	pending++;
      }
      NewLine();
      lines++;

      if (*pending == ctrl('L')) {
        pending++;
        if (*pending == '\0')
          pending = NULL;
        break;
      }

      /* Check to see if we are finished with this line */
      if (*pending == '\0')
	pending = NULL;
    }
    if (slurping){
      slurping = FALSE;
      idx -= LINES;
      if (idx < -textlen)
        idx = -textlen;
      if (idx >= 0) fseek (fp, *offsets[idx], 0);
      else fseek (fp, begin, 0);
      continue;
    }

    StartBold();
    if (idx < 0) {
      Write_to_screen ("More (you've seen 0%)", 0);
      len = 0;
      is_end = 0;
    }
    else {
      if (length < 1)
        len = 100;
      else {
        long pos = ftell (fp);

        len = 100 * (pos - begin) / length;
      }

      if (len >= 100) {
        Write_to_screen ("Command ('i' to return to index):", 0);
        is_end = 1;
      }
      else {
        sprintf (buffer, "MORE (you've seen %d%%):", len);
        Write_to_screen (buffer, 0);
        is_end = 0;
      }
    }
    EndBold();

    ch = ReadCh ();
    switch (ch) {
    case ' ':
    case '+':
      if (is_end) {
	DestroyDynamicArray (offsets);
	return (' ');
      }
      lines = 0;
      ClearScreen ();
      break;
    case '^':      
      idx = -textlen;
      fseek (fp, begin, 0);
      lines = 0;
      ClearScreen ();
      break;
    case 'G':
      idx = offset_len-1;
      if (idx >= 0) fseek (fp, *offsets[idx], 0);
      else fseek (fp, begin, 0);
      slurping = TRUE;
      lines = 0;
      ClearScreen ();
      break;
    case '/':
      ClearLine (LINES);
      PutLine0 (LINES, 0, "Search: ");
      searchword[0] = '\0';
      optionally_enter (searchword, LINES, 8, FALSE, FALSE);
      if (searchword[0])
	searching = TRUE;
      saveidx = idx;
      break;	
    case '-':
      idx -= LINES * 2;
      if (idx < -textlen)
	idx = -textlen;
      if (idx >= 0) fseek (fp, *offsets[idx], 0);
      else fseek (fp, begin, 0);
      ClearScreen ();
      lines = 0;
      break;
    case ctrl('P'):
      idx -= LINES + 1;
      if (idx < -textlen)
	idx = -textlen;
      if (idx >= 0) fseek (fp, *offsets[idx], 0);
      else fseek (fp, begin, 0);
      lines = 0;
      ClearScreen();
      break;
    case ctrl('D'):
      lines = LINES / 2;
      ClearScreen ();
      break;
    case ctrl('U'):
      idx -= LINES + (LINES / 2);
      if (idx < -textlen)
	idx = -textlen;
      if (idx >= 0) fseek (fp, *offsets[idx], 0);
      else fseek (fp, begin, 0);
      lines = 0;
      ClearScreen ();
      break;      
    case '?':
      builtin_help ();
    case ctrl('L'):
      idx -= LINES;
      if (idx < -textlen)
	idx = -textlen;
      if (idx >= 0) fseek (fp, *offsets[idx], 0);
      else fseek (fp, begin, 0);
      ClearScreen ();
      lines = 0;
      break;
    case '\n':
      {
	int x, y;

	GetXYLocation (&x, &y);
	ClearLine (x);
      }
      lines = LINES - 1;
      break;
    default:
      DestroyDynamicArray (offsets);
      return (ch == 'q' || ch == 'x' ? 0 : ch);
    }
  }
}

