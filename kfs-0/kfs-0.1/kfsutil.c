#include <stdio.h>
#include <termios.h>

#define	MAX_PHRASE_LEN		256

int fputs(char *s, FILE *stream);
char *fgets(char *s, int n, FILE *stream);
int fflush(FILE *stream);
int tcgetattr(int fd, struct termios *p);
int tcsetattr(int fd, int opt_act, struct termios *p);

char *
getphrase(char *prompt)
{
	int tty_fileno;
	FILE *tty;
	struct termios mytty, origtty;
	static char phrase[MAX_PHRASE_LEN];

	/* Open /dev/tty for reading & writing */
	if ((tty = fopen("/dev/tty", "r+")) == NULL) {
		fputs("Unable to open terminal\n", stderr);
		return NULL;
	}
	tty_fileno = fileno(tty);

	/* Turn of echoing */
	tcgetattr(tty_fileno, &origtty);
	memcpy(&mytty, &origtty, sizeof(struct termios));
	mytty.c_lflag &= ~(ICANON | ECHO);
	mytty.c_cc[VMIN] = 1;
	mytty.c_cc[VTIME] = 0;
	tcsetattr(tty_fileno, TCSANOW, &mytty);

	/* Output prompt and get pass phrase */
	phrase[0] = '\0';
	fputs(prompt, tty);
	fflush(tty);
	fgets(phrase, MAX_PHRASE_LEN, tty);
	phrase[strlen(phrase)-1] = '\0';	/* remove trailing newline */

	/* Restore echoing */
	tcsetattr(tty_fileno, TCSANOW, &origtty);

	return phrase;
}
