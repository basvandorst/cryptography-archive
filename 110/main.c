#include "arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef NO_TIMEB
#include <sys/timeb.h>
#endif
#include <assert.h>
#include "desc.h"
#include "net.h"
#ifdef WIN32
#include <windows.h>
#define close closesocket
#include "getopt.h"
#else
#include <unistd.h>
#endif
#ifdef IRIX
#include <limits.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/schedctl.h>
#endif /* IRIX */

int test_crypt(void);

void Usage(void) {
  fprintf(OUTPUT,"desclient [-v verboselevel] [-c blocksperrun] [-t totalruns] [-n nicelevel | 100]\n"
	  "[-f firewall] [-u firewall_userid] [-w firewall_password] [-b breakfile] \n"
	  "[-p] [-m] [-i] [-r] -s server -e email\n\n");
  fprintf(OUTPUT,"-p    -> Pause client before any connect to server (dialup users)\n");
  fprintf(OUTPUT,"-m    -> Perform speed measurement and then exit (don't do any keyblocks)\n");
  fprintf(OUTPUT,"-i    -> Print PID on standard error\n\n\n");
  fprintf(OUTPUT,"-r    -> Use Eli Bihams bitslice method\n\n\n");
  fflush(OUTPUT);
  exit(1);
}

char server[256],email[112];
char firewallhost[256];
char socks_username[256];
char socks_password[256];
char breakfile[1024];  /* filename for stop file */ 
int dopause = 0;
int verboselevel = 2;
#ifndef WIN32
pid_t procpid;
#endif


void SetNice(int level) {
  if (level == 100) {
#if defined(IRIX)
    schedctl(NDPRI,0,NDPLOMIN);
#elif defined(WIN32)
    level = 4;
#else
    fprintf(OUTPUT,"Sorry, IDLE priority not supported on your platform!\n");
#endif
  }
  else 
#ifndef WIN32
    nice(level);
#else
  /* Ok. Here is the Win32 code. Pretty darn specific... :) */
  /* We support the following nicelevels:
     0 -> ABOVE NORMAL
     1 -> NORMAL
     2 -> BELOW NORMAL
     3 -> LOWEST
     4 -> IDLE Priority 
     */
  switch(level) {
  case 0:SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);break;
  case 1:SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);break;
  case 2:SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);break;
  case 3:SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_LOWEST);break;
  case 4:SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_IDLE);break;
  default:
    fprintf(OUTPUT,"Sorry, only priorities 0-4 are allowed in a Win32 environment!\n");
  }
#endif /* Win32 */
}

