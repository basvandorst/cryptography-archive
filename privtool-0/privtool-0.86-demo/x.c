/*
 *	@(#)x.c	1.76 11/1/95
 *
 *	(c) Copyright 1993-1995 by Mark Grant, and by other
 *	authors as appropriate. All right reserved.
 *
 *	The authors assume no liability for damages resulting from the 
 *	use of this software, even if the damage results from defects in
 *	this software. No warranty is expressed or implied.
 *
 *	This software is being distributed under the GNU Public Licence,
 *	see the file COPYING for more details.
 *
 *			- Mark Grant (mark@unicorn.com) 29/6/94
 *	
 *      Menu item for selecting Mail files added by
 *      	- Keith Paskett (keith.paskett@sdl.usu.edu) 8 Sep 1994
 *
 *	Linux compatibility changes by 
 *		- David Summers (david@actsn.fay.ar.us) 6th July 1995
 *
 *	Various changes 
 *		- Anders Baekgaard (baekgrd@ibm.net) 10th August 1995
 *
 *      Linux icon changes by
 *              - Alan Teeder (ajteeder@dra.hmg.gb) 1 Sept 1995
 *                      changed to use .xbm files - some linux systems
 *                      corrupt normal icons.
 *
 *	Pushpins added to display windows
 *		- Gregory Margo (gmargo@newton.vip.best.com) 5th Oct 1995
 *
 *	Numerous compose window changes
 *		- Tony Gialluca (tony@hgc.edu)	27th Oct 1995
 */

/* We define UI_MAIN so that header files can tell how to define the
   neccesary structures */

#define UI_MAIN

#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/dirent.h>
#ifndef linux
#include <X11/Xos.h>
#else
#include <signal.h>
#define SCO324
#endif
#ifndef MAXPATHLEN
#include <sys/param.h>
#endif /* MAXPATHLEN */
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/canvas.h>
#include <xview/sel_attrs.h>
#include <xview/textsw.h>
#include <xview/notice.h>
#include <xview/svrimage.h>
#include <xview/icon.h>
#include <xview/font.h>
#include <xview/scrollbar.h>
#include <xview/notify.h>
#include <xview/cms.h>
#include <xview/defaults.h>
#ifdef linux
#include <X11/Xos.h>
#endif

#include "def.h"
#include "buffers.h"
#include "message.h"
#include "mailrc.h"
#include "gui.h"
#include "windows.h"
#ifdef USE_FLOPPY
#include "floppy.h"
#endif

#define	GC_KEY	10

#ifndef FIXED_WIDTH_FONT
#define FIXED_WIDTH_FONT FONT_FAMILY_DEFAULT_FIXEDWIDTH
#endif

#ifndef XRESOURCES
#define XRESOURCES ".Xdefaults"
#endif

static	Frame	main_frame;
static	Panel	top_panel;
static	Panel_item	file_name_item;

static	Panel_item	nym_item;
static	Canvas		list_canvas;
static	Scrollbar	v_scroll;
static	Xv_font		list_font;
static	Icon		icon,newmail_icon;
#ifdef USE_XBM
static	Pixmap		icon_pm, newmail_icon_pm, mask_icon_pm;
#endif
static	Server_image	icon_image, newmail_icon_image, icon_image_mask;
static	Xv_window	list_window;

static	COMPOSE_WINDOW	*compose_first = NULL;
static	COMPOSE_WINDOW	*compose_last = NULL;

static	DISPLAY_WINDOW	*display_first = NULL;
static	DISPLAY_WINDOW	*display_last = NULL;

#define LIST_DISPLACEMENT	16

#ifdef USE_XBM

#include "privtool.xbm"
#include "privtool-new.xbm"
#include "privtool-mask.xbm"

#else /* USE_XBM */

static unsigned short icon_bits[] = {
#ifdef NSA_ICON
#include "privtool.icon"
#else
#include <images/mailseen.icon>
#endif
};

static unsigned short	newmail_icon_bits[] = {
#ifdef NSA_ICON
#include "privtool-new.icon"
#else
#include <images/mail.icon>
#endif
};

#ifdef NSA_ICON
static unsigned short	icon_bits_mask[] = {
#include "privtool-mask.icon"
};
#endif
#endif /* USE_XBM */

extern	char	default_mail_file[];
extern	char	*our_userid;

static	Frame	pass_frame;
static	Panel_item	pass_item;
static	Menu		files_menu[3];
static	Menu_item	files_item[3];
	Menu_item	add_path_to_menu();

static	Window  map_win;
static	Display *dpy;
static	GC	gc;

#ifdef XSAFEMAIL
char	APPL[] = "Xsafemail";
#else
char	APPL[] = "Privtool";
#endif

static	char	*fixedwidthfont = NULL;
static	char	headerwin_sticky = 0;

static	void	load_xresources();
static	void	find_xresources();
static	void	save_xresources();
static	void	defaults_get_rect();

/* Take a frame and find a compose window */

static	COMPOSE_WINDOW	*compose_from_frame (frame)

Frame	frame;

{
	COMPOSE_WINDOW	*w;

	w = compose_first;

	while (w) {
		if (w->deliver_frame == frame) 
			return w;

		w = w->next;
	}

	return NULL;
}

/* Take a log_item and find a compose window */

static	COMPOSE_WINDOW	*compose_from_log_item (item)

Panel_item	item;

{
	COMPOSE_WINDOW	*w;

	w = compose_first;

	while (w) {
		if (w->log_item == item) 
			return w;

		w = w->next;
	}

	return NULL;
}

/* Take a deliver_item and find a compose window */

static	COMPOSE_WINDOW	*compose_from_deliver_item (item)

Panel_item	item;

{
	COMPOSE_WINDOW	*w;

	w = compose_first;

	while (w) {
		if (w->deliver_item == item) 
			return w;

		w = w->next;
	}

	return NULL;
}

/* Take a clear_item and find a compose window */

static	COMPOSE_WINDOW	*compose_from_clear_item (item)

Panel_item	item;

{
	COMPOSE_WINDOW	*w;

	w = compose_first;

	while (w) {
		if (w->clear_item == item) 
			return w;

		w = w->next;
	}

	return NULL;
}

/* Look for a free window */

static	COMPOSE_WINDOW	*compose_find_free ()

{
	COMPOSE_WINDOW	*w;

	w = compose_first;

	while (w) {
		if (!w->in_use)
			return w;

		w = w->next;
	}

	return NULL;
}

/* Find a free display window */

static	DISPLAY_WINDOW	*display_find_free (n)

int	n;

{
	DISPLAY_WINDOW	*w;

	w = display_first;
	while (w) {
		if (w->number == n)
			return w;

		w = w->next;
	}

	w = display_first;
	while (w) {
		if (!xv_get (w->display_frame, FRAME_CMD_PUSHPIN_IN))
			return w;

		w = w->next;
	}

	return NULL;
}

/* Find a display window from its frame */

static	DISPLAY_WINDOW	*display_from_frame (frame)

Frame	frame;

{
	DISPLAY_WINDOW	*w;

	w = display_first;

	while (w) {
		if (w->display_frame == frame) 
			return w;

		w = w->next;
	}

	return NULL;
}

/* Find a display window from its panel */

static	DISPLAY_WINDOW	*display_from_panel (panel)

Panel	panel;

{
	DISPLAY_WINDOW	*w;

	w = display_first;

	while (w) {
		if (w->display_panel == panel) 
			return w;

		w = w->next;
	}

	return NULL;
}

/* Count the number of open compose windows */

int	compose_windows_open ()

{
	int	c = 0;
	COMPOSE_WINDOW	*w;

	w = compose_first;

	while (w) {
		if (w->in_use)
			c++;

		w = w->next;
	}

	return c;
}

static	void	passphrase_events (win, event, arg)

Xv_Window	win;
Event		*event;
Notify_arg	arg;

{
	switch (event_action(event)) {

		case ACTION_DISMISS:
#if 0
		case ACTION_CLOSE:
#endif
		abort_passphrase_proc ();
		break;

	}
}

/* Handle compose window events */

static void	compose_events (win, event, arg)

Xv_Window	win;
Event		*event;
Notify_arg	arg;

{
	COMPOSE_WINDOW	*w;

	switch (event_action(event)) {

		case ACTION_DISMISS:
		w = compose_from_frame (win);

		if (w)
			w->in_use = FALSE;

		break;
		
	}
}

/* Draw a box around the message in the list to show it's selected */

static	void	draw_box_round_message(m)

MESSAGE	*m;

{
	int	y,w;
	XPoint	points[5];

	y = m->list_pos * LIST_DISPLACEMENT;
	w = xv_get (list_canvas, XV_WIDTH);

	points[0].x = 0;
	points[0].y = y - LIST_DISPLACEMENT + 4;
	points[1].x = w - 2;
	points[1].y = points[0].y;
	points[2].x = points[1].x;
	points[2].y = points[1].y + LIST_DISPLACEMENT - 1;
	points[3].x = 0;
	points[3].y = points[2].y;
	points[4].x = 0;
	points[4].y = points[0].y;

	XDrawLines(dpy, map_win, gc, points, 5, CoordModeOrigin);
}

/* Display the message description in the list */

void	display_message_description(m)

MESSAGE	*m;

{
	int	y,w;

	y = m->list_pos * LIST_DISPLACEMENT;
	w = xv_get (list_canvas, XV_WIDTH);

	XClearArea(dpy, map_win, 0, y - LIST_DISPLACEMENT + 4,
		w, LIST_DISPLACEMENT, FALSE);

	XDrawString(dpy,map_win,gc,0,
		y,
		m->description,strlen(m->description));

		if (m->flags & MESS_SELECTED)
			draw_box_round_message(m);
}

/* Display a footer string */

void	set_main_footer(s)

char	*s;

{
	if (main_frame)
		xv_set(main_frame,
			FRAME_LEFT_FOOTER,s,
			NULL);
}

/* Clear the footer string */

void	clear_main_footer()

{
	set_main_footer("");
}

#define NORMAL_ICON	0
#define NEWMAIL_ICON	1

static	int	icon_type;

/* Show the icon for new mail */

void	show_newmail_icon()

{
	if (newmail_icon && icon_type != NEWMAIL_ICON) {

		/* Update the icon image */

#ifndef USE_XBM
		xv_set (icon_image,
			SERVER_IMAGE_BITS, newmail_icon_bits,
			NULL);
#endif

		/* Force a repaint */

		if (xv_get (main_frame, FRAME_CLOSED)) {
			xv_set (main_frame,
				WIN_ALARM, 
				NULL);
#ifndef USE_XBM
			xv_set (icon,
				ICON_IMAGE, icon_image,
				NULL);
#else
			xv_set (icon,
				ICON_IMAGE, newmail_icon_image,
				ICON_MASK_IMAGE, icon_image_mask,
				ICON_TRANSPARENT, TRUE,
				NULL);
#endif
		}

		icon_type = NEWMAIL_ICON;
	}
}

/* Display the icon for no new mail */

void	show_normal_icon()

{
	if (icon && icon_type != NORMAL_ICON) {

		/* Update the icon image */

#ifndef USE_XBM
		xv_set (icon_image,
			SERVER_IMAGE_BITS, icon_bits,
			NULL);
#endif

		/* Force a repaint */

		if (xv_get (main_frame, FRAME_CLOSED)) {
			xv_set (icon,
				ICON_IMAGE, icon_image,
#ifdef USE_XBM
				ICON_TRANSPARENT, TRUE,
#endif
				NULL);
		}

		icon_type = NORMAL_ICON;
	}
}

