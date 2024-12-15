/*
#  Copyright (c) 1995 by Gino Corradin - Fri Dec 2 11:28:33 MET DST 1995 
#
#             Gino Corradin
#             via T. Albinoni 11
#             45100 Rovigo - ITALY 
#  
#     phone : +39 (0)425 34465 (Q) - +39 (0)425 35523 (fax)
#
#     E-mail : Gino.Corradin@dei.unipd.it
#              gino@maya.dei.unipd.it
 
 
   See distribution terms in License file.
*/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "defs.h"

extern void handle_quit(); 
extern char *actualtime();

#if defined (SEND_MAIL)
     struct letter *hletp, *letp;     /* head and element for mail text */
#endif

/*
** JobOwn : check owner 
*/
int JobOwn(jobs)
struct jobslist *jobs;
{
register int i, j, h;
char   basename[BUF_SIZE];                      /* path */
char   buffer[BUF_SIZE];                        /* I/O buffer */
char   filename[BUF_SIZE];                      /* check this file */
char   ownstr[BUF_SIZE];                        /* control string */
char   prefix[BUF_SIZE];                        /* 1st part of filename */
char   st[BUF_SIZE];                            /* general variable */
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
        printf("own start=%s\n", actualtime());
#   elif (INFO >= 2)
        printf("own job\n");
#   endif
    if (CountSIGQUIT > 0) {
#       if (INFO >= 1)
            printf("Quitting own job...\n");
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

    /* read config file and let's work */
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
                h=-1; while ((prefix[++h]=buffer[++i]) != ':') ; prefix[h]=0;
                h=-1; while (++i < j) ownstr[++h]=buffer[i]; ownstr[h]=0;
                /* now I've : basename contains all chars at the left 
                ** of last '/', prefix the chars at the right of last
                ** '/', and finally ownstr contains the 2nd field. */
                if (prefix[0] == '*') {
                    /* all files in this dir : prefix="*" */
                    if ((dirp=opendir(basename)) == 0) {
                        /* directory not exist */
                        fprintf(out, "Warning [%s] : <%s> dir not exist.\n", actualtime(), basename);
                    } else {
                        /* directory exist */
                        while ((direntp=readdir(dirp)) != 0) {
                            sprintf(filename, "%s/%s", basename, direntp->d_name);
                            do_own(filename, ownstr, out);
                        }
                        closedir(dirp);
                    }
                } else if (prefix[strlen(prefix)-1] == '*') {
                    /* a set of files in this dir : prefix="xxx...x*" */
                    prefix[strlen(prefix)-1]=0;  /* delete final "*" */
                    if ((dirp=opendir(basename)) == 0) {
                        fprintf(out, "Warning [%s] : <%s> dir not exist.\n", actualtime(), basename);
                    } else {
                        while ((direntp=readdir(dirp)) != 0) {
                            if (strstr(direntp->d_name, prefix) == direntp->d_name) {
                                /* prefix is in 1st part of filename */
                                sprintf(filename, "%s/%s", basename, direntp->d_name);
                                do_own(filename, ownstr, out);
                            }
                        }
                        closedir(dirp);
                    }
                } else {
                    /* a single file : prefix="xxx...x" */
                    sprintf(filename, "%s/%s", basename, prefix);
                    do_own(filename, ownstr, out);
                }
            } else {
                /* error : another field must be present */
                buffer[strlen(buffer)-1]=0;
                fprintf(out, "Ignored [%s] : <%s> need 2nd field.\n", actualtime(), buffer); 
            }
        } else if (isempty(buffer) != 0) {       /* else if buffer[0] */
            /* this line isn't empty : ignore it */
            fprintf(out, "Ignored [%s] : <%s> invalid line.\n", actualtime(), buffer);
        }                                        /* end if/else buffer[0] */
    }                                            /* end while fgets */
    fclose(cfg);
    fclose(out);
    free(direntp);

#   if (INFO >= 3)
        printf("own end=%s\n", actualtime());
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

    if (CountSIGQUIT == 0) sleep(jobs->sleep);   /* sleep for a while */
}                                                /* end infinite for */
}

