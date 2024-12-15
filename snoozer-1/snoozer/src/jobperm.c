/*
#  Copyright (c) 1995 by Gino Corradin - Fri Dec 2 11:29:43 MET DST 1995 
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
#include <sys/stat.h>
#include <unistd.h>
#include "defs.h"

extern void handle_quit(); 
extern char *actualtime();

#if defined (SEND_MAIL)
    struct letter *hletp, *letp;      /* head and element for mail text */
#endif

/*
** JobPerm : check various permission
*/
int JobPerm(jobs)
struct jobslist *jobs;
{
register int i,j;
char   basename[BUF_SIZE];                      /* file path */
char   buffer[BUF_SIZE];                        /* I/O buffer */
char   filename[BUF_SIZE];                      /* basename+/+prefix */
char   perm[5];                                 /* check for not(perm) */
char   prefix[BUF_SIZE];                        /* filename */
char   st[BUF_SIZE];                            /* common variable */
struct dirent *direntp;
struct jobslist *son;
FILE   *cfg;                                    /* config file */
FILE   *log;                                    /* dummy file */
FILE   *out;                                    /* output file */
DIR    *dirp;

signal(SIGQUIT, handle_quit); CountSIGQUIT=0;
signal(SIGINT, SIG_IGN); 
signal(SIGTERM, SIG_IGN);
strncpy(ProgramName, SPACES, strlen(ProgramName)); /* hyde real name */
strcpy(ProgramName, jobs->name);                /* substitute the name */

/* free memory from jobslist */
if ((int)jobs->prev != -1) {
    /* element is in the middle of list */
    jobs->prev->next=jobs->next;                /* exclude this from list */
    jobs->next->prev=jobs->prev;
    son=jobs->next; freeall(son);               /* delete all except jobs */
    (int)jobs->prev=-1; (int)jobs->next=-1;     /* make order */
} else {
    /* this is the head of list */
    (int)jobs->next->prev=-1;                   /* exclude .... */
    freeall(jobs->next);                        /* delete ... */
    (int)jobs->next=-1;
}

/* now the infinite loop */
out=fopen(jobs->outfile, "w"); fclose(out);     /* clean outfile */
for (;;) {
#   if (INFO >= 3)
        printf("perm start=%s\n", actualtime());
#   elif (INFO >= 2)
        printf("perm job\n");
#   endif
    if (CountSIGQUIT > 0) {
#       if (INFO >= 1)
            printf("Quitting perm job...\n");
#       endif
        exit(0);
    }

#   if defined (SEND_MAIL)
        hletp=(struct letter *)malloc(sizeof(struct letter));
        sprintf(st, "From: snoozer\nSubject: Danger in son %s\n", jobs->name);
        hletp->line=malloc(sizeof(st)+1); strcpy(hletp->line, st);
        hletp->next=(struct letter *)malloc(sizeof(struct letter));
        letp=hletp->next; (int)letp->next=-1;
#   endif

    /* read config file and check permissions */
    cfg=fopen(jobs->cfg, "r");
    out=fopen(jobs->outfile, "a");
    direntp=(struct dirent *)malloc(sizeof(struct dirent));
    while ((char *)fgets(buffer, BUF_SIZE, cfg) != 0) {
        /* consider only absolute pathnames */
        if (buffer[0] == '/') {
            j=strlen(buffer);
            i=-1; while ((buffer[++i] != ':') && (i < j)) ;
            if (buffer[i] == ':') {
                while (buffer[--i] != '/') ;
                strncpy(basename, buffer, i); basename[i]=0;
                j=-1; while ((prefix[++j]=buffer[++i]) != ':') ; prefix[j]=0;
                perm[0]=buffer[i+1]; perm[1]='0'; 
                perm[2]=buffer[i+3]; perm[3]=buffer[i+4]; perm[4]=0;
                /* now : basename contains all chars at the left
                ** of last '/', prefix the chars at the right of last
                ** '/', and finally perm contains the 2nd field. */
                if (isperm(perm) != 0) {
                    /* perm sequence isn't valid */
                    fprintf(out, "Error [%s] : <%s %s> invalid perm sequence.\n", actualtime(), filename, perm);
                } else if (prefix[0] == '*') {
                    /* all files in this dir : prefix="*" */
                    if ((dirp=opendir(basename)) == 0) {
                        /* this directory not exist */
                        fprintf(out, "Warning [%s] : <%s> dir not exist.\n", actualtime(), basename);
                    } else {
                        /* this directory exist */
                        while ((direntp=readdir(dirp)) != 0) {
                            sprintf(filename, "%s/%s", basename, direntp->d_name);
                            do_perm(filename, perm, out);
                        }
                        closedir(dirp);
                    }
                } else if (prefix[j-1] == '*') {
                    /* a fixed set of files : prefix="xxx...x*" */
                    prefix[j-1]=0;    /* delete final "*" */
                    if ((dirp=opendir(basename)) == 0) {
                        fprintf(out, "Warning [%s] : <%s> dir not exist.\n", actualtime(), basename);
                    } else {
                        while ((direntp=readdir(dirp)) != 0) {
                            if (strstr(direntp->d_name, prefix) == direntp->d_name) {
                                /* prefix is in 1st part of filename */
                                sprintf(filename, "%s/%s", basename, direntp->d_name);
                                do_perm(filename, perm, out);
                            }
                        }
                        closedir(dirp);
                    }
                } else {
                    /* this is a single file */
                    sprintf(filename, "%s/%s", basename, prefix);
                    do_perm(filename, perm, out);
                }                                    /* end if filename "*" */
            } else {
                /* error : another field must be present */
                buffer[strlen(buffer)-1]=0;
                fprintf(out, "Ignored [%s] : <%s> need 2nd field.\n", actualtime(), buffer);
            }
        } else if (isempty(buffer) != 0) {       /* else if buffer '/' */
            /* this line isn't empty : ignore it */
            fprintf(out, "Ignored [%s] : <%s> invalid line.\n", actualtime(), buffer);
        }                                        /* end if/else buffer '/' */
    }                                            /* end while */
    fclose(cfg);
    fclose(out);
    free(direntp);

#   if (INFO >= 3)
        printf("perm end=%s\n", actualtime());
#   endif

#   if defined (SEND_MAIL)
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

    if (CountSIGQUIT == 0) sleep(jobs->sleep);
}
}

