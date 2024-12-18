/*   change password or add user to S/KEY authentication system.
 *   S/KEY is a tradmark of Bellcore  */

#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include "skey.h"
#include <stdio.h>
#include <time.h>

extern int optind;
extern char *optarg;

char * readpass();

#ifdef SKEYD
#ifdef SKEYINITINSECURE
/* this violates standard skeylookup() syntax */
int skeylookup __ARGS((struct skey *mp,char *name, int staylocal));
#endif /* SKEYINITINSECURE */
#else
int skeylookup __ARGS((struct skey *mp,char *name));
#endif


#define NAMELEN 2

int
main(argc,argv)
int argc;
char *argv[];
{
	struct skey skey;
	int rval,n,nn,i,defaultsetup;
	char seed[18],tmp[80],key[8];
	struct passwd *ppuser,*pp;
	/* increased defaultseed size, tnx to markl@rugrat.glyphic.com */
	char defaultseed[100], passwd[256],passwd2[256] ;
#ifdef SKEYINITINSECURE
#ifdef SKEYD
	char line[BUFLEN], answer[BUFLEN];
#endif
#endif /* SKEYINITINSECURE */

	time_t now;
	struct tm *tm;
	char tbuf[27],buf[60];
	char lastc, me[80];
	int l;

	time(&now);
	tm = localtime(&now);
	strftime(tbuf, sizeof(tbuf), "%M%j", tm);

	if ((gethostname (defaultseed, sizeof (defaultseed))) < 0)
		exit (-1);
	strcpy(&defaultseed[NAMELEN],tbuf);

	pp = ppuser = getpwuid(getuid());
	strcpy(me,pp->pw_name);
	defaultsetup = 1;
	if( argc > 1){
		if(strcmp("-s", argv[1]) == 0)
			defaultsetup = 0;
		else
			pp = getpwnam(argv[1]);
		if(argc > 2)
			pp = getpwnam(argv[2]);

	}
	if(pp == NULL){
		printf("User unknown\n");
		exit(1);
	}
	if(strcmp( pp->pw_name,me) != 0){
		if(getuid() != 0){
			/* Only root can change other's passwds */
			printf("Permission denied.\n");
			exit(1);
		}
	}

#ifdef SKEYD
#ifndef SKEYINITINSECURE
	printf("Notice: skey.init works with LOCAL key file only\n");
#endif
#endif

	/* vince */
	bzero(&skey, sizeof(skey));
#ifdef SKEYD
#ifdef SKEYINITINSECURE
	rval = skeylookup(&skey,pp->pw_name,0); /* use skeyd if available */
#else /* it is secure, i.e., local updates only */
	rval = skeylookup(&skey,pp->pw_name,1); /* prevent using of network */
#endif /* SKEYINITINSECURE */
#else
	rval = skeylookup(&skey,pp->pw_name);
#endif /* SKEYD */
	switch(rval){
	case -1:
		perror("error in opening database");
		exit(1);
	case 0:
		printf("Updating %s:\n",pp->pw_name);
		printf("Old key: %s\n",skey.seed);
		/* lets be nice if they have a skey.seed that ends in 0-8 just add one*/
		l = strlen(skey.seed);
		if( l > 0){
			lastc = skey.seed[l-1];
			if( isdigit(lastc) && lastc != '9' ){
				strcpy(defaultseed, skey.seed);
				defaultseed[l-1] = lastc + 1;
			}
			if( isdigit(lastc) && lastc == '9' && l < 16){
				strcpy(defaultseed, skey.seed);
				defaultseed[l-1] = '0';
				defaultseed[l] = '0';
				defaultseed[l+1] = '\0';
			}
		}
		break;
	case 1:
		printf("Adding %s:\n",pp->pw_name);
		break;
	}
    n = 99;
    if( ! defaultsetup){
	printf("Reminder you need the 6 english words from the skey command.\n");
	for(i=0;;i++){
		if(i >= 2) exit(1);
		printf("Enter sequence count from 1 to 10000: ");
		fgets(tmp,sizeof(tmp),stdin);
		n = atoi(tmp);
		if(n > 0 && n < 10000)
			break;	/* Valid range */
		printf("Count must be > 0 and < 10000\n");
	}
    }
    if( !defaultsetup){
	printf("Enter new key [default %s]: ", defaultseed);
	fflush(stdout);
	fgets(seed,sizeof(seed),stdin);
	rip(seed);
	if(strlen(seed) > 16){
		printf("Seed truncated to 16 chars\n");
		seed[16] = '\0';
	}
	if( seed[0] == '\0') strcpy(seed,defaultseed);
	for(i=0;;i++){
		if(i >= 2) exit(1);
		printf("s/key %d %s\ns/key access password: ",n,seed);
		fgets(tmp,sizeof(tmp),stdin);
		rip(tmp);
		backspace(tmp);
		if(tmp[0] == '?'){
			printf("Enter 6 English words from secure S/Key calculation.\n");
			continue;
		}
		if(tmp[0] == '\0'){
			exit(1);
		}
		if(etob(key,tmp) == 1 || atob8(key,tmp) == 0)
			break;	/* Valid format */
		printf("Invalid format, try again with 6 English words.\n");
	}
    } else {
	/* Get user's secret password */
	fprintf(stderr,"Reminder - Only use this method if you are directly connected.\n");
	fprintf(stderr,"If you are using telnet or dial-in exit with no password and use keyinit -s.\n");
	for(i=0;;i++){
		if(i >= 2) exit(1);
		fprintf(stderr,"Enter secret password: ");
		readpass(passwd,sizeof(passwd));
		if(passwd[0] == '\0'){
			exit(1);
		}
		fprintf(stderr,"Again secret password: ");
		readpass(passwd2,sizeof(passwd));
		if(passwd2[0] == '\0'){
			exit(1);
		}
		if(strlen(passwd) < 4 && strlen(passwd2) < 4) {
			fprintf(stderr, "Sorry your password must be longer\n\r");
			exit(1);
		}
		if(strcmp(passwd,passwd2) == 0) break;
		fprintf(stderr, "Sorry no match\n");
		
	
	}
	strcpy(seed,defaultseed);

	/* Crunch seed and password into starting key */
	if(keycrunch(key,seed,passwd) != 0){
		fprintf(stderr,"%s: key crunch failed\n",argv[0]);
		exit(1);
	}
	nn = n;
	while(nn-- != 0)
		fff(key);
    }
	time(&now);
	tm = localtime(&now);
	strftime(tbuf, sizeof(tbuf), " %b %d,%Y %T", tm);

	/*DAVE*/
	if (skey.val == NULL)
		skey.val = (char *) malloc(16+1);
	/*DAVE*/
	btoa8(skey.val,key);

#ifdef SKEYINITINSECURE /* unsafe -- see below */
#ifdef SKEYD
	sprintf(line,"%s %04d %-16s %s %-21s\n",pp->pw_name,n,
		seed,skey.val, tbuf);
	if (skeytalk(FORCEKEYUPDATE, line, answer)) {
		printf(
		"NOT using skey authentication server for key file updating\n");
#endif
		/* did we used local key file? */
		if (skey.keyfile == NULL) {
			printf("Can not use local key file!\n");
			exit(1);
		}
		
		fprintf(skey.keyfile,"%s %04d %-16s %s %-21s\n",
					pp->pw_name,n, seed,skey.val, tbuf);
		fclose(skey.keyfile);
#ifdef SKEYD
	}
#endif
#else /* !SKEYINITINSECURE */
	/* not using skeyd for key initalization. this way the protocol
	   is safer. it is not acceptable, in fact, to allow clients
	   to init the centralized key file.  it would be easy, for
	   a malicious hacker, to create a new key file entry or to
	   change (that is, initialise) root's key entry.  the security
	   of all systems should depend upon the security of the master
	   site only, so the server site does not trust client sites
	   and remote inits are forbidden. */
	fprintf(skey.keyfile,"%s %04d %-16s %s %-21s\n",pp->pw_name,n,
		seed,skey.val, tbuf);
	fclose(skey.keyfile);
#endif /* SKEYINITINSECURE */

	printf("\nID %s s/key is %d %s\n",pp->pw_name,n,seed);
	printf("%s\n",btoe(buf,key));
#ifdef HEXIN
	printf("%s\n",put8(buf,key));
#endif
	return 0;
}
