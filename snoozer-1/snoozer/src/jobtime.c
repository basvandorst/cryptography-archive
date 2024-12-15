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
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include "defs.h"

extern void handle_int();
extern void handle_quit();
extern char *actualtime();

void do_time();

struct timelist {
    struct timelist *next;
    char *name;
    long int lctime;
    int seen;                             /* 1 : present in config file */
};

#if defined(SEND_MAIL)
     struct letter *hletp, *letp;         /* struct for mail text */
#endif

/*
** JobTime : this type of job check changed time
*/
int JobTime(jobs)
struct jobslist *jobs;
{
register int i, j, l;
int    run;
char   basename[BUF_SIZE];                /* file path */
char   buffer[BUF_SIZE];                  /* I/O buffer */
char   filename[BUF_SIZE];                /* absolute filename */
char   logfile[BUF_SIZE];                 /* logfile for runtime acts */
char   prefix[BUF_SIZE];                  /* 1st part of filename */
char   st[BUF_SIZE];                      /* common variable */
struct dirent *direntp;                   /* used by readdir routine */
struct timelist *timep, *head, *prec, *p; /* one record for each file */
struct jobslist *son;
struct stat *statp;                       /* used by stat call */
FILE   *cfg;                              /* configuration file */
FILE   *nseen;                            /* new time file */
FILE   *oseen;                            /* old time file */
FILE   *log;                              /* logfile for runtime results */
DIR    *dirp;

signal(SIGINT, handle_int);                   /* restore default action */
signal(SIGQUIT, handle_quit);                 /* arrange the routine */
signal(SIGTERM, SIG_IGN);                     /* ignore it */
CountSIGQUIT=0; CountSIGINT=0;
strncpy(ProgramName, SPACES, strlen(ProgramName)); /* hyde real name */
strcpy(ProgramName, jobs->name);                /* substitute the name */
if (jobs->extra1 == -1) jobs->extra1=1;       /* default rereading value */

/* free memory from "old" jobslist */
if ((int)jobs->prev != -1) {
    /* element in the middle of list */
    jobs->prev->next=jobs->next;              /* exclude son from list */
    jobs->next->prev=jobs->prev;
    son=jobs->next; freeall(son);             /* delete all, preserve jobs */
    (int)jobs->prev=-1; (int)jobs->next=-1;   /* an orderly work :-) */
} else {
    /* this element is the head of list */
    (int)jobs->next->prev=-1;                 /* exclude element from list */
    freeall(jobs->next);                      /* delete the rest of list */
    (int)jobs->next=-1; 
}

/* create the head of the dynamics list, NOTE that to speedup
** the algorithm during various search phases the 1st element 
** of list is ALWAYS EMPTY. */
head=(struct timelist *)malloc(sizeof(struct timelist));
(int)head->name=-1; head->lctime=0; head->seen=0;
head->next=(struct timelist *)malloc(sizeof(struct timelist));
timep=head->next; (int)timep->next=-1; 
(int)timep->name=-1; timep->lctime=0; timep->seen=0;

/* open old result file and creates the dynamic list */
timep=head->next;
if ((oseen=fopen(jobs->infile, "r")) != 0) {
    while ((char *)fgets(buffer, BUF_SIZE, oseen) != 0) {
        i=-1; while (buffer[++i] != ':') ;
        strncpy(st, buffer, i); st[i]=0; 
        timep->name=malloc(i+1); strcpy(timep->name, st);
        j=-1; while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
        timep->lctime=atoi(st);
        timep->next=(struct timelist *)malloc(sizeof(struct timelist));
        timep=timep->next; (int)timep->next=-1; 
        (int)timep->name=-1; timep->lctime=0; timep->seen=0;
    }
    fclose(oseen);
} 

/* now the infinite loop */
sprintf(logfile, "%s.tmp", jobs->outfile);
log=fopen(logfile, "w"); fclose(log);           /* clean runtime log file */
run=0;                                          /* reset counter */
CountSIGINT=1;                                  /* force reread config */ 
for (;;) {
#   if (INFO >= 3)
        printf("time start=%s\n", actualtime());
#   elif (INFO >= 2)
          printf("time job\n"); 
#   endif
    if (CountSIGQUIT > 0) {
        /* there is one or more pending SIGQUIT */
#       if (INFO >= 1)
            printf("Quitting time job...\n");
#       endif
        nseen=fopen(jobs->outfile, "w");
        /* free timelist */
        timep=head->next;
        while ((int)timep->next != -1) {
            fprintf(nseen, "%s:%d\n", timep->name, timep->lctime);
            free(timep->name); p=timep; timep=timep->next; free(p); 
        }
        free(timep); free(head);
        fclose(nseen);
        exit(0);
    }

#   if defined (SEND_MAIL)
        /* Prepare the header of an eventual mail */
        hletp=(struct letter *)malloc(sizeof(struct letter));
        sprintf(st, "From: snoozer\nSubject: Danger in son %s\n", jobs->name);
        hletp->line=malloc(strlen(st)+1); strcpy(hletp->line, st);
        hletp->next=(struct letter *)malloc(sizeof(struct letter));
        letp=hletp->next; (int)letp->next=-1;
#   endif

    if (CountSIGINT > 0) {
        /* I've sent a SIGINT : reread configuration file and
        ** (eventually) rebuilds the dynamics list. */ 
#       if (INFO >= 3)
            printf("Rereading time configuration file.\n");
#       endif
        cfg=fopen(jobs->cfg, "r");
        log=fopen(logfile, "a");
        direntp=(struct dirent *)malloc(sizeof(struct dirent));
        while ((char *)fgets(buffer, BUF_SIZE, cfg) != 0) {
            /* consider only absolute pathnames */
            if (buffer[0] == '/') {
                l=strlen(buffer); buffer[l-1]=0; --l;
                i=l; while (buffer[--i] != '/') ;
                strncpy(basename, buffer, i); basename[i]=0;
                j=-1; while (++i < l) prefix[++j]=buffer[i]; prefix[++j]=0;
                /* basename contains all chars at the left of last
                ** '/', prefix the chars at the right of last '/' */
                if (prefix[0] == '*') {
                    /* this is a directory (i.e. a set of files) */
                    if ((dirp=opendir(basename)) == 0) {
                        /* directory not exist */
                        fprintf(log, "Warning [%s] : <%s> dir not exist.\n", actualtime(), basename);
                    } else {
                        /* directory exist */
                        while ((direntp=readdir(dirp)) != 0) {
                            sprintf(filename, "%s/%s", basename, direntp->d_name);
                            do_time(head, filename, log);
                        }
                        closedir(dirp);
                    }
                } else if (prefix[j-1] == '*') {
                    /* a fixed set of files : prefix="xxx...x*" */
                    prefix[j-1]=0;   /* delete final "*" */
                    if ((dirp=opendir(basename)) == 0) {
                        /* dir not exist */
                        fprintf(log, "Warning [%s] : <%s> dir not exist.\n", actualtime(), basename);
                    } else {
                        while ((direntp=readdir(dirp)) != 0) {
                            if (strstr(direntp->d_name, prefix) == direntp->d_name) {
                                /* prefix is in 1st part of filename */
                                sprintf(filename, "%s/%s", basename, direntp->d_name);
                                do_time(head, filename, log);
                            }
                        }
                        closedir(dirp);
                    }
                } else {
                    /* this is a single file */
                    sprintf(filename, "%s/%s", basename, prefix);
                    do_time(head, filename, log);
                }                                 /* end if buffer == "*" */
            } else if (isempty(buffer) != 0) {    /* else if buffer[0] */
                /* line isn't empty : ignore it */
                fprintf(log, "Ignored [%s] : <%s> invalid line.\n", actualtime(), buffer);
            }                                     /* end if/else buffer[0] */
        }                                         /* end while fgets */
        fclose(cfg);
        fclose(log);
        free(direntp);

        /* now clean dynamic list from old elements */
        timep=head->next; prec=head;
        while ((int)timep->next != -1) {
            if (timep->seen == 0) {
                /* this is an old element */
                prec->next=timep->next;
                p=timep;
                timep=timep->next;
                free(p->name); free(p);
            } else {
                timep->seen=0;
                prec=timep;
                timep=timep->next;
            }
        }

        /* restablish initial conditions */
        CountSIGINT=0;
        run=0;
    } else {                                     /* if CountSIGINT ... */
        /* no pending SIGINT : check each 
        ** element in the dynamics list. */
        log=fopen(logfile, "a");
        statp=(struct stat *)malloc(sizeof(struct stat));
        prec=head; timep=head->next; 
        while ((int)timep->next != -1) {
            if (stat(timep->name, statp) < 0) i=-1; 
            else if (timep->lctime != statp->st_ctime) i=1; 
            else i=0;

            if (i == 0) {
                /* old checksum identical to new */
                timep->seen=1;
            } else if (i == 1) {
                /* new checksum differ from old */
                timep->seen=1;
                timep->lctime=statp->st_ctime;
                sprintf(st, "Danger [%s] : <%s> changed !!!\n", actualtime(), timep->name);
#               if defined(SEND_MAIL)
                    letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                    letp->next=(struct letter *)malloc(sizeof(struct letter));
                    letp=letp->next; (int)letp->next=-1;
#               endif
                fputs(st, log);
            } else if (i == -1) {
                /* file deleted */
                prec->next=timep->next;          /* delete from list */
                sprintf(st, "Danger [%s] : <%s> deleted !!\n", actualtime(), timep->name);
#               if defined(SEND_MAIL)
                    letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                    letp->next=(struct letter *)malloc(sizeof(struct letter));
                    letp=letp->next; (int)letp->next=-1;
#               endif
                fputs(st, log);
                free(timep->name); free(timep);
            }
            prec=timep; timep=timep->next;
        }
        fclose(log);
        free(statp);
    
        if (++run >= jobs->extra1) {                      
            /* completed another check cycle, max number exceeded, 
            ** reread config file. */
            ++CountSIGINT; run=0;
        }
    }                                            /* endif CountSIGINT */

#   if (INFO >= 3)
        printf("time end=%s\n", actualtime());
#   endif

#   if defined(SEND_MAIL)
        if ((int)hletp->next->next != -1) {
            /* danger, send a mail */
            letp->line=malloc(3); strcpy(letp->line, ".\n");
            log=fopen("/dev/null", "w");
            send_letter(hletp, log);
            fclose(log);
        } else {
            /* no problems */
            free(hletp->next); free(hletp->line); free(hletp);
        }
#   endif

    if (jobs->quit == 1) {
        /* quit mode active : exit now */
        kill(getppid(), SIGQUIT);
        sleep(5);                                /* skip next sleep if lucky */
    }

    if (CountSIGQUIT == 0) sleep(jobs->sleep);   /* sleep for a while */
}                                                /* end infinite for */
}

