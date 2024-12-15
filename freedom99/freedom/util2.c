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
#include <unistd.h>
#include "config.h"

void pgpdecrypt(char *encrypted, char *decrypted)
{
  int status;
  char pathenv[255], passenv[255];

  sprintf(passenv, "PGPPASS=%s", PASS_PHRASE);
  putenv(passenv); 
  sprintf(pathenv, "PGPPATH=%s", PGP_DIR);
  putenv(pathenv);

  if (fork() == 0) {
    execlp(PGP, PGP, "+batchmode", "+verbose=0", "-d", 
	   encrypted, "-o", decrypted, NULL);
  } else {
    wait(&status);
  }

  putenv("PGPPASS=");
  putenv("PGPPATH=");

}

void chop(char *s)
{
  s[strlen(s) - 1] = '\0';
}

int blocked(char *address, char *bfile)
{
  FILE *file;
  char blockFile[255], input[255];
  int block = 0;

  sprintf(blockFile, "%s/%s", REMAILER_DIR, bfile);
  file = fopen(blockFile, "r");

  stringlower(address);
  while(!feof(file)) {
    if (fgets(input, 255, file) > 0) {
      stringlower(input);
      chop(input);
      if (strstr(address, input) != NULL)
	block = 1;
    }
  }
  fclose(file);
  return block;
}

void cut(char *origfile, char *cutfile, char *cutmarks, char *header,
         int fakeheader)
{
  char tempfile[255], input[255], tmp[255];
  FILE *in, *out;
  int cut = 0, headers = 1, linenum = 0;

  sprintf(tempfile, "%sC", origfile);
  rename(origfile, tempfile);

  in = fopen(tempfile, "r");
  out = fopen(origfile, "w");

  while (!feof(in)) {
    if (fgets(input, 255, in) > 0) {
      chop(input);
      if (!headers) {
	/* we're not in the real headers */
	linenum++;
	if ((strstr(input, "::") != NULL) && (linenum == 1)) {
	  /* we're in the fake headers */
	  fprintf(out, "%s\n", input);
	  fgets(input, 255, in);
	  chop(input);
	  while (strlen(input) > 1) {
	    /* process fake-header line */
	    strcpy(tmp, input);
	    stringlower(tmp);
	    if (strncmp(tmp, header, strlen(header)) != 0) {
	      fprintf(out, "%s\n", input);
	    }
	    fgets(input, 255, in);
	    chop(input);
	  }
	  fputs("\n", out);
	} else {
	  /* we're in the body */
	  if ((!cut) && (strcmp(input, cutmarks) == 0)) {
	    fclose(out);
	    out = fopen(cutfile, "w");
	    if (fakeheader) {
	      fputs("Cut-Recursive: Yes\n\n", out);
	    }
	    cut = 1;
	  } else {
	    fprintf(out, "%s\n", input);
	  }
	}
      } else {
	if (strlen(input) <= 1) {
	  /* we've hit the end of the real headers */
	  headers = 0;
	  fputs("\n", out);
	} else {
	  /* we're in the real headers */
	  strcpy(tmp, input);
	  stringlower(tmp);
	  if (strncmp(tmp, header, strlen(header)) != 0) {
	    fprintf(out, "%s\n", input);
	  }
	}
      }
    }
  }
  fclose(in);
  fclose(out);
  unlink(tempfile);
}

void mailhelp(char *to)
{
  FILE *p, *in;
  char sendmail[255], helpfile[255], input[255];

  sprintf(sendmail, "%s %s", SENDMAIL, to);
  sprintf(helpfile, "%s/%s", REMAILER_DIR, HELP_FILE);

  p = popen(sendmail, "w");
  in = fopen(helpfile, "r");

  fprintf(p, "From: %s <%s>\nTo: %s\n", REMAILER_NAME, REMAILER_ADDR,
	  to);
  fprintf(p, "Subject: Remailer Help\n\n");

  while(!feof(in)) {
    if (fgets(input, 255, in) > 0)
      fputs(input, p);
  }

  fclose(in);
  pclose(p);
}

void mailkey(char *to)
{
  FILE *p, *in;
  char sendmail[255], keyfile[255], input[255];

  sprintf(sendmail, "%s %s", SENDMAIL, to);
  sprintf(keyfile, "%s/%s", REMAILER_DIR, PGP_KEY_FILE);

  p = popen(sendmail, "w");
  in = fopen(keyfile, "r");

  fprintf(p, "From: %s <%s>\nTo: %s\n", REMAILER_NAME, REMAILER_ADDR,
	  to);
  fprintf(p, "Subject: Remailer PGP Key\n\n");

  while(!feof(in)) {
    if (fgets(input, 255, in) > 0)
      fputs(input, p);
  }

  fclose(in);
  pclose(p);
}

