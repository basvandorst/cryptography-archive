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
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defs.h"
#include "../config/selfcheck.h"
#include "version.h"
/*#include "debug.h"/**/

int  sleeptime=SLEEP;               /* main sleeps for sleeptime seconds */
char elog[BUF_SIZE];                /* snoozer errorlog filename */
char jobtab[BUF_SIZE];              /* snoozer jobtab filename */
FILE *elog_f;                       /* filedes for errlog file */
FILE *jobtab_f;                     /* filedes for jobtable file */

extern char *actualtime();
extern void handle_int();
extern void handle_quit();
void usage();

main(argc, argv)
int argc;
char *argv[];
{
register int i,j;
int    retval;                        /* used only by wait */
short  Changed;                       /* main is changed ? 0=no */
short  InMain;                        /* I'm in main seq ? 0=no */
short  InOwn=0;                       /* I'm in own seq ? 0=no */
short  InPerm=0;                      /* I'm in perm seq ? 0=no */
short  InSign=0;                      /* I'm in sign seq ? 0=no */
short  InTime=0;                      /* I'm in time seq ? 0=no */
short  QuitMode=0;                    /* 0 : normal working mode */
int    RetValue=0;                    /* exit and return this value */
int    line;                          /* Counts lines in jobtab */
char   buffer[BUF_SIZE];              /* general buffer string */
char   st[BUF_SIZE];                  /* common variable */
struct jobslist *jobs, *head, *tmpjob, *p;
struct letter *hletp, *letp;
FILE   *tmpfile;

/* Initialize signal routines */
signal(SIGINT, handle_int); CountSIGINT=0;
signal(SIGTERM, handle_quit);
signal(SIGQUIT, handle_quit); CountSIGQUIT=0; 

/* parse line parameters */
if (lineparser(argc, argv) == 2) QuitMode=1; 

/* move in the right place */
chdir(SNOOZERHOME);

/* I'll use this pointer to change
** the name of various son-jobs. */ 
ProgramName=argv[0]; 

/* Open errorlog : check only */
if ((elog_f=fopen(elog, "w")) == 0) {
    printf("<%s> can't be opened.\n", elog);
    freeall(head); 
    exit(1);
}
fclose(elog_f);

/* Write the program pid into a file */
sprintf(st, "%s.pid", jobtab);
if ((tmpfile=fopen(st, "w")) != 0) {
    i=getpid();
    fprintf(tmpfile, "%d", i); 
    fclose(tmpfile);
#   if defined(SEND_MAIL)
        strcpy(buffer, "From: snoozer\nSubject: Warning : snoozer started\n");
        if (QuitMode == 1) 
            sprintf(st, "Start snoozer (auto quitmode) with pid <%d>.\n.\n", i);
        else 
            sprintf(st, "Start snoozer with pid <%d>.\n.\n", i);
        strcat(buffer, st);
        hletp=(struct letter *)malloc(sizeof(struct letter)); 
        (int)hletp->next=-1;
        hletp->line=malloc(strlen(buffer)+1); strcpy(hletp->line, buffer);
        elog_f=fopen(elog, "a");
        send_letter(hletp, elog_f);
        fclose(elog_f);
#   endif
} else {
    printf("<%s> can't be opened.\n", st);
    freeall(head); 
    exit(5);
}

#if defined (AUTO)
     /* Auto-signature method : snoozer computes the checksum 
     ** of itself, so nobody can modify the program. */
     self_check();
#endif

/* Create the 1st element of the queue and a temporary one */
head=(struct jobslist *)malloc(sizeof(struct jobslist));
(int)head->prev=-1; (int)head->next=-1; 
cleanjob(head);                     /* not necessary, only prudence */
tmpjob=(struct jobslist *)malloc(sizeof(struct jobslist));

ReCycle :
#if (INFO >= 3)
    printf("main start=%s\n", actualtime());
#elif (INFO >= 2)
    printf("main\n");
#endif
/* Any pending sigquit ? If yes then : 
**    1) kill every job, 2) dealloc used memory, 3) exit
*/
if (CountSIGQUIT > 0) {
    signal(SIGINT, SIG_IGN); 
    signal(SIGQUIT, SIG_IGN);
#   if (INFO >= 3)
        printf("main : send SIGQUIT signal.\n");
#   endif
#   if defined(SEND_MAIL)
        hletp=(struct letter *)malloc(sizeof(struct letter));
        sprintf(st, "From: snoozer\nSubject: Warning : snoozer quitted\n");
        hletp->line=malloc(strlen(st)+1); strcpy(hletp->line, st);
        hletp->next=(struct letter *)malloc(sizeof(struct letter));
        letp=hletp->next; 
        if (QuitMode == 1)
            sprintf(st, "\nParent (auto quitmode) : pid=%d name=%s\n", getpid(), ProgramName);
        else
            sprintf(st, "\nParent : pid=%d name=%s\n", getpid(), ProgramName);
        letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
        letp->next=(struct letter *)malloc(sizeof(struct letter));
        letp=letp->next; (int)letp->next=-1;
#   endif
    jobs=head;
    while ((int)jobs->next != -1) {
        kill(jobs->pid, SIGQUIT);
        wait(&retval);
#       if defined(SEND_MAIL)
            sprintf(st, "<name=%s cfg=%s pid=%d> killed.\n", jobs->name, jobs->cfg, jobs->pid);
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        jobs=jobs->next;
    }
    freeall(head);
#   if defined(SEND_MAIL)
        letp->line=malloc(3); strcpy(letp->line, ".\n");
        elog_f=fopen(elog, "a");
        send_letter(hletp, elog_f);
        fclose(elog_f);
#   endif
    exit(RetValue);
}

/* Any pending sigint ? if yes then :
**    1) forward signal to any jobs
*/
if (CountSIGINT > 0) {
#   if (INFO >= 3)
        printf("main : send SIGINT signal.\n");
#   endif
    jobs=head;
    while ((int)jobs->next != -1) {
        kill(jobs->pid, SIGINT);
        jobs=jobs->next;
    }
    CountSIGINT=0;
}

/* Open errorlog : append mode (periodically reopened) */
if ((elog_f=fopen(elog, "a")) == 0) {
    printf("<%s> can't be opened.\n", elog);
    kill(getpid(), SIGQUIT);
    RetValue=1;
    goto ReCycle;
}

/* Open jobtab file */
if ((jobtab_f=fopen(jobtab, "r")) == 0) {
    fprintf(elog_f, "<%s> file not found, exit !\n", jobtab);
    fclose(elog_f);
    kill(getpid(), SIGQUIT);
    RetValue=6;
    goto ReCycle;
}

Changed=0;
line=0;
InMain=0; 
/* Reads smain-emain sequence */
while ((char *)fgets(buffer, BUF_SIZE, jobtab_f) != 0) {
    ++line;
    if (buffer[0] != '#') {
        if (InMain == 0) {
            if (strstr(buffer, "smain") != 0) InMain=1;
            else if (strstr(buffer, "emain") != 0) {
                badsintax(3, line, buffer); 
                goto ReCycle;
            } else if (isempty(buffer) != 0) {
                /* smain-emain sequence not exist, reset filepointer 
                ** to the beginning of the file and starts to examine 
                ** various sjob-ejob sequences. */
                fseek(jobtab_f, 0L, SEEK_SET);
                break;
            }
        } else if (InMain == 1) 
            if (strstr(buffer, "name:") != 0) {
                j=-1; i=3; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                strncpy(ProgramName, SPACES, strlen(ProgramName));
                strcpy(ProgramName, st);
            } else if (strstr(buffer, "sleep:") != 0) {
                j=-1; i=4; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                sleeptime=atoi(st);
            } else if (strstr(buffer, "emain") != 0) {
                /* Construct 1st element of list */
                InMain=0;
                break;
            } else if (isempty(buffer) != 0) {
                /* unkown field */
                badsintax(3, line, buffer);
                goto ReCycle;
            }
    }
}

/* a little initialization */
InOwn=0; InPerm=0; InSign=0; InTime=0;
cleanjob(tmpjob);

/* search trough jobtable for possible sons */
while ((char *)fgets(buffer, BUF_SIZE, jobtab_f) != 0) {
    ++line;
    if (buffer[0] != '#') {
        /* Guess the job type */
        if ((InOwn == 0) && (InPerm == 0) && (InSign == 0) && (InTime == 0)) {
            if (strstr(buffer, "sjob") != 0) {
                if (strstr(buffer, " own") != 0) {
                    InOwn=1; strcpy(tmpjob->type, "own");
                } else if (strstr(buffer, " perm") != 0) {
                    InPerm=1; strcpy(tmpjob->type, "perm"); 
                } else if (strstr(buffer, " sign") != 0) {
                    InSign=1; strcpy(tmpjob->type, "sign");
                } else if (strstr(buffer, " time") != 0) {
                    InTime=1; strcpy(tmpjob->type, "time");
                } else {
                    /* unkown job type */
                    badsintax(6, line, buffer);
                    goto ReCycle;
                }
            } else if (strstr(buffer, "ejob") != 0) {
    		    badsintax(5, line, buffer);
    		    goto ReCycle;
            } else if (strstr(buffer, "smain") != 0) {
                /* no recursion :-) */
                badsintax(4, line, buffer);
    		    goto ReCycle;
            } else {
                /* unkown word, use only sjob, ejob, ... */
                i=-1; while (buffer[++i] != '\n') ; buffer[i]=0;
                j=i; i=-1; while ((buffer[++i] == ' ') && (i<j)) ;
                if (buffer[i] != 0) {
                    fprintf(elog_f, "%s\n", actualtime()); 
                    fputs("Warning : unkown word, line ignored.\n", elog_f);
                    fprintf(elog_f, "          line %d : <%s>\n\n", line, buffer);
                }
            }
        } else {
            /* examine a particular type of job */
            if (strstr(buffer, "name:") != 0) {
                j=-1; i=3; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                strcpy(tmpjob->name, st);
            } else if (strstr(buffer, "sleep:") != 0) {
                j=-1; i=4; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                tmpjob->sleep=atoi(st);
            } else if (strstr(buffer, "cfg:") != 0) {
                j=-1; i=2; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                if ((tmpfile=fopen(st, "r")) == 0) {
                    /* configuration file not exist */
                    badsintax(11, line, buffer); goto ReCycle;
                }
                fclose(tmpfile);
                strcpy(tmpjob->cfg, st);
            } else if (strstr(buffer, "nseen:") != 0) {
                if ((InSign == 0) && (InTime == 0)) {
                    /* field not allowed in this type of job */
                    badsintax(8, line, buffer); goto ReCycle;
                }
                j=-1; i=4; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                strcpy(tmpjob->outfile, st);
            } else if (strstr(buffer, "oseen:") != 0) {
                if ((InSign == 0) && (InTime == 0)) {
                    /* field not allowed in this type of job */
                    badsintax(8, line, buffer); goto ReCycle;
                }
                j=-1; i=4; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                strcpy(tmpjob->infile, st);
            } else if (strstr(buffer, "out:") != 0) {
                if ((InPerm == 0) && (InOwn == 0)) {
                    /* field not allowed in this type of job */
                    badsintax(8, line, buffer); goto ReCycle;
                }
                j=-1; i=2; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                strcpy(tmpjob->outfile, st);
            } else if (strstr(buffer, "run:") != 0) {
                if ((InSign == 0) && (InTime == 0)) {
                    /* field not allowed in this type of job */
                    badsintax(8, line, buffer); goto ReCycle;
                }
                j=-1; i=2; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                /* In jobsign, and jobtime configuration file must be 
                ** reread every "extra1" times, the default value is 1. */
                tmpjob->extra1=abs(atoi(st));
            } else if (strstr(buffer, "sig:") != 0) {
                if (InSign == 0) {
                    /* field not allowed in this type of job */
                    badsintax(8, line, buffer); goto ReCycle;
                }
                j=-1; i=2; while (buffer[++i] != ':') ;
                while (buffer[++i] != '\n') st[++j]=buffer[i]; st[++j]=0;
                /* In jobsign, it's possible to choose a particular
                ** type of file checksum. CRC32, MD4, MD5 are available. */
                if (strcmp(st, "crc32") == 0) tmpjob->extra2=CRC32_SIG;
                else if (strcmp(st, "md5") == 0) tmpjob->extra2=MD5_SIG;
                else if (strcmp(st, "md4") == 0) tmpjob->extra2=MD4_SIG;
                else {
                    /* unknown checksum type */
                    badsintax(10, line, buffer); goto ReCycle;
                }
            } else if (strstr(buffer, "ejob") != 0) {
                /* I've reached the end of this sequence */
                InOwn=0; InPerm=0; InSign=0; InTime=0;

                /* Global (end last) check for sintax errors */
                if (strcmp(tmpjob->name, "") == 0) {
                    badsintax(7, 0, 0); goto ReCycle; }
                if (strcmp(tmpjob->cfg, "") == 0) {
                    badsintax(7, 0, 0); goto ReCycle; }
                if ((strcmp(tmpjob->infile, "") == 0) && (strcmp(tmpjob->outfile, "") == 0)) {
                    badsintax(7, 0, 0); goto ReCycle; }

                /* Check if tmpjob it's a new job or a new version
                ** of a job that already exist in the jobslist. */
                jobs=head;
                while ((int)jobs->next != -1) {
                    if (strcmp(tmpjob->cfg, jobs->cfg) == 0) {
                        /* this job already exists in jobslist */ 
                        jobs->seen=1;
                        if (strcmp(jobs->type, tmpjob->type) != 0) {
                            strcpy(jobs->type, tmpjob->type); jobs->changed=1; }
                        if (strcmp(jobs->name, tmpjob->name) != 0) {
                            strcpy(jobs->name, tmpjob->name); jobs->changed=1; }
                        if (jobs->sleep != tmpjob->sleep) {
                            jobs->sleep=tmpjob->sleep; jobs->changed=1; }
                        if (strcmp(jobs->infile, tmpjob->infile) != 0) {
                            strcpy(jobs->infile, tmpjob->infile); jobs->changed=1; }
                        if (strcmp(jobs->outfile, tmpjob->outfile) != 0) {
                            strcpy(jobs->outfile, tmpjob->outfile); jobs->changed=1; }
                        if (jobs->extra1 != tmpjob->extra1) {
                            jobs->extra1=tmpjob->extra1; jobs->changed=1; }
                        if (jobs->extra2 != tmpjob->extra2) {
                            jobs->extra2=tmpjob->extra2; jobs->changed=1; }
                        cleanjob(tmpjob);
                        break;
                    } else 
                        jobs=jobs->next;
                }

                if ((strcmp(tmpjob->cfg, jobs->cfg) != 0) 
                && (strcmp(tmpjob->cfg, "") != 0)){
                    /* This is a real new element */
                    strcpy(jobs->type, tmpjob->type); 
                    strcpy(jobs->name, tmpjob->name);
                    jobs->sleep=tmpjob->sleep; 
                    strcpy(jobs->cfg, tmpjob->cfg); 
                    strcpy(jobs->infile, tmpjob->infile);
                    strcpy(jobs->outfile, tmpjob->outfile);
                    jobs->extra1=tmpjob->extra1;
                    jobs->extra2=tmpjob->extra2;
                    jobs->quit=QuitMode;
                    jobs->seen=1; jobs->changed=1;

                    /* Creates an other queue element for next 
                    ** possible element that will be read from
                    ** jobslist. */
                    jobs->next=(struct jobslist *)malloc(sizeof(struct jobslist));
                    jobs->next->prev=jobs;          /* make the link */
                    jobs=jobs->next;                /* go into new son */
                    (int)jobs->next=-1;             /* end queue */
                    cleanjob(jobs);                 /* delete old traces */

                    /* Clean and prepare tmpjob for next read */
                    cleanjob(tmpjob);               /* delete old traces */
                }
            } else if (strstr(buffer, "sjob") != 0) {
                /* no recursion */
                badsintax(5, line, buffer); goto ReCycle;
            } else if (isempty(buffer) != 0) {
                /* unkown field type */
                badsintax(9, line, buffer); goto ReCycle;
            }
        }                                    /* end if (In* == 0) && (.. */
    }                                        /* end if buffer[0] == # */
}
fclose(jobtab_f);

/* Global check about various used files :
**     . check if a couple of jobs use the same outfile.
**     . a job cannot use like outfile the input file of 
**       another job, or viceversa.
**     . old jobs (seen=0) are useless in this analysis.
*/
if ((int)head->next != -1) {
    p=head;
    while ((int)p->next != -1) {
        jobs=p->next;
        if (p->seen == 1)
            while ((int)jobs->next != -1) {
                if ((strcmp(p->outfile, jobs->outfile) == 0) && (jobs->seen == 1)) {
                    fprintf(elog_f, "%s\n", actualtime()); 
                    fputs("Each job must have its proper outfile.\n", elog_f);
                    fprintf(elog_f, "Check <%s> & <%s>.\n\n", p->name, jobs->name);
                    fclose(elog_f); 
                    sleep(sleeptime); goto ReCycle;
                }
                if ((strcmp(p->outfile, jobs->infile) == 0) && (jobs->seen == 1)) {
                    fprintf(elog_f, "%s\n", actualtime()); 
                    fputs("Output file cannot be used like input file.\n", elog_f);
                    fprintf(elog_f, "Check <%s> & <%s>.\n\n", p->name, jobs->name);
                    fclose(elog_f); 
                    sleep(sleeptime); goto ReCycle;
                }
                jobs=jobs->next;
            }
        p=p->next; 
    }
}

/* 
** Nuke of program : it creates new jobs (via fork), kills old jobs,
** cleans the jobslist (deletes old jobs and adds the newest, making
** all necessary links between two elements).
** Check each element in the queue : 
**   - if (changed==1) then kill and restart the job with new parameters.
**   - if (seen==0) then kill job (actually not exist in jobtab).
*/
#if defined(SEND_MAIL)
    /* Compose the header of mail 
    ** and alloc another record. */
    hletp=(struct letter *)malloc(sizeof(struct letter)); (int)hletp->next=-1;
    strcpy(st, "From: snoozer\nSubject: job management\n");
    hletp->line=malloc(strlen(st)+1); strcpy(hletp->line, st);
    hletp->next=(struct letter *)malloc(sizeof(struct letter)); 
    letp=hletp->next; 
    if (QuitMode == 1)
        sprintf(st, "\nParent (auto quitmode) : pid=%d name=%s\n", getpid(), ProgramName);
    else
        sprintf(st, "\nParent : pid=%d name=%s\n", getpid(), ProgramName);
    letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
    letp->next=(struct letter *)malloc(sizeof(struct letter));
    letp=letp->next; (int)letp->next=-1;
#endif
jobs=head;
while ((int)jobs->next != -1) {
    if (jobs->seen == 0) {
        /* this job is old, not exist in jobtab */
#       if defined(SEND_MAIL)
            sprintf(st, "<name=%s cfg=%s pid=%d> is old, killed.\n", jobs->name, jobs->cfg, jobs->pid);
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        kill(jobs->pid, SIGQUIT);
        wait(&retval);
        if (jobs == head) {
            head=jobs->next;                  /* make a new "head" */
            (int)head->prev=-1;               /* no previous elements */
        } else {
            jobs->prev->next=jobs->next;      /* link the prev with next */
            jobs->next->prev=jobs->prev;      /* link the next with prev */
        }
        p=jobs;
        jobs=jobs->next;                      /* move in the next */
        free(p);                              /* frees old element */
    } else if (jobs->changed == 1) {
        /* this job has been modified, kill and restart */
        if (jobs->pid != 0) { kill(jobs->pid, SIGQUIT); wait(&retval); }
        i=createjob(jobs);
        if (i == 1) {
            /* No more processes to fork */
            kill(getpid(), SIGQUIT);         /* clean exit */
            fprintf(elog_f, "%s\n", actualtime());
            fputs("Fork error, total panic !\n\n", elog_f);
            fclose(elog_f);
            RetValue=7;
            goto ReCycle;
        }
#       if defined(SEND_MAIL)
            sprintf(st, "<name=%s cfg=%s pid=%d> (re)start job.\n", jobs->name, jobs->cfg, jobs->pid);
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        jobs=jobs->next;                      /* move in the next */
    } else {
        /* only seen (i.e. exists in jobtab), but not modified */
        jobs->seen=0;
        jobs=jobs->next;                      /* move in the next */
    }
}
#if defined(SEND_MAIL)
    letp->line=malloc(3); strcpy(letp->line, ".\n");
    if ((int)hletp->next->next->next != -1) {
        /* mail isn't empty, send it */
        send_letter(hletp, elog_f);
    } else {
        /* mail is empty */
        free(hletp->next); free(hletp->line); free(hletp);
    }
#endif
fclose(elog_f);

/* Now sleep for a while, and then restart 
** with a new read of the jobtab file. */
#if (INFO >= 3)
    printf("main end=%s\n", actualtime());
#endif
sleep(sleeptime);
goto ReCycle;

} /* end main */



