/* term.c - curses environment terminal functions for YTalk V2.0 */

/*			   NOTICE
 *
 * Copyright (c) 1990 Britt Yenne.  All rights reserved.
 * 
 * This software is provided AS-IS.  The author gives no warranty,
 * real or assumed, and takes no responsibility whatsoever for any 
 * use or misuse of this software, or any damage created by its use
 * or misuse.
 * 
 * This software may be freely copied and distributed provided that
 * no part of this NOTICE is deleted or edited in any manner.
 * 
 */

/* Mail comments or questions to yenne@ccwf.cc.utexas.edu */

/* A valid terminal interface to YTalk implements these functions:
 *
 * init_term()		Initialize the terminal interface and create
 *			the main console terminal for input.
 *
 * close_term()		Shut down the terminal system.
 *
 * add_window(title, f)	Add a new window, using the specified title.  This
 *			function should return an integer window descriptor
 *			which will remain constant for the life of this
 *			particular window.  All subsequent calls which
 *			process this window will be sent this window
 *			descriptor.  Program main.c assumes that the main
 *			window (input window) has a descriptor of zero.
 *
 * del_window(w)	Delete the window with descriptor w.
 *
 * redraw()		Redraw all windows.
 *
 * add_char(w, c)	Add the character c to window w.  Do NOT attempt
 *			to translate the character to anything except
 *			newlines and tabs.
 *
 * w_rub(w)		Destructively backspace one character in window w.
 * w_kill(w)		Erase the current line in window w.
 * w_word(w)		Erase one word in window w.
 * w_clr(w)		Clear window w.
 *
 * showerror(str, e)	Display a user-friendly error message by outputting
 *			the string str and displaying the system error
 *			number e, if e is not zero.
 *
 * showmenu()		Display the main menu, and input the various fields
 *			of the 'inptype' structure from the user.
 *
 * yes_no(str)		Display the string str, then read a yes/no response
 *			from the user and return 'y' or 'n'.
 */

#include "ytalk.h"
#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <curses.h>
#include <stdio.h>

#define TLEN	60

typedef struct {
    WINDOW *c_win;	/* curses window */
    char name[TLEN];	/* name of window */
} wind;

#define XPOS	c_win->_curx
#define YPOS	c_win->_cury
#define XMAX	c_win->_maxx
#define YMAX	c_win->_maxy

wind win[MAXC];		/* curses window array */
int wp[MAXC];		/* translates user window ID to array above */
int scr = -1;		/* number of active screens */
extern char edit[4];	/* for word wrap */
extern inptype inp;

/* Initialize terminal
 */
int init_term()
{
    int i;

    if(scr >= 0)
	close_term();
    for(i = 0; i < MAXC; i++)
	wp[i] = -1;
    initscr();
    noraw();
    cmode();
    noecho();
    scr = 0;
    if(add_window("YTalk 2.1", NULL, 0) < 0)
    {
	close_term();
	return -1;
    }
    return 0;
}

/* Shut down terminal
 */
close_term()
{
    if(scr == -1)
	return;
    clear();
    refresh();
    endwin();
    scr = -1;
}

shell_mode()
{
    endwin();
    system("clear");
    fflush(stderr);
}

curses_mode()
{
    noraw();
    cmode();
    noecho();
    redraw();
}

/* resize_win is an internal routine to resize a curses window and copy
 * as much of the original window as possible into the new one.
 */