/* Close the passphrase window */

void	close_passphrase_window()

{
	if (pass_frame)
		xv_set(pass_frame,
			XV_SHOW, FALSE,
			NULL);
}

/* Open the passphrase window */

void	open_passphrase_window(s)

char	*s;

{
	if (pass_frame)
		xv_set(pass_frame,
			XV_SHOW, TRUE,
			FRAME_LABEL, s,
			FRAME_CLOSED, FALSE, /* Just in case */
			NULL);
}

/* MUST be possible to call this function before pass_item is set up */

char	*read_passphrase_string()

{
	if (!pass_item)
		return NULL;

	return (char *)xv_get(pass_item,
		PANEL_VALUE);
}

/* Clear the displayed passphrase string */

void	clear_passphrase_string()

{
	if (pass_item)
		xv_set(pass_item,
			PANEL_VALUE,"",
			NULL);
}

static	Panel	pass_panel;

/* Create the passphrase window */

void	create_passphrase_window()

{
	if (!pass_frame) {
		pass_frame = (Frame) xv_create(main_frame, FRAME,
			XV_SHOW, FALSE,
			XV_X, xv_get(main_frame, XV_X) + 100,
			XV_Y, xv_get(main_frame, XV_Y)+xv_get(main_frame, 
				XV_HEIGHT)/2 - 25,
			WIN_EVENT_PROC, passphrase_events,
			WIN_CONSUME_EVENTS,
				WIN_UNMAP_NOTIFY,
				NULL,
			NULL);

		pass_panel = (Panel) xv_create(pass_frame, PANEL,
			XV_HEIGHT, 80,
			PANEL_LAYOUT, PANEL_HORIZONTAL,
			NULL);

		pass_item = xv_create (pass_panel, PANEL_TEXT,
			PANEL_LABEL_STRING, "Passphrase :",
			PANEL_VALUE_DISPLAY_LENGTH,20,
			PANEL_MASK_CHAR,'*',
			PANEL_NOTIFY_PROC, got_passphrase,
			NULL);

		(void) xv_create(pass_panel, PANEL_BUTTON,
			PANEL_LABEL_STRING, "Abort",
			PANEL_NOTIFY_PROC, abort_passphrase_proc,
			NULL);

		window_fit(pass_panel);
		window_fit(pass_frame);
	}
}

/* Set the value of the log/encrypt/etc item */

void	update_log_item(w)

COMPOSE_WINDOW	*w;

{
	if (w->log_item)
		xv_set(w->log_item,
			PANEL_VALUE, w->deliver_flags,
			NULL);
}

#ifndef NO_MIXMASTER
void	remail_failed_notice_proc()

{
	notice_prompt(top_panel, NULL,
		NOTICE_MESSAGE_STRINGS, 
			"Remail Failed !",
			NULL,
		NOTICE_BUTTON_YES, "Abort",
		NOTICE_BUTTON_NO, "Ignore",
		NULL);

	update_random ();
}
#endif

/* Oops, we asked to quit when there's open compose windows */

int	dont_quit_notice_proc ()

{
	int	choice = NOTICE_YES;

	choice = notice_prompt(top_panel, NULL,
		NOTICE_MESSAGE_STRINGS,
			"You have open compose windows. Quitting will",
			"lose the messages in those windows. Press Abort",
			"to cancel the quit operation, or Continue to",
			"exit Privtool.",
			NULL,
		NOTICE_BUTTON_YES, "Abort",
		NOTICE_BUTTON_NO, "Continue",
		NULL);

	update_random ();

	return (choice == NOTICE_YES);
}

int	read_only_notice_proc ()

{
	int	choice = NOTICE_YES;

	choice = notice_prompt(top_panel, NULL,
		NOTICE_MESSAGE_STRINGS, "Mail file is read-only so changes", 
			"Cannot be saved.",
			NULL,
		NOTICE_BUTTON_YES, "Abort",
		NOTICE_BUTTON_NO, "Ignore",
		NULL);

	update_random ();

	return (choice == NOTICE_YES);
}

/* Do we send to the reply-to address ? */

int	use_reply_to_notice_proc ()

{
	int	choice = NOTICE_YES;

	choice = notice_prompt(top_panel, NULL,
		NOTICE_MESSAGE_STRINGS, 
			"Use Reply-To: address rather than From: address ?",
			NULL,
		NOTICE_BUTTON_YES, "Yes",
		NOTICE_BUTTON_NO, "No",
		NULL);

	update_random ();

	return (choice == NOTICE_YES);
}

/* Warn the user that we failed to save correctly */

int	failed_save_notice_proc ()

{
	int	choice = NOTICE_YES;

	choice = notice_prompt(top_panel, NULL,
		NOTICE_MESSAGE_STRINGS, "Error occured while writing to disk !", 
			"Press abort to cancel this operation, or ignore to",
			"ignore the error (which may cause a loss of data !).",
			NULL,
		NOTICE_BUTTON_YES, "Abort",
		NOTICE_BUTTON_NO, "Ignore",
		NULL);

	update_random ();

	return (choice == NOTICE_YES);
}

/* Tell the user that they don't have the correct secret key */

int	no_sec_notice_proc(w)

int	w;

{
	int	choice;
	Panel	p;

	switch (w) {

		case ERROR_DELIVERY:
		case ERROR_READING:
		p = top_panel;
		break;

	}

	choice = notice_prompt(p, NULL,
		NOTICE_MESSAGE_STRINGS, "Error: No secret key found !", 
			NULL,
		NOTICE_BUTTON_YES, "Send Unsigned",
		NOTICE_BUTTON_NO, "Abort",
		NULL);

	update_random();

	return (choice == NOTICE_YES);
}

/* Confirm that we want to read the file despite failure to lock it */

int	confirm_unlocked_read ()

{
	int	choice;

	/* Firewall in case we're called before opening the UI */

	if (!top_panel)
		return FALSE;

	choice = notice_prompt (top_panel, NULL,
		NOTICE_MESSAGE_STRINGS,
			"File cannot be locked for read. Open anyway ?",
			NULL,
		NOTICE_BUTTON_YES, "Open",
		NOTICE_BUTTON_NO, "Abort",
		NULL);

	update_random ();

	return (choice == NOTICE_YES);
}

/* Warn the user that we couldn't find a public key for the recipient */

int	no_key_notice_proc(w)

int	w;

{
	int	choice;
	Panel	p;

	switch (w) {

		case ERROR_READING:
		case ERROR_DELIVERY:
		p = top_panel;
		break;

	}

	choice = notice_prompt(p, NULL,
		NOTICE_MESSAGE_STRINGS, "Error: Recipient has no public key !", 
			NULL,
		NOTICE_BUTTON_YES, "Send Unencrypted",
		NOTICE_BUTTON_NO, "Abort",
		NULL);

	update_random();

	return (choice == NOTICE_YES);
}

/* Warn the user that we got a bad PGP message (bad format, I mean 8-)) */

void	bad_file_notice(w)

int	w;

{
	Panel	p;

	switch (w) {

		case ERROR_READING:
		case ERROR_DELIVERY:
		p = top_panel;
		break;

	}

	(void) notice_prompt(p, NULL,
		NOTICE_MESSAGE_STRINGS, "Error: Bad message format !", NULL,
		NOTICE_BUTTON_YES, "Abort",
		NULL);

	update_random();
}

/* We got a bad key when adding to the keyring */

void	bad_key_notice_proc()

{
	(void) notice_prompt(top_panel, NULL,
		NOTICE_MESSAGE_STRINGS, "Error: No key found in message, or",
			"bad key format !", NULL,
		NOTICE_BUTTON_YES, "Abort",
		NULL);

	update_random();
}

/* Tell the user that they entered an incorrect passphrase */

int	bad_pass_phrase_notice(w)

int	w;

{
	Panel	p;
	int	choice;

	switch (w) {

		case ERROR_READING:
		case ERROR_DELIVERY:
		p = top_panel;
		break;

	}

	choice = notice_prompt(p, NULL,
		NOTICE_MESSAGE_STRINGS, "Error: Bad Pass Phrase !", NULL,
		NOTICE_BUTTON_YES, "Reenter",
		NOTICE_BUTTON_NO, "Abort",
		NULL);

	update_random();

	return (choice == NOTICE_YES);
}

/* Return the currently selected file name */

char	*read_file_name()

{
	return  (char *)xv_get (file_name_item,PANEL_VALUE);
}

/* Set the currently selected file name */

void	set_file_name (s)

char	*s;

{
	xv_set (file_name_item, 
		PANEL_VALUE, s,
		NULL);
}

/* Update the main list of mail messages */

void	update_message_list()

{
	char	s[128];
	char	b[64];
	int	oldh, newh;
	int	o,h;
	int	w;
	int	v_pos;

	/* Firewall */

	if (!list_canvas)
		return;

	oldh = xv_get (list_canvas, CANVAS_HEIGHT);
	newh = messages.number * LIST_DISPLACEMENT + 4;

	o = oldh / LIST_DISPLACEMENT;
	h = messages.number;

	w = xv_get (list_canvas, XV_WIDTH);
	if (dpy)
		XClearArea (dpy, map_win, 0, newh, w, oldh - newh, FALSE);

	if ((h+15)/16 != (o+15)/16 || (h > o)) {
		xv_set (list_canvas, CANVAS_HEIGHT,
			(newh+15) & ~15,
			NULL);
	}

	if (!deleted.number)
		sprintf (s, "%d messages", messages.number);
	else
		sprintf (s, "%d messages, %d deleted", messages.number,
			deleted.number);

	if (messages.new) {
		sprintf (b, ", %d new", messages.new);
		strcat (s, b);
	}

	if (messages.unread) {
		sprintf (b, ", %d unread", messages.unread);
		strcat (s, b);
	}

#ifdef SHOW_ENCRYPTED
	if (messages.encrypted) {
		sprintf (b, ", %d encrypted", messages.encrypted);
		strcat (s, b);
	}
#endif

	xv_set (main_frame,
		FRAME_RIGHT_FOOTER, s,
		NULL);
}


#define READ_SIZE	128

/* Read the message out of the edit window into a buffer */

void	read_message_to_deliver(w, b)

COMPOSE_WINDOW	*w;
BUFFER	*b;

{
	char	buff[READ_SIZE];
	Textsw_index	next_pos = (Textsw_index) 0;
	Textsw_index	last_pos;

	do {
		last_pos = next_pos;
		next_pos = (Textsw_index) xv_get(w->deliver_body_window,
			TEXTSW_CONTENTS,next_pos,buff,
			READ_SIZE);
		if (last_pos != next_pos)
			add_to_buffer(b,buff,next_pos-last_pos);
	} while (last_pos != next_pos);
}

int	read_deliver_flags (w)

COMPOSE_WINDOW	*w;

{
	return w->deliver_flags;
}

/* Get the recipient for the message */

char	*read_recipient(w)

COMPOSE_WINDOW	*w;

{
	return (char *)xv_get(w->send_to_item,PANEL_VALUE);
}

/* Get the subject for the message */

char	*read_subject(w)

COMPOSE_WINDOW	*w;

{
	 return (char *)xv_get(w->send_subject_item,PANEL_VALUE);
}

/* Get the cc: line */

char	*read_cc(w)

COMPOSE_WINDOW	*w;