/****************************************************************************
******************** Procedures and functions section ***********************
****************************************************************************/
/* snoozer computes the checksum of itself : reads 
** the old checksum from the file OSS (defined in 
** config/selfcheck.h) and compare it with the checksum 
** just computed, then snoozer writes the new checksum 
** in file NSS (defined in config/selfcheck.h). */
self_check()
{
int    i, j;
char   st[BUF_SIZE], buffer[BUF_SIZE];
struct letter *hletp;
FILE   *f;

set_sigfiles();
if ((f=fopen(OSS, "r")) != 0) {
    if ((char *)fgets(st, BUF_SIZE, f) != 0) {
        j=strlen(st);
        i=-1; while((st[++i] != '\n') && (i<j)); st[i]=0;
    } else 
        st[0]=0;
    fclose(f);
    if (md5_sig("snoozer", st) != 0) {
        if ((f=fopen(NSS, "w")) != 0) {
            fprintf(f, "%s\n", st); 
            fclose(f); 
            f=fopen(elog, "a");
            fprintf(f, "%s\n", actualtime()); 
            fprintf(f, "New & old sig differ <new=%s>.\n\n", st);
            fclose(f);
#           if defined(SEND_MAIL)
                hletp=(struct letter *)malloc(sizeof(struct letter));
                sprintf(buffer, "From: snoozer\nSubject: auto-check\n");
                hletp->line=malloc(strlen(buffer)+1); strcpy(hletp->line, buffer);
                hletp->next=(struct letter *)malloc(sizeof(struct letter));
                (int)hletp->next->next=-1;
                sprintf(buffer, "\nNew & old sig differ [%s] : \n            <new=%s>.\n.\n", actualtime(), st);
                hletp->next->line=malloc(strlen(buffer)+1); 
                strcpy(hletp->next->line, buffer);
                f=fopen("/dev/null", "w");
                send_letter(hletp, f);
                fclose(f);
#           endif
            printf("Move ~NSS file to ~OSS and restart snoozer.\n");
            exit(4);
        } else {
            printf("NSS can't be opened.\n"); 
            exit(3);
        }
    }
} else {
    /* OSS not found */
    printf("OSS can't be opened.\n"); 
    exit(2);
}

}

