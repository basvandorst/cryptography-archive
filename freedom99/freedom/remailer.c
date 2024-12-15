/****************************************************************************
    This file is part of the Freedom Remailer.  It is:
    Copyright (C) 1995  John B. Fleming (jfleming@indiana.edu)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

void main(int argc, char *argv[])
{
  FILE *file1, *file2, *p;
  char tempfilename[255], bFilename[255], dFilename[255], eFilename[255],
       cFilename[255], qFilename[255], mFilename[255], input[255],
       kFilename[255], send_to[255], message_from[255];
  int pid, anon, recursing, trimmings = 0, count;

  if (argc > 1) {
    if (strcmp(argv[1], "-q") == 0) {
      runlatentqueue();
      exit(0);
    } else {
      printf("%s: Usage: %s [-q]\n", argv[0], argv[0]);
      printf("\t-q:\tRun queue of latent messages.\n");
      printf("\twith no options, reads mail on stdin and processes.\n");
      exit(1);
    }
  }

  pid = getpid();

  sprintf(tempfilename, "%s/mail%d", REMAILER_DIR, pid);
  sprintf(bFilename, "%sb", tempfilename);
  sprintf(dFilename, "%sd", tempfilename);
  sprintf(eFilename, "%se", tempfilename);
  sprintf(cFilename, "%sc", tempfilename);
  sprintf(kFilename, "%sk", tempfilename);

  /* copy message coming in on stdin to a temporary file */
  file1 = fopen(tempfilename, "w");
  while (!feof(stdin)) {
    if (fgets(input, 255, stdin) > 0)
      fputs(input, file1);
  }
  fclose(file1);

  recursing = 1;
  while (recursing) {
    recursing = 0;

    if (getcount() < 0) {
      makecount();
    }
    pluscount();
    count = getcount();
    sprintf(qFilename, "%s/q%d", QUEUE_DIR, count);
    sprintf(mFilename, "%s/m%d", QUEUE_DIR, count);

    anon = scan_message(tempfilename, send_to, message_from);
    chop(send_to);
    if (anon == SUBJECT_BLOCKED) {
      if (DEBUG) printf("Subject blocked. Not processing.\n");
      unlink(tempfilename);
    } else if (anon == ANON_MESSAGE) {
      if (DEBUG) printf("Anonymous message detected.\n");
      if (strcmp(send_to, "null") == 0) {
	if (DEBUG) printf("It goes to the bit-bucket.\n");
	unlink(tempfilename);
      } else if (blocked(send_to, DEST_BLOCK)) {
	if (DEBUG) printf("The destination is blocked.  Not remailing.\n");
	unlink(tempfilename);
      } else if (blocked(message_from, SOURCE_BLOCK)) {
	if (DEBUG) printf("The source is blocked.  Not remailing.\n");
	unlink(tempfilename);
      } else {
	if (DEBUG) printf("Remailing.\n");
	if (USE_STATS) {
	  updatestats(STATS_MESSAGE);
	}
	remove_headers(tempfilename, bFilename);
	sprintf(input, "%s %s", SENDMAIL, send_to);
	p = popen(input, "w");
	
	fprintf(p, "From: %s <%s>\n", REMAILER_NAME, REMAILER_ADDR);
	fprintf(p, "Comments: This message did not originate from the address above.  It was remailed by an anonymous remailing service.  If you have questions or complaints, please direct them to <%s>\n", COMPLAINTS_TO);
	fprintf(p, "To: %s\n", send_to);
	
	file1 = fopen(bFilename, "r");
	while (!feof(file1)) {
	  if (fgets(input, 255, file1) > 0)
	    fputs(input, p);
	}
	fclose(file1);
	pclose(p);
	unlink(tempfilename);
	unlink(bFilename);
      }
    } else if ((anon == STATS_REQ) && (USE_STATS)) {
      if (DEBUG) printf("Statistics were requested.  Mailing.\n");
      mailstats(message_from);
      unlink(tempfilename);
    } else if (anon == HELP_REQ) {
      if (DEBUG) printf("Help was requested.  Mailing.\n");
      mailhelp(message_from);
      unlink(tempfilename);
    } else if ((anon == KEY_REQ) && (USE_PGP)) {
      if (DEBUG) printf("PGP Key requested.  Mailing.\n");
      mailkey(message_from);
      unlink(tempfilename);
    } else if ((anon == ANON_POST) && (ALLOW_POST)) {
      if (DEBUG) printf("Anonymous post detected.\n");
      if (blocked(send_to, DEST_BLOCK)) {
	if (DEBUG) printf("Destination is blocked.  Not posting.\n");
        unlink(tempfilename);
      } else if (blocked(message_from, SOURCE_BLOCK)) {
	if (DEBUG) printf("Source is blocked.  Not posting.\n");
        unlink(tempfilename);
      } else {
	if (DEBUG) printf("Posting.\n");
	if (USE_STATS) {
	  updatestats(STATS_MESSAGE);
	}
	remove_headers(tempfilename, bFilename);
	sprintf(input, "%s -h", INEWS);
	p = popen(input, "w");
	fprintf(p, "Newsgroups: %s\n", send_to);
	fprintf(p, "Organization: %s\n", ORGANIZATION);
	fprintf(p, "From: %s <%s>\n", REMAILER_NAME, REMAILER_ADDR);
	fprintf(p, "Comments: This message did not originate from the address above.  It was posted by an anonymous posting service.  If you have questions or complaints, please direct them to <%s>\n", COMPLAINTS_TO);
	file1 = fopen(bFilename, "r");
	while (!feof(file1)) {
	  if (fgets(input, 255, file1) > 0)
	    fputs(input, p);
	}
	fclose(file1);
	pclose(p);
	unlink(tempfilename);
	unlink(bFilename);
      }
    } else if (anon == CUTMARKS) {
      if (DEBUG) printf("Cutmarks detected.  Processing.\n");
      cut(tempfilename, cFilename, send_to, "cutmarks: ", 1);
      trimmings = 1;
    } else if ((anon == ENCRYPT_KEY) && (USE_PGP)) {
      if (DEBUG) printf("Encrypt-Key detected.  Processing.\n");
      cut(tempfilename, kFilename, "**", "encrypt-key: ", 0);
      encrypt_key(kFilename, tempfilename, send_to);
      recursing = 1;
    } else if (anon == LATENT_TIME) {
      if (DEBUG) printf("Latent-Time detected.  Queueing.\n");
      if (USE_STATS) {
        updatestats(STATS_LATENT);
      }
      queuelatent(tempfilename, qFilename, mFilename, send_to);
      unlink(tempfilename);
    } else if ((anon == PGP_MESSAGE) && (USE_PGP)) {
      if (DEBUG) printf("PGP message detected.  Processing.\n");
      if (USE_STATS) {
        updatestats(STATS_PGP);
      }
      pgpdecrypt(tempfilename, dFilename);
      rename(tempfilename, eFilename);
      file1 = fopen(tempfilename, "w");
      file2 = fopen(dFilename, "r");
      fputs("PGP-Recursive: Yes\n\n", file1);
      while (!feof(file2)) {
	if (fgets(input, 255, file2) > 0)
	  fputs(input, file1);
      }
      fclose(file2);
      file2 = fopen(eFilename, "r");
      while(!feof(file2)) {
	if (fgets(input, 255, file2) > 0)
	  if (strstr(input, "-----END PGP MESSAGE-----") != NULL) {
	    while(!feof(file2)) {
	      if (fgets(input, 255, file2) > 0)
		fputs(input, file1);
	    }
	  }
      }
      fclose(file2);
      fclose(file1);
      unlink(eFilename);
      unlink(dFilename);
      recursing = 1;
    } else if (trimmings) {
      if (DEBUG) printf("Processing cutmarks leftovers.\n");
      rename (cFilename, tempfilename);
      trimmings = 0;
      recursing = 1;
    } else {
      if (DEBUG) printf("Non-anonymous detected.\n");
      file1 = fopen(MAIL_SPOOL, "a");
      file2 = fopen(tempfilename, "r");
      if (fgets(input, 255, file2) > 0) {
	if (strncmp(input, "Cut-Recursive:", 14) != 0) {
	  if (DEBUG) printf("Saving to spool.\n");
	  fputs(input, file1);
	  while (!feof(file2)) {
	    if (fgets(input, 255, file2) > 0)
	      fputs(input, file1);
	  }
	} else {
	  if (DEBUG) printf("It's cutmarks leftovers. Going to bit-bucket.\n");
	}
      }
      fclose(file1);
      fclose(file2);
      unlink(tempfilename);
    }
    if (trimmings)
      recursing = 1;
  }
  if (DEBUG) printf("Running the latent message queue.\n");
  runlatentqueue();
  exit(0);
}