/*
** Do_Own : check the correct owner/groupship of a file
**
**   return codes :
**       . -1 : file not exist
**       .  0 : all right
**       .  1 : error, bad owner/group ship
**       .  2 : sintax error, line ignored
*/
int do_own(filename, ownstr, out)
char filename[BUF_SIZE];
char ownstr[BUF_SIZE];
FILE *out;
{
int i, h, l;
char st[BUF_SIZE];
struct stat *statp;

statp=(struct stat *)malloc(sizeof(struct stat));
if (stat(filename, statp) < 0) {
    /* filename not exist */
    fprintf(out, "Warning [%s] : <%s> file not exist.\n", actualtime(), filename);
    free(statp);
    return(-1);
} 

/* filename exist check it */
l=strlen(ownstr);
if (ownstr[0] == 'n') {                    /* Negative Ship */
    /* neither file owner or group must be in this set */
    if (ownstr[1] == 'o') {
        /* check ownership */
        h=-1;
        for (i=2; i<=l; ++i) {
            if ((ownstr[i] != ' ') && (i < l)) st[++h]=ownstr[i];
            else if (h >= 0) {
                st[++h]=0;
                if (atoi(st) == statp->st_uid) {
                    sprintf(st, "Danger [%s] : <%s> bad own.\n", actualtime(), filename);
#                   if defined (SEND_MAIL)
                        letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                        letp->next=(struct letter *)malloc(sizeof(struct letter));
                        letp=letp->next; (int)letp->next=-1;
#                   endif
                    fputs(st, out);
                    free(statp);
                    return(1);
                } 
                h=-1;
            }
        }
        /* Ok : owner of this file isn't in this owner set */
        free(statp);
        return(0);
    } else if (ownstr[1] == 'g') {
        /* chech groupship */
        h=-1;
        for (i=2; i<=l; ++i) {
            if ((ownstr[i] != ' ') && (i < l)) st[++h]=ownstr[i];
            else if (h >= 0) {
                st[++h]=0;
                if (atoi(st) == statp->st_gid) {
                    sprintf(st, "Danger [%s] : <%s> bad group.\n", actualtime(), filename);
#                   if defined (SEND_MAIL)
                        letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
                        letp->next=(struct letter *)malloc(sizeof(struct letter));
                        letp=letp->next; (int)letp->next=-1;
#                   endif
                    fputs(st, out);
                    free(statp);
                    return(1);
                } 
                h=-1;
            }
        }
        /* Ok : group of this file isn't in this group set */
        free(statp);
        return(0);
    } else {
        /* error : char must be "g" or "o" */
        fprintf(out, "Ignored [%s] : <%s:%s> 2nd char != g|o.\n", actualtime(), filename, ownstr); 
        free(statp);
        return(2);
    }
} else {                                   /* Positive Ship */ 
    /* file owner or group must be in this set */
    if (ownstr[0] == 'o') {
        /* chech groupship */
        h=-1;
        for (i=1; i<=l; ++i) {
            if ((ownstr[i] != ' ') && (i < l)) st[++h]=ownstr[i];
            else if (h >= 0) {
                st[++h]=0;
                h=atoi(st);
                if (h == statp->st_uid) { free(statp); return(0); }
                h=-1;
            }
        }
        /* Bad : owner isn't in this owner set */
        sprintf(st, "Danger [%s] : <%s> bad own.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
        free(statp);
        return(1);
    } else if (ownstr[0] == 'g') {
        /* chech groupship */
        h=-1;
        for (i=1; i<=l; ++i) {
            if ((ownstr[i] != ' ') && (i < l)) st[++h]=ownstr[i];
            else if (h >= 0) {
                st[++h]=0;
                if (atoi(st) == statp->st_gid) { free(statp); return(0); }
                h=-1;
            }
        }
        /* Bad : group isn't in this group set */
        sprintf(st, "Danger [%s] : <%s> bad group.\n", actualtime(), filename);
#       if defined (SEND_MAIL)
            letp->line=malloc(strlen(st)+1); strcpy(letp->line, st);
            letp->next=(struct letter *)malloc(sizeof(struct letter));
            letp=letp->next; (int)letp->next=-1;
#       endif
        fputs(st, out);
        free(statp);
        return(1);
    } else {
        /* error : char must be "g" or "o" */
        fprintf(out, "Ignored [%s] : <%s:%s> 1st char != g|o.\n", actualtime(), filename, ownstr); 
        free(statp);
        return(2);
    }
}                   /* end if/else ownstr[0] */

}                   /* end proc */