/*
** BadSintax : describe the error, end exits with a proper return code
*/
badsintax(n, line, buffer)
int n;
int line;
char buffer[BUF_SIZE];
{
int i;
char st[BUF_SIZE];

fprintf(elog_f, "%s\n", actualtime()); 
switch (n) {
    case 3:
        fputs("Bad sintax in smain-emain sequence.\n", elog_f);
        i=-1; while (buffer[++i] != '\n') ; buffer[i]=0;
        fprintf(elog_f, "Bad line %d : <%s>\n\n", line, buffer);
        break;
    case 4:
        fputs("Only one smain-emain sequence is allowed.\n", elog_f);
        fprintf(elog_f, "Bad line %d : <%s>\n\n", line, buffer);
        break;
    case 5:
        fputs("Recursion isn't allowed in sjob-ejob sequence.\n", elog_f);
        i=-1; while (buffer[++i] != '\n') ; buffer[i]=0;
        fprintf(elog_f, "Bad line %d : <%s>\n\n", line, buffer);
        break;
    case 6:
        fputs("Unkown job type.\n", elog_f);
        i=-1; while (buffer[++i] != '\n') ; buffer[i]=0;
        fprintf(elog_f, "Bad line %d : <%s>\n\n", line, buffer);
        break;
    case 7:
        fputs("Incomplete sintax in <sjob perm-ejob>.\n", elog_f);
        fputs("The following fields must be present : name, cfg, out.\n\n", elog_f);
        break;
    case 8:
        fputs("This field is unallowed in this type of job.\n", elog_f);
        i=-1; while (buffer[++i] != '\n') ; buffer[i]=0;
        fprintf(elog_f, "Bad line %d : <%s>\n\n", line, buffer);
        break;
    case 9:
        fputs("Unkown field type.\n", elog_f);
        i=-1; while (buffer[++i] != '\n') ; buffer[i]=0;
        fprintf(elog_f, "Bad line %d : <%s>\n\n", line, buffer);
        break;
    case 10:
        fputs("Unkown checksum type.\n", elog_f); 
        i=-1; while (buffer[++i] != '\n') ; buffer[i]=0;
        fprintf(elog_f, "Bad line %d : <%s>\n\n", line, buffer);
        break;
    case 11:
        fputs("Configuration file not exist.\n", elog_f);
        i=-1; while (buffer[++i] != '\n') ; buffer[i]=0;
        fprintf(elog_f, "Bad line %d : <%s>\n\n", line, buffer);
        break;
    default:
        /* Not reached */
        fputs("Unkown error. Total Panic !\n", elog_f);
        fclose(elog_f);
        fclose(jobtab_f);
        exit(100);
}
fclose(elog_f);
fclose(jobtab_f);
sleep(sleeptime);
}