{
	 return (char *)xv_get(w->send_cc_item,PANEL_VALUE);
}

/* Get the bcc: line */

char	*read_bcc(w)

COMPOSE_WINDOW	*w;

{
	if (w->send_bcc_item)
		return (char *)xv_get(w->send_bcc_item,PANEL_VALUE);
	else
		return NULL;
}

char   *read_extra_headerline(w,i)

COMPOSE_WINDOW	*w;
int	i;

{
	if (w->compose_extra_headerlines[i]) {
		char	*label;
		char	*value;
		char	*line = NULL;

		label = (char*) xv_get(w->compose_extra_headerlines[i], 
			PANEL_LABEL_STRING);
		value = (char*) xv_get(w->compose_extra_headerlines[i], 
			PANEL_VALUE);

		if (label && *label && value && *value) {
			line = (char*) malloc(strlen(label)+strlen(value)+3);
			if (line)
				sprintf(line, "%s %s\n", label, value);
		}
		return line;
	}
	else
		return NULL;
}

/* Show the message composition window */

void	show_deliver_frame(w)

COMPOSE_WINDOW	*w;

{
	if (w->deliver_frame)
		xv_set(w->deliver_frame,
			XV_SHOW,TRUE,
			NULL);
}

/* And close it */

void	close_deliver_window(w)

COMPOSE_WINDOW	*w;

{
	if (w->deliver_frame)
		xv_set(w->deliver_frame,
			XV_SHOW,FALSE,
			NULL);

	w->in_use = FALSE;
}


void	set_focus_to_body_proc(item)

Panel_item	item;

{
	COMPOSE_WINDOW	*w;

	w = (COMPOSE_WINDOW *)xv_get (item, PANEL_CLIENT_DATA);
	xv_set(w->deliver_body_window, WIN_SET_FOCUS, NULL);
}

/* Set the flags as appropriate when the user changes the value */

static	void	options_proc(item,value,event)

Panel_item	item;
int	value;
Event	*event;

{
	COMPOSE_WINDOW	*w;

	w = compose_from_log_item (item);

	if (w)
		w->deliver_flags = value;
}

static	void	x_deliver_proc (item, event)

Panel_item	item;
Event		*event;

{
	COMPOSE_WINDOW	*w;

	w = compose_from_deliver_item (item);
	if (w)
		deliver_proc (w);
}

 /* insert signature in local window*/

static        void    insert_signature(menu, item)

Menu		menu;
Menu_item	item;

{
	char    *s;
	COMPOSE_WINDOW *w;
	char    *loc;
	char    path[MAXPATHLEN];

	update_random();

	set_reply (last_message_read);

	w = (COMPOSE_WINDOW *) xv_get (item, MENU_CLIENT_DATA);

	xv_set (w->send_cc_item,
		PANEL_VALUE, "",
	NULL);

	/* Lets get the signature from .mailrc or $HOME/.signature  */

	if( (s = find_mailrc ("Sign")) == NULL){
		if( (s = find_mailrc ("sign")) == NULL) {
			loc = getenv("HOME");
			if (loc) {
				strcpy(path, loc);
				strcat(path, "/.signature");
			}
		}else {
			strcpy(path,s);
		}
	} else {
		strcpy(path,s);
	}

	/* Insert the file pointed to by path */

	xv_set(w->deliver_body_window,
		TEXTSW_INSERT_FROM_FILE,path,
		NULL);

	clear_main_footer ();
}

static char	attribution_string[] = " said :\n\n";

static	void	set_send_to(w)

COMPOSE_WINDOW	*w;

{
	char	*send_to = last_message_read->email;

	if (last_message_read->reply_to &&
		!find_mailrc("defaultusefrom")) {
		if (find_mailrc("defaultusereplyto") ||
			use_reply_to_notice_proc()) {
			send_to = last_message_read->reply_to;
		}
	}

	xv_set(w->send_to_item,
		PANEL_VALUE, send_to,
		NULL);
}

/* Reply_to sender and include message in local window*/

static        void    reply_to_sender_local(menu, item)

Menu		menu;
Menu_item       item;

{
	int     i;
	byte    *mess;
	byte    *m;
	BUFFER  *b;
	char    subject[256];
	char    *indent;
	COMPOSE_WINDOW *w;
	int     indent_l = 2;

	update_random();

	if (!last_message_read)
		return;

	set_reply (last_message_read);

	if (!(indent = find_mailrc("indentprefix")))
		indent = "> ";
	else
		indent_l = strlen (indent);

	w = (COMPOSE_WINDOW *) xv_get (item, MENU_CLIENT_DATA);

	if (last_message_read->decrypted)
		b = last_message_read->decrypted;
	else
		b = message_contents(last_message_read);

	mess = b->message;
	i = b->length;

	textsw_insert(w->deliver_body_window,
		last_message_read->sender,
		strlen(last_message_read->sender));

	textsw_insert(w->deliver_body_window,
		attribution_string,
		strlen(attribution_string));

	while (i > 0) {
		if (indent_l)
			textsw_insert(w->deliver_body_window,
				indent, indent_l);
		m = mess;
		while (i-- && *m && *m!= '\n')
			m++;

		textsw_insert(w->deliver_body_window,(char *)mess,
			m - mess + 1);

		mess = m+1;
	}

	xv_set(w->deliver_body_window,
		TEXTSW_INSERTION_POINT,0,
		TEXTSW_FIRST_LINE,0,NULL);
}

/* Clear local window */

static	void	clear_local (item, event)

Panel_item	item;
Event		*event;

{
	byte    *m;
	char    subject[256];
	char    *indent;
	COMPOSE_WINDOW *w;

	update_random();

	if (!last_message_read)
		return;

	set_reply (last_message_read);

	w = compose_from_clear_item (item);

	textsw_delete(w->deliver_body_window,
		0, TEXTSW_INFINITY);

	clear_main_footer ();
}

/* Setup the deliver window */

COMPOSE_WINDOW	*x_setup_send_window()

{
	char	*log,*dontlog;
	char	*nym;
	int	i;
	char	mailrcline[10];
	char	*headerline;
	Menu	button_menu;
	COMPOSE_WINDOW	*w;

	/* See if we have a window we can use */

	w = compose_find_free ();

	/* Create the frame if not already done */

	if (!w) {

		w = (COMPOSE_WINDOW *)malloc (sizeof (COMPOSE_WINDOW));

		w->deliver_frame = (Frame) xv_create(main_frame, FRAME,
			XV_WIDTH, 640,
			WIN_EVENT_PROC, compose_events,
			WIN_CONSUME_EVENTS,
				WIN_UNMAP_NOTIFY,
				NULL,
			FRAME_LABEL, "Compose Window",
			FRAME_NO_CONFIRM, FALSE,
			NULL);


		button_menu = (Menu) xv_create(NULL, MENU,
			MENU_ITEM,
				MENU_STRING, "Orig Mail..",
				MENU_NOTIFY_PROC, reply_to_sender_local,
				MENU_CLIENT_DATA, w,
				NULL,
			MENU_ITEM,
				MENU_STRING, "Signature",
				MENU_NOTIFY_PROC, insert_signature,
				MENU_CLIENT_DATA, w,
				NULL,
			MENU_ITEM,
				MENU_STRING, "File ...",
				MENU_INACTIVE, TRUE,
				MENU_CLIENT_DATA, w,
				NULL,
			NULL);

		w->deliver_panel = (Panel) xv_create(w->deliver_frame, PANEL,
			PANEL_LAYOUT, PANEL_HORIZONTAL,
			PANEL_ITEM_Y_GAP, 6,
			NULL);

		w->include_item = xv_create (w->deliver_panel, PANEL_BUTTON,
			PANEL_LABEL_STRING, "Include",
			PANEL_ITEM_MENU, button_menu,
			NULL);

		w->deliver_item = xv_create (w->deliver_panel, PANEL_BUTTON,
			PANEL_LABEL_STRING, "Deliver",
			PANEL_NOTIFY_PROC, x_deliver_proc,
			NULL);

		w->header_item = xv_create (w->deliver_panel, PANEL_BUTTON,
			PANEL_LABEL_STRING, "Header",
			NULL);

		w->clear_item = xv_create (w->deliver_panel, PANEL_BUTTON,
			PANEL_LABEL_STRING, "Clear",
			PANEL_NOTIFY_PROC, clear_local,
			NULL);

		w->send_to_item = (Panel_item) xv_create(w->deliver_panel,
			PANEL_TEXT,
			PANEL_LABEL_STRING,"To :",
			PANEL_VALUE_DISPLAY_LENGTH, 45,
			NULL);

		w->send_subject_item = (Panel_item) xv_create(w->deliver_panel,
			PANEL_TEXT,
			PANEL_LABEL_STRING,"Subject :",
			PANEL_VALUE_DISPLAY_LENGTH, 40,
			NULL);

		w->send_cc_item = (Panel_item) xv_create(w->deliver_panel,
			PANEL_TEXT,
			PANEL_LABEL_STRING,"Cc :",
			PANEL_VALUE_DISPLAY_LENGTH, 45,
			NULL);

		if (find_mailrc("askbcc")) {
			w->send_bcc_item = (Panel_item) xv_create(
				w->deliver_panel,
				PANEL_TEXT,
				PANEL_LABEL_STRING,"Bcc :",
				PANEL_CLIENT_DATA, w,
				PANEL_VALUE_DISPLAY_LENGTH, 45,
				PANEL_NOTIFY_PROC, set_focus_to_body_proc,
				NULL);
		}
		else {
			w->send_bcc_item = (Panel_item)0;
			xv_set (w->send_cc_item, 
				PANEL_CLIENT_DATA, w,
				PANEL_NOTIFY_PROC, set_focus_to_body_proc,
				NULL);
		}

		for (i = 1; i < MAX_EXTRA_HEADERLINES; i++) {
			sprintf(mailrcline, "header%d", i);

			if ((headerline = find_mailrc(mailrcline))) {
				char	 *label;

#ifdef ALLOCA
				label = (char *)alloca(strlen(headerline)+2);
#else
				label = (char *)malloc(strlen(headerline)+2);
#endif
			
				sprintf(label, "%s%s", headerline, ":");
				w->compose_extra_headerlines[i] = 
					(Panel_item) xv_create(
					w->deliver_panel,
					PANEL_TEXT,
					PANEL_LABEL_STRING, label,
					PANEL_VALUE_DISPLAY_LENGTH, 45,
					PANEL_CLIENT_DATA, w,
					PANEL_NOTIFY_PROC, 
						set_focus_to_body_proc,
					NULL);

#ifndef ALLOCA
				free (label);
#endif
			}
			else
				w->compose_extra_headerlines[i] =
					(Panel_item) 0;
		}

		w->deliver_body_window = (Textsw) xv_create(w->deliver_frame, TEXTSW,
			XV_HEIGHT, 288,
			XV_Y, 120,
			TEXTSW_MEMORY_MAXIMUM,1000000,
			NULL);

		w->log_item = xv_create (w->deliver_panel, PANEL_CHECK_BOX,
			PANEL_LABEL_STRING, "Options",
			PANEL_CHOOSE_ONE, FALSE,
			PANEL_CHOICE_STRINGS, "Sign", "Encrypt", "Log", 
				"Raw", 
#ifndef NO_MIXMASTER
				"Remail", 
#endif
				NULL,
			PANEL_NOTIFY_PROC, options_proc,
			XV_X, 0,
			XV_Y, 90,
			NULL);

		window_fit_height (w->deliver_panel);
		window_fit(w->deliver_body_window);
		window_fit(w->deliver_frame);

		w->next = NULL;
		if (compose_last) {
			compose_last->next = w;
			w->prev = compose_last;
			compose_last = w;
		}
		else {
			w->prev = NULL;
			compose_first = compose_last = w;
		}

		defaults_get_rect("ComposeWindow", w->deliver_frame);
	}
	else

		/* Open frame just in case ! */

		xv_set (w->deliver_frame,
			FRAME_CLOSED, FALSE,
			NULL);

	/* Default to encryption and signature */

	w->deliver_flags = DELIVER_SIGN|DELIVER_ENCRYPT;

	/* Default to remail if using a nym */

	nym = current_nym();
	if (strcmp(nym, our_userid)) 
		w->deliver_flags |= DELIVER_REMAIL;

	/* Clear the bits if told to by mailrc */

	if (find_mailrc("nodefaultsign"))
		w->deliver_flags &= ~DELIVER_SIGN;
	if (find_mailrc("nodefaultencrypt"))
		w->deliver_flags &= ~DELIVER_ENCRYPT;
	if (find_mailrc("nodefaultremail"))
		w->deliver_flags &= ~DELIVER_REMAIL;

	/* Check mailrc for logging options */

	log = find_mailrc("nodontlogmessages");
	dontlog = find_mailrc("dontlogmessages");

	/* If logging enabled, set log options */

	if (log && !dontlog) {
		w->deliver_flags |= DELIVER_LOG;
	}

	if (find_mailrc("log-raw")) {
		w->deliver_flags |= DELIVER_RAW;
	}

	/* Finally, set the panel item */

	update_log_item(w);

	/* And empty the text panel */

	textsw_erase(w->deliver_body_window,0,TEXTSW_INFINITY);

	/* All set ? Ok, show the frame ! */

	show_deliver_frame(w);

	/* And mark it as in use */

	w->in_use = TRUE;

	return	w;
}

