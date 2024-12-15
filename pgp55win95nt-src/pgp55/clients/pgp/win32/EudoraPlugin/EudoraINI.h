/*
** Copyright (C)1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/

#ifndef _EUDORAINI_H
#define _EUDORAINI_H


typedef struct _EUDORAMAPPINGS{
	char extension[10];
	char type[10];
	char creator[10];
	char mimetype[256];
	struct _EUDORAMAPPINGS* next;

}EUDORAMAPPINGS, *PEUDORAMAPPINGS;

void SetDefaultMailAction(void);
BOOL GetEudoraINIPath(char* Path);
BOOL GetEudoraUserId(char* buffer);
PEUDORAMAPPINGS LoadEudoraMappings(void);
	

#endif