/*
** CleanJob : cleans all fields in a new record.
**            This must be used just after a malloc call to avoid 
**            any interferences from a previous allocation.
**            The free call don't clean the area itself !
*/
cleanjob(jobsp)
struct jobslist *jobsp;
{
jobsp->type[0]=0; jobsp->pid=0; jobsp->name[0]=0;
jobsp->sleep=SLEEP;
jobsp->cfg[0]=0; jobsp->infile[0]=0; jobsp->outfile[0]=0;
jobsp->extra1=-1; jobsp->extra2=-1;
jobsp->quit=0;
jobsp->changed=0; jobsp->seen=0;
}

/*
** CreateJob : create a new job via fork() 
**   
**  return values : 
**     . 0 : all right
**     . 1 : fork error
*/
int createjob(jobs)
struct jobslist *jobs;
{
int i;
int ChildPid;
char st[BUF_SIZE];

signal(SIGINT, SIG_IGN);
signal(SIGQUIT, SIG_IGN);

ChildPid=fork();
if (ChildPid < 0) {
	fprintf(elog_f, "%s\n", actualtime()); 
	fputs("Danger : child not created (1st time), retry...\n", elog_f);
	sleep(60);
    ChildPid=fork();
    if (ChildPid < 0) {
        fputs("Danger : child not created (2nd time), retry...\n", elog_f);
	    sleep(60);
        ChildPid=fork();
        if (ChildPid < 0) {
            fputs("Danger : child not created (3rd time) !\n", elog_f);
            fputs("         Abort the entire program.\n", elog_f);
            return(1);
        }
    }
    fputs("\n", elog_f);
}

if (ChildPid > 0) {
    /* parent */
    jobs->pid=ChildPid;
    jobs->seen=0; jobs->changed=0;
    signal(SIGINT, handle_int);
    signal(SIGQUIT, handle_quit);
    return(0);
} else {
    /* child */
    close(0);
    if (strstr(jobs->type, "own") != 0) JobOwn(jobs);
    else if (strstr(jobs->type, "perm") != 0) JobPerm(jobs);
    else if (strstr(jobs->type, "sign") != 0) JobSign(jobs);
    else if (strstr(jobs->type, "time") != 0) JobTime(jobs);
    exit(2);                      /* not reached, never */
}
}

