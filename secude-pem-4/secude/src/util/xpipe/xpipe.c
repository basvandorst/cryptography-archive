/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/termsw.h>
#include <xview/xv_xrect.h>
#include <xview/canvas.h>
#include <xview/icon_load.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/seln.h>
#include <xview/notice.h>
#include <xview/font.h>

#include "xpipe.h"




int popup_dy = 350;
int popup_dx = 600;
int popup_y = 500;
int popup_x = 200;
int xpipe_dy = 78;
int xpipe_dx = 93;
int xpipe_y = 200;
int xpipe_x = 90;
int tty_dy = 240;
int tty_dx = 600;
int tty_y = -1;
int tty_x = -1;


Attr_attribute	INSTANCE;


Xv_Server       server;

char *label, *string;
char *xpipe_label = "XPipe 1.0";
int fddevtty;

xpipe_window_objects	*xpipe_window;
xpipe_xv_tty_window_objects *xpipe_tty_window;
static
xpipe_xv_text_window_objects	*xpipe_xv_text_window = 0;


Menu_Commands *menu_commands, *menu_commands_tmp, *menu_commands1 = 0;

int verbose = 0;
FILE *kb;
char *rc_file;
char slave_string[200];
int ttyicon = 0;







void
main(argc, argv)
	int		argc;
	char		**argv;
{

	char  *newargv[100];
	int newargc;

	int sizemode = 0, n;

	strcpy(slave_string, " ");

	newargc = 0;
	for(n = 0; n < argc; n++) {
		if(!strcmp(argv[n], "-control_window")) {
			sizemode = 1;
		}
		else if(!strcmp(argv[n], "-label")) {
			xpipe_label = argv[++n];
		}
		else if(!strcmp(argv[n], "-rcfile")) {
			sizemode = 2;
		}
		else if(!strcmp(argv[n], "-control_icon")) {
			ttyicon = 1;
		}
		else if(!strcmp(argv[n], "-output_window")) {
			sizemode = 3;
		}
		else if(!strcmp(argv[n], "-verbose")) {
			verbose = 1;
		}
		else if(!strcmp(argv[n], "-VERBOSE")) {
			verbose = 2;
		}
		else if(!strcmp(argv[n], "help") || !strcmp(argv[n], "-help") || !strcmp(argv[n], "-h")) {
			fprintf(stderr, "xpipe [-verbose/-VERBOSE] [-label <label>] [-control_icon] [XView args] \n\t[-rcfile [ -Wp <x> <y> ] [ -Ws <x> <y> ]] \n\t[-control_window [ -Wp <x> <y> ] [ -Ws <x> <y> ]]\n\t[-output_window [XView args]].\n\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "-label          : The label of the window frame.\n");
			fprintf(stderr, "-control_icon   : The control window can made iconified.\n");
			fprintf(stderr, "-control_window : The next -Ws or -Wp is passed to the control window.\n");
			fprintf(stderr, "-output_window  : The following text is passed to the output window\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "xpipe scans the file $HOME/.xpiperc.\n");
			fprintf(stderr, "One line of this file creates one menu item.\n");
			fprintf(stderr, "The syntax of a line is:\n\n<name> [-input] [-output/-optional_output] [-select] [-nuhup] [-control]\n\t[-control_window [ -Wp <x> <y> ] [ -Ws <x> <y> ]]\n\t[-output_window [XView args]]:<systemcall>.\n\n");
			fprintf(stderr, "-input          : Do not evaluate the selection.\n");
			fprintf(stderr, "-output         : The process has no output to show.\n");
			fprintf(stderr, "-optional_output: The output window is only created in case of output.\n");
			fprintf(stderr, "-control        : The control window comes up.\n");
			fprintf(stderr, "-select         : Do not put the output to the clipboard.\n");
			fprintf(stderr, "-nohup          : The output window is no child of xpipe.\n");
			fprintf(stderr, "-control_window : The next -Ws or -Wp is passed to the control window.\n");
			fprintf(stderr, "-output_window  : The following text is passed to the output window\n");
			fprintf(stderr, "In systemcall ' :' will be replaced the first line of the selected text.\n");
			exit(1);
		}
		else if(sizemode==0) {
			if(!strcmp(argv[n], "-Wp")) {
				xpipe_x = atoi(argv[++n]);
				xpipe_y = atoi(argv[++n]);
			}
			else newargv[newargc++] = argv[n];
		}
		else if(sizemode==1) {
			if(!strcmp(argv[n], "-Wp")) {
				tty_x = atoi(argv[++n]);
				tty_y = atoi(argv[++n]);
			}
			else if(!strcmp(argv[n], "-Ws")) {
				tty_dx = atoi(argv[++n]);
				tty_dy = atoi(argv[++n]);
			}
			else if(!strcmp(argv[n], "icon")) {
				ttyicon = 1;
			}
		}
		else if(sizemode==2) {
			if(!strcmp(argv[n], "-Wp")) {
				popup_x = atoi(argv[++n]);
				popup_y = atoi(argv[++n]);
			}
			else if(!strcmp(argv[n], "-Ws")) {
				popup_dx = atoi(argv[++n]);
				popup_dy = atoi(argv[++n]);
			}
		}
		else if(sizemode==3) {
			strcat(slave_string, argv[n]);
			strcat(slave_string, " ");
		}
	} 
	newargv[newargc] = 0;


	kb = fopen("/dev/console", "w");

	rc_file = (char *) malloc(200);
	strcpy(rc_file, getenv("HOME"));
	strcat(rc_file, "/.xpiperc");



	/*
	 * Initialize XView.
	 */
	xv_init(XV_INIT_ARGC_PTR_ARGV, &newargc, newargv, 0);
	INSTANCE = xv_unique_key();


	

	/*
	 * Initialize user interface components.
	 */
	xpipe_window = xpipe_window_objects_initialize(NULL, NULL);
	xpipe_tty_window = open_tty_window(xpipe_window->window, "Control-Window");


	server = (Xv_Server)xv_get(xv_get(xpipe_window->window, XV_SCREEN), SCREEN_SERVER);
	readrc();


	/*
	 * Turn control over to XView.
	 */

	xv_main_loop(xpipe_window->window);

	fclose(kb);
	exit(0);
}





char *catstr(s)
char *s;
{
	char *t;
	t = (char *) malloc(strlen(s)+1);
	strcpy(t,s);
	return(t);
}




void
scan_line(line)
char *line;

{
	char    name[100], word[100], slave[100];
	int	n, m, parm = 0, input, output, select, nohup, opencontrol, x, dx, y, dy, mode;


	for(n = 0; line[n] != '\0' && line[n] != '#'; n++) ;
	line[n] = '\0';


	input = 1;
	output = 1;
	select = 1;
	nohup = 0;
	opencontrol = 0;
	x = -1;
	y = -1;
	dx = -1;
	dy = -1;
	slave[0] = '\0';

	n = 0;
	name[0] = '\0';

	while(line[n] != ':' && line[n] != '\0' ) {
		if(line[n] == ' ' || line[n] == '\t') n++;
		else {
			m = 0;
			while(line[n] != ':' && line[n] != ' ' && line[n] != '\t') 
				word[m++] = line[n++]; 
			word[m] = '\0';
			if(!m) continue;

			if(!strcmp(word, "-input")) 
				input = 0;
			else if(!strcmp(word, "-output")) 
				output = 0;
			else if(!strcmp(word, "-optional_output")) 
				output = 2;
			else if(!strcmp(word, "-output_window")) {
				for(m = 0; line[n+m] != ':' && line[n+m] != '\0'; m++) slave[m] = line[n+m];
				slave[m] = '\0';
				n = n+m;
			}
			else if(!strcmp(word, "-control")) 
				opencontrol = 1;
			else if(!strcmp(word, "-control_window")) 
				mode = 1;
			else if(!strcmp(word, "-nohup")) 
				nohup = 1;
			else if(!strcmp(word, "-select")) 
				select = 0;
			else if(!strcmp(word, "-Ws")) {
				if(mode != 1) {
					fprintf(stderr, "Error with '-Ws' in '%s'.\n", line);
					return;
				}
				mode = 2;
			}
			else if(!strcmp(word, "-Wp")) {
				if(mode != 1) {
					fprintf(stderr, "Error with '-Wp' in '%s'.\n", line);
					return;
				}
				mode = 4;
			}
			else if(mode == 2) {
				dx = atoi(word);
				mode++;
			}
			else if(mode == 4) {
				x = atoi(word);
				mode++;
			}
			else if(mode == 3) {
				dy = atoi(word);
				mode = 1;
			}
			else if(mode == 5) {
				y = atoi(word);
				mode = 1;
			}
			else{
				strcat(name, word);
				strcat(name, " ");
			}

		}

	}

	if(name[0] == '\0') {
		return;
	}
	if(line[n] == '\0') {
		fprintf(stderr, "No ':' found in '%s'.\n", line);
		return;
	}
	name[strlen(name)-1] = '\0';


	if(menu_commands1)
		for(menu_commands_tmp = menu_commands; menu_commands_tmp; menu_commands_tmp = menu_commands_tmp->next)
			if(!strcmp(name, menu_commands_tmp->name)) {
				fprintf(kb, "Label <%s> is defined twice\n", name);
				return;
			}


	if(menu_commands1) {
		menu_commands1->next = ( Menu_Commands * ) malloc(sizeof(Menu_Commands));
		if(!menu_commands1->next) {
			fprintf(kb, "Malloc failed\n");
			exit(1);
		}
		menu_commands1 = menu_commands1->next;
	}
	else {
		menu_commands = menu_commands1 = ( Menu_Commands * ) malloc(sizeof(Menu_Commands));
		if(!menu_commands) {
			fprintf(kb, "Malloc failed\n");
			exit(1);
		}
	}

	menu_commands1->next = ( Menu_Commands * ) 0;

	menu_commands1->x = x;
	menu_commands1->dx = dx;
	menu_commands1->y = y;
	menu_commands1->dy = dy;
	menu_commands1->slave = catstr(slave);
	menu_commands1->opencontrol = opencontrol;
	menu_commands1->input = input;
	menu_commands1->output = output;
	menu_commands1->line = 0;
	menu_commands1->select = select;
	menu_commands1->nohup = nohup;

	menu_commands1->parms = catstr(line+n+1);
	menu_commands1->name = catstr(name);

	for(n++ ; n<strlen(line); n++) if(line[n] == ':' ) {
		menu_commands1->line = 1;
		break;
	}
	if(verbose) fprintf(kb, "%s : %s\n", menu_commands1->name, menu_commands1->parms);
}

void
read_menu_commands()
{
	FILE 	*fp;
	char	line[1000];
	int	n = 0;
	char	c;

	if (verbose>1)  fprintf(kb, "--> read_menu_commands\n");

	menu_commands1 = 0;


	

	fp = fopen(rc_file, "r");
	if(!fp) {
		fprintf(kb, "Can't read %s\n", rc_file);
		if(notice(xpipe_window->window, "Can't read .xpiperc", "Create", "Exit", 0)) exit(1);
		fp = fopen(rc_file, "w");
		fclose(fp);
		menu_commands = 0;
		return;
	}

	while ((c = getc(fp)) != EOF) {
		if (c == '\n') {
			line[n] = '\0';
			n = 0;
			scan_line(line);
		} else  line[n++] = c;
	
	}
	line[n] = '\0';

	scan_line(line);

	fclose(fp);
	if (verbose>1)  fprintf(kb, "<-- read_menu_commands\n");

}





/*
 *	Fill menu from .xpiperc
 */
int
readrc()
{
	Menu				menu = xv_get(xpipe_window->button, PANEL_ITEM_MENU);
	Menu_item			mi;
	xpipe_window_objects		*ip = xpipe_window;
	
	Menu_Commands *menu_commands1;
	
	if (verbose>1)  fprintf(kb, "--> readrc\n");



	/* read menu list structure */

	read_menu_commands();

	menu_commands1 = menu_commands;


	/* create menu items */

	while (menu_commands1) {

		mi = (Menu_item)xv_create(menu,	MENUITEM,
						XV_KEY_DATA, INSTANCE, ip,
						MENU_STRING, menu_commands1->name,
						MENU_GEN_PROC, menu_handler,
						MENU_RELEASE,
						NULL);
	
		if (mi == XV_NULL) exit(-1);
	
		xv_set(menu,	MENU_APPEND_ITEM, mi,
				PANEL_PAINT, PANEL_NONE,
				NULL);

		menu_commands1 = menu_commands1->next;
	}

	/* create standard menu items */

	mi = (Menu_item)xv_create(menu,	MENUITEM,
					XV_KEY_DATA, INSTANCE, ip,
					MENU_STRING, "EDIT .xpiperc",
					MENU_GEN_PROC, menu_handler,
					MENU_RELEASE,
					NULL);

	if (mi == XV_NULL) exit(-1);

	xv_set(menu,	MENU_APPEND_ITEM, mi,
			PANEL_PAINT, PANEL_NONE,
			NULL);



	if (verbose>1)  fprintf(kb, "<-- readrc\n");


}

int
rereadrc()
{
	Menu				menu = xv_get(xpipe_window->button, PANEL_ITEM_MENU);
	Menu_item			mi;
	int				nitems = xv_get(menu, MENU_NITEMS);
	int				menu_row;
	xpipe_window_objects		*ip = xpipe_window;

	Menu_Commands *menu_commands1;

	int		isshown;
	Frame	frame = 0;

	frame = (Panel)xv_get(menu, MENU_PIN_WINDOW);
	if (verbose>1)  fprintf(kb, "--> frame = %d\n", frame);
	isshown = FALSE;
	if (frame) 
		isshown = xv_get(frame, XV_SHOW);

	if (verbose>1)  fprintf(kb, "--> isshown = %d\n", isshown);
	if (isshown) {
		xv_set(frame, 
			FRAME_CMD_PUSHPIN_IN, FALSE,
			XV_SHOW, FALSE,
			NULL);
	}
	if (verbose>1)  fprintf(kb, "--> rereadrc\n");
	

	/* clear menu items */

	for (menu_row = nitems; menu_row ; menu_row--)
		xv_set(menu, MENU_REMOVE, menu_row, NULL);

	/* clear menu list structure */

	while(menu_commands) {
		menu_commands1 = menu_commands->next;
		free(menu_commands->parms);
		free(menu_commands->name);
		free(menu_commands);
		menu_commands = menu_commands1;
	}


	/* read menu list structure */

	read_menu_commands();

	menu_commands1 = menu_commands;


	/* create menu items */

	while (menu_commands1) {

		mi = (Menu_item)xv_create(menu,	MENUITEM,
						XV_KEY_DATA, INSTANCE, ip,
						MENU_STRING, menu_commands1->name,
						MENU_GEN_PROC, menu_handler,
						MENU_RELEASE,
						NULL);
	
		if (mi == XV_NULL) exit(-1);

	
		xv_set(menu,	MENU_APPEND_ITEM, mi,
				PANEL_PAINT, PANEL_NONE,
				NULL);


		menu_commands1 = menu_commands1->next;
	}

	/* create standard menu items */


	mi = (Menu_item)xv_create(menu,	MENUITEM,
					XV_KEY_DATA, INSTANCE, ip,
					MENU_STRING, "EDIT .xpiperc",
					MENU_GEN_PROC, menu_handler,
					MENU_RELEASE,
					NULL);

	if (mi == XV_NULL) exit(-1);

	xv_set(menu,	MENU_APPEND_ITEM, mi,
			PANEL_PAINT, PANEL_NONE,
			NULL);



	if (isshown) {
		xv_set(frame, 
			FRAME_CMD_PUSHPIN_IN, TRUE,
			XV_SHOW, TRUE,
			NULL);
	}
	xv_set(menu,	MENU_PIN, TRUE, NULL);

	if (verbose>1)  fprintf(kb, "<-- rereadrc\n");



}




/*
 * Menu handler
 */
Menu_item
menu_handler(item, op)
	Menu_item	item;
	Menu_generate	op;
{	

	char		*sel, syscmdstring[256], sel_line1[256];
	int		n, m, k;
	Menu_Commands *menu_commands1;
	FILE		*fp;
	static	pid_t 	pgrp = 0, pid = 0;

	switch (op) {
	case MENU_DISPLAY:
		break;

	case MENU_DISPLAY_DONE:
		break;

	case MENU_NOTIFY:
		menu_commands1 = menu_commands;

		if (verbose>1)  fprintf(kb, "--> menu_handler\n");


		/* get  menu item */

		string = (char *)xv_get(item, MENU_STRING);

		label = (char *)malloc(256);
		strcpy(label, string);

		if(verbose) fprintf(kb, "%s\n", label);


		if(!strcmp(string, "EDIT .xpiperc")) {
			open_text_window(xpipe_window->window, rc_file, rc_file, 1);
			if (verbose>1)  fprintf(kb, "<-- menu_handler\n");
			break;
		}


		/* get  selection */

		/* search for item in menu list */

		while(menu_commands1 && strcmp(string, menu_commands1->name))
			menu_commands1 = menu_commands1->next;

		if(menu_commands1) {



			if(menu_commands1->line) {
				sel = get_selection();
				if(sel) {
					for(n = 0 ; n<255 && sel[n] != '\n' && sel[n] != '\0'; n++) sel_line1[n] = sel[n];
					sel_line1[n] = '\0';

				}
				else	sel_line1[0] = '\0';


			}
			if(menu_commands1->input) strcpy(syscmdstring, "xpipe_sel  | ( ");
			else strcpy(syscmdstring, "( ");

			m = strlen(syscmdstring);

			/* create system call line */

			for(n = 0; n<=strlen(menu_commands1->parms); n++)
				if(menu_commands1->parms[n] == ':' ) 
					for(k = 0; k<strlen(sel_line1); k++) syscmdstring[m++] = sel_line1[k];
				else syscmdstring[m++] = menu_commands1->parms[n];


			if(menu_commands1->output) 
				{
					strcat(syscmdstring, " ) | xpipe_slave -name \"");
					strcat(syscmdstring, xpipe_label);
					strcat(syscmdstring, " : ");
					strcat(syscmdstring, label);
					strcat(syscmdstring, "\"");
					if(menu_commands1->nohup) strcat(syscmdstring, " -nohup");
					if(menu_commands1->select) strcat(syscmdstring, " -select");
					if(menu_commands1->output == 2) strcat(syscmdstring, " -optional_output");
					if(strlen(menu_commands1->slave)) strcat(syscmdstring, menu_commands1->slave);
					else strcat(syscmdstring, slave_string);
				}
			else strcat(syscmdstring, " )");



			if(verbose) fprintf(kb, "system(%s)\n", syscmdstring);
			switch(pid=fork()) {
			case -1:
				if (verbose)  fprintf(kb, "fork failed\n");
				break;
			case 0:

				if (verbose>1)  fprintf(kb, "--> system\n");
				execl("/bin/sh", "sh", "-c", syscmdstring, NULL);
				if (verbose>1)  fprintf(kb, "<-- system\n");

				_exit();
				break;
			default:
				if (menu_commands1->x > -1)	xv_set(xpipe_tty_window->tty_window, 
										XV_X, menu_commands1->x, 
										XV_Y, menu_commands1->y, 
										NULL);
				if (menu_commands1->dx > -1)	xv_set(xpipe_tty_window->tty_window, 
										XV_WIDTH, menu_commands1->dx, 
										XV_HEIGHT, menu_commands1->dy, 
										NULL);
				if(menu_commands1->opencontrol) if(ttyicon)
					xv_set(xpipe_tty_window->tty_window, XV_SHOW, TRUE, FRAME_CLOSED, FALSE, NULL);
				else 	xv_set(xpipe_tty_window->tty_window, XV_SHOW, TRUE, FRAME_CLOSED, FALSE,
						/*FRAME_CMD_PUSHPIN_IN, TRUE,*/ NULL);
				break;
			}


		}

		if (verbose>1)  fprintf(kb, "<-- menu_handler\n");

		break;

	case MENU_NOTIFY_DONE:
		break;
	}



	return(item);

}


/*
 * Create object `menu' in the specified instance.

 */
Xv_opaque
xpipe_menu_create(ip, owner)
	caddr_t		*ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	if (verbose>1)  fprintf(kb, "--> xpipe_menu_create\n");
	obj = xv_create(XV_NULL, MENU_COMMAND_MENU,
		XV_KEY_DATA, INSTANCE, ip,
		MENU_GEN_PIN_WINDOW, ip[0], xpipe_label,
		/*MENU_PIN_WINDOW, xv_create(XV_NULL, FRAME_CMD, NULL),*/
		MENU_PIN, TRUE,
		NULL);
 	if (verbose>1)  fprintf(kb, "<-- xpipe_menu_create\n");
	return obj;
}

/*
 * Initialize an instance of object `window'.
 */
xpipe_window_objects *
xpipe_window_objects_initialize(ip, owner)
	xpipe_window_objects	*ip;
	Xv_opaque	owner;
{
	if (verbose>1)  fprintf(kb, "--> xpipe_window_objects_initialize\n");
	if (!ip && !(ip = (xpipe_window_objects *) calloc(1, sizeof (xpipe_window_objects))))
		return (xpipe_window_objects *) NULL;
	if (!ip->window)
		ip->window = xpipe_window_window_create(ip, owner);
	if (!ip->controls)
		ip->controls = xpipe_window_controls_create(ip, ip->window);
	if (!ip->button)
		ip->button = xpipe_window_button_create(ip, ip->controls);
 	if (verbose>1)  fprintf(kb, "<-- xpipe_window_objects_initialize\n");
	return ip;
}

/*
 * Create object `window' in the specified instance.

 */
Xv_opaque
xpipe_window_window_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	Xv_opaque		window_image;
	static unsigned short	window_bits[] = {
#include "xpipe.icon"
	};
	Xv_opaque		window_image_mask;
	static unsigned short	window_mask_bits[] = {
#include "xpipe.mask.icon"
	};
	if (verbose>1)  fprintf(kb, "--> xpipe_window_window_create\n");
	
	window_image = xv_create(XV_NULL, SERVER_IMAGE,
		SERVER_IMAGE_BITS, window_bits,
		SERVER_IMAGE_DEPTH, 1,
		XV_WIDTH, 64,
		XV_HEIGHT, 64,
		NULL);
	window_image_mask = xv_create(XV_NULL, SERVER_IMAGE,
		SERVER_IMAGE_BITS, window_mask_bits,
		SERVER_IMAGE_DEPTH, 1,
		XV_WIDTH, 64,
		XV_HEIGHT, 64,
		NULL);
	obj = xv_create(owner, FRAME,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, xpipe_x,
		XV_Y, xpipe_y,
		XV_WIDTH, xpipe_dx,
		XV_HEIGHT, xpipe_dy,
		XV_LABEL, xpipe_label,
		FRAME_CLOSED, FALSE,
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, FALSE,
		FRAME_ICON, xv_create(XV_NULL, ICON,
			ICON_IMAGE, window_image,
			ICON_MASK_IMAGE, window_image_mask,
			NULL),
		NULL);
 	if (verbose>1)  fprintf(kb, "<-- xpipe_window_window_create\n");
	return obj;
}

