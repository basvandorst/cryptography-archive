#ifndef	xpipe_slave_HEADER
#define	xpipe_slave_HEADER

/*
 * xpipe_slave_ui.h - User interface object and function declarations.
 * This file was generated by `gxv' from `xpipe_slave.G'.
 * DO NOT EDIT BY HAND.
 */

extern Attr_attribute	INSTANCE;


typedef struct {
	Xv_opaque	window1;
	Xv_opaque	textpane1;
} xpipe_slave_window1_objects;

extern xpipe_slave_window1_objects	*xpipe_slave_window1_objects_initialize();

extern Xv_opaque	xpipe_slave_window1_window1_create();
extern Xv_opaque	xpipe_slave_window1_textpane1_create();
#endif
