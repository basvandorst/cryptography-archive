/* Copyright 1993 by Scott Gustafson (scott@cadence.com) */

#include "csu.h"
#include "log.h"

#define PGMNAME "respond"

void main(int argc, char **argv) {
	/* string variables */
	char *randstr, *string, *password, *userid;
	char response;
#ifdef TERMIO
        struct termios old, new;
#endif

	setuploging(PGMNAME);

        randstr = (char *)malloc(sizeof(randstr) * MAX_LENGTH);
        string = (char *)malloc(sizeof(string) * MAX_LENGTH * 2);
	password = (char *)malloc(sizeof(password) * MAX_LENGTH);
	userid = (char *)malloc(sizeof(userid) * MAX_LENGTH);

	/* get userid of caller */
	fprintf(stdout,"Is your remote login the same as your local login? (y/n) ");
	fflush(stdin);
	fscanf(stdin, "%c", &response);
	if ((response == 'y') || (response == 'Y'))
		sprintf(userid, "%s", getenv("USER"));
	else {
		fprintf(stdout, "User ID(login): ");
		fflush(stdin);
		fscanf(stdin, "%s", userid);
	}

	/* verify that program was invoked with the correct name */
	if (strcmp(PGMNAME,argv[0]) != 0 )
	    error("INVALID PROGRAM NAME Access Denied: %s", userid);

	/* get remote prompt*/
	fprintf(stdout, "Delta: ");
	fflush(stdin);
	fscanf(stdin, "%s", randstr);

	/* get the user's password */
	fprintf(stdout, "Password: ");
#ifdef TERMIO
        tcgetattr(fileno(stdin), &old);
        tcgetattr(fileno(stdin), &new);
        new.c_lflag &= ~ECHO;
        tcsetattr(fileno(stdin),NULL, &new);
        fflush(stdin);
        fgets(password, MAX_LENGTH, stdin);
        account_encode(password, userid);
        tcsetattr(fileno(stdin), NULL, &old);
        fprintf(stdout, "\n");
#else
	fflush(stdin);
	fgets(password, MAX_LENGTH, stdin);
	account_encode(password, userid);
#endif /* TERMIO */

	/*   Compute local response*/
	strcpy(string, randstr);
	strcat(string, password);
#ifdef SECURE
	account_encode(string, userid);
#else
	encode(string, userid);
#endif /* SECURE */

	fprintf(stdout, "Response: %s\n", string);
}
