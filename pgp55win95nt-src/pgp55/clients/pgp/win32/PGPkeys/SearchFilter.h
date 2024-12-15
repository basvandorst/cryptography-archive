#ifndef _SEARCHFILTER_H
#define _SEARCHFILTER_H

#define ACTION_UNION		1
#define ACTION_INTERSECT	2
#define ACTION_NEGATE		3

PGPError CreateFilter(HWND hwnd, PGPFilterRef* filter, int* action);

#endif