/* 
** LineParser : parses the line parameters
**
**   return values :
**     . 0 : no line parameters, use default values.
**     . 1 : correct line parameters, use these. 
**     . 2 : quit mode (execute every son only one time, then exit).
*/
int lineparser(argc, argv)
int argc;
char *argv[];
{
int i;
int QuitMode;                    /* 1 : execute every son only one time */
char st[BUF_SIZE];

/* set the default filename 
** for jobtab and errorlog */
strcpy(jobtab, JOBTAB);
strcpy(elog, ELOG);

/* parse line parameters */
if (argc > 1) {
    QuitMode=0;                  /* start snoozer in a normal way */
    /* jobtab and errorlog filenames
    ** specified via parameters line */
    for (i=1; i<argc; ++i) {
        if ((strcmp(argv[i], "-f") == 0) && (i+1 < argc)) {
            ++i;
            chdir(SNOOZERHOME);
            if ((jobtab_f=fopen(argv[i], "r")) == 0) {
                usage(argv[0]); exit(6);
            } else {
                /* jobtab file exist */
                fclose(jobtab_f);
                /* extract jobtab and errorlog filename */
                strcpy(jobtab, argv[i]);
                sprintf(elog, "%s.errlog", jobtab);
                /* now hyde parameters line */
                strcpy(argv[i-1], "  ");
                strncpy(argv[i], SPACES, strlen(argv[i]));
                if ((elog_f=fopen(elog, "w")) == 0) {
                    usage(argv[0]); exit(1);
                } else 
                    fclose(elog_f);
            }
        } else if (strcmp(argv[i], "-q") == 0) {
            strcpy(argv[i], "  ");           /* hyde parameter */
            QuitMode=1;
        } else if (strcmp(argv[i], "-V") == 0) {
            printf("%s\n", VERSION); exit(0);
        } else {
            usage(argv[0]); exit(10);
        } 
    }          /* end for */
    if (QuitMode == 0) return(1); else return(2);
} else {       /* else if argc */
    /* no additional parameters */
    return(0);
}              /* end if/else argc */
}              /* end proc */

/*
** Usage : show the correct snoozer command line sintax
*/
void usage(name)
char *name;
{
printf("Usage : %s [-V] [-q] [-f jobtab]\n", name);
}

