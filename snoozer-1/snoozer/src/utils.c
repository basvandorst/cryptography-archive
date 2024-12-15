/*
#  Copyright (c) 1995 by Gino Corradin - Fri Aug 11 21:20:43 MET DST 1995 
#
#             Gino Corradin
#             via T. Albinoni 11
#             45100 Rovigo - ITALY 
#  
#     phone : +39 (0)425 34465 (Q) - +39 (0)425 35523 (fax)
#
#     E-mail : Gino.Corradin@dei.unipd.it
               gino@maya.dei.unipd.it


   See distribution terms in License file.
*/

/* 
** Utils.c : This file contains all commons procedures.
*/
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defs.h"
#include "../config/mail.h"

/*
** ActualTime : determine the date and local time 
*/
#include <time.h>
char *actualtime()
{
long now;
static char st1[30], st2[10];
char *stret;
struct tm *tm;
char *month[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

time(&now);
tm=localtime(&now);
sprintf(st1, "19%d %s %d", tm->tm_year, month[tm->tm_mon], tm->tm_mday);
sprintf(st2, " %d:%d:%d", tm->tm_hour, tm->tm_min, tm->tm_sec);
strcat(st1, st2);
if (tm->tm_isdst > 0) { sprintf(st2, " DST"); strcat(st1, st2); }
(int)stret=&st1;
return(stret);
}

/*
** FreeAll : frees all space used by the jobslist queue structure.
*/
void freeall(jobs)
struct jobslist *jobs;
{
while((int)jobs->next != -1) jobs=jobs->next;
while((int)jobs->prev != -1) {
    jobs=jobs->prev;
    free(jobs->next);
}
free(jobs);
}

/*
** Handle_QUIT : manage various quit signals
*/
void handle_quit()
{
#if (INFO >= 2)
    int  i;
    char st[BUF_SIZE];
#endif

++CountSIGQUIT;
signal(SIGQUIT, handle_quit);
#if (INFO >= 2)
    i=-1; while (ProgramName[++i] != ' ') ;
    strncpy(st, ProgramName, i); st[i]=0;
    printf("Received SIGQUIT (%s) : %d\n", st, CountSIGQUIT);
#endif
}

/*
** Handle_INT : manage various kill signals
*/
void handle_int() 
{ 
#if (INFO >= 2)
    int  i;
    char st[BUF_SIZE];
#endif

++CountSIGINT;
signal(SIGINT, handle_int); 
#if (INFO >= 2)
    i=-1; while (ProgramName[++i] != ' ') ;
    strncpy(st, ProgramName, i); st[i]=0;
    printf("Received SIGINT (%s) : %d\n", st, CountSIGINT);
# endif
}

/*
** IsEmpty : check if a string is empty, i.e. contains only
**           newline (\n) or spaces.
**
**   return codes :
**     .    0 : string is empty
**     . else : position of 1st character in the string
*/
int isempty(st)
char *st;
{
int i, l;

i=-1;
if ((st[0] != '#') && (st[0] != '\n')) {
    l=strlen(st); 
    while ((++i < l) && (st[i] == ' ')) ;
    if ((st[i] == '\n') || (st[i] == '#'))
        return(0); 
    else {
        st[l-1]=0; return(i); }
} else 
    return(0);
}

/*
** Send_Letter : send the text contained in letter structure via mail.
**               No empty records are allowed in the dynamics list.
**               Mailer is MAILER (default /usr/lib/sendmail), 
**               destination user is defined via SEND_MAIL macro 
**               in Makefile.
*/
int send_letter(hletp, elog)
struct letter *hletp;
FILE *elog;
{
int fd[2];
int i;
int pid;
int retval;
struct letter *letp;
char buffer[BUF_SIZE];

/* The idea is that I use a child to execute the mailer, the father 
** of this child send a string to the mailer, end finally, because
** I don't know the way to reopen the stdout file after a close, then
** a fork another child that contains the two processes above.
** In this way the principal father could be replaced by the father 
** forked in the child... */
pid=fork();
if (pid == 0) {
    /* main child : exec the mailer and send a mail */
    pipe(fd);
    switch (fork()) {
        case 0:
            /* little child : exec mailer */
            close(fd[1]); close(0); dup2(fd[0], 0); close(fd[0]);
            set_sendto();
            execl(MAILER, MAILER, SENDTO, 0);
            exit(2);
        case -1:
            /* error cannot fork */
            exit(1);
        default:
            /* little father : send a string to little child (mailer) */
            close(fd[0]); close(1); dup2(fd[1], 1); close(fd[1]);
            letp=hletp;
            while ((int)letp->next != -1) {
                printf("%s", letp->line); fflush(stdout);
                letp=letp->next;
            }
            printf("%s", letp->line); fflush(stdout);     
            wait(&retval);              /* wait the mailer return code */
            if (WEXITSTATUS(retval) == 2) exit(2); 
            exit(0);
    }                                     /* end switch */
} else if (pid == -1) {
    /* cannot fork */
    i=1;
} else {                                   
    /* main father */
    wait(&retval);                        /* wait the child return code */
    i=WEXITSTATUS(retval); 
}

/* now examine various exit codes */
if ((i == 1) || (i == 2)) {
    if (i == 1) {
        fprintf(elog, "%s\n", actualtime()); 
        fputs("Cannot fork, sendmail not executed !\n", elog);
    } else if (i == 2) {
        fprintf(elog, "%s\n", actualtime());
        fputs("Mailer not found, cannot send mail.\n\n", elog);
    }
    /* write mail text to error logfile */
    letp=hletp;
    while ((int)letp->next != -1) {
        fputs(letp->line, elog); 
        letp=letp->next;
    }
}

/* now dealloc mail text from memory */
letp=hletp;
while ((int)letp->next != -1) {
    hletp=letp;
    letp=letp->next;
    free(hletp->line); free(hletp);
}
free(letp->line); free(letp);

return(0);                                /* clean exit */
}