/* Set up a compose window */

static	void	send_message()

{
	COMPOSE_WINDOW	*w;

	w = setup_send_window();

	xv_set(w->send_subject_item,
		PANEL_VALUE,"",
		NULL);

	xv_set(w->send_to_item,
		PANEL_VALUE,"",
		NULL);

	xv_set(w->send_cc_item,
		PANEL_VALUE,"",
		NULL);

	if (w->send_bcc_item)
		xv_set (w->send_bcc_item,
			PANEL_VALUE,"",
			NULL);
}

/* Reply to the sender without including the message */

static	reply_sender_no_include ()

{
	char	subject[256];
	COMPOSE_WINDOW	*w;

	update_random();

	if (!last_message_read)
		return;

	set_reply (last_message_read);

	w = setup_send_window();

	xv_set (w->send_cc_item,
		PANEL_VALUE, "",
		NULL);

	if (w->send_bcc_item)
		xv_set (w->send_bcc_item,
			PANEL_VALUE,"",
			NULL);
	
	if (last_message_read->subject) {
		if (strncasecmp(last_message_read->subject,"Re:",3)) {
			sprintf(subject,"Re: %s\n",
				last_message_read->subject);
		}
		else
			strcpy(subject,last_message_read->subject);

		xv_set(w->send_subject_item,
			PANEL_VALUE, subject,
			NULL);
	}
	else
		xv_set(w->send_subject_item,
			PANEL_VALUE,"",
			NULL);

	set_send_to(w);

	xv_set(w->deliver_body_window,
		TEXTSW_INSERTION_POINT,0,
		TEXTSW_FIRST_LINE,0,NULL);
}

static	char	begin_forward[] = "-- Begin forwarded message ---\n";
static	char	end_forward[] = "-- End forwarded message ---\n";

static	void	forward_message ()

{
	BUFFER	*b;
	char	subject[256];
	char	*c;
	COMPOSE_WINDOW	*w;

	update_random();

	if (!last_message_read)
		return;

	set_reply (last_message_read);

	w = setup_send_window();
	
	xv_set (w->send_cc_item,
		PANEL_VALUE, "",
		NULL);
	
	if (w->send_bcc_item)
		xv_set (w->send_bcc_item,
			PANEL_VALUE,"",
			NULL);

	if ((c = last_message_read->subject)) {
		c += strlen (c) - 6;
		if (strncasecmp(last_message_read->subject,"(fwd)",5)) {
			sprintf(subject,"%s (fwd)\n",
				last_message_read->subject);
		}
		else
			strcpy(subject,last_message_read->subject);

		xv_set(w->send_subject_item,
			PANEL_VALUE, subject,
			NULL);
	}
	else
		xv_set(w->send_subject_item,
			PANEL_VALUE,"",
			NULL);

	if (last_message_read->decrypted)
		b = last_message_read->decrypted;
	else
		b = message_contents(last_message_read);

	textsw_insert(w->deliver_body_window,
		begin_forward,
		strlen(begin_forward));

	textsw_insert(w->deliver_body_window,
		b->message,
		b->length);

	textsw_insert(w->deliver_body_window,
		end_forward,
		strlen(end_forward));

	xv_set(w->send_to_item,
		PANEL_VALUE,"",
		NULL);

	xv_set(w->deliver_body_window,
		TEXTSW_INSERTION_POINT,0,
		TEXTSW_FIRST_LINE,0,NULL);
}

/* Resend a message */

static	void	resend_proc ()

{
	BUFFER	*b;
	char	subject[256];
	char	*c, *s;
	COMPOSE_WINDOW	*w;

	update_random();

	if (!last_message_read)
		return;

	w = setup_send_window();

	if (last_message_read->to) {
		c = last_message_read->to;
		while (*c && *c != '<')
			c++;

		if (!*c) {
			xv_set (w->send_to_item,
				PANEL_VALUE, last_message_read->to,
				NULL);
		}
		else {
			s = subject;
			c++;
			while (*c && *c != '>')
				*s++ = *c++;
			*s = 0;

			xv_set (w->send_to_item,
				PANEL_VALUE, subject,
				NULL);
		}
	}
	
	xv_set (w->send_cc_item,
		PANEL_VALUE, "",
		NULL);
	
	if (w->send_bcc_item)
		xv_set (w->send_bcc_item,
			PANEL_VALUE,"",
			NULL);

	if ((c = last_message_read->subject)) {
		xv_set(w->send_subject_item,
			PANEL_VALUE, last_message_read->subject,
			NULL);
	}
	else
		xv_set(w->send_subject_item,
			PANEL_VALUE,"",
			NULL);

	if (last_message_read->decrypted)
		b = last_message_read->decrypted;
	else
		b = message_contents(last_message_read);

	textsw_insert(w->deliver_body_window,
		b->message,
		b->length);

	xv_set(w->deliver_body_window,
		TEXTSW_INSERTION_POINT,0,
		TEXTSW_FIRST_LINE,0,NULL);
}

/* Reply_to sender and include message */

static	void	reply_to_sender()

{
	int	i;
	byte	*mess;
	byte	*m;
	BUFFER	*b;
	char	subject[256];
	char	*indent;
	int	indent_l = 2;
	COMPOSE_WINDOW	*w;

	update_random();

	if (!last_message_read)
		return;

	set_reply (last_message_read);

	if (!(indent = find_mailrc("indentprefix"))) 
		indent = "> ";
	else
		indent_l = strlen (indent);

	w = setup_send_window();
	
	xv_set (w->send_cc_item,
		PANEL_VALUE, "",
		NULL);
	
	if (last_message_read->subject) {
		if (strncasecmp(last_message_read->subject,"Re:",3)) {
			sprintf(subject,"Re: %s\n",
				last_message_read->subject);
		}
		else
			strcpy(subject,last_message_read->subject);

		xv_set(w->send_subject_item,
			PANEL_VALUE, subject,
			NULL);
	}
	else
		xv_set(w->send_subject_item,
			PANEL_VALUE,"",
			NULL);

	set_send_to(w);

	if (last_message_read->decrypted)
		b = last_message_read->decrypted;
	else
		b = message_contents(last_message_read);

	mess = b->message;
	i = b->length;

	textsw_insert(w->deliver_body_window,
		last_message_read->sender,
		strlen(last_message_read->sender));

	textsw_insert(w->deliver_body_window,
		attribution_string,
		strlen(attribution_string));

	while (i > 0) {
		if (indent_l)
			textsw_insert(w->deliver_body_window,
				indent, indent_l);
		m = mess;
		while (i-- && *m && *m!= '\n')
			m++;

		textsw_insert(w->deliver_body_window,(char *)mess,
			m - mess + 1);

		mess = m+1;
	}

	xv_set(w->deliver_body_window,
		TEXTSW_INSERTION_POINT,0,
		TEXTSW_FIRST_LINE,0,NULL);
}

void	beep_display_window ()

{
	xv_set (main_frame,
		WIN_ALARM, 
		NULL);
}

/* Are we displaying a message ? */

int	is_displaying_message()

{
	DISPLAY_WINDOW	*w;

	w = display_first;

	while (w) {
		if (xv_get (w->display_frame, XV_SHOW))
			return TRUE;
		w=w->next;
	}

	return FALSE;
}

/* Set the footer on the display window */

void	set_display_footer(w,s)

DISPLAY_WINDOW	*w;
char	*s;

{
	if (w->display_frame)
		xv_set(w->display_frame,
			FRAME_LEFT_FOOTER,s,
			NULL);
}

static	Frame	header_frame;

/* Close the header window */

void	hide_header_frame()

{
	if (headerwin_sticky)
		return;

	if (header_frame) {
		xv_set(header_frame,
			FRAME_CMD_PUSHPIN_IN, FALSE,
			XV_SHOW, FALSE,
			NULL);
	}
}

/* Display the header */

static	void	display_header_proc(item, event)

Panel_item	item;
Event		*event;

{
	static	Textsw	header_window;
	DISPLAY_WINDOW	*w;
	MESSAGE	*m;

	/* find window from panel item */
	w = display_from_panel((Panel)xv_get(item, PANEL_PARENT_PANEL));

	/* find message from window */
	m = message_from_number(w->number);

	/* Prevent an unpinned display window from being popped down */
	xv_set(item, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);

	update_random();

	if (!m)
		return;

	if (!header_frame) {
		header_frame = (Frame) xv_create(main_frame, FRAME_CMD,
			XV_WIDTH, 640,
			XV_HEIGHT, 240,
			FRAME_SHOW_FOOTER, FALSE,
			FRAME_SHOW_RESIZE_CORNER, TRUE,
			FRAME_LABEL, "Message Header",
			NULL);

		/* Make zero height panel (zero generates warning, use 1) */
		xv_set((Panel) xv_get(header_frame, FRAME_CMD_PANEL),
			XV_HEIGHT, 1,
			NULL);

		header_window = (Textsw) xv_create(header_frame, TEXTSW,
			TEXTSW_IGNORE_LIMIT, TEXTSW_INFINITY,
			TEXTSW_MEMORY_MAXIMUM,1000000,
			NULL);

		defaults_get_rect ("HeaderWindow", header_frame);
	}

	xv_set(header_window, TEXTSW_READ_ONLY, FALSE,
			NULL);

	textsw_delete(header_window,
		0, TEXTSW_INFINITY);

	textsw_reset (header_window, 0, 0);

	textsw_insert(header_window,(char *)m->header->message,
		m->header->length);

	xv_set(header_window,TEXTSW_FIRST_LINE,0,
		TEXTSW_INSERTION_POINT, 0,
		TEXTSW_READ_ONLY, TRUE,
		NULL);

	xv_set(header_frame,
		XV_SHOW, TRUE,
		NULL);
}