/* 
** Do_Perm : check filename's permission mode 
** 
**   return codes:
**     .  0 : file checked
**     . -1 : file not exist
*/
int do_perm(filename, perm, out) 
char filename[BUF_SIZE];
char perm[5];
FILE *out;
{
register int i;
char   st[200];
struct stat *buf;

buf=(struct stat *)malloc(sizeof(struct stat));
if (stat(filename, buf) < 0) {
    fprintf(out, "Warning [%s] : <%s> not exist.\n", actualtime(), filename);
    return(-1);
}

i=buf->st_mode;
/* Check suid sgid sticky permissions */
if (perm[0] == '0') {
} else if (perm[0] == '7') {
    if ((i & 0004000) || (i & 0002000) || (i & 0001000)) {
        sprintf(st, "Danger [%s] : <%s> check suid, sgid, sticky.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[0] == '6') {
    if ((i & 0004000) || (i & 0002000)) {
        sprintf(st, "Danger [%s] : <%s> check suid, sgid.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[0] == '5') {
    if ((i & 0004000) || (i & 0001000)) {
        sprintf(st, "Danger [%s] : <%s> check suid, sticky.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[0] == '4') {
    if (i & 0004000) {
        sprintf(st, "Danger [%s] : <%s> suid.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[0] == '3') {
    if ((i & 0002000) || (i & 0001000)) {
        sprintf(st, "Danger [%s] : <%s> check sgid, sticky.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[0] == '2') {
    if (i & 0002000) {
        sprintf(st, "Danger [%s] : <%s> sgid.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[0] == '1') {
    if (i & 0001000) {
        sprintf(st, "Danger [%s] : <%s> sticky.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
}                                                /* end multiple if */

/* check group permissions */
if (perm[2] == '7') {
    if ((i & 00040) || (i & 00020) || (i & 00010)) {
        sprintf(st, "Danger [%s] : <%s> g=rwx ?\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[2] == '6') {
    if ((i & 00040) || (i & 00020)) {
        sprintf(st, "Danger [%s] : <%s> g=rw ?\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[2] == '5') {
    if ((i & 00040) || (i & 00010)) {
        sprintf(st, "Danger [%s] : <%s> g=rx ?\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[2] == '4') {
    if (i & 00040) {
        sprintf(st, "Danger [%s] : <%s> g=r.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[2] == '3') {
    if ((i & 00020) || (i & 00010)) {
        sprintf(st, "Danger [%s] : <%s> g=wx ?\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[2] == '2') {
    if (i & 00020) {
        sprintf(st, "Danger [%s] : <%s> g=w.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[2] == '1') {
    if (i & 00010) {
        sprintf(st, "Danger [%s] : <%s> g=x.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
}

/* check other permission */
if (perm[3] == '7') {
    if ((i & 00004) || (i & 00002) || (i & 00001)) {
        sprintf(st, "Danger [%s] : <%s> o=rwx ?\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[3] == '6') {
    if ((i & 00004) || (i & 00002)) {
        sprintf(st, "Danger [%s] : <%s> o=rw ?\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[3] == '5') {
    if ((i & 00004) || (i & 00001)) {
        sprintf(st, "Danger [%s] : <%s> o=rx ?\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[3] == '4') {
    if (i & 00004) {
        sprintf(st, "Danger [%s] : <%s> o=r.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[3] == '3') {
    if ((i & 00002) || (i & 00001)) {
        sprintf(st, "Danger [%s] : <%s> o=wx ?\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[3] == '2') {
    if (i & 00002) {
        sprintf(st, "Danger [%s] : <%s> o=w.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
} else if (perm[3] == '1') {
    if (i & 00001) {
        sprintf(st, "Danger [%s] : <%s> o=x.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
    }
}
free(buf);
}

/*
** IsPerm : check for a validity of perm sequence
*/
int isperm(perm)
char perm[5];
{
if ((perm[0] < '0') || (perm[0] > '7')) return(1);
if ((perm[2] < '0') || (perm[2] > '7')) return(3);
if ((perm[3] < '0') || (perm[3] > '7')) return(4);
return(0);
}

