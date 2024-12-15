/*
#  Copyright (c) 1995 by Gino Corradin - Fri Dec 2 11:20:43 MET DST 1995 
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
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include "../config/config.h"

/* general length for various buffers */
#define BUF_SIZE 1024

extern void free();
extern void *malloc();

/* I built the list of active sons with this structure */
struct jobslist {
     struct jobslist *prev;
     struct jobslist *next;

     char   type[7];            /* job types : perm, sign, ... */
     int    pid;
     char   name[BUF_SIZE];
     int    sleep; 
     char   cfg[BUF_SIZE];      /* configuration file */
     char   infile[BUF_SIZE];   /* input file, not always used */
     char   outfile[BUF_SIZE];  /* output file */

     short  extra1;             /* extra parameter 1 */
     short  extra2;             /* extra parameter 2 */

     short  quit;               /* 1 : quitmode, son exit after 1st cycle */

     int    changed;            /* if 1 then the job is changed */
     int    seen;               /* 1 => actually this job exists in jobtab */
} ;

/* A mail is a set of lines, each of these 
** contained in one record, with a pointer
** that points to next element (line). */
struct letter {
    struct letter *next;
    char *line;
};

/* A pointer to argv[0], initialized in main.
** I use this to change the name of various 
** processes either sons or father. */
char *ProgramName;

/* Counts every received SIGQUIT */
int CountSIGQUIT;

/* Counts every received SIGINT */
int CountSIGINT;

/* Known checksum types */
#define CRC32_SIG 1
#define MD5_SIG 2
#define MD4_SIG 3

/* used to hyde a long program name */
#define SPACES "                                                                        "

#if !defined(SEEK_SET)
#	define SEEK_SET 0
#endif