WINDOW *resize_win(c_win, len, width, sy, sx)
WINDOW *c_win;
int len, width, sy, sx;
{
    register int x, y, ny, my;
    register char gotsp;
    int oldx, oldy;
    WINDOW *new;

    if((new = newwin(len, width, sy, sx)) == (WINDOW *)0)
    {
	panic("newwin");
	yytalkabort(6);
    }
    wclear(new);
    if(c_win != (WINDOW *)0)
    {
	oldx = XPOS;
	oldy = YPOS;
	my = (len<YMAX)?len:YMAX;
	my -= 2;
	y = oldy - my;
	while(y < 0)
	    y += YMAX;
	for(gotsp = 0, ny = 0; ny <= my; ny++, y++)
	{
	    if(y >= YMAX)
		y = 0;
	    for(x = 0; x < XMAX; x++)
	    {
		wmove(c_win, y, x);
		if(winch(c_win) != ' ')
		{
		    gotsp = 1;
		    break;
		}
	    }
	    if(gotsp)
		break;
	}
	for(; ny <= my; ny++, y++)
	{
	    if(y >= YMAX)
		y = 0;
	    for(x = 0; x < width; x++)
	    {
		if(ny == my && x >= oldx)
		    break;
		if(x < XMAX)
		{
		    wmove(c_win, y, x);
		    waddch(new, winch(c_win));
		}
		else
		{
		    waddch(new, '\n');
		    break;
		}
	    }
	}
	delwin(c_win);
	if(new->_cury == (new->_maxy-1))
	{
	    x = new->_curx;
	    y = new->_cury;
	    wmove(new, 0, 0);
	    wclrtoeol(new);
	    wmove(new, y, x);
	}
    }
    return new;
}

/* Add a new window and return a window descriptor.
 */
int add_window(title1, title2, flags)
char *title1;
char *title2;
int flags;
{
    int pt, n, y, r;

    if(scr < 0)
	return -1;
    for(pt = 0; pt < MAXC; pt++)
	if(wp[pt] == -1)
	    break;
    if(pt >= MAXC)
	return -1;
    if (pt > scr) abort();
    
    wp[pt] = scr++;
    n = LINES / scr;
    if(n < 3)			/* always keep at least 3 lines per window */
	return -1;

    strncpy(win[scr-1].name, title1, TLEN-12);
    if (title2)
    {
	strcat(win[scr-1].name, "@");
	strncat(win[scr-1].name, title2, TLEN - 11 - strlen(win[scr-1].name));
    }

    if (flags & P_YTALK)
	    strcat(win[scr-1].name, " - YTalk");
    
    win[scr-1].name[TLEN-1] = '\0';

    clear();
    for(y = 0, r = 0; y < scr; y++, r += n)
    {
	if(y == (scr-1))
	{
	    n = LINES - r;
	    win[y].c_win = (WINDOW *)0;
	}
	win[y].c_win = resize_win(win[y].c_win, n-1, COLS, r+1, 0);
	barit(r, win[y].name);
	wrefresh(win[y].c_win);
    }
    refresh();
    return pt;
}

int change_title(which, title1, title2, flags)
char *title1;
char *title2;
int which;
int flags;
{
    int pt, n, y, r;

    if(which < 0 || which >= MAXC)
	return -1;
    if((which = wp[which]) == -1)
	return -1;
	
    n = LINES / scr;
    if(n < 3)			/* always keep at least 3 lines per window */
	return -1;

    strncpy(win[which].name, title1, TLEN-22);
    if (title2)
    {
	strcat(win[which].name, "@");
	strncat(win[which].name, title2, TLEN - 21 - strlen(win[which].name));
    }
    
    if (flags & P_YTALK)
	    strcat(win[which].name, " - YTalk");
	    
    if (flags & P_CRYPT)
    {
	if (flags & P_CRYPTACK)
		strcat(win[which].name, " - Crypt");
	else
		strcat(win[which].name, " - Negotiating");
    }
	    
    win[which].name[TLEN-1] = '\0';

    barit(n * which, win[which].name);
    wrefresh(win[which].c_win);
    
    refresh();
    return 0;
}
/* Delete a window by window descriptor.
 */