/*
 * Create object `controls' in the specified instance.

 */
Xv_opaque
xpipe_window_controls_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	if (verbose>1)  fprintf(kb, "--> xpipe_window_controls_create\n");
	
	obj = xv_create(owner, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		WIN_BORDER, FALSE,
		NULL);
 	if (verbose>1)  fprintf(kb, "<-- xpipe_window_controls_create\n");

	return obj;
}

/*
 * Create object `button' in the specified instance.

 */
Xv_opaque
xpipe_window_button_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	Xv_opaque		button_image;
	static unsigned short	button_bits[] = {
#include "xpipe.glyph"
	};
	
	if (verbose>1)  fprintf(kb, "--> xpipe_window_button_create\n");
	button_image = xv_create(XV_NULL, SERVER_IMAGE,
		SERVER_IMAGE_BITS, button_bits,
		SERVER_IMAGE_DEPTH, 1,
		XV_WIDTH, 64,
		XV_HEIGHT, 64,
		NULL);
	obj = xv_create(owner, PANEL_BUTTON,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 2,
		XV_Y, 4,
		XV_WIDTH, 88,
		XV_HEIGHT, 74,
		PANEL_LABEL_IMAGE, button_image,
		PANEL_ITEM_MENU, xpipe_menu_create((caddr_t *) ip, XV_WIDTH, 300, NULL),
		NULL);
 	if (verbose>1)  fprintf(kb, "<-- xpipe_window_button_create\n");
	return obj;
}