/* Clear the footer on the display window */

void	clear_display_footer(w)

DISPLAY_WINDOW	*w;

{
	set_display_footer(w,"");
}

/* Catch resize events on the display frame */

static	void	display_frame_proc(w,e,a)

Xv_Window	w;
Event		*e;
Notify_arg	a;

{
	int	newh;
	DISPLAY_WINDOW	*dw;

	if (event_id(e) != WIN_RESIZE)
		return;

	dw = display_from_frame (w);

	if (!dw)
		return;

	newh = (int) xv_get (dw->display_frame, XV_HEIGHT);

	xv_set (dw->body_window, XV_HEIGHT, newh - 82, NULL);
	xv_set (dw->sig_window, 
		XV_HEIGHT, 32,
		XV_Y, newh - 32,
		NULL);
}

/* Show the add key button */

void	show_addkey (w)

DISPLAY_WINDOW	*w;

{
	xv_set (w->addkey_item,
		PANEL_INACTIVE, FALSE,
		NULL);
}

void	destroy_display_window(w)

DISPLAY_WINDOW	*w;

{
	if (w) {
		/* destroy display frame and all children */
		if (w->display_frame) {
			xv_destroy_safe(w->display_frame);
		}

		/* Unlink w from window list */

		if ((w == display_first) && (w == display_last)) {
			display_first = display_last = NULL;
		} else if (w == display_first) {
			display_first = w->next;
			display_first->prev = NULL;
		} else if (w == display_last) {
			display_last = w->prev;
			display_last->next = NULL;
		} else {
			w->prev->next = w->next;
			w->next->prev = w->prev;
		}
		free(w);
	}
}

display_frame_done_proc (f)

Frame	f;

{
	/* get the window pointer for that display_frame, */
	/* and destroy that window. */
	destroy_display_window(display_from_frame(f));
}

/* Create the display window */

DISPLAY_WINDOW	*create_display_window(m)

MESSAGE	*m;

{
	DISPLAY_WINDOW	*w;

	w = display_find_free (m->number);

	if (!w) {

		w = (DISPLAY_WINDOW *)malloc (sizeof (DISPLAY_WINDOW));

		w->display_frame = (Frame) xv_create(main_frame, FRAME_CMD,
			XV_WIDTH, 640,
			XV_X, xv_get(main_frame, XV_X)-5,
			XV_Y, xv_get(main_frame, XV_Y)+xv_get(main_frame, 
				XV_HEIGHT)+24,
			XV_HEIGHT, 400,
			XV_WIDTH, xv_get(main_frame, XV_WIDTH),
			FRAME_SHOW_FOOTER, TRUE,
			FRAME_SHOW_RESIZE_CORNER, TRUE,
			WIN_EVENT_PROC, display_frame_proc,
			FRAME_DONE_PROC, display_frame_done_proc,
			NULL);

		w->display_panel = (Panel) xv_get(w->display_frame, 
			FRAME_CMD_PANEL);

		xv_set (w->display_panel,
			XV_HEIGHT, 50,
			PANEL_LAYOUT, PANEL_HORIZONTAL,
			NULL);

		w->sender_item = (Panel_item) xv_create(w->display_panel,
			PANEL_TEXT,
			PANEL_LABEL_STRING,"Sender :",
			PANEL_VALUE_DISPLAY_LENGTH, 40,
			XV_X, 0,
			XV_Y, 5,
			NULL);

		(void) xv_create(w->display_panel,
			PANEL_BUTTON,
			PANEL_LABEL_STRING,"Header",
			PANEL_NOTIFY_PROC, display_header_proc,
			NULL);

		w->addkey_item = xv_create (w->display_panel, 
			PANEL_BUTTON,
			PANEL_LABEL_STRING, "Add Key",
			PANEL_INACTIVE, TRUE,
			PANEL_NOTIFY_PROC, add_key_proc,
			NULL);

		w->date_item = (Panel_item) xv_create(w->display_panel,
			PANEL_TEXT,
			PANEL_LABEL_STRING,"Date    :",
			PANEL_VALUE_DISPLAY_LENGTH, 40,
			XV_Y, 30,
			XV_X, 0,
			NULL);

		w->body_window = (Textsw) xv_create(w->display_frame, TEXTSW,
			XV_HEIGHT, 288,
			TEXTSW_IGNORE_LIMIT, TEXTSW_INFINITY,
			TEXTSW_MEMORY_MAXIMUM,1000000,
			NULL);

		w->sig_window = (Textsw) xv_create(w->display_frame, TEXTSW,
			TEXTSW_IGNORE_LIMIT, TEXTSW_INFINITY,
			XV_HEIGHT, 32,
			NULL);

		window_fit(w->body_window);
		window_fit(w->display_frame);

		w->next = NULL;
		if (display_last) {
			display_last->next = w;
			w->prev = display_last;
			display_last = w;
		}
		else {
			w->prev = NULL;
			display_first = display_last = w;
		}

		defaults_get_rect("MessageWindow", w->display_frame);
	}
	else {
		/* Open Window just in case */

		xv_set (w->display_frame,
			FRAME_CLOSED, FALSE,
			NULL);

		/* Clear add key item */

		xv_set (w->addkey_item,
			PANEL_INACTIVE, TRUE,
			NULL);
	}

	w->number = m->number;

	return	w;
}

/* Display the information about the sender */

void	display_sender_info(m,w)

MESSAGE	*m;
DISPLAY_WINDOW	*w;

{
	xv_set(w->sender_item,PANEL_READ_ONLY,FALSE,NULL);
	xv_set(w->sender_item,PANEL_VALUE,m->sender,NULL);
	xv_set(w->sender_item,PANEL_READ_ONLY,TRUE,NULL);

	if (m->header_date)
		xv_set(w->date_item,PANEL_VALUE,m->header_date,NULL);
	else
		xv_set(w->date_item,PANEL_VALUE,m->date,NULL);

	xv_set(w->date_item,PANEL_READ_ONLY,TRUE,NULL);
}

/* Clear the display window */

void	clear_display_window(w)

DISPLAY_WINDOW	*w;

{
	xv_set(w->sig_window, TEXTSW_READ_ONLY, FALSE,
			NULL);

	xv_set(w->body_window, TEXTSW_READ_ONLY, FALSE,
			NULL);

	textsw_delete(w->sig_window,
		0, TEXTSW_INFINITY);

	textsw_delete(w->body_window,
		0, TEXTSW_INFINITY);

	textsw_reset (w->sig_window, 0, 0);
	textsw_reset (w->body_window, 0, 0);
}

/* Set the display window to read-only */

void	lock_display_window(w)

DISPLAY_WINDOW	*w;

{
	xv_set(w->sig_window,TEXTSW_FIRST_LINE,0,
		TEXTSW_INSERTION_POINT, 0,
		TEXTSW_READ_ONLY, TRUE,
		NULL);

	xv_set(w->body_window,TEXTSW_FIRST_LINE,0,
		TEXTSW_INSERTION_POINT, 0,
		TEXTSW_READ_ONLY, TRUE,
		NULL);
}

/* Show the display window */

void	show_display_window(m,w)

MESSAGE	*m;
DISPLAY_WINDOW	*w;

{
	xv_set(w->display_frame,
			FRAME_LABEL, m->subject,
			XV_SHOW, TRUE,
			NULL);
}

/* Hide the display window */

void	close_display_windows()

{
	DISPLAY_WINDOW	*w;

	w = display_first;
	while (w) {
		if (w->display_frame)
			xv_set (w->display_frame,
				XV_SHOW, FALSE,
				NULL);
		w = w->next;
	}
}

/* Display the message body from the buffer */

void	display_message_body(b,w)

BUFFER	*b;
DISPLAY_WINDOW	*w;

{
	textsw_insert(w->body_window,b->message,b->length);
}

void	display_message_sig(b,w)

BUFFER	*b;
DISPLAY_WINDOW	*w;

{
	textsw_insert(w->sig_window,b->message,b->length);
}

void	delete_message_proc()

{
	MESSAGE	*om,*m;
	int	n = 0;
	int	y;

	update_random();

	/* Start at the head of the list */

	m = messages.start;

	while (m) {

		/* For each message, decrease list position by the number
		   of deleted messages. */

		if (!(m->flags & MESS_DELETED))
			m->list_pos -= n;

		/* Store message pointer in om, then set m to next message */

		om = m;
		m = m->next;

		if (om->flags & MESS_SELECTED) {

			delete_message(om);

			/* Then increment number of deleted messages */

			n++;
		}
		else
			if (n) {
				display_message_description(om);
			}
	}

	/* If any messages were deleted, clear the bottom lines of the list */

	if (n) {
		int	w;
		MESSAGE	*m;

		w = xv_get (list_canvas, XV_WIDTH);

		/* Remember - messages.end may be NULL ! */

		m = messages.end;

		while (m && (m->flags & MESS_DELETED))
			m = m->prev;

		if (m)
			y = m->list_pos;
		else
			y = 0;

		XClearArea(dpy, map_win, 0, y*LIST_DISPLACEMENT + 4,
			w, LIST_DISPLACEMENT*n, FALSE);
	}

	/* Move current selected message pointer if neccesary */

	if (last_message_read) {
		last_message_read->flags |= MESS_SELECTED;
		display_message_description (last_message_read);

		if (is_displaying_message())
			display_message(last_message_read);
	}
	else
		close_display_windows();

	update_message_list();
}

static	void	list_repaint_proc(canvas,pw,r_a)

Canvas  canvas;
Xv_window       pw;
Rectlist        *r_a;

{
	MESSAGE	*m;
	int32	min_pos, max_pos;

	list_window = pw;

	min_pos = (r_a->rl_bound.r_top / LIST_DISPLACEMENT) - 1;
	max_pos = ((r_a->rl_bound.r_top+r_a->rl_bound.r_height) / 
		LIST_DISPLACEMENT) + 1;

	gc = (GC) xv_get (canvas, XV_KEY_DATA, GC_KEY);
        dpy=(Display *)xv_get(pw,XV_DISPLAY);
        map_win=(Window)xv_get(pw,XV_XID);

	m = messages.start;

	while (m) {
		if (m->list_pos >= min_pos && m->list_pos <= max_pos &&
			!(m->flags & MESS_DELETED)) 
			display_message_description(m);
		m = m->next;
	}
}

static	void	list_notify_proc(window,event)

Xv_Window	*window;
Event	*event;

{
	MESSAGE	*m,*lm;
	int	n;

	update_random();

	if (event_is_up(event))
	switch (event_action(event)) {

		/* Middle button - select/deselect another message */

		case ACTION_ADJUST:
		n = (event_y(event) / LIST_DISPLACEMENT) + 1;

		m = messages.start;
		while (m && m->list_pos != n) {
			m = m->next;
		}

		if (m) {
			m->flags ^= MESS_SELECTED;
			display_message_description(m);
		}
		break;

		/* Left button - select one message, or display */

		case ACTION_SELECT:
		n = (event_y(event) / LIST_DISPLACEMENT) + 1;

		m = messages.start;
		while (m && m->list_pos != n) {
			m = m->next;
		}

		if (m) {

			/* If already selected, display it */

			if (m->flags & MESS_SELECTED) {
				set_main_footer ("Displaying message...");
				display_message (m);
				clear_main_footer ();
			}
			else {
				/* Clear all selected messages */

				lm = messages.start;

				while (lm) {
					if (lm->flags & MESS_SELECTED) {
						lm->flags &= ~MESS_SELECTED;
						display_message_description (lm);
					}
					lm = lm->next;
				}

				/* Then select this one ! */

				m->flags |= MESS_SELECTED;
				draw_box_round_message(m);
				last_message_read = m;
			}
		}
	}
}