int del_window(pt)
{
    int n, y, r, w;

    if(pt < 0 || pt >= MAXC || scr == 1)
	return -1;
    if((w = wp[pt]) == -1)
	return -1;

    delwin(win[w].c_win);

    scr--;
    for(y = w; y < scr; y++)
	win[y] = win[y+1];
    for(y = 0; y < MAXC; y++)
	if(wp[y] > w)
	    wp[y]--;
    wp[pt] = -1;
    n = LINES / scr;

    clear();
    for(y = 0, r = 0; y < scr; y++, r += n)
    {
	if(y == (scr-1))
	    n = LINES - r;
	win[y].c_win = resize_win(win[y].c_win, n-1, COLS, r+1, 0);
	barit(r, win[y].name);
	wrefresh(win[y].c_win);
    }
    refresh();
    return 0;
}

/* Delete a temporary window by window pointer.
 */
void kill_window(w)
WINDOW *w;
{
    int y;

    if(w != (WINDOW *)0)
	delwin(w);
    refresh();
    for(y = 0; y < scr; y++)
	wrefresh(win[y].c_win);
}

void kill_smallwin(w)
WINDOW *w;
{
    int y;
    for(y = (scr-1)/2; y <= scr/2; y++)
	    wtouch(w, win[y].c_win);
    wtouch(w, stdscr);
    refresh();
    delwin(w);
    for(y = (scr-1)/2; y <= scr/2; y++)
	wrefresh(win[y].c_win);
}
	
/* barit() is an internal routine to display a title bar across the screen
 * for each window.
 */
barit(r, t)
char *t;
{
    int x, c;

    x = (COLS/2) - (strlen(t)/2) - 2;
    move(r, 0);
    c = 0;
    if(x > 1)
    {
	for(; c < x; c++)
	    addch('-');
	printw("= %s =", t);
	c += strlen(t) + 4;
    }
    for(; c < COLS; c++)
	addch('-');
    refresh();
}

/* Erase and redraw the entire screen.
 */
int redraw()
{
    int y;

    if(scr < 0)
	return -1;
/*     touchwin(stdscr); */
    clearok(stdscr, TRUE);
    refresh();
    for(y = 0; y < scr; y++)
    {
	touchwin(win[y].c_win);
	wrefresh(win[y].c_win);
    }
    return 0;
}

/* newline() is an internal routine to process a newline.
 */
newline(c_win)
WINDOW *c_win;
{
    int y;

    y = YPOS;
    if(++y >= YMAX)
    {
	y = 0;
	wmove(c_win, 0, 0);
    }
    else
	waddch(c_win, '\n');
    clrnxtline(c_win);
}

/* clrnxtline() is an internal routine to clear the next available line
 * in a given curses window.
 */
clrnxtline(c_win)
WINDOW *c_win;
{
    int y, sx, sy;

    sx = XPOS;
    y = sy = YPOS;
    if(++y >= YMAX)
	y = 0;
    wmove(c_win, y, 0);
    wclrtoeol(c_win);
    wmove(c_win, sy, sx);
}

/* Add a character to a window.
 */
add_char(w, ch)
char ch;
{
    register int x, y, i;

    if(w < 0 || w >= MAXC)
	return -1;
    if((w = wp[w]) == -1)
	return -1;

/* MOD for auto word wrap - BSY 02/13/91 */
    if(w == 0 && ch != '\t' && ch != '\n' && COLS > 40)
    {
	if(win[w].XPOS + ((ch<32)?2:1) >= COLS)
	{
	    char saveit[30], *s;

	    if(ch == ' ')
	    {
		xmit_char('\n');
		return -1;
	    }
	    x = win[w].XPOS;
	    y = win[w].YPOS;
	    for(s = saveit; x - win[w].XPOS < 20; s++)
	    {
		wmove(win[w].c_win, y, win[w].XPOS-1);
		if((*s = winch(win[w].c_win)) == ' ')
		    break;
	    }
	    wmove(win[w].c_win, y, x);
	    if(s == saveit)
		xmit_char('\n');
	    else if(*s == ' ')
	    {
                for (i=s-saveit;i>0;i--) xmit_char(RUB);
/*		xmit_char(WORD);*/
		xmit_char('\n');
		for(s--; s >= saveit; s--)
		    xmit_char(*s);
	    }
	}
    }
/* end of MOD for auto word wrap */

    wadd_char(win[w].c_win, ch);
    return 0;
}