/* 
** Do_Time : if file "buffer" exists then read its last changed
**           time and alloc another empty list record. The 1st element 
**           of the list is head->next (head is empty to speedup
**           search phases). Any actions or errors will be logged
**           in a runtime file pointed by log.
**           No return codes are provided because are unecessary.
*/
void do_time(head, buffer, log)
struct timelist *head;
char buffer[BUF_SIZE];
FILE *log;
{
register int i;
int    found;
char   st[BUF_SIZE];
struct timelist *prec, *timep;
struct stat *statp;

/* this is a filename, is it present in the list ? */
found=0;                               /* 0 : element not found in list */
statp=(struct stat *)malloc(sizeof(struct stat));
timep=head->next; prec=head;
while ((int)timep->next != -1) {
    if (strcmp(timep->name, buffer) == 0) {
        /* element already exist in the list */
        if (stat(timep->name, statp) < 0) i=-1;
        else if (timep->lctime != statp->st_ctime) i=1; 
        else i=0; 
 
        if (i == 0) {
            /* old checksum identical to new */
            timep->seen=1;
        } else if (i == 1) {
            /* new checksum differ from old */
            timep->seen=1;
            timep->lctime=statp->st_ctime;
            sprintf(st, "Danger [%s] : <%s> changed !!!\n", actualtime(), timep->name);
#           if defined(SEND_MAIL)
                letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                letp->next=(struct letter *)malloc(sizeof(struct letter));
                letp=letp->next; (int)letp->next=-1;
#           endif
            fputs(st, log);
        } else if (i == -1) {
            /* file deleted */
            prec->next=timep->next;     /* delete from list */
            sprintf(st, "Danger [%s] : <%s> deleted !!\n", actualtime(), timep->name);
#           if defined(SEND_MAIL)
                letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                letp->next=(struct letter *)malloc(sizeof(struct letter));
                letp=letp->next; (int)letp->next=-1;
#           endif
            fputs(st, log);
            free(timep->name); free(timep);
        }
        found=1;                      /* found the element */
        break;
    } else {
        prec=timep;
        timep=timep->next;
    }
}                                     /* end while "-1" */

if (found != 1) {
    /* this is a new element */
    if (stat(buffer, statp) < 0) i=-1;
    else i=1;

    if (i == 1) {
        /* filename exist and will be added to the list */
        timep->name=malloc(strlen(buffer)+1);
        strcpy(timep->name, buffer); timep->seen=1;
        timep->lctime=statp->st_ctime;
        fprintf(log, "Added [%s] : <%s>.\n", actualtime(), timep->name);
        timep->next=(struct timelist *)malloc(sizeof(struct timelist));
        timep=timep->next; (int)timep->next=-1; 
        (int)timep->name=-1; timep->lctime=0; timep->seen=0;
    } else if (i == -1) {
        /* filename not exist */
        fprintf(log, "Warning [%s] : <%s> not exist (check config).\n", actualtime(), buffer);
    }
}
free(statp);
}                                     /* end proc */

ShowTimeList(timep)
struct timelist *timep;
{
struct timelist {
	struct timelist *next;
	char *name;
	long int time;
	int seen;
};

while ((int)timep->next != -1) {
    printf("-curr=%d-next=%d-\n", timep, timep->next);
	printf("-name=%s-time=%d-seen=%d-\n", timep->name, timep->lctime, timep->seen);
	timep=timep->next;
}
}
