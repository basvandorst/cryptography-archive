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
#include "config.h"

int scan_message(char *infilename, char *to, char *from)
{
  int i, headers = 1, x, y, linenum = 0,
      ctflag = 0, ekflag = 0, ltflag = 0, anflag = 0, ptflag = 0;
  FILE *input;
  char line[255], tmpline[255], subj[255],
       cutmarks[255], encryptkey[255], latency[255];

  input = fopen(infilename, "r");
  if (input == NULL)
	return NON_ANON;

  while (!feof(input)) {
    if (fgets(line, 255, input) > 0) {
      strcpy(tmpline, line);
      stringlower(tmpline);
      if (!headers) {
	linenum++;
	if ((strstr(line, "::") != NULL) && (linenum == 1)) {
	  fgets(line, 255, input);
	  while (strlen(line) > 1) {
            strcpy(tmpline, line);
	    stringlower(tmpline);
	    if (strncmp(tmpline, "anon-to:", 8) == 0) {
	      strcpy(to, line + 9);
	      anflag = 1;
	    } else if (strncmp(tmpline, "request-remailing-to:", 21) == 0) {
	      strcpy(to, line + 22);
	      anflag = 1;
	    } else if (strncmp(tmpline, "remail-to:", 10) == 0) {
	      strcpy(to, line + 11);
	      anflag = 1;
	    } else if (strncmp(tmpline, "anon-post-to:", 13) == 0) {
	      strcpy(to, line + 14);
	      ptflag = 1;
	    } else if (strncmp(tmpline, "anon-send-to:", 13) == 0) {
	      strcpy(to, line + 14);
	      anflag = 1;
	    } else if (strncmp(tmpline, "x-anon-to:", 10) == 0) {
	      strcpy(to, line + 11);
	      anflag = 1;
	    } else if (strncmp(tmpline, "encrypted: pgp", 14) == 0) {
	      return PGP_MESSAGE;
	    } else if (strncmp(tmpline, "cutmarks:", 9) == 0) {
	      strcpy(cutmarks, line + 10);
	      ctflag = 1;
	    } else if (strncmp(tmpline, "encrypt-key:", 12) == 0) {
	      strcpy(encryptkey, line + 13);
	      ekflag = 1;
	    } else if (strncmp(tmpline, "latent-time:", 12) == 0) {
	      strcpy(latency, line + 13);
	      ltflag = 1;
	    }
	    fgets(line, 255, input);
	  }
	}
      } else {
	if (strlen(line) <= 1) {
	  headers = 0;
	} else {
	  if (strncmp(tmpline, "anon-to:", 8) == 0) {
	    strcpy(to, line + 9);
	    anflag = 1;
	  } else if (strncmp(tmpline, "request-remailing-to:", 21) == 0) {
	    strcpy(to, line + 22);
	    anflag = 1;
	  } else if (strncmp(tmpline, "remail-to:", 10) == 0) {
	    strcpy(to, line + 11);
	    anflag = 1;
	  } else if (strncmp(tmpline, "encrypted: pgp", 14) == 0) {
	    return PGP_MESSAGE;
	  } else if (strncmp(tmpline, "anon-post-to:", 13) == 0) {
	    strcpy(to, line + 14);
	    ptflag = 1;
	  } else if (strncmp(tmpline, "anon-send-to:", 13) == 0) {
	    strcpy(to, line + 14);
	    anflag = 1;
	  } else if (strncmp(tmpline, "x-anon-to:", 10) == 0) {
	    strcpy(to, line + 11);
	    anflag = 1;
	  } else if (strncmp(tmpline, "cutmarks:", 9) == 0) {
	    strcpy(cutmarks, line + 10);
	    ctflag = 1;
	  } else if (strncmp(tmpline, "encrypt-key:", 12) == 0) {
	    strcpy(encryptkey, line + 13);
	    ekflag = 1;
	  } else if (strncmp(tmpline, "latent-time:", 12) == 0) {
	    strcpy(latency, line + 13);
	    ltflag = 1;
	  } else if (strncmp(tmpline, "from:", 5) == 0) {
	    if (strchr(line, '<') != NULL) {
	      /* get <address> */
	      x = y = 0;
	      while (line[x] != '<') {
		x++;
	      }
	      x++;
	      while (line[x] != '>') {
		from[y] = line[x];
		x++;
		y++;
	      }
	      from[y] = '\0';
	    } else {
	      strcpy(from, line + 6);
	    }
	  } else if (strncmp(tmpline, "subject:", 8) == 0) {
	    strcpy (subj, line + 9);
	    stringlower(subj);
	    chop(subj);
	    if (blocked(subj, SUBJ_BLOCK)) {
	      return SUBJECT_BLOCKED;
	    } else if (strcmp(subj, "remailer-stats") == 0) {
	      strcpy(to, from);
	      return STATS_REQ;
	    } else if (strcmp(subj, "remailer-help") == 0) {
	      strcpy(to, from);
	      return HELP_REQ;
	    } else if (strcmp(subj, "remailer-key") == 0) {
	      strcpy(to, from);
	      return KEY_REQ;
	    }
	  }
	}
      }
    }
  }
  fclose(input);

  /* process headers in order: cutmarks, encrypt-key, latent-time */
  /* if these don't exist, then remail or post the message if that is where
     it's headed; else, return NON_ANON to send it to the spool or bucket */

  if (ctflag) {
    strcpy(to, cutmarks);
    return CUTMARKS;
  }
  if (ekflag) {
    strcpy(to, encryptkey);
    return ENCRYPT_KEY;
  }
  if (ltflag) {
    strcpy(to, latency);
    return LATENT_TIME;
  }
  if (anflag)
    return ANON_MESSAGE;
  if (ptflag)
    return ANON_POST;
  
  return NON_ANON;
}

void remove_headers(char *in, char *out)
{
  FILE *infile, *outfile;
  int headers = 1, colhead = 0, linenum = 0;
  char line[255];

  infile = fopen(in, "r");
  outfile = fopen(out, "w");

  while (!feof(infile)) {
    if (fgets(line, 255, infile) > 0) {
      if (!headers) {
	linenum++;
	if (strstr(line, "::") != NULL) {
	  colhead++;
	  if (colhead == 2)
	    fputs("\n", outfile);
	  if (linenum == 1) {
	    fgets(line, 255, infile);
	    while (strlen(line) > 1) {
	      fgets(line, 255, infile);
	    }
	  } else {
	    fputs(line, outfile);
	  }
	} else if (strstr(line, "##") != NULL) {
	  if (colhead <= 1) {
	    fgets(line, 255, infile);
	    while (strlen(line) > 1) {
	      fputs(line,outfile);
	      fgets(line, 255, infile);
	    }
	    fputs("\n", outfile);
	  } else {
	    fputs(line, outfile);
	  }
	} else {
	  fputs(line, outfile);
	}
      } else {
	if (strlen(line) <= 1)
	  headers = 0;
      }
    }
  }
  
  fclose(infile);
  fclose(outfile);
}

void stringlower(char *string)
{
  int i;
  for (i = 0; i < strlen(string); i++)
    string[i] = tolower(string[i]);
}