/* wadd_char is an internal routine to add a character to a window.
 */
wadd_char(c_win, ch)
WINDOW *c_win;
char ch;
{
    if(ch == '\n')
	newline(c_win);
    else if(ch == '\t')
	waddch(c_win, ch);
    else if(ch < 32)
    {
	wadd_char(c_win, '^');
	wadd_char(c_win, ch+64);
	return;
    }
    else if(ch < 127)
    {
	if(XPOS+1 >= COLS)
	    newline(c_win);
	waddch(c_win, ch);
    }
    wrefresh(c_win);
}

/* Process a rubout in a window.
 */
w_rub(w)
{
    if(w < 0 || w >= MAXC)
	return;
    if((w = wp[w]) == -1)
	return;
    if(win[w].XPOS == 0)
	return;
    waddch(win[w].c_win, '\010');
    waddch(win[w].c_win, ' ');
    waddch(win[w].c_win, '\010');
    wrefresh(win[w].c_win);
}

/* Kill the current line in a window.
 */
w_kill(w)
{
    if(w < 0 || w >= MAXC)
	return;
    if((w = wp[w]) == -1)
	return;
    if(win[w].XPOS == 0)
	return;
    wmove(win[w].c_win, win[w].YPOS, 0);
    wclrtoeol(win[w].c_win);
    wrefresh(win[w].c_win);
}

/* Erase a word in a window.
 */
w_word(w)
{
    register char *c;

    if(w < 0 || w >= MAXC)
	return;
    if((w = wp[w]) == -1)
	return;
    if(win[w].XPOS == 0)
	return;
    do
    {
	waddch(win[w].c_win, '\010');
    } while(win[w].XPOS > 0 && winch(win[w].c_win) == ' ');
    do
    {
	waddch(win[w].c_win, '\010');
    } while(win[w].XPOS > 0 && winch(win[w].c_win) != ' ');
    if(win[w].XPOS > 0)
	wmove(win[w].c_win, win[w].YPOS, win[w].XPOS+1);
    wclrtoeol(win[w].c_win);
    wrefresh(win[w].c_win);
}

/* Clear a window.
 */
w_clr(w)
{
    int i;

    if(w < 0 || w >= MAXC)
	return;
    if((w = wp[w]) == -1)
	return;
    wclear(win[w].c_win);
    wrefresh(win[w].c_win);
}

/* Display a user-friendly error message.
 */
showerror(str, e)
char *str;
{
    int n, x, y, w = 66;
    extern char *sys_errlist[];
    WINDOW *new;

    if(scr < 0)
        return -1;
    if((x = (COLS / 2) - (w / 2)) <= 0)
    {
	x = 0;
	w = COLS;
    }
    if((y = (LINES / 2) - 3) < 0)
	y = 0;
    if((new = newwin(6, w, y, x)) == (WINDOW *)0)
	return -1;

    wmove(new, 1, 0);
    if(str != NULL)
	if(*str != '\0');
	    wprintw(new, "  YTalk error:  %s\n", str);
    if(e != 0)
	wprintw(new, "  System error: %s\n", sys_errlist[e]);
    wmove(new, 4, 0);
    wprintw(new, "  Press ESC to continue: ");

    x = new->_curx;
    y = new->_cury;
    box(new, '#', '#');

    wmove(new, y, x);
    wrefresh(new);
    putc('\07', stderr);
    while(getch() != 27)
	continue;
    for(y = 0; y < scr; y++)
	wtouch(new, win[y].c_win);
    wtouch(new, stdscr);
    kill_window(new);
    return 0;
}

