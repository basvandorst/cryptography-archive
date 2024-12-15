/* $Id: menu2.c,v 1.1 1995/08/28 02:08:24 elkins Exp $
 *
 * This code originally written by Michael Elkins <elkins@aero.org>.
 *
 * $Log: menu2.c,v $
 * Revision 1.1  1995/08/28 02:08:24  elkins
 * Initial revision
 *
 */

#include "headers.h"
#include "menu2.h"

#define BOL 1
#define STR 2
#define INT 3

void
do_item (it)
     struct menu_item *it;
{
  short len;
  char ch, buf[STRING];

  ClearLine (LINES-2);
  Write_to_screen (it->option, 0);
  len = strlen (it->option) + 1;
  if (it->type == STR)
    optionally_enter (it->d.c, LINES-2, len, TRUE, FALSE);
  else if (it->type == BOL) {
    for (;;) {
      MoveCursor (LINES-2, len);
      CleartoEOLN ();
      if (*it->d.i == TRUE)
	Write_to_screen ("TRUE", 0);
      else if (*it->d.i == FALSE)
	Write_to_screen ("FALSE", 0);
      ch = ReadCh ();
      if (ch == '\n')
	return;
      else if (ch == ' ')
	*it->d.i = *it->d.i ? FALSE : TRUE;
    }
  }
  else if (it->type == INT) {
    sprintf (buf, "%d", *it->d.i);
    optionally_enter (buf, LINES-2, len, TRUE, FALSE);
    *it->d.i = atoi (buf);
  }
  return;
}

void
generic_menu (items, max, title, prompt)
     struct menu_item items[];
     short max;
     char *prompt, *title;
{
  short i = 0;
  char buf[STRING], buf2[STRING];
  char ch;
  short update = TRUE;

  ClearScreen ();
  for (;;) {
    if (update) {
      Centerline (1, title);
      for (i = 0 ; i < max ; i++) {
	sprintf (buf, "%-30.30s", items[i].option);
	if (items[i].type == BOL) {
	  if (*items[i].d.i == TRUE)
	    strcat (buf, "TRUE");
	  else
	    strcat (buf, "FALSE");
	}
	else if (items[i].type == STR)
	  strcat (buf, items[i].d.c);
	else if (items[i].type == INT) {
	  sprintf (buf2, "%d", *items[i].d.i);
	  strcat (buf, buf2);
	}
	ClearLine(items[i].offset);
	PutLine0 (items[i].offset, 0, buf);
      }
      update = FALSE;
    }
    ClearLine (LINES-2);
    PutLine0 (LINES-2, 0, prompt);
    ch = ReadCh ();
    switch (ch) {
    case '\n':
      return;
    default:
      for (i = 0; i < max ; i++) {
	if (ch == items[i].key) {
	  do_item (items[i]);
	  update = TRUE;
	  break;
	}
      }
    }
  }
  /* Not reached. */
}
