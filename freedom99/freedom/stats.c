/****************************************************************************
    This file is part of the Freedom Remailer.  It is mostly borrowed from
    Matt Ghio's statistics functions as given in his remailer distribution.

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
#include <time.h>
#include "config.h"

void updatestats(int flag)
{
  char statsFilename[255], month[24][5];
  struct tm *curtime;
  time_t now;
  FILE *file;
  int hour, ccm, ccpgp, ccl, x, date[24], m[24], p[24], l[24], currenthour;

  sprintf(statsFilename, "%s/%s", REMAILER_DIR, STATS_FILE);
  now = time(NULL);
  curtime = localtime(&now);

  file=fopen(statsFilename, "r");
  fscanf(file, "%d", &hour);
  fscanf(file, "%d %d %d", &ccm, &ccpgp, &ccl);
  for(x = 0; x < 24; x++) {
    fscanf(file, "%s %d %d %d %d", month[x], &date[x], &m[x], &p[x], &l[x]);
  }
  fclose(file);
  
  currenthour = curtime->tm_hour;
  x = hour;
  while (x != currenthour) {
    if (x > 0) {
      strcpy(month[x], month[x-1]);
      date[x] = date[x - 1];
    } else {
      strftime(month[0], 5, "%b", curtime);
      date[0] = curtime->tm_mday;
    }
    m[x] = 0;
    p[x] = 0;
    l[x] = 0;
    x++;
    if (x > 23)
      x=0;
  }

  if (hour != currenthour) {
    m[hour] = ccm;
    p[hour] = ccpgp;
    l[hour] = ccl;
    ccm = 0;
    ccpgp = 0;
    ccl = 0;
  }

  if (flag == STATS_MESSAGE)
    ccm++;
  else if (flag == STATS_PGP)
    ccpgp++;
  else if (flag == STATS_LATENT)
    ccl++;
  
  file = fopen(statsFilename, "w");
  fprintf(file, "%d\n", currenthour);
  fprintf(file, "%d %d %d\n", ccm, ccpgp, ccl);
  for(x = 0; x < 24; x++) {
    fprintf(file, "%s %d %d %d %d\n", month[x], date[x], m[x], p[x], l[x]);
  }
  fclose(file);
}

void mailstats(char *to)
{
  char statsFilename[255], month[24][5], sendmail[255];
  struct tm *curtime;
  time_t now;
  FILE *file;
  int hour, ccm, ccpgp, ccl, x, y, date[24], m[24], p[24], l[24], currenthour;

  now = time(NULL);
  curtime = localtime(&now);
  currenthour = curtime->tm_hour;
  sprintf(statsFilename, "%s/%s", REMAILER_DIR, STATS_FILE);

  file = fopen(statsFilename, "r");
  fscanf(file, "%d", &hour);
  fscanf(file, "%d %d %d", &ccm, &ccpgp, &ccl);
  for(x = 0; x < 24; x++) {
    fscanf(file, "%s %d %d %d %d", month[x], &date[x], &m[x], &p[x], &l[x]);
  }
  fclose(file);

  x = hour;
  while (x != currenthour) {
    if (x > 0) {
      strcpy(month[x], month[x-1]);
      date[x] = date[x - 1];
    } else {
      strftime(month[0], 5, "%b", curtime);
      date[0] = curtime->tm_mday;
    }
    m[x] = 0;
    p[x] = 0;
    l[x] = 0;
    x++;
    if (x > 23)
      x=0;
  }

  if (hour != currenthour) {
    m[hour] = ccm;
    p[hour] = ccpgp;
    l[hour] = ccl;
    ccm = 0;
    ccpgp = 0;
    ccl = 0;
  }

  sprintf(sendmail, "%s %s", SENDMAIL, to);
  file = popen(sendmail, "w");
  fprintf(file, "From: %s <%s>\nTo: %s\n", REMAILER_NAME, REMAILER_ADDR,
	  to);
  fprintf(file, "Subject: Remailer Statistics\n\n");
  fprintf(file, "Statistics for last 24 hours from anonymous remailer at\n");
  fprintf(file, "e-mail address: %s\n\n", REMAILER_ADDR);
  fprintf(file, "Number of messages ");
  fprintf(file, "per hour from %s %d %d:00 to %s %d %d:59\n\n",
	  month[23],date[23],currenthour,month[0],date[0],(currenthour+23)%24);
  for(x = 0; x < 24; x++) {
    fprintf(file, " %2d:00 (%2d) ", x, m[x]);
    if (m[x] > 0) {
      y = 0;
      while((y < m[x]) && (y < 67)) {
	fprintf(file, "*");
	y++;
      } 
      ccm += m[x];
      ccpgp += p[x];
      ccl += l[x];
    }
    fprintf(file, "\n");
  }
  fprintf(file, "\n");
  fprintf(file, "Total messages remailed in last 24 hours: %d\n",ccm);
  fprintf(file, "Number of messages encrypted with PGP: %d\n",ccpgp);
  fprintf(file, "Number of messages queued with latency: %d\n",ccl);
  
  pclose(file);
}
