/* atob: version 4.0
 * stream filter to change printable ascii from "btoa" back into 8 bit bytes
 * if bad chars, or Csums do not match: exit(1) [and NO output]
 *
 *  Paul Rutter 	Joe Orost
 *  philabs!per 	petsd!joe
 */

#include <stdio.h>
#include <ctype.h>

#define reg register

#define streq(s0, s1)  strcmp(s0, s1) == 0

#define times85(x)	((((((x<<2)+x)<<2)+x)<<2)+x)

long int Ceor = 0;
long int Csum = 0;
long int Crot = 0;
long int word = 0;
long int bcount = 0;
long int line = 1;

fatal(s)
   char *s;
{
  fprintf(stderr, "atob: error -- %s line %d after header\n", s, line);
  exit(1);
}

#define DE(c) ((c) - '!')

decode(c)
  reg c;
{
  char errbuf[128];

  if (c == 'z') {
    if (bcount != 0) {
      fatal("alignment error");
    } else {
      byteout(0);
      byteout(0);
      byteout(0);
      byteout(0);
    }
  } else if ((c >= '!') && (c < ('!' + 85))) {
    if (bcount == 0) {
      word = DE(c);
      ++bcount;
    } else if (bcount < 4) {
      word = times85(word);
      word += DE(c);
      ++bcount;
    } else {
      word = times85(word) + DE(c);
      byteout((int)((word >> 24) & 255));
      byteout((int)((word >> 16) & 255));
      byteout((int)((word >> 8) & 255));
      byteout((int)(word & 255));
      word = 0;
      bcount = 0;
    }
  } else {
    sprintf(errbuf, "invalid character %d", c);
    fatal(errbuf);
  }
}

FILE *tmp_file;

byteout(c)
  reg c;
{
  Ceor ^= c;
  Csum += c;
  Csum += 1;
  if ((Crot & 0x80000000)) {
    Crot <<= 1;
    Crot += 1;
  } else {
    Crot <<= 1;
  }
  Crot += c;
  putc(c, tmp_file);
}

main(argc, argv)
  char **argv;
{
  reg c;
  reg long int i;
  char tmp_name[100];
  char buf[100];
  long int n1, n2, oeor, osum, orot;

  if (argc != 1) {
    fprintf(stderr,"bad args to %s\n", argv[0]);
    exit(2);
  }
  sprintf(tmp_name, "/usr/tmp/atob.%x", getpid());
  tmp_file = fopen(tmp_name, "w+");
  if (tmp_file == NULL) {
    fatal("create of temp file failed");
  }
  unlink(tmp_name);	/* Make file disappear */
  /*search for header line*/
  for (;;) {
    if (fgets(buf, sizeof buf, stdin) == NULL) {
      fatal("no header found");
    }
    if (memcmp(buf, "xbtoa Begin", 11) == 0) {
      break;
    }
  }

  while ((c = getchar()) != EOF) {
    if (isspace(c)) {
      if (c == '\n') line++;
      continue;
    } else if (c == 'x') {
      break;
    } else {
      decode(c);
    }
  }
  if(scanf("btoa End N %ld %lx E %lx S %lx R %lx ",
	 &n1, &n2, &oeor, &osum, &orot) != 5) {
    fatal("invalid trailer");
  }
  if (n1 != n2) {
    fatal("bad trialer bytecount");
  } else if (oeor != Ceor) {
    fatal("bad XOR check");
  } else if (osum != Csum) {
    fatal("bad check sum");
  } else if (orot != Crot) {
    fatal("bad rotate check");
  } else {
    /*copy OK tmp file to stdout*/;
    fseek(tmp_file, 0L, 0);
    for (i = n1; --i >= 0;) {
      putchar(getc(tmp_file));
    }
  }
  exit(0);
}
