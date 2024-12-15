typedef enum Page
{
	intro = 0,
	info,
	status,
	finish,
	NUM_PAGES
} Page;

typedef struct wizard_data
{
	HINSTANCE		hinst;
	HWND			hwnd;
	HBITMAP			bmp;
	HPALETTE		palette;
	PGPContextRef	context;
	int				cookie;
	int				passes;
	char			drive[5];

}wizard_data;
