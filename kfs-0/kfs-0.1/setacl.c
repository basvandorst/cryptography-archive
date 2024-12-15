/* setacl.c -- set ACL of a file.
 * 
 * Copyright (c) 1993 by William W. Dorsey
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#undef KERNEL

#include <stdio.h>
#include <ctype.h>
#include <sys/fcntl.h>

#include "kfs.h"
#include "kfs_acl.h"


extern char *optarg;
extern int optind;
extern int syscall(int, ...);


#define SEP	" \t\n"

extern int atoi(const char *);
extern void bzero(void *addr, int len);
extern int fclose(FILE *);
extern int fprintf(FILE *, const char *fmt, ...);
extern int getopt(int ac, char **av, char *optstr);
extern char *index(char *, char);
extern void perror(const char *msg);
extern int printf(char *fmt, ...);
extern char *strtok(const char *, const char *);
extern long strtol(const char *str, char **ptr, int base);

static int file_scan_acl(const char *file, aclist *acl);
static int str_scan_acl(const char *str, aclist *acl);
static int parse_line(char *line, ac_ent *e);
static void usage(void);


char		*pname;
static int	err_flag = 0;
static int	add_flag = 0;
static char	*acl_file = 0;
static char	*expr_str = 0;


int
main(int ac, char *av[])
{
aclist	acl;
int	a;

	if( (pname = index(av[0], '/')) )
		++pname;
	else
		pname = av[0];

	while ((a = getopt(ac, av, "af:s:")) != -1)
		switch (a) {
		case 'a':		/* add ACL (instead of set) */
			add_flag = 1;
			break;
		case 'f':		/* set ACL from file */
			acl_file = optarg;
			break;
		case 's':		/* set ACL from arg */
			expr_str = optarg;
			break;
		case '?':
			err_flag = 1;
			break;
		}

	if (!((acl_file != NULL) ^ (expr_str != NULL)))
		err_flag = 1;

	if (err_flag) {
		usage();
		exit(2);
	}

	if ( expr_str ) {
		if( str_scan_acl(expr_str, &acl) < 0 )
			exit(1);
	}
	else if ( acl_file ) {
		if( file_scan_acl(acl_file, &acl) < 0 )
			exit(1);
	}

	for( ; optind < ac ; optind++) {
		if( add_flag ) {
			if( syscall(198, av[optind], FIO_KFS_ADDACL, &acl) < 0 ) {
				perror(av[2]);
				exit(1);
			}
		}
		else {
			if( syscall(198, av[optind], FIO_KFS_SETACL, &acl) < 0 ) {
				perror(av[2]);
				exit(1);
			}
		}
	}

	return 0;
}


static int
file_scan_acl(const char *file, aclist *acl)
{
char	line[256];
FILE	*fp;
int	rval = 0;
int	i;

	if( (fp = fopen(file, "r")) == NULL ) {
		fprintf(stderr, "could not open %s\n", file);
		return -1;
	}

	bzero(acl, sizeof(*acl));

	for( i = 0; i < ACL_NUSERS ; ++i ) {
		if( fgets(line, sizeof(line), fp) == 0 ) {
			if( feof(fp) )
				break;
			fprintf(stderr, "fgets failed\n");
			rval = -1;
			goto done;
		}
		if( parse_line(line, &acl->actab[i]) < 0 ) {
			rval = -1;
			goto done;
		}
	}
	acl->inuse = i;

	printf("got %d lines\n", i);

done:
	fclose(fp);
	return rval;
}


static int
str_scan_acl(const char *str, aclist *acl)
{
char	line[1024];
int	rval = 0;

	if( strlen(str) >= sizeof(line) ) {
		fprintf(stderr, "%s: expr too long\n", pname);
		return -1;
	}
	strcpy(line, str);

	bzero(acl, sizeof(*acl));

	if( parse_line(line, &acl->actab[0]) < 0 ) {
		rval = -1;
		goto done;
	}
	acl->inuse = 1;

done:
	return rval;
}


/*
 * parse a string (line) of ACL spec into an ACL entry
 *
 * WARNING:
 *	line is modified
 */

static int
parse_line(char *line, ac_ent *e)
{
char	*cp;

	if( (cp = strtok(line, SEP)) == 0 ) {
		fprintf(stderr, "parse error\n");
		return -1;
	}
	if( strcmp(cp, "user") == 0 ) {
		e->id_type = ACL_IDTYPE_UID;
		if( (cp = strtok(0, SEP)) == 0 ) {
			fprintf(stderr, "parse error\n");
			return -1;
		}
		if( isdigit(cp[0]) )
			e->id.uid = atoi(cp);
		else {
			fprintf(stderr, "can't parse uid <%s>\n", cp);
			return -1;
		}
	} else if( strcmp(cp, "group") == 0 ) {
		e->id_type = ACL_IDTYPE_GID;
		if( (cp = strtok(0, SEP)) == 0 ) {
			fprintf(stderr, "parse error\n");
			return -1;
		}
		if( isdigit(cp[0]) )
			e->id.gid = atoi(cp);
		else {
			fprintf(stderr, "can't parse gid <%s>\n", cp);
			return -1;
		}
	} else {
		fprintf(stderr, "can't parse id_type <%s>\n", cp);
		return -1;
	}
	if( (cp = strtok(0, SEP)) == 0 ) {
		fprintf(stderr, "parse error\n");
		return -1;
	}
	if( strcmp(cp, "domode") == 0 ) {
		e->flags &= ~ACL_FLAG_DOMASK;
		e->flags |= ACL_FLAG_DOMODE;
		if( (cp = strtok(0, SEP)) == 0 ) {
			fprintf(stderr, "can't parse domode mode\n");
			return -1;
		}
		if( isdigit(cp[0]) )
			e->perm = strtol(cp, 0, 8);
		else {
			fprintf(stderr, "can't parse domode perm <%s>\n", cp);
			return -1;
		}
	} else if( strcmp(cp, "douser") == 0 ) {
		e->flags &= ~ACL_FLAG_DOMASK;
		e->flags |= ACL_FLAG_DOUSER;
	} else if( strcmp(cp, "dogroup") == 0 ) {
		e->flags &= ~ACL_FLAG_DOMASK;
		e->flags |= ACL_FLAG_DOGROUP;
	} else {
		fprintf(stderr, "can't parse mode <%s>\n", cp);
		return -1;
	}

	if( (cp = strtok(0, SEP)) != 0 ) {
		fprintf(stderr, "can't parse <%s> at end of line\n", cp);
		return -1;
	}
	return 0;
}


static void
usage()
{
	fprintf(stderr, "Usage: %s [-a] [-f acl_file] [-s acl_str] file1 [file2 ...]\n", pname);
	exit(1);
}
