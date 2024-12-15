#include <stdlib.h>

extern Attr_attribute	INSTANCE;

extern Xv_opaque	xpipe_menu_create();

typedef struct {
	Xv_opaque	tty_window;
	Xv_opaque	ttypane;
} xpipe_xv_tty_window_objects;

typedef struct {
	Xv_opaque	window;
	Xv_opaque	controls;
	Xv_opaque	button;
} xpipe_window_objects;

typedef struct {
	Xv_opaque	text_window;
	Xv_opaque	textpane;
	int		save;
	char		*file;
} xpipe_xv_text_window_objects;


extern xpipe_window_objects	*xpipe_window_objects_initialize();

extern Xv_opaque	xpipe_window_window_create();
extern Xv_opaque	xpipe_window_controls_create();
extern Xv_opaque	xpipe_window_button_create();

extern xpipe_xv_text_window_objects	*xpipe_xv_text_window_objects_initialize();
extern Xv_opaque	xpipe_xv_text_window_text_window_create();
extern Xv_opaque	xpipe_xv_text_window_textpane_create();
extern Xv_opaque	xpipe_xv_text_window_termpane_create();

Xv_Font			load_font();




extern xpipe_xv_tty_window_objects	*xpipe_xv_tty_window_objects_initialize();
extern Xv_opaque	xpipe_xv_tty_window_tty_window_create();
extern Xv_opaque	xpipe_xv_tty_window_ttypane_create();

xpipe_xv_tty_window_objects * open_tty_window();
char *get_selection();
Menu_item menu_handler();
void text_window_done_handler();

void (*default_done_proc)();





typedef struct Menu_Commands 	Menu_Commands;

struct Menu_Commands {
	int		input, output, line, select, nohup, opencontrol, x, y, dx, dy;
        char            *slave;
        char            *name;
        char   		*parms;
	Menu_Commands	*next;
};


#define NO	0
#define YES	1
#define BREAK   2