/* check for stopfile, return 0 if not found, 1 if found. */
int check_stopfile(int doprint) {
  
  char buf[1048];
  FILE *stopfile;

#ifndef WIN32
  memset(buf,0,sizeof(buf));
  sprintf(buf,"%s.%i",breakfile,procpid);
  if((((stopfile = fopen(breakfile,"r")) != NULL)) || ((stopfile = fopen(buf,"r")) != NULL)) {
    if (verboselevel && doprint) {
      fprintf(OUTPUT,"\n- Found 'stop' file, (%i) terminating...\n",procpid);
      fflush(OUTPUT);
    }
#else
    if((stopfile = fopen(breakfile,"r")) != NULL) {
      if (verboselevel && doprint)
	fprintf(OUTPUT,"\n- Found 'stop' file, terminating...\n");
#endif
      fclose(stopfile);
      fflush(OUTPUT);
      return 1;
    }
    return 0;
}

void main(int argc, char *argv[]) 
{
  /*
   * Default config opts
   */
  int nicelevel = 4;
  int totalruns = 0;
  int numblocks = 1;
  int onlyspeed = 0;
  int printpid = 0;
  int slice = 0;
  int x;
  
  /*
   * End config opts
   */
  char Rvec[MAX_BLOCKS][8];
  int i,loops=0;
  int r[MAX_BLOCKS];
  int blockcount=0;
  int ServerSocket;
  int perf_kps;
  int z = 0;
  int jj;
#if defined(TEST) && !defined(TEST_NET)
  //  char a_key[8] = {0x5e, 0xd9, 0x20,0x4f,0xec,0xe0,0xb9,0x67};
  char a_key[8] = {0x5e, 0x18, 0x20,0x4f,0xec,0xe0,0xb9,0x67};
#else
  char a_key[8];
#endif
  unsigned long b_key;
  time_t start, finish;
  struct tm *tme;
  UINT4 AllKeyspaces[MAX_BLOCKS];
#ifdef WIN32
  WSADATA wsaData;
#endif
  
  /*
   * Assert platform compatibility
   */
  assert(sizeof(UINT4) == 4);
  assert(sizeof(unsigned int) == 4);
#ifdef BIT64
  assert(sizeof(unsigned long) == 8);
#else
  assert(sizeof(unsigned long) == 4);
#endif
  /*
   * That part seemed good
   */
  
  memset(r,0,sizeof(r));
  memset(Rvec,0,sizeof(Rvec));
  memset(AllKeyspaces,0,sizeof(AllKeyspaces));
  memset(breakfile,0,sizeof(breakfile));strcpy(breakfile,"stop");
  
  /* Print greeting and parse command line arguments... */
#ifndef WIN32
  if (verboselevel)
    fprintf(OUTPUT,"DES Cracker client v1.1, (des@des.sollentuna.se)\n\n");
#else
    SetConsoleTitle("DES Cracker client v1.1, (des@des.sollentuna.se)");
#endif
	    
#ifdef WIN32
  if (WSAStartup(MAKEWORD(1,1),&wsaData)) {
    fprintf(OUTPUT,"Failed to initialise Windows Sockets Layer!\n");
    exit(1);
  }
#endif
	      
	      /*
	       * Check usage and stuff like that here!
	       */
	      memset(firewallhost,0,sizeof(firewallhost));
	      memset(socks_username,0,sizeof(socks_username));
	      memset(socks_password,0,sizeof(socks_password));
	      memset(server,0,sizeof(server));
	      memset(email,0,sizeof(email));
	      
	      while((x = getopt(argc, argv, "v:c:t:f:n:s:e:u:w:b:pmir")) != EOF)
	      switch(x) {
	      case 'v': verboselevel = atoi(optarg);
		if (verboselevel < 0 || verboselevel > 3) {
		  fprintf(OUTPUT,"Invalid verbose level!\n");
		  Usage();
		}
		break;
	      case 'c': numblocks = atoi(optarg);
		if (numblocks < 1 || numblocks > MAX_BLOCKS) {
		  fprintf(OUTPUT,"Invalid number of blocks!\n");
		  Usage();
		}
		break;
	      case 't': totalruns = atoi(optarg);
		if (totalruns < 1 || totalruns > MAX_RUNS) {
		  fprintf(OUTPUT,"Invalid number of total runs!\n");
		  Usage();
		}
		break;
	      case 'f': strcpy(firewallhost,optarg);
		break;
	      case 'n': nicelevel = atoi(optarg);
		if (nicelevel < 1 || nicelevel > 20 && nicelevel != 100) {
		  fprintf(OUTPUT,"Invalid nicelevel!\n");
		  Usage();
		}
		break;
	      case 's': strcpy(server,optarg);
		break;
	      case 'e': strcpy(email,optarg);
		break;
	      case 'u': strcpy(socks_username,optarg);
		break;
	      case 'w': strcpy(socks_password,optarg);
		break;
	      case 'b': strcpy(breakfile,optarg);
		break;
	      case 'p': dopause = 1;
		break;
	      case 'm': onlyspeed = 1;
		break;
	      case 'i': printpid = 1;
		break;
	      case 'r': slice = 1;
		break;
	      default:
		fprintf(OUTPUT,"Invalid option %c.\n",x);
		Usage();
	      }
	      
	      if (!onlyspeed && (!strlen(server) || !strlen(email)))
	      Usage();
	      
	      /*
	       * Command line completely parsed!
	       */
	      
	      /* make the program a little more -nice-.. :) */
	      SetNice(nicelevel);
	      
	      /* Get the process PID */
#ifndef WIN32
	      procpid = getpid();
	      if (printpid)
	      fprintf(stderr,"%i\n",procpid); /* printf the pid on stderr */
	      fflush(stderr);
#endif
	      
	      /* Test the crypt stuff and get performance */
	      if (verboselevel > 1) {
		fprintf(OUTPUT,"- Doing crypt test...\n");
		fflush(OUTPUT);
	      }
		if(slice) {
		  if(des_test_slice() == -1) {
		    fprintf(OUTPUT,"- Error in the bitslice crypt routines! This client is not working!\n"
			    "  Please contact des@des.sollentuna.se! \n\n");
		    fflush(OUTPUT);
		    exit(1);
		  }
		}
		else {
		  if(test_crypt() == -1) {
		    fprintf(OUTPUT,"- Error in normal crypt routines! This client is not working!\n"
			    "  Please contact des@des.sollentuna.se! \n\n");
		    fflush(OUTPUT);
		    exit(1);
		  }
		}
		  
		/* Passed crypt-test, now do performance test */
		if (verboselevel > 1) {
		  fprintf(OUTPUT,"- Doing performance test...\n");
		  fflush(OUTPUT);
		}

		if(slice)
		  perf_kps = crypt_perf_slice();
		else
		  perf_kps = crypt_perf();
		
		if(verboselevel > 1)
		  if(NUMKEYS*2/(perf_kps*60) > 60)
		    fprintf(OUTPUT,"- Did %u keys per second (%u hours, %u minutes to complete a block)\n",perf_kps,(NUMKEYS*2/perf_kps)/3600,((NUMKEYS*2/perf_kps)%3600)/60);
		  else
		    fprintf(OUTPUT,"- Did %u keys per second (%u minutes to complete a block)\n",perf_kps,NUMKEYS*2/(perf_kps*60));

		    if((((float)numblocks*(float)NUMKEYS*2) / ((float)perf_kps * (float)0.8)) > (unsigned int)BLOCKTIMEOUT * 3600) {
		      fprintf(OUTPUT,"- Error, too many blocks or too slow machine...\n");
    fflush(OUTPUT);    
    exit(1);
		    }
  
  
  if(onlyspeed) {
    if (verboselevel > 1)
      fprintf(OUTPUT,"- Running in test mode only, exiting...\n");
    fflush(OUTPUT);    
    exit(2);
  }

  /* start main loop... */
  while(1) {

    if (verboselevel > 1) {
      fprintf(OUTPUT,"Connecting to server: %s ",server);
      fflush(OUTPUT);
    }
    /* Connect to server... */
    if((ServerSocket = ServerConnect(server, (verboselevel > 1))) == 0xFFFFFFFF) {
      fprintf(OUTPUT,"\n- Aborting due to network error.\n");
      fflush(OUTPUT);      
      exit(1);
    }
    if (verboselevel > 1) {
      fprintf(OUTPUT," done!\n");
      fflush(OUTPUT);    
    }

    /* Get keyblock from server... */
    if (verboselevel) {
      fprintf(OUTPUT,"Getting keyblock(s)...");
      fflush(OUTPUT);
    }

    for (z = 0; z < numblocks; z++) {
      while ((AllKeyspaces[z] = RequestKeyblock(ServerSocket,Rvec[z])) == 0xFFFFFFFF && !z) {
	close(ServerSocket);
	fprintf(OUTPUT,"\n- Could not get keyblock, sleeping for %i minutes before retry...\n",NETTIMEOUTMIN);
	fflush(OUTPUT);
#ifdef NO_USLEEP
      sleep(NETTIMEOUT/1000000);
#else
#ifdef WIN32
	  Sleep(NETTIMEOUT/1000);
#else
      usleep(NETTIMEOUT);
#endif
#endif
	if((ServerSocket = ServerConnect(server, 0)) == 0xFFFFFFFF) {
	  fprintf(OUTPUT,"\n- Unknown error in connect... please try again...\n");
	  fflush(OUTPUT);
	  exit(1);
	}
      }
      if (AllKeyspaces[z] == 0xFFFFFFFF) 
	break;
    }
    /* How many blocks did we actually get!? */
    numblocks = z;

    if (verboselevel) {
      fprintf(OUTPUT," (%u blocks) done!\n",numblocks);
      fflush(OUTPUT);
    }

    close(ServerSocket);
    
    for (z = 0; z < numblocks; z++) {
      
#if !defined(TEST) || defined(TEST_NET)
      memset(a_key,0,8);
      /*
       * Det har ar kul - hamta en nyckel fran dockmaster, och testa :)
       */

      a_key[0] = 0;
      a_key[1] = (AllKeyspaces[z] << 3 ) & 24;
      a_key[2] = (AllKeyspaces[z] << 1 ) & 8 |
	(AllKeyspaces[z] << 3 ) & 192;
      a_key[3] = (AllKeyspaces[z] >> 4 ) & 14 |
	(AllKeyspaces[z] >> 2 ) & 192;
      a_key[4] = (AllKeyspaces[z] >> 9 ) & 254;
      a_key[5] = (AllKeyspaces[z] >> 16) & 2 |
	(AllKeyspaces[z] >> 15) & 24 |
	(AllKeyspaces[z] >> 14) & 192;
      a_key[6] = (AllKeyspaces[z] >> 21) & 2 |
	(AllKeyspaces[z] >> 20) & 8 |
	(AllKeyspaces[z] >> 17) & 128;
      a_key[7] = (AllKeyspaces[z] >> 24) & 2 |
	(AllKeyspaces[z] >> 23) & 8 |
	(AllKeyspaces[z] >> 20) & 128;
#endif
      /* Don't do the above if in test mode */
      
#if defined(TEST) 
      fprintf(OUTPUT,"Starting key (waaha!): ");
      for(i=0;i<8;i++)
	fprintf(OUTPUT,"%x ",a_key[i]);
      fprintf(OUTPUT,"\n");
#endif
      
      start=time(NULL);
      tme = localtime(&start);
      blockcount++;
      if (verboselevel > 1) {
	fprintf(OUTPUT,"Starting work on block %i (for this session) at %02i:%02i:%02i...\n",blockcount,tme->tm_hour,tme->tm_min,tme->tm_sec);
	fflush(OUTPUT);
      }
      if (verboselevel == 1) {
#ifndef WIN32
		  fprintf(OUTPUT,"S: %i at %02i:%02i:%02i on %i\n",AllKeyspaces[z],tme->tm_hour,tme->tm_min,tme->tm_sec,procpid);
#else
		  fprintf(OUTPUT,"S: %i at %02i:%02i:%02i\n",AllKeyspaces[z],tme->tm_hour,tme->tm_min,tme->tm_sec);
#endif
		  fflush(OUTPUT);
      }

      if(slice)
	r[z] = des_search_slice(a_key);
      else
	r[z] = des_search(a_key);

      fprintf(OUTPUT,"\n"); /* newline to compensate from stuff in descrack.c */
#ifdef TEST
      printf("\ndes_search reported: %i\n",r[z]);
#endif

      if(r[z] < 0) {
	fprintf(OUTPUT,"\n - Failed: Unkown error...\n");
	exit(1);
      }
      
      finish = time(NULL);
      if (verboselevel > 1) {
	fprintf(OUTPUT,"done!\n");
	fflush(OUTPUT);
      }
      if (verboselevel > 1) {
	fprintf(OUTPUT,"Finished keyblock in %i seconds -> %.0f keys/second\n", (finish-start), 2*((float )NUMKEYS/(finish-start)));
	fflush(OUTPUT);
      }
      if (verboselevel == 1) {
#ifndef WIN32
		  fprintf(OUTPUT,"D: %i at %02i:%02i:%02i by %i time: %i speed: %.0f kps\n",AllKeyspaces[z],tme->tm_hour,tme->tm_min,tme->tm_sec,procpid,(finish-start), 2*((float )NUMKEYS/(finish-start)));
#else
	      fprintf(OUTPUT,"D: %i at %02i:%02i:%02i time: %i speed: %.0f kps\n",AllKeyspaces[z],tme->tm_hour,tme->tm_min,tme->tm_sec,(finish-start), 2*((float )NUMKEYS/(finish-start)));
#endif
	      fflush(OUTPUT);
      }
    /* Check for stop file, if found bail, report blocks first. */
      if(check_stopfile(0)) {
	numblocks = z;
	break;
      }
    } /* The big for-loop */
#ifdef TEST
    exit(1);
#endif

    if (verboselevel) {
      fprintf(OUTPUT,"Reporting keyblock(s)...");
      fflush(OUTPUT);
    }
    if((ServerSocket = ServerConnect(server, 0)) == 0xFFFFFFFF) {
      fprintf(OUTPUT,"\n - Unknown error in connect... please try again later...\n");
      fflush(OUTPUT);
      exit(1);
    }
    for (z = 0; z < numblocks ; z++) {
      while(ReportKeyblock(ServerSocket,email,AllKeyspaces[z],!r[z]?3:2,Rvec[z])) {
	fprintf(OUTPUT,"\nFailed! sleeping %i min before retry...\n",NETTIMEOUTMIN);
	fflush(OUTPUT);
	close(ServerSocket);	
	if(loops++ == 10) {
	  fprintf(OUTPUT,"- Tried to report 10 times, halting...\n");
	  fflush(OUTPUT);
	  exit(1);
	}
#ifdef NO_USLEEP
      sleep(NETTIMEOUT/1000000);
#else
#ifdef WIN32
	  Sleep(NETTIMEOUT/1000);
#else
      usleep(NETTIMEOUT);
#endif
#endif
      if((ServerSocket = ServerConnect(server, 0)) == 0xFFFFFFFF) {
	fprintf(OUTPUT,"\n - Unknown error in connect... please try again...\n");
	fflush(OUTPUT);
	exit(1);
      }
      }
    }
    loops = 0;
    if (verboselevel > 1) {
      fprintf(OUTPUT," done!\n");
      fprintf(OUTPUT,"---------------------------\n");
      fflush(OUTPUT);
    }
    close(ServerSocket);
    if (totalruns && blockcount >= totalruns*numblocks) {
      if (verboselevel) {
	fprintf(OUTPUT,"Completed %i runs, terminating.\n",totalruns);
	fflush(OUTPUT);
      }
      exit(0);
    }
  
    /* Check for stop file, if sound bail, fast... no blocks */
    if(check_stopfile(1))
     exit(0);
  }
    }        