/*
 * return the text selected in the current selection rank.  Use
 * selection_query() to guarantee that the entire selection is
 * retrieved.  selection_query() calls our installed routine,
 * read_proc() (see below).
 */
#define FIRST_BUFFER            0
#define NOT_FIRST_BUFFER        !FIRST_BUFFER

static char *seln_buf;     /* contents of each of the three selections */
Seln_result read_proc(); /* supplied to selection_query() as reader */

char *
get_selection()
{
    Seln_holder   holder;
    Seln_result   result;
    Seln_request  *response;
    char          context = FIRST_BUFFER;
	if (verbose>1)  fprintf(kb, "--> get_selection\n");
    holder = selection_inquire(server, SELN_PRIMARY);
    /* result is based on the return value of read_proc() */
    result = selection_query(server, &holder, read_proc, &context,
        SELN_REQ_BYTESIZE,              NULL,
        SELN_REQ_CONTENTS_ASCII,        NULL,
        NULL);
    if (result == SELN_FAILED) {
        if(verbose>1) fprintf(kb, "couldn't get selection\n");
 	if (verbose>1)  fprintf(kb, "<-- get_selection\n");
        return NULL;
    }
 	if (verbose>1)  fprintf(kb, "<-- get_selection\n");

    return seln_buf;
}

