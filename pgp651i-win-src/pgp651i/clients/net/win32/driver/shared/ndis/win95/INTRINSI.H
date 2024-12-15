/*
Source: Writing Windows VxDs and Device Drivers, 2nd Edition 
by Karen Hazzah, R&D Books, Miller Freeman, Inc. 1996
*/

char *  __cdecl strcpy(char *, const char *);
char *  __cdecl strcat(char *, const char *);
int     __cdecl strcmp(const char *, const char *);
int  __cdecl strlen(const char *);
#pragma intrinsic(strcat)
#pragma intrinsic(strcpy)
#pragma intrinsic(strlen)