void set_initial_scrollbar_position()

{
	int p = 1;
	MESSAGE *m = messages.start;
	int	v;

	v = xv_get(v_scroll, SCROLLBAR_VIEW_LENGTH);

	/* need to set SCROLLBAR_VIEW_LENGTH, otherwise scrollbar
	   may not be positioned correctly */

	xv_set(v_scroll, SCROLLBAR_VIEW_LENGTH, 1, 0);
	if (messages.number > v) {
		p = messages.number - v + 3 /* the +3 is due to error in value returned for SCROLLBAR_VIEW_LENGTH) */;
		xv_set (v_scroll, SCROLLBAR_VIEW_START, p-1, 0);
	}

	while (m) {
		if (m->status == MSTAT_NONE)
			if (m->list_pos < p) {
				p = m->list_pos;
				xv_set(v_scroll, SCROLLBAR_VIEW_START, p-1, 0);
			}
		m = m->next;
	}
	xv_set(v_scroll, SCROLLBAR_VIEW_LENGTH, v, 0);
}

void	update_menu_item()

{
	char	*s;
	int	i;
	char	temp [MAXPATHLEN];

	s = read_file_name();

	if (s && *s) {
		copy_to_nl (s, temp);
		s = temp;
	}
	else
		s = "";

	for (i = 0; i < 3; i++) {
		if (files_item[i]) {
			xv_set (files_item[i], 
				MENU_STRING, strdup(s),
				NULL);
		}
	}
}

void	x_xxxx_message_proc(item,event)

Panel_item	item;
Event		*event;

{
	update_random();

	if (event_is_down(event))
		switch (event_action(event)) {

			case ACTION_MENU:
			case ACTION_SELECT:
			update_menu_item();
			break;

		}
}

void	copy_choice_proc(m,mi)

Menu	m;
Menu_item	mi;

{
	char	*s;

	s = (char *)xv_get (mi, MENU_STRING);

	if (s) {
		update_menu_item ();

		/* Oops, update_menu_item() may change the menu string ! */

		s = (char *)xv_get (mi, MENU_STRING);
		set_file_name (s);
		copy_message_proc(s);
	}
}

void	move_choice_proc(m,mi)

Menu	m;
Menu_item	mi;

{
	char	*s;

	s = (char *)xv_get (mi, MENU_STRING);

	if (s) {
		update_menu_item ();

		/* Oops, update_menu_item() may change the menu string ! */

		s = (char *)xv_get (mi, MENU_STRING);
		set_file_name (s);
		move_message_proc(s);
	}
}

void	load_choice_proc(m,mi)

Menu	m;
Menu_item	mi;

{
	char	*s;

	s = (char *)xv_get (mi, MENU_STRING);
	
	if (s) {
		update_menu_item ();

		/* Oops, update_menu_item() may change the menu string ! */

		s = (char *)xv_get (mi, MENU_STRING);
		set_file_name (s);
		load_file_proc(s);
	}
}

static	void	clear_passphrase_proc()

{
	destroy_passphrase (TRUE);
}

static	void	(*menus_proc[3])() = {

	move_choice_proc,
	copy_choice_proc,
	load_choice_proc

};

void	close_all_windows()

{
	COMPOSE_WINDOW	*w;

	close_display_windows ();

	w = compose_first;

	while (w) {
		close_deliver_window (w);
		w = w->next;
	}

	xv_set (main_frame,
		FRAME_CLOSED, TRUE,
		NULL);
}


/*
 * file_menu_action_proc - Set the file_name_item based on the menu item
 */
void
file_menu_action_proc(menu, menu_item)
Menu    menu;
Menu_item       menu_item;
{
  char	  folder_ending[MAXPATHLEN];
  char	  *menu_string, *menu_data, *s, *folder_dir, *home;

  menu_string = (char *)xv_get(menu_item, MENU_STRING);
  menu_data = (char *)xv_get(menu, MENU_CLIENT_DATA);
  home = getenv("HOME");
  
  if ((folder_dir = (char *)find_mailrc("folder")) && (*menu_data != '.'))
	  {
	    s = menu_data+strlen(folder_dir) +1;
	    if (*folder_dir != '/')
	      {
		s += strlen(home);
		if (*s == '/')
		    s += 1;
	      }
	      
	    sprintf(folder_ending, "+/%s/%s", s, menu_string);
	    if (s[0] == '\0')
	      {
		sprintf(folder_ending, "+/%s", menu_string);
	      }
	     else
	      {
		sprintf(folder_ending, "+/%s/%s", s, menu_string);
	      }
	  }
	 else
	  {
	    if (menu_data+1 != '\0')
	      {
		sprintf(folder_ending, "%s/%s", menu_data, menu_string);
	      }
	     else
	      {
		strcpy(folder_ending, menu_string);
	      }
	  }

    xv_set(file_name_item,
        PANEL_VALUE,      folder_ending,
        NULL);
}

/*
 * return an allocated char * that points to the last item in a path.
 */
char *
getfilename(path)
char *path;
{
    char *p;

    if ((p = rindex(path, '/')))
        p++;
    else
        p = path;
    return strcpy(malloc(strlen(p)+1), p);
}

/* gen_pullright() is called in the following order:
 *   Pullright menu needs to be displayed. (MENU_PULLRIGHT)
 *   Menu is about to be dismissed (MENU_DISPLAY_DONE)
 *      User made a selection (before menu notify function)
 *      After the notify routine has been called.
 * The above order is done whether or not the user makes a
 * menu selection.
 */
Menu
gen_pullright(mi, op)
Menu_item mi;
Menu_generate op;
{
    Menu menu;
    Menu_item new;
    char buf[MAXPATHLEN];

    if (op == MENU_DISPLAY) {
        menu = (Menu)xv_get(mi, MENU_PARENT);
        sprintf(buf, "%s/%s",
            (char *)xv_get(menu, MENU_CLIENT_DATA), (char *)xv_get(mi, MENU_STRING));
        /* get old menu and free it -- we're going to build another */
        if ((menu = (Menu)xv_get(mi, MENU_PULLRIGHT))) {
            free((char *)xv_get(menu, MENU_CLIENT_DATA));
            xv_destroy(menu);
        }
        if ((new = add_path_to_menu(buf))) {
            menu = (Menu)xv_get(new, MENU_PULLRIGHT);
            xv_destroy(new);
            return menu;
        }
    }
    if (!(menu = (Menu)xv_get(mi, MENU_PULLRIGHT)))
            menu = (Menu)xv_create(XV_NULL, MENU,
                MENU_STRINGS, "Couldn't build a menu.", NULL,
                NULL);
    return menu;
}

/*
 * The path passed in is scanned via readdir().  For each file in the
 * path, a menu item is created and inserted into a new menu.  That
 * new menu is made the PULLRIGHT_MENU of a newly created panel item
 * for the path item originally passed it.  Since this routine is
 * recursive, a new menu is created for each subdirectory under the
 * original path.
 */
Menu_item
add_path_to_menu(path)
char *path;
{
    DIR                 *dirp;
    struct dirent       *dp;
    struct stat         s_buf;
    Menu_item           mi;
    Menu                next_menu;
    char                buf[MAXPATHLEN];
    static int          recursion;

    /* don't add a folder to the list if user can't read it */
    if (stat(path, &s_buf) == -1 || !(s_buf.st_mode & S_IREAD))
        return XV_NULL;
    if (s_buf.st_mode & S_IFDIR) {
        int cnt = 0;
        if (!(dirp = opendir(path)))
            /* don't bother adding to list if we can't scan it */
            return XV_NULL;
        if (recursion)
            return (Menu_item)-1;
        recursion++;
        next_menu = (Menu)xv_create(XV_NULL, MENU, NULL);
        while ((dp = readdir(dirp)))
            if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
                (void) sprintf(buf, "%s/%s", path, dp->d_name);
                mi = add_path_to_menu(buf);
                if (!mi || mi == (Menu_item)-1) {
                    int do_gen_pullright = (mi == (Menu_item)-1);
                    /* unreadable file or dir - deactivate item */
                    mi = (Menu_item)xv_create(XV_NULL, MENUITEM,
                        MENU_STRING,  getfilename(dp->d_name),
                        MENU_RELEASE,
                        MENU_RELEASE_IMAGE,
                        NULL);
                    if (do_gen_pullright)
                        xv_set(mi,
                            MENU_GEN_PULLRIGHT, gen_pullright,
                            NULL);
                    else
                        xv_set(mi, MENU_INACTIVE, TRUE, NULL);
                }
                xv_set(next_menu, MENU_APPEND_ITEM, mi, NULL);
                cnt++;
            }
        closedir(dirp);
        mi = (Menu_item)xv_create(XV_NULL, MENUITEM,
            MENU_STRING,        getfilename(path),
            MENU_RELEASE,
            MENU_RELEASE_IMAGE,
            MENU_NOTIFY_PROC,   file_menu_action_proc,
            NULL);
        if (!cnt) {
            xv_destroy(next_menu);
            /* An empty or unsearchable directory - deactivate item */
            xv_set(mi, MENU_INACTIVE, TRUE, NULL);
        } else {
            xv_set(next_menu,
                MENU_TITLE_ITEM, strcpy(malloc(strlen(path)+1), path),
                MENU_CLIENT_DATA, strcpy(malloc(strlen(path)+1), path),
                NULL);
            xv_set(mi, MENU_PULLRIGHT, next_menu, NULL);
        }
        recursion--;
        return mi;
    }
    return (Menu_item)xv_create(XV_NULL, MENUITEM,
        MENU_STRING,            getfilename(path),
        MENU_RELEASE,
        MENU_RELEASE_IMAGE,
        MENU_NOTIFY_PROC,       file_menu_action_proc,
        NULL);
}

static 	void	nym_notify (menu, menu_item)

Menu    menu;
Menu_item       menu_item;

{
	char	*nym;

	nym = (char *)xv_get (menu_item, MENU_STRING);
	set_current_nym (nym);

	if (nym_item) {
		xv_set (nym_item, PANEL_READ_ONLY, FALSE, NULL);
		xv_set (nym_item, PANEL_VALUE, nym, NULL);
		xv_set (nym_item, PANEL_READ_ONLY, TRUE, NULL);
	}
}

static void	main_frame_events (win, event, arg)

Xv_Window	win;
Event		*event;
Notify_arg	arg;

{
	switch (event_action(event)) {

		case ACTION_CLOSE:
		show_normal_icon();
		break;
		
	}
}

void	setup_ui(level,argc,argv)

int	level;

