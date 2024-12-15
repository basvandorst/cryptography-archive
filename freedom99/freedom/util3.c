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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"

void queuelatent(char *origfile, char *queuefile, char *controlfile,
		 char *latentTime)
{
  char input[255], temp[255];
  FILE *in, *out;
  int time1, time2, linenum, headers, day;
  struct tm *curtime;
  time_t now;

  now = time(NULL);
  curtime = localtime(&now);
  time1 = (curtime->tm_hour * 60) + curtime->tm_min;
  day = curtime->tm_wday;
  time2 = parsetime(latentTime, time1);
  if (time2 < time1) {
    day++;
    if (day == 7)
      day = 0;
  }

  out = fopen(controlfile, "w");
  fprintf(out, "%d\n%d\n", day, time2);
  fclose(out);

  in = fopen(origfile, "r");
  out = fopen(queuefile, "w");

  headers = 1;
  linenum = 0;
  while (!feof(in)) {
    if (fgets(input, 255, in) > 0) {
      if (!headers) {
	linenum++;
	if ((strstr(input, "::") != NULL) && (linenum == 1)) {
	  fputs(input, out);
	  fgets(input, 255, in);
	  while (strlen(input) > 1) {
	    /* we are looking at :: headers now */
	    strcpy(temp, input);
	    stringlower(temp);
	    if (strncmp(temp, "latent-time:", 12) != 0)
	      fputs(input, out);
	    fgets(input, 255, in);
	  }
	  fputs("\n", out);
	} else {
	  /* we are looking at the body now */
	  fputs(input, out);
	}
      } else {
	if (strlen(input) <= 1) {
	  headers = 0;
	  fputs("\n", out);
	} else {
	  /* we are looking at real headers now */
	  strcpy(temp, input);
	  stringlower(temp);
	  if (strncmp(temp, "latent-time:", 12) != 0)
	    fputs(input, out);
	}
      }
    }
  }
  fclose(in);
  fclose(out);
}

int parsetime(char *latent, int time_now)
{
  char hour[3], minute[3];
  int tim, ranmax;

  if (latent[0] != '+') {
    /* parse as time given */
    hour[0] = latent[0]; hour[1] = latent[1]; hour[2] = '\0';
    minute[0] = latent[3]; minute[1] = latent[4]; minute[2] = '\0';
    tim = ((atoi(hour) * 60) + atoi(minute)) % 1440;
  } else {
    if (tolower(latent[6]) == 'r') {
      /* parse as + with random */
      hour[0] = latent[1]; hour[1] = latent[2]; hour[2] = '\0';
      minute[0] = latent[4]; minute[1] = latent[5]; minute[2] = '\0';
      ranmax = (atoi(hour) * 60) + atoi(minute);
      srand(time_now);
      tim = ((rand() % ranmax) + time_now) % 1440;
    } else {
      /* parse as + with no random */
      hour[0] = latent[1]; hour[1] = latent[2]; hour[2] = '\0';
      minute[0] = latent[4]; minute[1] = latent[5]; minute[2] = '\0';
      tim = (time_now + (atoi(hour) * 60) + atoi(minute)) % 1440;
    }
  }
  return tim;
}