/*
 * Called by selection_query for every buffer of information received.
 * Short messages (under about 2000 bytes) will fit into one buffer.
 * For larger messages, read_proc is called for each buffer in the
 * selection.  The context pointer passed to selection_query is
 * modified by read_proc so that we know if this is the first buffer
 * or not.
 */
Seln_result
read_proc(response)
Seln_request *response;
{
    char *reply;  /* pointer to the data in the response received */
    long seln_len; /* total number of bytes in the selection */
    static long seln_have_bytes;
        /* number of bytes of the selection
         * which have been read; cumulative over all calls for
         * the same selection (it is reset when the first
         * response of a selection is read)
         */

    if(verbose>1) fprintf(kb, "read_proc status: %s (%d)\n",
        response->status == SELN_FAILED? "failed" :
        response->status == SELN_SUCCESS? "succeeded" :
        response->status == SELN_CONTINUED? "continued" : "???",
        response->status);
    if (*response->requester.context == FIRST_BUFFER) {
        reply = response->data;

        /* read in the length of the selection -- first attribute.
         * advance "reply" passed attribute to point to actual data.
         */
        reply += sizeof(SELN_REQ_BYTESIZE);
        /* set seln_len to actual data now. (bytes selected) */
        seln_len = *(int *)reply;
        if(verbose>1) fprintf(kb, "selection size is %ld bytes\n", seln_len);
        /* advance "reply" to next attribute in list */
        reply += sizeof(long);

        /* create a buffer large enough to store entire selection */
        if (seln_buf != NULL)
            free(seln_buf);
        if (!(seln_buf = malloc(seln_len + 1))) {
            puts("out of memory");
            return(SELN_FAILED);
        }
        seln_have_bytes = 0;

        /* move "reply" passed attribute so it points to contents */
        reply += sizeof(SELN_REQ_CONTENTS_ASCII);
        *response->requester.context = NOT_FIRST_BUFFER;
    } else {
        /* this is not the first buffer, so the contents of the
	 * response is just more of the selection
         */
        reply = response->data;
    }

    /* copy data from received to the seln buffer allocated above */
     if(verbose>1) fprintf(kb, "read_proc status: %s (%d)\n", reply, strlen(reply));
   (void) strcpy(&seln_buf[seln_have_bytes], reply);
    seln_have_bytes += strlen(reply);

   return SELN_SUCCESS;
}




















