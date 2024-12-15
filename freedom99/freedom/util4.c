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
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include "config.h"

void runlatentqueue(void)
{
  DIR *queuedir;
  FILE *in, *p;
  struct dirent *dir_entry;
  struct tm *curtime;
  time_t now;
  char filename[255], queuefile[255], input[255];
  int ctime, stime, cday, sday;

  now = time(NULL);
  curtime = localtime(&now);
  ctime = (curtime->tm_hour * 60) + curtime->tm_min;
  cday = curtime->tm_wday;

  queuedir = opendir(QUEUE_DIR);
  dir_entry = readdir(queuedir);
  while (dir_entry != NULL) {
    strcpy(filename, dir_entry->d_name);
    if (filename[0] == 'm') {
      /* control file found - process it */
      sprintf(queuefile, "%s/%s", QUEUE_DIR, filename);
      in = fopen(queuefile, "r");
      fgets(input, 255, in);
      sday = atoi(input);
      fgets(input, 255, in);
      stime = atoi(input);
      fclose(in);

      if ((cday > sday) || ((cday == 0) && (sday == 6)) ||
	  ((cday == sday) && (ctime >= stime))) {
	/* time to send the message */
	filename[0] = 'q';
	sprintf(queuefile, "%s/%s", QUEUE_DIR, filename);
	sprintf(input, "%s/sending%d", REMAILER_DIR, getcount());
	rename(queuefile, input);
	filename[0] = 'm';
	sprintf(queuefile, "%s/%s", QUEUE_DIR, filename);
	unlink(queuefile);
	strcpy(queuefile, input);
        sprintf(input, "%s/%s", REMAILER_DIR, REMAILER_BIN);

	/* pipe queued message to another remailer process */
        p = popen(input, "w");
        in = fopen(queuefile, "r");
        while (!feof(in)) {
          if (fgets(input, 255, in) > 0)
            fputs(input, p);
        }
        fclose(in);
        pclose(p);
	unlink(queuefile);
      } 
    }
    dir_entry = readdir(queuedir);
  }
  closedir(queuedir);
}

void encrypt_key(char *toencrypt, char *toappendto, char *key)
{
  int status, i;
  char tempfile[255], line[255], pathenv[255];
  FILE *in, *out;

  sprintf(tempfile, "%s.asc", toencrypt);

  sprintf(pathenv, "PGPPATH=%s", PGP_DIR);
  putenv(pathenv);

  /* call PGP to do encryption */
  if (fork() == 0) {
    execlp(PGP, PGP, "-cat", "+batchmode", "+verbose=0", "-z", key,
	   "-o", tempfile, toencrypt, NULL);
  } else {
    wait(&status);
  }

  putenv("PGPPATH=");

  /* append encrypted text to end of message */
  in = fopen(tempfile, "r");
  out = fopen(toappendto, "a");
  fputs("**\n", out);
  while (!feof(in)) {
    if (fgets(line, 255, in) > 0) {
      fputs(line, out);
    }
  }
  fclose(out);
  fclose(in);

  unlink(tempfile);
  unlink(toencrypt);
}
