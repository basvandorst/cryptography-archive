/*
#  Copyright (c) 1995 by Gino Corradin - Fri Dec 2 11:27:43 MET DST 1995 
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
/*
** Debugging procedures for snoozer.c.
** Used only for debugging purpose.
*/

/*
** PrintJob : print a job-record
*/
printjob(jobs)
struct jobslist *jobs;
{
printf("-curr=%d-prev=%d-next=%d-\n", jobs, jobs->prev, jobs->next);
printf("-type=%s-pid=%d-name=%s-sleep=%d-\n", jobs->type, jobs->pid, jobs->name, jobs->sleep);
printf("-cfg=%s-infile=%s-outfile=%s-\n", jobs->cfg, jobs->infile, jobs->outfile);
printf("-extra1=%d-extra2=%d-seen=%d-changed=%d-\n", jobs->extra1, jobs->extra2, jobs->seen, jobs->changed);
}

/* 
** ShowJobsList : show the entire jobslist
*/
showjobslist(jobs)
struct jobslist *jobs;
{
printf("+curr=%d-prev=%d-next=%d-\n", jobs, jobs->prev, jobs->next);
printf("-type=%s-pid=%d-name=%s-sleep=%d-\n", jobs->type, jobs->pid, jobs->name, jobs->sleep);
printf("-cfg=%s-infile=%s-outfile=%s-\n", jobs->cfg, jobs->infile, jobs->outfile);
printf("-extra1=%d-extra2=%d-seen=%d-changed=%d-\n", jobs->extra1, jobs->extra2, jobs->seen, jobs->changed);
jobs=jobs->next;
while ((int)jobs->next != -1) {
	printf("+curr=%d-prev=%d-next=%d-\n", jobs, jobs->prev, jobs->next);
	printf("-type=%s-pid=%d-name=%s-sleep=%d-\n", jobs->type, jobs->pid, jobs->name, jobs->sleep);
	printf("-cfg=%s-infile=%s-outfile=%s-\n", jobs->cfg, jobs->infile, jobs->outfile);
    printf("-extra1=%d-extra2=%d-seen=%d-changed=%d-\n", jobs->extra1, jobs->extra2, jobs->seen, jobs->changed);
	jobs=jobs->next;
} 
printf("+curr=%d-prev=%d-next=%d-\n", jobs, jobs->prev, jobs->next);
printf("-type=%s-pid=%d-name=%s-sleep=%d-\n", jobs->type, jobs->pid, jobs->name, jobs->sleep);
printf("-cfg=%s-infile=%s-outfile=%s-\n", jobs->cfg, jobs->infile, jobs->outfile);
printf("-extra1=%d-extra2=%d-seen=%d-changed=%d-\n", jobs->extra1, jobs->extra2, jobs->seen, jobs->changed);
}

