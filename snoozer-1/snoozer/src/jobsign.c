/*
#  Copyright (c) 1995 by Gino Corradin - Fri Dec 2 11:30:43 MET DST 1995 
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
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include "defs.h"

extern void handle_int();
extern void handle_quit();
extern char *actualtime();

void do_sign();

struct signlist {
    struct signlist *next;
    char *name;
    char sum[33];
    int seen;                             /* 1 : present in config file */
};

#if defined (SEND_MAIL)
     struct letter *hletp, *letp;         /* head and element for mail text */
#endif

/*
** JobSign : this type of job signs various files
*/
int JobSign(jobs)
struct jobslist *jobs;
{
register int i, j;
int    l;
int    run;
char   basename[BUF_SIZE];                /* filepath */
char   buffer[BUF_SIZE];                  /* I/O buffer */
char   filename[BUF_SIZE];                /* complete filename */
char   logfile[BUF_SIZE];                 /* log here runtime results */
char   prefix[BUF_SIZE];                  /* 1st part of filename */
char   st[BUF_SIZE];                      /* common variable */
struct dirent *direntp;                   /* used by readdir routine */
struct signlist *signp, *head, *prec, *p; /* one record for each file */
struct jobslist *son;
FILE   *cfg;                              /* configuration file */
FILE   *nseen;                            /* new checksum file */
FILE   *oseen;                            /* old checksum file */
FILE   *log;                              /* logfile for runtime results */
DIR    *dirp;

signal(SIGINT, handle_int);                   /* restore default action */
signal(SIGQUIT, handle_quit);                 /* arrange the routine */
signal(SIGTERM, SIG_IGN);
CountSIGQUIT=0; CountSIGINT=0;
strncpy(ProgramName, SPACES, strlen(ProgramName)); /* hyde real name */
strcpy(ProgramName, jobs->name);                /* substitute the name */
if (jobs->extra1 == -1) jobs->extra1=1;       /* default rereading value */
if (jobs->extra2 == -1) jobs->extra2=MD5_SIG; /* default checksum value */

/* free memory from "old" jobslist */
if ((int)jobs->prev != -1) {
    /* element is in the middle of the list */
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
** of list is ALWAYS EMPTY. Last element is empty too. */
head=(struct signlist *)malloc(sizeof(struct signlist));
(int)head->name=-1; head->sum[0]=0; head->seen=0;
head->next=(struct signlist *)malloc(sizeof(struct signlist));
signp=head->next; (int)signp->next=-1; 
(int)signp->name=-1; signp->sum[0]=0; signp->seen=0;

/* open old result file and creates the dynamic list */
signp=head->next;
if ((oseen=fopen(jobs->infile, "r")) != 0) {
    while ((char *)fgets(buffer, BUF_SIZE, oseen) != 0) {
        i=-1; while (buffer[++i] != ':') ;
        strncpy(st, buffer, i); st[i]=0; 
        signp->name=malloc(i+1); strcpy(signp->name, st);
        j=-1; while (buffer[++i] != '\n') signp->sum[++j]=buffer[i]; 
        signp->sum[32]=0;
        signp->next=(struct signlist *)malloc(sizeof(struct signlist));
        signp=signp->next; (int)signp->next=-1; 
        (int)signp->name=-1; signp->sum[0]=0; signp->seen=0;
    }
    fclose(oseen);
}

/* now the infinite loop */
sprintf(logfile, "%s.tmp", jobs->outfile);      /* logfile for runtime acts */
log=fopen(logfile, "w"); fclose(log);           /* clean runtime log file */
run=0;                                          /* reset counter */
CountSIGINT=1;                                  /* force reread config */ 
for (;;) {
#   if (INFO >= 3)
        printf("sign start=%s\n", actualtime());
#   elif (INFO >= 2)
        printf("sign job\n"); 
#   endif
    if (CountSIGQUIT > 0) {
        /* there is one or more pending SIGQUIT */
#       if (INFO >= 1)
            printf("Quitting sign job...\n");
#       endif
        nseen=fopen(jobs->outfile, "w");
        /* free signlist */
        signp=head->next;
        while ((int)signp->next != -1) {
            fprintf(nseen, "%s:%s\n", signp->name, signp->sum);
            free(signp->name); p=signp; signp=signp->next; free(p); 
        }
        free(signp); free(head);
        fclose(nseen);
        exit(0);
    }

#   if defined(SEND_MAIL)
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
            printf("Rereading sign configuration file.\n");
#       endif
        cfg=fopen(jobs->cfg, "r");
        log=fopen(logfile, "a");
        direntp=(struct dirent *)malloc(sizeof(struct dirent));
        while ((char *)fgets(buffer, BUF_SIZE, cfg) != 0) {
            /* only absolute pathnames */
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
                        fprintf(log, "Warning [%s] : <%s> dir not exist (check config).\n", actualtime(), basename);
                    } else {
                        /* directory exist */
                        while ((direntp=readdir(dirp)) != 0) {
                            sprintf(filename, "%s/%s", basename, direntp->d_name);
                            do_sign(head, filename, log, jobs->extra2);
                        }
                        closedir(dirp);
                    }
                } else if (prefix[j-1] == '*') {
                    /* a fixed set of files : prefix="xxx...x*" */
                    prefix[j-1]=0;           /* delete final "*" */
                    if ((dirp=opendir(basename)) == 0) {
                        /* directory not exist */
                        fprintf(log, "Warning [%s] : <%s> dir not exist (check config).\n", actualtime(), basename);
                    } else {
                        /* dir exists */
                        while ((direntp=readdir(dirp)) != 0) {
                            if (strstr(direntp->d_name, prefix) == direntp->d_name) {
                                /* prefix is in 1st part of filename */
                                sprintf(filename, "%s/%s", basename, direntp->d_name);
                                do_sign(head, filename, log, jobs->extra2);
                            }
                        }
                        closedir(dirp);
                    }
                } else {
                    /* an unique file */
                    sprintf(filename, "%s/%s", basename, prefix);
                    do_sign(head, filename, log, jobs->extra2);
                }                                     /* end if buffer == "*" */
            } else if (isempty(buffer) != 0) {        /* else if buffer[0] */
                /* line isn't empty : ignore it */
                fprintf(log, "Ignored [%s] : <%s> invalid line.\n", actualtime(), buffer);
            }                                         /* end if buffer[0] */
        }                                             /* end while fgets */
        fclose(cfg);
        fclose(log);
        free(direntp);

        /* now clean dynamic list from old elements */
        signp=head->next; prec=head;
        while ((int)signp->next != -1) {
            if (signp->seen == 0) {
                /* this is an old element */
                prec->next=signp->next;
                p=signp;
                signp=signp->next;
                free(p->name); free(p);
            } else {
                signp->seen=0;
                prec=signp;
                signp=signp->next;
            }
        }

        /* restablish initial conditions */
        CountSIGINT=0;
        run=0;
    } else {                                     /* if CountSIGINT ... */
        /* no pending SIGINT : check each 
        ** element in the dynamics list. */
        log=fopen(logfile, "a");
        prec=head; signp=head->next; 
        while ((int)signp->next != -1) {
            switch (jobs->extra2) {
                case CRC32_SIG:
                    i=crc32_sig(signp->name, signp->sum); break;
                case MD5_SIG:
                    i=md5_sig(signp->name, signp->sum); break;
                case MD4_SIG:
                    i=md4_sig(signp->name, signp->sum); break;
            }
            if (i == 0) {
                /* old checksum identical to new */
                signp->seen=1;
            } else if (i == 1) {
                /* new checksum differ from old */
                signp->seen=1;
                sprintf(st, "Danger [%s] : <%s> changed !!!\n", actualtime(), signp->name);
#               if defined(SEND_MAIL)
                    letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                    letp->next=(struct letter *)malloc(sizeof(struct letter));
                    letp=letp->next; (int)letp->next=-1;
#               endif
                fputs(st, log);
            } else if (i == -1) {
                /* file deleted */
                prec->next=signp->next;     /* delete from list */
                sprintf(st, "Danger [%s] : <%s> deleted !!\n", actualtime(), signp->name);
#               if defined(SEND_MAIL)
                    letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                    letp->next=(struct letter *)malloc(sizeof(struct letter));
                    letp=letp->next; (int)letp->next=-1;
#               endif
                fputs(st, log);
                free(signp->name); free(signp);
            }
            prec=signp; signp=signp->next;
        }
        fclose(log);
    
        if (++run >= jobs->extra1) {                      
            /* completed another check cycle, max number exceeded, 
            ** reread config file. */
            ++CountSIGINT; run=0;
        }
    }                                               /* endif CountSIGINT */

