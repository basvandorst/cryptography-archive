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

#include <xview/textsw.h>

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/icon_load.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/termsw.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/xv_xrect.h>
#include "xpipe_slave.h"


/*
 * Instance XV_KEY_DATA key.  An instance is a set of related
 * user interface objects.  A pointer to an object's instance
 * is stored under this key in every object.  This must be a
 * global variable.
 */
Attr_attribute	INSTANCE;

int MEM_MAX = 70000;

void
main(argc, argv)
	int		argc;
	char		**argv;
{
	xpipe_slave_window1_objects	*xpipe_slave_window1;
	Xv_opaque			textsw;
	char 				name[100];

	int 				ins, input = 0, length, bufs_pos = -1, selection = 0, optional_output = 0, n, m;
	char 				buff[1025], *buffers[200];






	strcpy(name, "No Name");

	for(n = 1; n<argc;) {
		if(!strcmp(argv[n], "-name")) {
			strcpy(name, argv[n+1]);
			for(m = n; m<argc-1; m++) argv[m] = argv[m+2];
			argc -= 2;
		}
		else if(!strcmp(argv[n], "-select")) {
			selection = 1;
			for(m = n; m<argc; m++) argv[m] = argv[m+1];
			argc--;
		}
		else if(!strcmp(argv[n], "-optional_output")) {
			optional_output = 1;
			for(m = n; m<argc; m++) argv[m] = argv[m+1];
			argc--;
		}
		else if(!strcmp(argv[n], "-nohup")) {
			setsid();
			for(m = n; m<argc; m++) argv[m] = argv[m+1];
			argc--;
		} else  n++;
	}
	/*
	 * Initialize XView.
	 */
	xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, 0);
	INSTANCE = xv_unique_key();
	
	/*
	 * Initialize user interface components.
	 */

	do {
		if( input % 1024 == 0 ) {
			bufs_pos++;
			buffers[bufs_pos] = (char *) malloc(1024);
		}
		length = read(0, buffers[bufs_pos] + (input % 1024), 1024 - (input % 1024));

		input += length;

/*	fprintf(stderr, "%d %d %d\n", input, bufs_pos, length);*/

	} while (length != 0);


	MEM_MAX = input*2 + 2000;

	xpipe_slave_window1 = xpipe_slave_window1_objects_initialize(NULL, NULL, name);
	textsw = xpipe_slave_window1->textpane1;

	for(n = 0; n<bufs_pos; n++) {
		ins = textsw_insert(textsw, buffers[n], 1024);
/*		fprintf(stderr, "%d 1024 %d\n", n, ins);*/
	}
	if(input - n*1024) ins = textsw_insert(textsw, buffers[n], input - n*1024);
/*	fprintf(stderr, "%d %d %d\n", n, input - n*1024, ins);*/


	if(!input && optional_output) return;
	if(selection) {
		textsw_delete(textsw, 0, TEXTSW_INFINITY);

		for(n = 0; n<bufs_pos; n++) textsw_insert(textsw, buffers[n], 1024);
		textsw_insert(textsw, buffers[n], input - n*1024);

		/*textsw_set_selection(textsw, 0, TEXTSW_INFINITY, 1);*/
	}

	/*
	 * Turn control over to XView.
	 */
	xv_main_loop(xpipe_slave_window1->window1);
	exit(0);
}



/*
 * Initialize an instance of object `window1'.
 */
xpipe_slave_window1_objects *
xpipe_slave_window1_objects_initialize(ip, owner, name)
	xpipe_slave_window1_objects	*ip;
	Xv_opaque	owner;
	char		*name;
{
	if (!ip && !(ip = (xpipe_slave_window1_objects *) calloc(1, sizeof (xpipe_slave_window1_objects))))
		return (xpipe_slave_window1_objects *) NULL;
	if (!ip->window1)
		ip->window1 = xpipe_slave_window1_window1_create(ip, owner, name);
	if (!ip->textpane1)
		ip->textpane1 = xpipe_slave_window1_textpane1_create(ip, ip->window1);
	return ip;
}

/*
 * Create object `window1' in the specified instance.

 */
Xv_opaque
xpipe_slave_window1_window1_create(ip, owner, name)
	caddr_t		ip;
	Xv_opaque	owner;
	char		*name;
{
	Xv_opaque	obj;
	Xv_opaque		window_image;
	static unsigned short	window_bits[] = {
#include "xpipe_slave.icon"
	};
	Xv_opaque		window_image_mask;
	static unsigned short	window_mask_bits[] = {
#include "xpipe_slave.mask.icon"
	};
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
		XV_WIDTH, 500,
		XV_HEIGHT, 350,
		XV_LABEL, name,
		FRAME_CLOSED, FALSE,
		FRAME_SHOW_FOOTER, TRUE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		FRAME_ICON, xv_create(XV_NULL, ICON,
			XV_LABEL, name,
			ICON_IMAGE, window_image,
			ICON_MASK_IMAGE, window_image_mask,
			NULL),
		NULL);
	return obj;
}

/*
 * Create object `textpane1' in the specified instance.

 */
Xv_opaque
xpipe_slave_window1_textpane1_create(ip, owner)
	caddr_t		ip;
	Xv_opaque	owner;
{
	Xv_opaque	obj;
	
	obj = xv_create(owner, TEXTSW,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		OPENWIN_SHOW_BORDERS, TRUE,
		TEXTSW_MEMORY_MAXIMUM, MEM_MAX,
		TEXTSW_IGNORE_LIMIT, TEXTSW_INFINITY,
		NULL);
	return obj;
}