showmsg(str1, str2, str3)
char *str1, *str2, *str3;
{
    int n, x, y, w = 66;
    extern char *sys_errlist[];
    WINDOW *new;

    if(scr < 0)
        return -1;
    if((x = (COLS / 2) - (w / 2)) <= 0)
    {
	x = 0;
	w = COLS;
    }
    if((y = (LINES / 2) - 3) < 0)
	y = 0;
    if((new = newwin(6, w, y, x)) == (WINDOW *)0)
	return -1;

    wmove(new, 1, 0);
    if(str1 != NULL)
	if(*str1 != '\0');
	    wprintw(new, "  %s\n", str1);
	    
    if(str2 != NULL)
	if(*str2 != '\0');
	    wprintw(new, "  %s\n", str2);
	    
    if(str3 != NULL)
	if(*str3 != '\0');
	    wprintw(new, "  %s\n", str3);
    wmove(new, 4, 0);
    wprintw(new, "  Authentication result.  Press ESC to continue: ");

    x = new->_curx;
    y = new->_cury;
    box(new, '#', '#');

    wmove(new, y, x);
    wrefresh(new);
    putc('\07', stderr);
    while(getch() != 27)
	continue;
    for(y = 0; y < scr; y++)
	wtouch(new, win[y].c_win);
    wtouch(new, stdscr);
    kill_window(new);
    return 0;
}

extern int crypto_req;

/* Display the main menu.
 */
showmenu()
{
    int n, x, y, w = 50, l = 8, sel;
    static int out;
    struct timeval tv;
    WINDOW *new = (WINDOW *)0;

    if(scr < 0)
        return -1;
    new = (WINDOW *)0;
    out = 0;
    tv.tv_sec = 0L;
    tv.tv_usec = 500000L;
    sel = 1<<0;
    if(select(32, &sel, 0, 0, &tv) == 0)
    {
	if((x = (COLS / 2) - (w / 2)) <= 0)
	{
	    x = 0;
	    w = COLS;
	}
	if((y = (LINES / 2) - (l / 2)) < 0)
	{
	    y = 0;
	    l = LINES;
	}
	if((new = newwin(l, w, y, x)) == (WINDOW *)0)
	{
	    panic("Cannot create a new window");
	    return -1;
	}
	wmove(new, 1, 0);
	wprintw(new, "  a) Add a new user to session\n");
	wprintw(new, "  d) Delete a user from session\n");
	wprintw(new, "  o) Output a user to a file\n");
	wprintw(new, "  x) Encrypt toggle (asks for passphrase)\n");
	wprintw(new, "  p) Encrypt toggle (use PGP)\n");
	wmove(new, l-2, 0);
	wprintw(new, "  Your choice: ");
	x = new->_curx;
	y = new->_cury;
	box(new, '#', '#');
	wmove(new, y, x);
	wrefresh(new);
    }
    switch(inp.code = getch())
    {
	case 'a':	/* Add a user */
	    out = getusername("Add", NULL);
	    break;
	case 'd':
	    out = getusername("Delete", NULL);
	    break;
	case 'o':
	    out = getusername("Output", "Filename");
	    break;
	case 'x':
	    if (crypto_req)
		    out = 0;
	    else
		    out = getpassphrase();
	    break;
	case 'p':
	    out = 0;
	    break;
	default:
	    putc('\07', stderr);
	    out = -1;
	    break;
    }
    if(new != (WINDOW *)0)
    {
	for(y = 0; y < scr; y++)
	    wtouch(new, win[y].c_win);
	wtouch(new, stdscr);
	kill_window(new);
    }
    return out;
}

/* Internal routine to input a user name.
 */
getusername(str, prompt)
char *str, *prompt;
{
    int x, y, w = 70, l = 3;
    WINDOW *new;

    if(scr < 0)
        return -1;
    if(prompt != NULL)
	l++;
    if((x = (COLS / 2) - (w / 2)) <= 0)
    {
	x = 0;
	w = COLS;
    }
    if((y = (LINES / 2) - (l / 2)) < 0)
    {
	y = 0;
	l = LINES;
    }
    if((new = newwin(l, w, y, x)) == (WINDOW *)0)
    {
	panic("Cannot create a new window");
	return -1;
    }

    wmove(new, 1, 0);
    wprintw(new, "  %s which user? ", str);
    x = new->_curx;
    y = new->_cury;
    box(new, '#', '#');
    wmove(new, y, x);
    wrefresh(new);

    in_str(new, inp.name, w-x-1, 0);

    if(inp.name[0] != '\0' && prompt != NULL)
    {
	wmove(new, 2, 2);
	wprintw(new, "%s? ", prompt);
	x = new->_curx;
	y = new->_cury;
	wrefresh(new);
	in_str(new, inp.data, w-x-1, 0);
    }
    
    kill_smallwin(new);

    if(inp.name[0] == '\0')
	return -1;
    return 0;
}

