/* showencrypted.c
 *
 * This program is intended to decode and decrypt/verify a PGP
 * encrypted/signed message in the MIME format described by
 * the internet draft draft-elkins-pem-pgp-01.txt
 *
 * Author: Michael Elkins <elkins@aero.org>
 *         The Aerospace Corporation
 *         September 1995
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <string.h>

#define STRING 1024

/* Actions */
#define DECRYPT 1
#define VERIFY 2

/* Encodings */
#define E_QP 1
#define E_B64 2
#define E_7BIT 3

static int istrncmp (char *s, char *p, int len) {
  int i;

  for (i = 0; i < len; i++, s++, p++) {
    if (! *s)
      return -1;
    if (! *p)
      return 1;
    if (toupper(*s) > toupper(*p))
      return 1;
    else if (toupper(*s) < toupper(*p))
      return -1;
  }
  return 0;
}

static void fork_child (char *cmd, int childin[2], int childout[2])
{
  if (fork () == 0) {
    close (childin[1]);
    close (0);
    dup (childin[0]);
    close (childin[0]);
    
    close (childout[0]);
    close (1);
    dup (childout[1]);
    close (childout[1]);
    
    _exit (system (cmd));
  }
}

static int check_content (char *s, char *expected)
{
  while (*s != ':')
    if (*s++ == '\0')
      return -1;
  s++;
  while (isspace (*s))
    if (*++s == '\0')
      return -1;
  if (istrncmp (s, expected, strlen (expected)) != 0)
    return 0;
  else
    return 1;
}

/* says whether the line is a valid boundary */
/* Note additional check that the boundary must be
   followed by --<EOL> or just <EOL> or it won't be accepted
   (This wasn't done before) */
int line_is_boundary(char *line, char *boundary, int blen)
{
  if (line[0] != '-' || line[1] != '-')
    return 0;
  if (strncmp(line+2, boundary, blen))
    return 0;
  if (line[2+blen] == '-') {
    if (line[3+blen] == '-') {
      if (line[4+blen] != '\r' && line[4+blen] != '\n') return 0;
    } else return 0;
  } else {
    if (line[2+blen] != '\r' && line[2+blen] != '\n') return 0;
  }
  return 1;
}