#   if (INFO >= 3)
        printf("sign end=%s\n", actualtime());
#   endif

#   if defined(SEND_MAIL)
        if ((int)hletp->next->next != -1) {
            /* danger, send a mail */
            letp->line=malloc(3); strcpy(letp->line, ".\n");
            log=fopen("/dev/null", "w");
            send_letter(hletp, log);
            fclose(log);
        } else {
            /* no problem, all quiet */
            free(hletp->next); free(hletp->line); free(hletp);
        }
#   endif

    if (jobs->quit == 1) {
        /* quit mode active : exit now */
        kill(getppid(), SIGQUIT);
        sleep(5);                                /* skip next sleep if lucky */
    }

    if (CountSIGQUIT == 0) sleep(jobs->sleep);      /* sleep for a while */
}                                                   /* end infinite for */
}

/* 
** Do_Sign : if file "buffer" exists then calculates its checksum
**           and alloc another empty list record. The 1st element 
**           of the list is head->next (head is empty to speedup
**           search phases). Any actions or errors will be logged
**           in a runtime file pointed by log.
**           Sig_type is a particular signature type (md5, ...).
**           No return codes are provided because are unecessary.
*/
void do_sign(head, buffer, log, sig_type)
struct signlist *head;
char buffer[1024];
FILE *log;
int sig_type;
{
int  i;
int  found;
char st[1024];
register struct signlist *prec, *signp;

/* this is a filename, is it present in the list ? */
found=0;                               /* 0 : element not found in list */
signp=head->next; prec=head;
while ((int)signp->next != -1) {
    if (strcmp(signp->name, buffer) == 0) {
        /* element already exist in the list */
        switch (sig_type) {
            case CRC32_SIG:
                i=crc32_sig(signp->name, signp->sum); break;
            case MD5_SIG:
                i=md5_sig(signp->name, signp->sum); break;
            case MD4_SIG:
                i=md4_sig(signp->name, signp->sum); break;
        }
        if (i == 0) {
            /* old checksum identical to new */
            signp->seen=1;
        } else if (i == 1) {
            /* new checksum differ from old */
            signp->seen=1;
            sprintf(st, "Danger [%s] : <%s> changed !!!\n", actualtime(), signp->name);
#           if defined(SEND_MAIL)
                letp->line=malloc(strlen(st)+1); 
                strcpy(letp->line, st);
                letp->next=(struct letter *)malloc(sizeof(struct letter));
                letp=letp->next; (int)letp->next=-1;
#           endif
            fputs(st, log);
        } else if (i == -1) {
            /* file deleted */
            prec->next=signp->next;     /* delete from list */
            sprintf(st, "Danger [%s] : <%s> deleted !!\n", actualtime(), signp->name);
#           if defined(SEND_MAIL)
                letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                letp->next=(struct letter *)malloc(sizeof(struct letter));
                letp=letp->next; (int)letp->next=-1;
#           endif
            fputs(st, log);
            free(signp->name); free(signp);
        }
        found=1;                      /* found the element */
        break;
    } else {
        prec=signp;
        signp=signp->next;
    }
}                                     /* end while "-1" */

if (found != 1) {
    /* this is a new element */
    switch (sig_type) {
        case CRC32_SIG:
            i=crc32_sig(buffer, signp->sum); break;
        case MD5_SIG:
            i=md5_sig(buffer, signp->sum); break;
        case MD4_SIG:
            i=md4_sig(buffer, signp->sum); break;
    }
    if (i == 1) {
        /* filename exist, checksum calculated */
        signp->name=malloc(strlen(buffer)+1);
        strcpy(signp->name, buffer); signp->seen=1;
        sprintf(st, "Added [%s] : <%s>.\n", actualtime(), signp->name);
#       if defined(SEND_MAIL)
            letp->line=malloc(strlen(st)+1); 
            strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, log);
        signp->next=(struct signlist *)malloc(sizeof(struct signlist));
        signp=signp->next; (int)signp->next=-1; 
        (int)signp->name=-1; signp->sum[0]=0; signp->seen=0;
    } else if (i == -1) {
        /* filename not exist */
        fprintf(log, "Warning [%s] : <%s> not exist (check config).\n", actualtime(), buffer);
    }
}
}                                     /* end proc */




ShowList(signp)
struct signlist *signp;
{
struct signlist {
	struct signlist *next;
	char *name;
	char sum[33];
	int seen;
};

while ((int)signp->next != -1) {
    printf("-curr=%d-next=%d-\n", signp, signp->next);
	printf("-name=%s-sum=%s-seen=%d-\n", signp->name, signp->sum, signp->seen);
	signp=signp->next;
}
}