/* ---------------------------------------------------------------------------------------------------------------------
 *	Create a new text popup, load text file
 */

int
open_text_window(owner, filename, label, save)
	Xv_opaque			owner;
	char				*filename;
	char				*label;
	int				save;
{
	char				*proc = "open_text_window";
	Textsw_status			status;

	if (verbose>1)  fprintf(kb, "--> open_text_window\n");

	
	if(!xpipe_xv_text_window) 
	xpipe_xv_text_window = xpipe_xv_text_window_objects_initialize(NULL, owner, label);
	else {
		if(xpipe_xv_text_window->save) {
	if (verbose>1)  fprintf(kb, "must be saved\n");
			status = xv_get(xpipe_xv_text_window->textpane,	TEXTSW_MODIFIED);
	if (verbose>1)  fprintf(kb, "%d\n",status);
			if(status) {
				switch(notice(xpipe_window->window, "File was modified.", "Ignore", "Save", "Break")) {
					case YES:
						textsw_save(xpipe_xv_text_window->textpane, 100, 100);
						break;
					case BREAK:
						if (verbose>1)  fprintf(kb, "<-- open_text_window\n");
						return;
				}
			}

		}

	}

	xpipe_xv_text_window->save = save;
	xpipe_xv_text_window->file = filename;

	xv_set(xpipe_xv_text_window->textpane,	TEXTSW_STATUS,	&status,
						TEXTSW_FILE, 	catstr(filename),
						NULL);

	if (status != TEXTSW_STATUS_OKAY) {if(verbose>1) fprintf(kb, "Can't load file %s\n", filename);}
	xv_set(xpipe_xv_text_window->text_window, XV_SHOW, TRUE, NULL);


	if (verbose>1)  fprintf(kb, "<-- open_text_window\n");

	return;

}