int main (int argc, char **argv)
{
  FILE *fp, *fpout, *fpin, *tmpfp;
  /* Buffer is larger in case we canonicalize the line in it */
  unsigned char buffer_one[STRING+2], /* Be binary clean! */
                buffer_two[STRING+2];
  unsigned char *buffer, /* Current buffer */
                *buffer_lookahead; /* Lookahead buffer */
  char
    *boundary = argv[1],
    tempfile[STRING],
    sigfile[STRING];
  int part = 0;
  int inheader = 1;
  int action;
  int encoding = E_7BIT;
  int blen;
  int childin[2], childout[2];
  int status;
  int ret;
  int verified = 0;
  char temp_dir[MAXPATHLEN] = {"/tmp\0"};
  char *c;
  int len;

  if (argc < 2) {
    fprintf (stderr, "usage: %s boundary [ filename ]\n", argv[0]);
    exit (1);
  }

  if ((c = strrchr (argv[0], '/')) != NULL)
    c++;
  else
    c = argv[0];

  if (strcmp (c, "showencrypted") == 0)
    action = DECRYPT;
  else
    action = VERIFY;

  /* If there is an input file read from there, otherwise use stdin */
  if ((argc > 2) && (strcmp (argv[2], "-") != 0)) {
    fp = fopen (argv[2], "r");
    if (! fp) {
      perror (argv[2]);
      exit (1);
    }
  }
  else
    fp = fdopen (dup (0), "r");
  
  blen = strlen (boundary);

  if ((c = getenv ("METAMAIL_TMPDIR")))
    strncpy (temp_dir, c, sizeof (temp_dir));

  sprintf (tempfile, "%s/mm.%d", temp_dir, getpid ());
  tmpfp = fopen (tempfile, "w");
  if (! tmpfp) {
    perror(tempfile);
    exit (1);
  }
  if (action == VERIFY)
    sprintf (sigfile, "%s.sig", tempfile);    
  
  inheader = 0;
  if (fgets (buffer_one, STRING, fp) == NULL) {
	fprintf(stderr, "Empty input!");
	return 1;
  }
  buffer_lookahead = &(buffer_one[0]);
  for (;;) {
    if ((buffer = buffer_lookahead) == NULL) break;

    /* Read in a line ahead into buffer_lookahead. Remember if there was */
    /* a next line (buffer_lookahead is null or not -pkv */

    /* swap the value of buffer_lookahead */
    if (buffer == &(buffer_one[0])) buffer_lookahead = &(buffer_two[0]);
      else buffer_lookahead = &(buffer_one[0]);
    buffer_lookahead = fgets (buffer_lookahead, STRING, fp);

    if (inheader) {
      if (part == 1) {
        if (action == VERIFY) {
          /* Convert to canonical text if necessary */
          len = strlen (buffer);
          if (buffer[len - 2] != '\r') {
            buffer[len - 1] = '\r';
	    buffer[len] = '\n';
	    buffer[len + 1] = '\0';
	  }
	  fputs (buffer, tmpfp);
        }
        else if (action == DECRYPT) {
          if (istrncmp (buffer, "content-type", 12) == 0) {
            /* Check to make sure that this is the type of data expected */
            ret = check_content (buffer + 12, "application/pgp-encrypted");
            if (ret == -1) {
              fprintf (stderr, "Error!  Could not parse Content-Type header.\n");
              exit (1);
            }
            else if (! ret) {
              fprintf (stderr, "Error!  This does not appear to be APPLICATION/PGP-ENCRYPTED data!\n");
              exit (1);
            }
            verified = 1;
          }
        }
      }
      else if (part == 2) {
	/* Need to save the value of the encoding for future use! */
	if (istrncmp (buffer, "content-transfer-encoding", 25) == 0) {
	  unsigned char *c = buffer + 25;
	  
	  while (*c && *c != ':')
	    c++;
	  c++;
	  while (*c && isspace (*c))
	    c++;	    
	  if (istrncmp (c, "quoted-printable", 16) == 0)
	    encoding = E_QP;
	  else if (istrncmp (c, "base64", 6) == 0)
	    encoding = E_B64;
	  else if (istrncmp (c, "7bit", 4) == 0)
	    encoding = E_7BIT;
	  else {
	    fprintf (stderr, "%s: invalid encoding.\n", c);
	    return -1;
	  }
	}
        else if (action == VERIFY &&
                 istrncmp (buffer, "content-type", 12) == 0) {
          ret = check_content (buffer + 12, "application/pgp-signature");
          if (ret == -1) {
            fprintf (stderr, "Error!  Could not parse Content-Type header!\n");
            exit (1);
          }
          else if (! ret) {
            fprintf (stderr, "Error!  Expected APPLICATION/PGP-SIGNATURE data!\n");
            exit (1);
          }
          verified = 1;
        }
      }

      if (buffer[0] == '\n' || (buffer[0] == '\r' && buffer[1] == '\n')) {
	inheader = 0;
        if (action == DECRYPT && part == 1 && !verified) {
          fprintf (stderr, "Error!  Expected APPLICATION/PGP-ENCRYPTED data!\n");
          exit (1);
        }
        else if (action == VERIFY && part == 2 && !verified) {
          fprintf (stderr, "Error!  Expected APPLICATION/PGP-SIGNATURE data!\n");
          exit (1);
        }

        if (part == 2) {
	  pipe (childin);
	  pipe (childout);
	  
	  if (action == DECRYPT) {
	    if (encoding != E_7BIT)
	      /* We are required to decode before sending to PGP */
	      sprintf (buffer, "mimencode -u -%c | ",
		       encoding == E_QP ? 'q' : 'b');
	    else
	      buffer[0] = '\0';
	    
            strncat (buffer, PGP_PATH, sizeof (buffer) - strlen (buffer) - 1);
	    strncat (buffer, " -f", sizeof (buffer) - strlen (buffer) - 1);
	  }
	  else if (action == VERIFY) {
	    fclose (tmpfp);
	    sprintf (buffer, "mimencode -u -%c", encoding == E_QP ? 'q' : 'b');
	  }
	  
	  fork_child (buffer, childin, childout);
	  close (childin[0]);
	  close (childout[1]);
	  fpout = fdopen (childin[1], "w");
	  fpin = fdopen (childout[0], "r");
        }
      }
    }
    else { /* !inheader */
      if (line_is_boundary(buffer, boundary, blen)) {
        if (buffer[2+blen] == '-') break;

        inheader = 1;
        part++;
        continue;
      }
      if (line_is_boundary(buffer_lookahead, boundary, blen)) {
        /* According to RFC1521, the CRLF on *this* line is
           conceptually part of the boundary on the next line.
           Remove it -pkv */
        len = strlen(buffer);
        if ((len > 0) && (buffer[len-1] == '\n')) {
          buffer[len-1] = 0;
          if ((len > 1) && (buffer[len-2] == '\r'))
            buffer[len-2] = 0;
        }
      }

      if (part == 1 && action == VERIFY) {
        len = strlen (buffer);
        /* (It might not end with a EOL at all. If it does, canon it) */
        if (len && (buffer[len - 2] != '\r') && (buffer[len - 1] == '\n')) {
          /* Convert to canonical text. */
	  buffer[len - 1] = '\r';
	  buffer[len] = '\n';
          buffer[len + 1] = '\0';
        }
	fputs (buffer, tmpfp);
      }
      else if (part == 2) {
	if (fputs (buffer, fpout) == EOF) {
	  fprintf (stderr, "fputs returned EOF!\n");
	  exit (1);
	}
      }
    }
  }

  if (part < 2) {
    fprintf (stderr, "Error!  Could not parse MIME file!\n");
    exit (1);
  }

  fclose (fp);
  if (ferror (fpout) != 0) {
    fprintf (stderr, "ferror returned non-zero on fpout!\n");
    exit (1);
  }
  if (fclose (fpout) != 0) {
    fprintf (stderr, "fclose returned non-zero on fpout!\n");
    exit (1);
  }
  wait(&status);
  if (status != 0) {
    fprintf (stderr, "child exited code %d!", status);
    exit (1);
  }
  if (action == DECRYPT)
    while (fgets (buffer, STRING, fpin) != NULL)
      fputs (buffer, tmpfp);
  else {
    tmpfp = fopen (sigfile, "w");
    while ((len = fread (buffer, 1, sizeof (buffer), fpin)) > 0)
      fwrite (buffer, 1, len, tmpfp);
  }
  fclose (fpin);
  fclose (tmpfp);
  
  if (action == VERIFY) {
    char local[STRING];

    sprintf (buffer, "%s +verbose=0 %s", PGP_PATH, tempfile);
    system (buffer);
    unlink (sigfile);

    /* The file must now be converted back to local newline convention, or
     * else metamail will fail!
     */
    fpin = fopen (tempfile, "r");
    sprintf (local, "%s.local", tempfile);
    fpout = fopen (local, "w");
    while (fgets (buffer, STRING, fpin) != NULL) {
      len = strlen (buffer);
      if ((len > 1) && (buffer[len - 2] == '\r') && (buffer[len - 1] == '\n')) {
        buffer[len - 2] = '\n';
        buffer[len - 1] = '\0';
      }
      fputs (buffer, fpout);
    }
    fclose (fpin);
    fclose (fpout);
#ifdef RENAME
    rename (local, tempfile);
#else
    link (local, tempfile);
    unlink (local);
#endif
  }
  sprintf (buffer, "metamail %s", tempfile);
  system (buffer);
  unlink (tempfile);
  exit (0);
}