{
	int	i, l;
	MESSAGE	*m;
	GC	gc;
	Display	*dpy;
	Window	root_win;
	XGCValues	gcvalues;
	unsigned	long	*xcolors;
	char	app_name[128];
	char    folder_dir[MAXPATHLEN];
	Menu_item	mi;
	char	*files,*f,*fs, *folder;
	Menu	filebutton_menu;
	Menu	button_menu, sort_menu, undelete_menu, file_menu;
	struct itimerval	timer;

	sprintf(app_name,"%s %s [ Security Level %d ]",prog_name,
		prog_ver,level);

	load_xresources();
	xv_init(XV_INIT_ARGS, argc, argv, NULL);
	find_xresources();

	main_frame = (Frame) xv_create(NULL, FRAME,
		XV_HEIGHT, 320,
		XV_WIDTH, 800,
		FRAME_LABEL, app_name,
		FRAME_SHOW_FOOTER, TRUE,
#ifndef START_OPEN
		FRAME_CLOSED, TRUE,
#endif
		WIN_EVENT_PROC, main_frame_events,
		WIN_CONSUME_EVENTS,
			WIN_UNMAP_NOTIFY,
			NULL,
		NULL);

	top_panel = (Panel) xv_create(main_frame, PANEL,
		XV_HEIGHT, 64,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		NULL);

	filebutton_menu = (Menu) xv_create(XV_NULL, MENU,
		MENU_ITEM,
			MENU_STRING, "Load In-Box",
			MENU_NOTIFY_PROC, inbox_proc,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Save Changes",
			MENU_NOTIFY_PROC, save_changes_proc,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Done",
			MENU_NOTIFY_PROC, done_proc,
			NULL,
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "File",
		PANEL_ITEM_MENU, filebutton_menu,
		NULL);

	sort_menu = (Menu) xv_create (XV_NULL, MENU,
		MENU_ITEM,
			MENU_STRING, "Time and Date",
			MENU_NOTIFY_PROC, sort_by_time,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Sender",
			MENU_NOTIFY_PROC, sort_by_sender,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Subject",
			MENU_NOTIFY_PROC, sort_by_subject,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Size",
			MENU_NOTIFY_PROC, sort_by_size,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Status",
			MENU_NOTIFY_PROC, sort_by_status,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Message Number",
			MENU_NOTIFY_PROC, sort_by_number,
			NULL,
		NULL);

	button_menu = (Menu) xv_create(XV_NULL, MENU,
		MENU_ITEM,
			MENU_STRING, "Previous",
			MENU_NOTIFY_PROC, prev_message_proc,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Next",
			MENU_NOTIFY_PROC, next_message_proc,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Sort By",
			MENU_PULLRIGHT, sort_menu,
			NULL,
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "View",
		PANEL_ITEM_MENU, button_menu,
		NULL);

	undelete_menu = (Menu) xv_create (XV_NULL, MENU,
		MENU_ITEM,
			MENU_STRING, "Last",
			MENU_NOTIFY_PROC, undelete_last_proc,
			NULL,
		MENU_ITEM,
			MENU_STRING, "From List",
			MENU_INACTIVE, TRUE,
			NULL,
		NULL
	);

	button_menu = (Menu) xv_create(XV_NULL, MENU,
		MENU_ITEM,
			MENU_STRING, "Cut",
			MENU_INACTIVE, TRUE,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Copy",
			MENU_INACTIVE, TRUE,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Delete",
			MENU_INACTIVE, TRUE,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Undelete",
			MENU_PULLRIGHT, undelete_menu,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Clear Passphrase",
			MENU_NOTIFY_PROC, clear_passphrase_proc,
			NULL,
#ifdef USE_FLOPPY
		MENU_ITEM,
			MENU_STRING, "Eject Floppy",
			MENU_NOTIFY_PROC, eject_floppy,
			NULL,
#endif
#ifdef PGPTOOLS
		MENU_ITEM,
			MENU_STRING, "Reseed Random",
			MENU_NOTIFY_PROC, reseed_random_generator,
			NULL,
#endif
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Edit",
		PANEL_ITEM_MENU, button_menu,
		NULL);

	button_menu = (Menu) xv_create (XV_NULL, MENU,
		MENU_ITEM, 
			MENU_STRING, "New",
			MENU_NOTIFY_PROC, send_message,
			NULL,
		MENU_ITEM, 
			MENU_STRING, "Reply",
			MENU_NOTIFY_PROC, reply_to_sender,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Forward",
			MENU_NOTIFY_PROC, forward_message,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Resend",
			MENU_NOTIFY_PROC, resend_proc,
			NULL,
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Compose",
		PANEL_ITEM_MENU, button_menu,
		NULL);

	button_menu = (Menu) xv_create (XV_NULL, MENU,
		MENU_ITEM,
			MENU_STRING, "Save Changes And Quit",
			MENU_NOTIFY_PROC, save_and_quit_proc,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Quit, Ignore Changes",
			MENU_NOTIFY_PROC, quit_proc,
			NULL,
		NULL
	);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Quit",
		PANEL_ITEM_MENU, button_menu,
		NULL);


	if ((folder = (char *)find_mailrc("folder")))
	  {
	    struct stat	  s_buf;
	    if (*folder != '/')
	      {
		char * home;
		home = getenv ("HOME");
		sprintf(folder_dir, "%s/%s", home, folder);
	      }
	     else
	      {
		strcpy(folder_dir, folder);
	      }
	    /* if directory doesn't exist set to cwd */
	    if (stat(folder_dir, &s_buf) == -1 || !(s_buf.st_mode & S_IREAD))
		strcpy(folder_dir, ".");
	  }
	 else
	  {
	    strcpy(folder_dir, ".");
	  }
	  
	mi = add_path_to_menu(folder_dir);
	if (mi)
	  {
	    file_menu = (Menu)xv_get(mi, MENU_PULLRIGHT);
	    /* We no longer need the item since we have the menu from it */
	    xv_destroy(mi);
	    (void) xv_create (top_panel, PANEL_ABBREV_MENU_BUTTON,
		    PANEL_LABEL_STRING, "Mail File:",
		    PANEL_ITEM_MENU, file_menu,
		    NULL);
	  }
	  

	file_name_item = (Panel_item) xv_create (top_panel, PANEL_TEXT,
		PANEL_VALUE_DISPLAY_LENGTH, 20,
		NULL);

	if (!layout_compact) 
		xv_set (file_name_item, PANEL_VALUE_DISPLAY_LENGTH, 35,
			NULL);

	xv_set (top_panel, PANEL_LAYOUT, PANEL_VERTICAL, NULL);
	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Done",
		PANEL_NOTIFY_PROC, done_proc,
		NULL);
	xv_set (top_panel, PANEL_LAYOUT, PANEL_HORIZONTAL, NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Next",
		PANEL_NOTIFY_PROC, next_message_proc,
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Delete",
		PANEL_NOTIFY_PROC, delete_message_proc,
		NULL);

	button_menu = (Menu) xv_create(XV_NULL, MENU,
		MENU_ITEM,
			MENU_STRING, "Print Decrypted/Authenticated",
			MENU_NOTIFY_PROC, print_cooked_proc,
			NULL,
		MENU_ITEM,
			MENU_STRING, "Print Encrypted/Clearsigned",
			MENU_NOTIFY_PROC, print_raw_proc,
			NULL,
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Print",
		PANEL_ITEM_MENU, button_menu,
		NULL);

	button_menu = (Menu) xv_create(NULL, MENU,
		MENU_ITEM,
			MENU_STRING, "To Sender",
			MENU_NOTIFY_PROC, reply_sender_no_include,
			NULL,
		MENU_ITEM,
			MENU_STRING, "To Sender, Include",
			MENU_NOTIFY_PROC, reply_to_sender,
			NULL,
		MENU_ITEM,
			MENU_STRING, "To All",
			MENU_INACTIVE, TRUE,
			NULL,
		MENU_ITEM,
			MENU_STRING, "To All, Include",
			MENU_INACTIVE, TRUE,
			NULL,
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Reply",
		PANEL_ITEM_MENU, button_menu,
		NULL);

	files = find_mailrc("filemenu2");

	for (i = 0; i < 3 ; i++)  {
		files_menu[i] = (Menu) xv_create (XV_NULL,
			MENU,
			NULL);

		files_item[i] = (Menu_item) xv_create(XV_NULL,
			MENUITEM,
			MENU_STRING, strdup(""),
			MENU_NOTIFY_PROC, menus_proc[i],
			MENU_RELEASE,
			MENU_RELEASE_IMAGE,
			NULL);

		xv_set(files_menu[i], 
			MENU_APPEND_ITEM, files_item[i],
			NULL);
	}

	if (files) {
		files = strdup(files);

		f = files;
		while (*f) {
			fs = f;
			while (*f && *f != ' ')
				f++;
			if (*f)
				*f++ = 0;

			for (i = 0; i < 3; i++) {
				mi = (Menu_item) xv_create(XV_NULL,
					MENUITEM,
					MENU_STRING, strdup(fs),
					MENU_RELEASE,
					MENU_RELEASE_IMAGE,
					MENU_NOTIFY_PROC, menus_proc[i],
					NULL);

				xv_set(files_menu[i], 
					MENU_APPEND_ITEM, mi,
					NULL);
			}
		}

		free (files);
	}

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Move",
		PANEL_NOTIFY_PROC, x_xxxx_message_proc,
		PANEL_ITEM_MENU, files_menu[0],
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Copy",
		PANEL_NOTIFY_PROC, x_xxxx_message_proc,
		PANEL_ITEM_MENU, files_menu[1],
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Load",
		PANEL_NOTIFY_PROC, x_xxxx_message_proc,
		PANEL_ITEM_MENU, files_menu[2],
		NULL);

	(void) xv_create (top_panel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Props",
		PANEL_ITEM_MENU,
			xv_create(XV_NULL, MENU, MENU_ITEM,
				MENU_STRING, "Save layout",
				MENU_NOTIFY_PROC, save_xresources,
				NULL, NULL),
		NULL);

	button_menu = (Menu) xv_create(XV_NULL, MENU, NULL);

	l = nym_count ();
	for (i = 0 ; i < l; i++) {
		mi = (Menu_item) xv_create (XV_NULL, MENUITEM,
			MENU_STRING, nym_name (i),
			MENU_NOTIFY_PROC, nym_notify, 
			NULL);
		xv_set (button_menu, MENU_APPEND_ITEM, mi, NULL);
	}

	if (layout_compact) {
		mi = (Menu_item) xv_create (XV_NULL, MENUITEM,
			MENU_STRING, "Nym",
			MENU_PULLRIGHT, button_menu,
			NULL);
		xv_set (filebutton_menu, MENU_INSERT, 2, mi, NULL);
	}
	else {
		(void) xv_create (top_panel, PANEL_ABBREV_MENU_BUTTON,
			PANEL_LABEL_STRING, "Nym:",
			PANEL_ITEM_MENU, button_menu,
			NULL);
		nym_item = (Panel_item) xv_create (top_panel, PANEL_TEXT,
			PANEL_VALUE_DISPLAY_LENGTH, 12,
			PANEL_VALUE, nym_name (0),
			PANEL_READ_ONLY, TRUE,
			NULL);
	}

	list_canvas = (Canvas) xv_create(main_frame,
		CANVAS,
		CANVAS_AUTO_SHRINK, FALSE,
		CANVAS_AUTO_EXPAND, TRUE,
		CANVAS_REPAINT_PROC, list_repaint_proc,
		WIN_RETAINED, FALSE,
		NULL);

	if (fixedwidthfont)
		list_font = (Xv_font) xv_find(main_frame, FONT, FONT_NAME,
			fixedwidthfont, NULL);
	else
		list_font = (Xv_font) xv_find(main_frame, FONT, FONT_FAMILY,
			FIXED_WIDTH_FONT, NULL);

	/* If we failed to open it, get the default font */

	if (!list_font) {
		list_font = (Xv_font) xv_get (main_frame,
			XV_FONT);
	}

	/* Oh shit, we failed to find a font at all ! */

	if (!list_font) {
		printf("Failed to load suitable font !\n");
		exit (1);
	}

	dpy = (Display *) xv_get(main_frame, XV_DISPLAY);
	root_win = RootWindow (dpy, DefaultScreen(dpy));

	xcolors = (unsigned long *) xv_get(main_frame, WIN_X_COLOR_INDICES);

	gcvalues.font = xv_get(list_font, XV_XID);
	gcvalues.foreground = xcolors[xv_get(list_canvas,WIN_FOREGROUND_COLOR)];
	gcvalues.background = xcolors[xv_get(list_canvas,WIN_BACKGROUND_COLOR)];
	gcvalues.graphics_exposures = FALSE;

	gc = XCreateGC(dpy, RootWindow(dpy, DefaultScreen(dpy)),
		GCForeground|GCBackground|GCFont|GCGraphicsExposures,
		&gcvalues);

	xv_set(list_canvas, XV_KEY_DATA, GC_KEY, gc, NULL);

	/* Pixels per unit should be computed from something
	 * but I'm not sure what.  KP
	 */ 
	v_scroll = (Scrollbar) xv_create(list_canvas,
		SCROLLBAR,
		SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
		SCROLLBAR_SPLITTABLE, FALSE,
		SCROLLBAR_PIXELS_PER_UNIT, 16, 
		NULL);

	xv_set (canvas_paint_window(list_canvas), 
		WIN_EVENT_PROC, list_notify_proc,
		WIN_CONSUME_EVENTS, WIN_MOUSE_BUTTONS, NULL,
		WIN_RETAINED, FALSE,
		NULL);