/*
 * Initialize an instance of object `text_window'.
 */
xpipe_xv_text_window_objects *
xpipe_xv_text_window_objects_initialize(ip, owner, label)
	xpipe_xv_text_window_objects	*ip;
	Xv_opaque			owner;
	char				*label;
{

	if (verbose>1)  fprintf(kb, "--> xpipe_xv_text_window_objects_initialize\n");

	if (!ip && !(ip = (xpipe_xv_text_window_objects *) calloc(1, sizeof (xpipe_xv_text_window_objects))))
		return (xpipe_xv_text_window_objects *) NULL;
	if (!ip->text_window)
		ip->text_window = xpipe_xv_text_window_text_window_create(ip, owner, label);
	if (!ip->textpane)
		ip->textpane = xpipe_xv_text_window_textpane_create(ip, ip->text_window);

	if (verbose>1)  fprintf(kb, "<-- xpipe_xv_text_window_objects_initialize\n");

	return ip;
}


/*
 * Create object `text_window' in the specified instance.
 */
Xv_opaque
xpipe_xv_text_window_text_window_create(ip, owner, label)
	caddr_t		ip;
	Xv_opaque	owner;
	char		*label;
{
	Xv_opaque	obj;
	char		full_label[450];

	if (verbose>1)  fprintf(kb, "--> xpipe_xv_text_window_text_window_create\n");

	strcpy(full_label, xpipe_label);
	strcat(full_label, " : ");
	strcat(full_label, label);

	obj = xv_create(owner, FRAME_CMD,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, popup_x,
		XV_Y, popup_y,
		XV_WIDTH, popup_dx,
		XV_HEIGHT, popup_dy,
		XV_LABEL, catstr(full_label),
		FRAME_CLOSED, FALSE,
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		FRAME_DONE_PROC, text_window_done_handler, 
		FRAME_CMD_PUSHPIN_IN, TRUE,
		XV_FONT, (Xv_Font)load_font(),
		NULL);

	default_done_proc = ( void (*)() )xv_get(obj, FRAME_DEFAULT_DONE_PROC);
	xv_set(obj, FRAME_DONE_PROC, text_window_done_handler, NULL);


	xv_set(xv_get(obj, FRAME_CMD_PANEL), WIN_SHOW, TRUE, NULL);

	if (verbose>1)  fprintf(kb, "<-- xpipe_xv_text_window_text_window_create\n");
	return obj;
}



