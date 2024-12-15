/* Copyright 1993 by Scott Gustafson (scott@cadence.com) */

#include "csu.h"
#include "log.h"

#define PGMNAME "new-account"


/* This program is for creating password entries in the csu.local file */
void main(void) {
	char *password, *buffer, *tempbuffer, *newuserid, *userid;
	FILE *csu_file;
#ifdef TERMIO
	struct termios old, new;
#endif
	
	setuploging(PGMNAME);

	password = (char *)malloc(sizeof(password) * MAX_LENGTH);
	userid = (char *)malloc(sizeof(userid) * MAX_LENGTH);
	newuserid = (char *)malloc(sizeof(newuserid) * MAX_LENGTH);
	buffer = (char *)malloc(sizeof(buffer) * MAX_LENGTH);
	tempbuffer = (char *)malloc(sizeof(tempbuffer) * MAX_LENGTH);

        /* get the userid of the caller for loggin info */
        sprintf(userid, "%s", getenv("USER"));

	fprintf(stdout, "User ID(login) to create: ");
	fflush(stdin);
	fscanf(stdin, "%s", newuserid);

	/* check to see if the userid is already in the password file */
	if((csu_file = fopen(CSUNAME, "r")) == NULL)
		error("FILE Read Access Denied for password file: %s", userid);

	while (fgets(buffer, MAX_LENGTH, csu_file) != NULL) {
		strcpy(tempbuffer, strtok(buffer, "\t\n "));

		if (strcmp(tempbuffer, newuserid) == 0) {
			fprintf(stdout, "You need to manually delete the old account for user: %s\n", newuserid);
			fclose(csu_file);
			exit(0);
		}
	}
	fclose(csu_file);

	fprintf(stdout, "Password: ");
#ifdef TERMIO
	tcgetattr(fileno(stdin), &old);
	tcgetattr(fileno(stdin), &new);
	new.c_lflag &= ~ECHO;
	tcsetattr(fileno(stdin), NULL, &new);
	fflush(stdin);
	fgets(password, MAX_LENGTH, stdin);
        account_encode(password, newuserid);
	tcsetattr(fileno(stdin), NULL, &old);

	fprintf(stdout, "\nConfirm your password: ");
	tcsetattr(fileno(stdin), NULL, &new);
	fflush(stdin);
	fgets(tempbuffer, MAX_LENGTH, stdin);
	account_encode(tempbuffer, newuserid);
        tcsetattr(fileno(stdin), NULL, &old);
	fprintf(stdout, "\n");
	if(strcmp(password, tempbuffer) != 0) {
		fprintf(stdout, "Your passwords didn't match.\n");
		exit(0);
	}
#else
	fflush(stdin);
	fgets(password, MAX_LENGTH, stdin);
	account_encode(password, newuserid);
#endif

	if ((csu_file = fopen(CSUNAME, "a")) == NULL)
		error("FILE Write access denied to password file: %s", userid);

	fprintf(csu_file, "%s\t%s\n", newuserid, password);
	fclose(csu_file);
}