/* Internal routine to input a user name.
 */
getpassphrase()
{
    int x, y, w = 70, l = 3;
    WINDOW *new;

    if(scr < 0)
        return -1;
    if((x = (COLS / 2) - (w / 2)) <= 0)
    {
	x = 0;
	w = COLS;
    }
    if((y = (LINES / 2) - (l / 2)) < 0)
    {
	y = 0;
	l = LINES;
    }
    if((new = newwin(l, w, y, x)) == (WINDOW *)0)
    {
	panic("Cannot create a new window");
	return -1;
    }

    wmove(new, 1, 0);
    wprintw(new, "  Passphrase: ");
    x = new->_curx;
    y = new->_cury;
    box(new, '#', '#');
    wmove(new, y, x);
    wrefresh(new);

    in_str(new, inp.data, w-x-1, 1);

    kill_smallwin(new);
    
    if(inp.data[0] == '\0')
	return -1;
    return 0;
}
/* Internal routine to read a string from a curses window.
 */
in_str(c_win, str, maxc, hide)
WINDOW *c_win;
char *str;
{
    int n;
    char *c;

    for(c = str, n = 0; n < maxc;)
    {
	*c = getch();
	if(*c == '\n')
	    break;
	if(n > 0 && (*c == 8 || *c == 127))
	{
	    c--;
	    n--;
	    waddch(c_win, 8);
	    waddch(c_win, ' ');
	    waddch(c_win, 8);
	    wrefresh(c_win);
	    continue;
	}
	if(*c < 32 || *c > 126 || n >= maxc-1)
	{
	    putc('\07', stderr);
	    continue;
	}
	waddch(c_win, hide ? '*' : *c);
	wrefresh(c_win);
	n++;
	c++;
    }
    *c = '\0';
}

/* Display a prompt and read a yes/no response from user.
 */
char yes_no(str)
char *str;
{
    int n, x, y, w = 60, l = 3, sel;
    struct timeval tv;
    static char ch;
    WINDOW *new;

    if(scr < 0)
        return -1;
    new = (WINDOW *)0;
    if((x = (COLS / 2) - (w / 2)) <= 0)
    {
	x = 0;
	w = COLS;
    }
    if((y = (LINES / 2) - (l / 2)) < 0)
    {
	y = 0;
	l = LINES;
    }
    if((new = newwin(l, w, y, x)) == (WINDOW *)0)
    {
	panic("Cannot create a new window");
	return -1;
    }

    wmove(new, 1, 0);
    wprintw(new, "  %s ", str);
    x = new->_curx;
    y = new->_cury;
    box(new, '#', '#');
    wmove(new, y, x);
    wrefresh(new);

    tv.tv_sec = 0L;
    tv.tv_usec = 200000L;
    sel = 1<<0;
    while(select(32, (int *) &sel, 0, 0, &tv) == 1)
    {
	tv.tv_sec = 0L;
	tv.tv_usec = 200000L;
	sel = 1<<0;
	getch();
    }

    do
    {
	ch = getch();
	if(isupper(ch))
	    ch -= 32;
    } while(ch != 'y' && ch != 'n');

    for(y = 0; y < scr; y++)
	wtouch(new, win[y].c_win);
    wtouch(new, stdscr);
    kill_window(new);
    return ch;
}

mycbreak()
{
    cmode();
    noecho();
}