/*
 * Create object `textpane' in the specified instance.
 */
Xv_opaque
xpipe_xv_text_window_textpane_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	if (verbose>1)  fprintf(kb, "--> xpipe_xv_text_window_textpane_create\n");
	
	obj = xv_create(owner, TEXTSW,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		OPENWIN_SHOW_BORDERS, TRUE,
		TEXTSW_BROWSING, FALSE,
		TEXTSW_READ_ONLY, FALSE,
		TEXTSW_DISABLE_LOAD, TRUE,
		NULL);
	if (verbose>1)  fprintf(kb, "<-- xpipe_xv_text_window_textpane_create\n");
	return obj;
}



/* ---------------------------------------------------------------------------------------------------------------------
 * 	Load font: Lucida Sans Fixedwidth for lists and textpanes (if not found, load Courier or take default font)
 */
Xv_Font
load_font()
{
	char		*proc = "load_font";
	Xv_Font		font;

	if (verbose>1)  fprintf(kb, "--> load_font\n");
	
	font = (Xv_Font)xv_find(XV_NULL, FONT, FONT_FAMILY, FONT_FAMILY_LUCIDA_FIXEDWIDTH, FONT_SCALE, WIN_SCALE_MEDIUM, NULL);
	if (!font)  {
		if(verbose>1) fprintf(kb, "SecTool.load_font: cannot use default fixedwidth font.\n");
		font = (Xv_Font)xv_find(XV_NULL, FONT, FONT_NAME, "courier", NULL);
		if (!font)  {
			if(verbose>1) fprintf(kb, "SecTool.load_font: cannot use font 'courier'.\n");
			font = (Xv_Font)xv_get(XV_NULL, XV_FONT);
		}
	}
/*	if(verbose>1) fprintf(kb, "  SecTool.load_font: selected font is: %d %s\n", font, (char *)xv_get(font, FONT_NAME));
*/

	if (verbose>1)  fprintf(kb, "<-- load_font\n");
	return(font);

}



int get_file_size(name)
char *name;
{
	struct stat buf;
	int ret;

	if(name==0) return(0);

	if(stat(name, &buf)<0) return(0);

	ret = (int) buf.st_size;	
	

	return(ret);
}















xpipe_xv_tty_window_objects
* open_tty_window(owner, label)
	Xv_opaque			owner;
	char				*label;
{
	char				*proc = "open_tty_window";
	xpipe_xv_tty_window_objects	*xpipe_xv_tty_window = 0;
	Textsw_status			status;

	if (verbose>1)  fprintf(kb, "--> open_tty_window\n");

	xpipe_xv_tty_window = xpipe_xv_tty_window_objects_initialize(NULL, owner, label);


	xv_set(xpipe_xv_tty_window->tty_window, XV_SHOW, TRUE, FRAME_CLOSED, TRUE, NULL);



	if (verbose>1)  fprintf(kb, "<-- open_tty_window\n");

	return(xpipe_xv_tty_window);

}



/*
 * Initialize an instance of object `text_window'.
 */
xpipe_xv_tty_window_objects *
xpipe_xv_tty_window_objects_initialize(ip, owner, label)
	xpipe_xv_tty_window_objects	*ip;
	Xv_opaque			owner;
	char				*label;
{

	if (verbose>1)  fprintf(kb, "--> xpipe_xv_tty_window_objects_initialize\n");

	if (!ip && !(ip = (xpipe_xv_tty_window_objects *) calloc(1, sizeof (xpipe_xv_tty_window_objects))))
		return (xpipe_xv_tty_window_objects *) NULL;
	if (!ip->tty_window)
		ip->tty_window = xpipe_xv_tty_window_tty_window_create(ip, owner, label);
	if (!ip->ttypane)
		ip->ttypane = xpipe_xv_tty_window_ttypane_create(ip, ip->tty_window);
	if (verbose>1)  fprintf(kb, "<-- xpipe_xv_tty_window_objects_initialize\n");
	return ip;
}


/*
 * Create object `text_window' in the specified instance.
 */
