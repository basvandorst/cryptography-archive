#ifdef SECURID

#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>

#include "sdi_athd.h"
#include "sdi_size.h"
#include "sdi_type.h"
#include "sdacmvls.h"
#include "sdconf.h"
union config_record configure;

static struct termios saved_ttymode;

#ifdef TEST
main(argc, argv)
int	argc;
char	*argv[];
{
	if (argc != 2) {
		(void)fprintf(stderr, "usage: %s username\n", argv[0]);
		exit(1);
	}

	if (sdiauth(isatty(0), argv[1]))
		puts("FAILED");
	else
		puts("SUCCEEDED");
	return(0);
}
#endif

#define CANCELLED	1

sdiauth(usepty, user)
char	*user;
{
	struct SD_CLIENT sd_dat, *sd;
	int	ret;
	char	passcode[LENPRNST];
	char	*c, reply[3], newpin[LENPRNST], pintype[20], pinsize[20];
	int	failed = 0;
	struct termios noecho_ttymode;
	void    (*oldsig) ();
	static void	sd_restore_ttymode();

	memset(&sd_dat, 0, sizeof(sd_dat));   /* clear struct */
	sd = &sd_dat;
	creadcfg();		/*  access sdconf.rec  */
	if (sd_init(sd)) {
#ifdef TEST
		fprintf(stderr, "Cannot initialize client-server session\n");
#endif
		return(1);
	}

	if (usepty) {
		printf("Enter PASSCODE for %s: ", user);
		(void)fflush(stdout);

		/* ECHO OFF */
		if (tcgetattr(0, &saved_ttymode) == -1) {
			perror("tcgetattr()");
			return(1);
		}
		if ((oldsig = signal(SIGINT, SIG_IGN)) != SIG_IGN)
			signal(SIGINT, sd_restore_ttymode);
		if (tcgetattr(0, &noecho_ttymode) == -1) {
			perror("tcgetattr()");
			return(1)
		}
		noecho_ttymode.c_lflag &= ~ECHO;
		if (tcsetattr(0, TCSANOW, &noecho_ttymode) == -1) {
			perror("tcsetattr()");
			return(1);
		}
	}
	else {
		printf("Enter PASSCODE for %s (echo is on):", user);
		(void)fflush(stdout);
	}

	if (fgets(passcode, sizeof(passcode), stdin) == NULL)
		return(1);

	if (usepty) {
		/* ECHO ON */
		if (tcsetattr(0, TCSANOW, &saved_ttymode) == -1) {
			perror("tcsetattr()");
			return(1);
		}
		if (oldsig != SIG_IGN)
			signal(SIGINT, oldsig);
		printf("\n");
	}

#ifdef TEST
	passcode[strlen(pin) - 1] = '\0';
#else /* rip() is contained in skey library */
	rip(passcode);
#endif

	ret = sd_check(passcode, user, sd);
	switch (ret) {		/* switch on ret from sd_check  */
	case ACM_OK:
		break;

	case ACM_ACCESS_DENIED:
		puts("Access Denied");
		failed = 1;
		break;

	case ACM_NEXT_CODE_REQUIRED:
		printf("Please enter NEXT cartcode: ");
		if (fgets(passcode, sizeof(passcode), stdin) == NULL) {

			failed = 1;
			break;
		}
#ifdef TEST
		passcode[strlen(passcode) - 1] = '\0';
#else /* rip() is contained in skey library */
		rip(passcode);
#endif
		if (sd_next(passcode, sd) == ACM_OK) {
			break;
		}
		printf("BAD NEXT cartcode, access denied.\n");
		failed = 1;
		break;

	case	ACM_NEW_PIN_REQUIRED:
		printf("New PIN required; do you wish to continue? (y/n) [n]: ");
		c = fgets(reply, 3, stdin);
		if (strncmp(reply, "y", 1)) {
			sd_pin("", CANCELLED, sd);
			printf("Cancelled new PIN protocol, access denied.\n");
			failed = 1;
			break;
		}
		if (sd->alphanumeric)
			strcpy(pintype, "characters");
		else
			strcpy(pintype, "digits");

		if (sd->min_pin_len == sd->max_pin_len)
			sprintf(pinsize, "%d", sd->min_pin_len);
		else
			sprintf(pinsize, "%d to %d",
					sd->min_pin_len, sd->max_pin_len);

		if (sd->user_selectable) {
			printf("\nEnter your new PIN, containing %s %s,\n",
							pinsize, pintype);
			printf("                           or\n\
<Return> to generate a new PIN and display it on the screen\n");
			c = fgets(newpin, sd->max_pin_len + 1, stdin);
			if (!strncmp(newpin, "\n", 1)) {
				strncpy(newpin, sd->system_pin, LENMAXPIN);
				printf("\n\nPIN:        %s \n", newpin);
			}
			else
				newpin[strlen(newpin) - 1] = '\0';
			if (sd_pin(newpin, 0, sd) == ACM_NEW_PIN_ACCEPTED) {
				printf("New PIN accepted \n");
				printf(
"Wait for the code on your card to change, then log in with the new PIN\n");
				failed = 1; /* be standard */
				break;
			}
			else {
				printf("New PIN rejected, access denied\n");
				failed = 1;
				break;
			}
		}
		else { /*  system-generated PIN  */
			printf(
"\nPress <Return> to generate a new PIN and display it on the screen\n");
			fgets(reply, 3, stdin);
			if (!strncmp(reply, "\n", 1)) {
				strncpy(newpin, sd->system_pin, LENMAXPIN);
				printf("\n\nPIN:        %s \n", newpin);
			}
			else {
				printf("New PIN protocol aborted, access denied\n");
				failed = 1;
				break;
			}
			if (sd_pin(newpin, 0, sd) == ACM_NEW_PIN_ACCEPTED) {
				printf("New PIN accepted \n");
				printf(
"Wait for the code on your card to change, then log in with the new PIN\n");
				failed = 1; /* be standard */
			}
			else {
				printf("New PIN rejected, access denied.\n");
				failed = 1;
				break;
			}
		}
		break;

	default:
		printf("sdi panics, unknown return code %d \n", ret);
		failed = 1;
		break;
	}
	return(failed);
}

/* restore - restore terminal modes when user aborts command */
void sd_restore_ttymode()
{
	if (tcsetattr(0, TCSANOW, &saved_ttymode) == -1) {
		perror("tcsetattr()");
		return(1);
	}
	exit(1);
}

#endif /* SECURID */