#ifndef USE_XBM
	newmail_icon_image = (Server_image) xv_create(XV_NULL,SERVER_IMAGE,
		XV_WIDTH, 64,
		XV_HEIGHT, 64,
		SERVER_IMAGE_BITS, newmail_icon_bits,
		NULL);

	icon_image = (Server_image) xv_create(XV_NULL,SERVER_IMAGE,
		XV_WIDTH, 64,
		XV_HEIGHT, 64,
		SERVER_IMAGE_BITS, icon_bits,
		NULL);

#ifdef NSA_ICON
	icon_image_mask = (Server_image) xv_create(XV_NULL,SERVER_IMAGE,
		XV_WIDTH, 64,
		XV_HEIGHT, 64,
		SERVER_IMAGE_BITS, icon_bits_mask,
		NULL);
#endif
#else /* USE_XBM */
	icon_pm = XCreatePixmapFromBitmapData(dpy,
			root_win, icon_bits, icon_width, icon_height,
			1, 0, 1);

	newmail_icon_pm = XCreatePixmapFromBitmapData(dpy,
			root_win, newmail_icon_bits, newmail_icon_width,
			newmail_icon_height, 1, 0, 1);

	mask_icon_pm = XCreatePixmapFromBitmapData(dpy,
			root_win, mask_icon_bits, mask_icon_width,
			mask_icon_height, 1, 0, 1);

        icon_image = (Server_image) xv_create(NULL,SERVER_IMAGE,
				SERVER_IMAGE_PIXMAP, icon_pm, NULL);
 
	newmail_icon_image = (Server_image) xv_create(NULL,SERVER_IMAGE,
				SERVER_IMAGE_PIXMAP, newmail_icon_pm, NULL);

	icon_image_mask = (Server_image) xv_create(NULL,SERVER_IMAGE,
				SERVER_IMAGE_PIXMAP, mask_icon_pm,
				NULL);

#endif /* USE_XBM */

	icon = (Icon) xv_create(main_frame, ICON,
		ICON_IMAGE, icon_image,
#ifdef NSA_ICON
		ICON_MASK_IMAGE, icon_image_mask,
#endif
		ICON_TRANSPARENT, TRUE,
		NULL);

	newmail_icon = (Icon) xv_create(main_frame, ICON,
		ICON_IMAGE, newmail_icon_image,
		ICON_TRANSPARENT, TRUE,
		NULL);

	xv_set(main_frame,
		FRAME_ICON, icon,
		NULL);

	m = messages.start;
	i = 1;
	l = 1;
	while (m) {
		m->list_pos = l;
		m->number = i;

		if (!(m->flags & MESS_DELETED))
			l++;
		else
			delete_message (m);

		if (m->status == MSTAT_NONE)
			show_newmail_icon();

		set_message_description(m);
		m = m->next;
		i++;
	}

	update_message_list();

	window_fit(main_frame);

	f = find_mailrc("retrieveinterval");
	if (!f) {
		timer.it_value.tv_sec = DEFAULT_CHECK_TIME;
	}
	else {
		timer.it_value.tv_sec = atoi(f);
	}

	timer.it_value.tv_usec = 0;
	timer.it_interval = timer.it_value;

	if (timer.it_value.tv_sec > 0) {
		notify_set_itimer_func(main_frame,
			(Notify_func) check_for_new_mail,
			ITIMER_REAL,
			&timer,
			NULL);
	}

	if (layout_compact)
		xv_set(main_frame, XV_WIDTH, 640, XV_HEIGHT, 280, NULL);

	/* set positions and sizes based on X resources */
	defaults_get_rect("ListWindow", main_frame);

	set_initial_scrollbar_position ();

	xv_main_loop(main_frame);
}

void	shutdown_ui()

{
	xv_destroy_safe(main_frame);
}

show_busy ()

{
	COMPOSE_WINDOW	*w;
	DISPLAY_WINDOW	*dw;

	xv_set (main_frame, FRAME_BUSY, TRUE, NULL);

	dw = display_first;
	while (dw) {
		xv_set (dw->display_frame, FRAME_BUSY, TRUE, NULL);
		dw = dw->next;
	}

	if (pass_frame)
		xv_set (pass_frame, FRAME_BUSY, TRUE, NULL);

	w = compose_first;
	while (w) {
		xv_set (w->deliver_frame, FRAME_BUSY, TRUE, NULL);
		w = w->next;
	}
}

clear_busy()

{
	COMPOSE_WINDOW	*w;
	DISPLAY_WINDOW	*dw;

	xv_set (main_frame, FRAME_BUSY, FALSE, NULL);

	dw = display_first;
	while (dw) {
		xv_set (dw->display_frame, FRAME_BUSY, FALSE, NULL);
		dw = dw->next;
	}

	if (pass_frame)
		xv_set (pass_frame, FRAME_BUSY, FALSE, NULL);

	w = compose_first;
	while (w) {
		xv_set (w->deliver_frame, FRAME_BUSY, FALSE, NULL);
		w = w->next;
	}
}

/* handling of X resources */

static void defaults_save_string(res, val)

char	*res;
char	*val;

{
	char what[128];

	sprintf(what, "%s.%s", APPL, res);
	if (val)
		defaults_set_string(what, val);
}

static char* defaults_load_string(res)

char	*res;

{
	char	name[256];
	char	clas[256];

	sprintf(clas, "%s.%s", APPL, res);
	strcpy (name, clas);
	name[0] = tolower (name[0]);
	return defaults_get_string(name, clas, NULL);
}

static int defaults_load_boolean(res)

char	*res;

{
	char name[128];
	char clas[128];

	sprintf(clas, "%s.%s", APPL, res);
	strcpy(name, clas);
	name[0] = tolower(name[0]);

	return defaults_get_boolean(name, clas, 0);
}

static void defaults_get_rect(res, frame)

char	*res;
Frame	frame;

{
	Rect	rect;
	char	*geom;
	char	what[128];

	if (frame) {
		sprintf(what, "%s.Geometry", res);
		if ((geom = defaults_load_string(what)) != NULL) {
			sscanf(geom, "%hdx%hd+%hd+%hd", &rect.r_width, 
				&rect.r_height,
				&rect.r_left, &rect.r_top);
			frame_set_rect(frame, &rect);
#ifdef linux
		/* XView/ol[v]wm has a bug when the frame for a window is
		  placed at a fixed position, but the window is initially
		  mapped as an icon. The bug is circumvented by moving the
		  frame around a bit */

			if ((frame == main_frame) && 
				(xv_get(frame, FRAME_CLOSED))) {
				xv_set(frame, XV_X, rect.r_left-1, NULL);
				xv_set(frame, XV_Y, rect.r_top-1, NULL);
				xv_set(frame, XV_X, rect.r_left, NULL);
				xv_set(frame, XV_Y, rect.r_top, NULL);
			}
#endif
		}

	}
}

static void defaults_save_rect(res, frame)

char	*res;
Frame	frame;

{
	Rect	rect;
	char	geom[128];
	char	what[128];

	if (frame) {
		frame_get_rect(frame, &rect);
		sprintf(geom, "%hdx%hd+%hd+%hd", rect.r_width,
			rect.r_height,
			rect.r_left, rect.r_top);
		sprintf(what, "%s.Geometry", res);
		defaults_save_string(what, geom);
	}
}

static char *find_dotfile(dotfile)

char	*dotfile;

{
	char	*home;
	char	*filename = NULL;

	home = getenv("HOME");
	if (home != NULL && home[0] != '\0') {
		/* try $HOME/dotfile */

		filename = (char *) malloc((unsigned int)(strlen(home)+1+strlen(dotfile)+1));

		if (filename == NULL)
			return NULL;

		sprintf(filename, "%s/%s", home, dotfile);
		if (access(filename, F_OK) == -1) {
			free(filename);
			return NULL;
		}

		/* found it */
		return filename;
	}

	return NULL;
}

static void load_xresources()

{
	char	*xapplresdir, res_file[MAXPATHLEN + 10];

	if ((xapplresdir = getenv("XAPPLRESDIR")) != NULL) {
		sprintf(res_file, "%s/%s", xapplresdir, APPL);
		if (access(res_file, R_OK) == 0) {
			defaults_load_db(res_file);
		}
	}
	else {
		if ((xapplresdir = getenv("OPENWINHOME")) != NULL) {
			sprintf(res_file, "%s/lib/app-defaults/%s", 
				xapplresdir, APPL);
			if (access(res_file, R_OK) == 0) {
				defaults_load_db(res_file);
			}
		}
	}
}

static void find_xresources()

{
	char	*str;

	str = defaults_load_string("ListWindow.Font");
	if (str)
		fixedwidthfont = strdup(str);

	str = defaults_load_string("ListWindow.Layout");
#ifdef COMPACT
	layout_compact = !str || (strcmp(str, "Compact") == 0);
#else
	if (str)
		layout_compact = (strcmp(str, "Compact") == 0);
#endif

	headerwin_sticky = defaults_load_boolean("HeaderWindow.Sticky");
}

static void set_xresources()

{
	Frame	f, df;

	if (compose_first)
		f = compose_first->deliver_frame;
	else
		f = NULL;

	if (display_first)
		df = display_first->display_frame;
	else
		df = NULL;

	defaults_save_rect("ListWindow", main_frame);
	defaults_save_rect("MessageWindow", df);
	defaults_save_rect("HeaderWindow", header_frame);
	defaults_save_rect("ComposeWindow", f);
}

static void save_xresources()

{
	char *filename=NULL;

	filename = find_dotfile(XRESOURCES);
	if (filename) {
		set_xresources();
		defaults_store_db(filename);
		free(filename);
	}
}