Xv_opaque
xpipe_xv_tty_window_tty_window_create(ip, owner, label)
	caddr_t		ip;
	Xv_opaque	owner;
	char		*label;
{
	Xv_opaque	obj;
	char		full_label[450];

	Xv_opaque		window_image;
	static unsigned short	window_bits[] = {
#include "xpipe-tty.icon"
	};
	Xv_opaque		window_image_mask;
	static unsigned short	window_mask_bits[] = {
#include "xpipe-tty.mask.icon"
	};


	if (verbose>1)  fprintf(kb, "--> xpipe_xv_tty_window_tty_window_create\n");


	strcpy(full_label, xpipe_label);
	strcat(full_label, " : ");
	strcat(full_label, label);

	if(ttyicon) {
		window_image = xv_create(XV_NULL, SERVER_IMAGE,
			SERVER_IMAGE_BITS, window_bits,
			SERVER_IMAGE_DEPTH, 1,
			XV_WIDTH, 64,
			XV_HEIGHT, 64,
			NULL);
		window_image_mask = xv_create(XV_NULL, SERVER_IMAGE,
			SERVER_IMAGE_BITS, window_mask_bits,
			SERVER_IMAGE_DEPTH, 1,
			XV_WIDTH, 64,
			XV_HEIGHT, 64,
			NULL);


		obj = xv_create(owner, FRAME,
			XV_KEY_DATA, INSTANCE, ip,
			XV_WIDTH, tty_dx,
			XV_HEIGHT, tty_dy,
			XV_LABEL, catstr(full_label),
			FRAME_CLOSED, TRUE,
			FRAME_SHOW_FOOTER, TRUE,
			FRAME_SHOW_RESIZE_CORNER, TRUE,
			FRAME_ICON, xv_create(XV_NULL, ICON,
				ICON_IMAGE, window_image,
				ICON_MASK_IMAGE, window_image_mask, 
				NULL),
			XV_FONT, (Xv_Font)load_font(),
			NULL);
	}
	else obj = xv_create(owner, FRAME_CMD,
		XV_KEY_DATA, INSTANCE, ip,
		XV_WIDTH, tty_dx,
		XV_HEIGHT, tty_dy,
		XV_LABEL, catstr(full_label),
		FRAME_SHOW_FOOTER, TRUE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		FRAME_CLOSED, TRUE,
		XV_FONT, (Xv_Font)load_font(),
		NULL);

	if (tty_x >= 0) xv_set(obj, XV_X, tty_x, XV_Y, tty_y, NULL);

	if (verbose>1)  fprintf(kb, "<-- xpipe_xv_tty_window_tty_window_create\n");

	return obj;
}



/*
 * Create object `textpane' in the specified instance.
 */
Xv_opaque
xpipe_xv_tty_window_ttypane_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;

	if (verbose>1)  fprintf(kb, "--> xpipe_xv_tty_window_ttypane_create\n");

	obj = xv_create(owner, TERMSW,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		OPENWIN_SHOW_BORDERS, TRUE,
		TTY_ARGV, TTY_ARGV_DO_NOT_FORK,

		NULL);


	fddevtty = xv_get(obj, TTY_TTY_FD);

	setsid();



	dup2(fddevtty,0);
	dup2(fddevtty,1);
	dup2(fddevtty,2); 

	if (verbose>1)  fprintf(kb, "<-- xpipe_xv_tty_window_ttypane_create\n");


	return obj;
}



/*
 *	Continue/Show-Notice
 */

int
notice(item, text, no, yes, brea)
	Panel_item		item;
	char			*text, *yes, *no, *brea;
{
	char			*proc = "notice";
	Panel			panel = (Panel)xv_get(item, PANEL_PARENT_PANEL);
	Xv_notice 		notice;
	int			result;

	if (verbose)  fprintf(kb, "--> notice\n");


	if(brea) notice = xv_create(panel, 	NOTICE,
					NOTICE_MESSAGE_STRINGS, text, NULL,
					NOTICE_BUTTON, catstr(no), NO,
					NOTICE_BUTTON, catstr(yes), YES,
					NOTICE_BUTTON, catstr(brea), BREAK,
					NOTICE_STATUS, &result,
					XV_SHOW, TRUE,
					NULL);
	else if(yes) notice = xv_create(panel, 	NOTICE, 
					NOTICE_MESSAGE_STRINGS, text, NULL,
					NOTICE_BUTTON, catstr(no), NO,
					NOTICE_BUTTON, catstr(yes), YES,
					NOTICE_STATUS, &result,
					XV_SHOW, TRUE,
					NULL);
	else notice = xv_create(panel, 	NOTICE,
					NOTICE_MESSAGE_STRINGS, text, NULL,
					NOTICE_BUTTON, catstr(no), NO,
					NOTICE_STATUS, &result,
					XV_SHOW, TRUE,
					NULL);
	xv_destroy_safe(notice);

	if (verbose)  fprintf(kb, "<-- notice\n");
	return(result);

}





void
text_window_done_handler(frame)
	Frame		frame;
{
	Textsw_status			status;


	if (verbose)  fprintf(kb, "--> text_window_done_handler (frame = %d)\n", frame);

			status = xv_get(xpipe_xv_text_window->textpane,	TEXTSW_MODIFIED);
			if (verbose>1)  fprintf(kb, "file status: %d\n",status);
			if(status) {

				switch(notice(xpipe_window->window, "File was modified.", "Ignore", "Save", "Break")) {
					case NO:
						textsw_reset(xpipe_xv_text_window->textpane, 0, 0);
						break;
					case YES:
						textsw_save(xpipe_xv_text_window->textpane, 100, 100);
						break;
					case BREAK:
						if (verbose>1)  fprintf(kb, "<-- text_window_done_handler\n");
						return;
				}
			}
			rereadrc();

	default_done_proc(frame);

	if (verbose)  fprintf(kb, "<-- text_window_done_handler\n");

}



