/* Copyright 1993 by Scott Gustafson (scott@cadence.com) */

/* This program needs to have suid root and owner root */

#include "csu.h"
#include "log.h"
#include "genstr.h"

/* the program name is needed for sysloging information and to check that you
 * are running the program with the correct name */
#define PGMNAME "csu"

/* this function reads the CSUNAME file to get the users password */
char *load_key(const char *path, char *userid) {
	char *buffer, *tempbuffer; 
	FILE *csu_file;
	short int done = FALSE;

	buffer = (char *)malloc(sizeof(buffer) * MAX_LENGTH);
	tempbuffer = (char *)malloc(sizeof(tempbuffer) * MAX_LENGTH);

	/* open the file */
	if ((csu_file = fopen(path, "r")) == NULL)
		error("FILE Read Access Denied for password file: %s", userid);

	/* read the file line by line until done is true */
	while ((fgets(buffer, MAX_LENGTH, csu_file) != NULL) && (done != TRUE)) {

		/* grap the first token in the file which is the user name */
		strcpy(tempbuffer, strtok(buffer, "\t\n "));

		/* compare the token with the real userid */
		if (strcmp(tempbuffer, userid) == 0) {

			/* if they are the same, then grap the next token from
			 * the buffer and that will be the password */
			strcpy(tempbuffer, strtok(NULL, "\t\n "));
			done = TRUE;
		}
	}
	/* close the file before leaving */
	fclose(csu_file);
#ifdef DEBUG
	fprintf(stdout, "User ID:%s\tKey:%s\n", userid, tempbuffer);
#endif
	/* return the password string*/
	return(tempbuffer);
}

void main (int argc, char **argv) {
	struct stat filestat;
	/* string variables */
	char *userid, *string, *randstr, *remoteresponse, *localresponse;
	char *key = NULL;

	setuploging(PGMNAME);

	userid = (char *)malloc(sizeof(userid) * MAX_LENGTH);
	remoteresponse = (char *)malloc(sizeof(remoteresponse) * MAX_LENGTH);
	localresponse = (char *)malloc(sizeof(localresponse) * MAX_LENGTH);
	string = (char *)malloc(sizeof(string) * MAX_LENGTH);

	/* get the userid of the caller */
	sprintf(userid, "%s", getenv("USER"));

	/* log the attempts to run the program */
	log("Starting up. User: %s", userid);

	/* verify that program was invoked with the correct name */
	if (strcmp(PGMNAME, argv[0]) != 0 )
		error("INVALID NAME Access Denied: %s", userid);

	/* stat secure file -check permissions, uid, gid */
	/* this only checks the csu.local file */
	stat(CSUNAME, &filestat);
	if (filestat.st_uid != 0 || filestat.st_gid != GID || filestat.st_mode != MODE)
		error("PASSWORD FILE MODE CORRUPT. Access denied: %s", userid);

	/* Generate a random string and crypt it */
	randstr = genstr();
#ifdef SECURE
	account_encode(randstr, userid);
#else
	encode(randstr, userid);
#endif /* SECURE */

	/* print the Delta over time and then get the remote response */
	fprintf(stdout, "Delta: %s\nResponse: ", randstr);
	scanf("%s", remoteresponse);

	/* Compute expected repsonse */
	/* Read in the key for this userid */
	if ((key = load_key(CSUNAME, userid)) == NULL)
		error("UNAUTHORIZED USER. Access denied: %s", userid);

	strcpy(string, randstr);
	strcat(string, key);
#ifdef SECURE
	account_encode(string, userid);
#else
	encode(string, userid);
#endif /*SECURE */

	if (strcmp(string,remoteresponse) == 0 ) {
		log("SUCCESS. Root access granted to User: %s", userid);
		setuid((int)0);
		system("/bin/sh");	/* WARNING SUPER USER ACCESS */
	}
	else
		error("FAILURE. Incorrect response from User: %s", userid);
}
