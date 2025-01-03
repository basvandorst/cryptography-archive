/*
 *	language.c - Foreign language translation for PGP
 *	Finds foreign language "subtitles" for English phrases 
 *	in external foriegn language text file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "usuals.h"
#include "fileio.h"
#include "language.h"
#include "pgp.h"

#define SUBTITLES_FILE	"language.txt"
#define LANG_INDEXFILE	"language.idx"

#define	STRBUFSIZE		2048

char language[16] = "en";	/* The language code, defaults to English */
static char	*strbuf;
static char	lang[16];	/* readstr sets this to the language id of the msg it last read */
static int	subtitles_available = 0;
static int line = 0;
/*	subtitles_available is used to determine if we know whether the special
	subtitles_file exists.  subtitles_available has the following values:
	0  = first time thru, we don't yet know if subtitles_file exists.
	1  = we have already determined that subtitles_file exists.
	-1 = we have already determined that subtitles_file does not exist.
*/

#define	NEWLINE		0
#define	COMMENT		1
#define	INSTRING	2
#define	ESCAPE		3
#define	IDENT		4
#define	DONE		5
#define	ERROR		6
#define	ERR1		7

/* Look for and return a quoted string from the file.
 * If nlabort is true, return failure if we find a blank line
 * before we find the opening quote.
 */
static char	*
readstr (FILE *f, char *buf, int nlabort)
{	int		c, d;
	char *p = buf;
	int state = NEWLINE;
	int i = 0;
	
	while ((c = getc(f)) != EOF)
	{
		if (c == '\r')
			continue;
		/* line numbers are only incremented when creating index file */
		if (line && c == '\n')
			++line;
		switch (state)
		{
			case NEWLINE:
				switch(c)
				{
					case '#': state = COMMENT; break;
					case '"': state = INSTRING; break;
					case '\n':
						if (nlabort)
						{	*buf = '\0';
							return(buf);
						}
					default:
						if (i == 0 && isalnum(c))
						{
							state = IDENT;
							lang[i++] = c;
							break;
						}
						if (!isspace(c))
						{
							fprintf(stderr, "language.txt:%d: syntax error\n", line);
							state = ERROR;
						}
				}
				break;
			case COMMENT:
				if (c == '\n')
					state = NEWLINE;
				break;
			case INSTRING:
				switch(c)
				{
					case '\\': state = ESCAPE; break;
					case '"': state = DONE; break;
					default: *p++ = c;
				}
				break;
			case ESCAPE:
				switch (c)
				{
					case 'n':	*p++ = '\n';	break;
					case 'r':	*p++ = '\r';	break;
					case 't':	*p++ = '\t';	break;
					case 'e':	*p++ = '\033';	break;
					case 'a':	*p++ = '\007';	break;
					case '#':
					case '"':
					case '\\':	*p++ = c; break;
					case '\n':	break;
					case '0':
							d = 0;
							while ((c = fgetc(f)) >= '0' && c <= '7')
								d = 8 * d + c - '0';
							*p++ = d;
							ungetc(c, f);
							break;
					default:
							fprintf(stderr, "language.txt:%d: illegal escape sequence: '\\%c'\n", line, c);
							break;
				}
				state = INSTRING;
				break;
			case IDENT:		/* language identifier */
				if (c == ':') {
					state = NEWLINE;
					break;
				}
				if (c == '\n' && strncmp(lang, "No translation", 14) == 0)
				{
					i = 0;
					state = NEWLINE;
					break;
				}
				lang[i++] = c;
				if (i == 15 || !isalnum(c) && !isspace(c))
				{
					lang[i] = '\0';
					fprintf(stderr, "language.txt:%d: bad language identifier: '%s'\n", line, lang);
					state = ERROR;
					i = 0;
				}
				break;
			case DONE:
				if (c == '\n')
				{
					lang[i] = '\0';
					*p = '\0';
					return(buf);
				}
				if (!isspace(c))
				{
					fprintf(stderr, "language.txt:%d: extra characters after '\"'\n", line);
					state = ERROR;
				}
				break;
			case ERROR:
				if (c == '\n')
					state = ERR1;
				break;
			case ERR1:
				state = (c == '\n' ? NEWLINE : ERROR);
				break;
		}
	}
	if (state != NEWLINE)
		fprintf(stderr, "language.txt: unexpected EOF\n");
	return(NULL);
}

#ifdef TEST
main()
{
	char buf[2048];

	line = 1;
	while (readstr(stdin, buf, 0)) {
		printf("\nen: <%s>\n", buf);
		while (readstr(stdin, buf, 1) && *buf != '\0')
			printf("%s: <%s>\n", lang, buf);
	}
	exit(0);
}
#else

static struct indx_ent {
	word32	crc;
	long	offset;
} *indx_tbl = NULL;

static int max_msgs = 0;
static int nmsg = 0;

static FILE *langf;

static void init_lang();
static int make_indexfile(char *);
word32 crcupdate(byte, word32);
void init_crc();

/*
 * uses 24-bit CRC function from armor.c
 */
static word32
message_crc(char *s)
{
	word32 crc = 0;

	while (*s)
		crc = crcupdate(*s++, crc);
	return(crc);
}

/*
 * lookup file offset in indx_tbl
 */
static long
lookup_offset(word32 crc)
{
	int i;
	
	for (i = 0; i < nmsg; ++i)
		if (indx_tbl[i].crc == crc)
			return(indx_tbl[i].offset);
	return(-1);
}


/*
 * return foreign translation of s
 */
char *
PSTR (char *s)
{
	long filepos;

	if (subtitles_available == 0)
		init_lang();
	if (subtitles_available < 0)
		return(s);

	filepos = lookup_offset(message_crc(s));
	if (filepos == -1)
		return(s);
	else
	{
		fseek(langf, filepos, SEEK_SET);
		readstr(langf, strbuf, 1);
	}

	if (strbuf[0] == '\0')
		return(s);

	for (s = strbuf; *s; ++s)
		*s = EXT_C(*s);
	return(strbuf);
}


static struct {
	long lang_fsize;	/* size of language.txt */
	char lang[16];		/* language identifier */
	int nmsg;			/* number of messages */
} indx_hdr;


/*
 * initialize the index table: read it from language.idx or create
 * a new one and write it to the index file. A new index file is
 * created if the language set in config.pgp doesn't match the one
 * in language.idx or if the size of language.txt has changed.
 */
static void
init_lang()
{
	char indexfile[MAX_PATH];
	char subtitles_file[MAX_PATH];
	FILE *indexf;

	if (strcmp(language, "en") == 0)
	{	subtitles_available = -1;
		return;		/* use default messages */
	}

	buildfilename (subtitles_file, SUBTITLES_FILE);
	if ((langf = fopen(subtitles_file, FOPRTXT)) == NULL)
	{
		subtitles_available = -1;
		return;
	}
	init_crc();
	if ((strbuf = (char *) malloc(STRBUFSIZE)) == NULL)
	{
		fprintf(stderr, "Not enough memory for foreign subtitles\n");
		fclose(langf);
		subtitles_available = -1;
		return;
	}
	buildfilename(indexfile, LANG_INDEXFILE);
	if ((indexf = fopen(indexfile, FOPRBIN)) != NULL)
	{
		if (fread(&indx_hdr, 1, sizeof(indx_hdr), indexf) == sizeof(indx_hdr) &&
			indx_hdr.lang_fsize == fsize(langf) &&
			strcmp(indx_hdr.lang, language) == 0)
		{
			nmsg = indx_hdr.nmsg;
			indx_tbl = (struct indx_ent *) malloc(nmsg * sizeof(struct indx_ent));
			if (indx_tbl == NULL)
			{
				fprintf(stderr, "Not enough memory for foreign subtitles\n");
				fclose(indexf);
				fclose(langf);
				subtitles_available = -1;
				return;
			}
			if (fread(indx_tbl, sizeof(struct indx_ent), nmsg, indexf) != nmsg)
			{
				free(indx_tbl);	/* create a new one */
				indx_tbl = NULL;
			}
		}
		fclose(indexf);
	}
	if (indx_tbl == NULL && make_indexfile(indexfile) < 0)
	{
		fclose(langf);
		subtitles_available = -1;
	}
	else
		subtitles_available = 1;
}


static int
make_indexfile(char *indexfile)
{
	FILE *indexf;
	long filepos;
	int total_msgs = 0;
	char *res;

	if (verbose)	/* must be set in config.pgp */
		fprintf(stderr, "Creating language index file '%s' for language \"%s\"\n",
				indexfile, language);
	rewind(langf);
	indx_hdr.lang_fsize = fsize(langf);
	strncpy(indx_hdr.lang, language, 15);
	init_crc();
	line = 1;
	nmsg = 0;
	while (readstr(langf, strbuf, 0))
	{
		if (nmsg == max_msgs)
		{
			if (max_msgs)
			{	max_msgs *= 2;
				indx_tbl = (struct indx_ent *) realloc(indx_tbl, max_msgs *
							sizeof(struct indx_ent));
			}
			else
			{	max_msgs = 400;
				indx_tbl = (struct indx_ent *) malloc(max_msgs *
							sizeof(struct indx_ent));
			}
			if (indx_tbl == NULL)
			{
				fprintf(stderr, "Not enough memory for foreign subtitles\n");
				return(-1);
			}
		}
		++total_msgs;
		indx_tbl[nmsg].crc = message_crc(strbuf);
		if (lookup_offset(indx_tbl[nmsg].crc) != -1)
			fprintf(stderr, "language.txt:%d: Message CRC not unique: \"%s\"\n",
					line, strbuf);
		do
		{
			filepos = ftell(langf);
			res = readstr (langf, strbuf, 1);		/* Abort if find newline first */
		} while (res && strbuf[0] != '\0' && strcmp(language, lang) != 0);

		if (res == NULL)
			break;
		if (strbuf[0] == '\0')	/* No translation */
			continue;

		indx_tbl[nmsg].offset = filepos;
		++nmsg;
		do
			res = readstr (langf, strbuf, 1);		/* Abort if find newline first */
		while (res && strbuf[0] != '\0');
	}
	line = 0;
	indx_hdr.nmsg = nmsg;
	if (nmsg == 0)
	{	fprintf(stderr, "No translations available for language \"%s\"\n\n",
				language);
		return(-1);
	}
	if (verbose || total_msgs != nmsg)
		fprintf(stderr, "%d messages, %d translations\n\n", total_msgs, nmsg);

	if ((indexf = fopen(indexfile, FOPWBIN)) == NULL)
		fprintf(stderr, "Cannot create %s\n", indexfile);
	else
	{
		fwrite(&indx_hdr, 1, sizeof(indx_hdr), indexf);
		fwrite(indx_tbl, sizeof(struct indx_ent), nmsg, indexf);
		if (ferror(indexf) || fclose(indexf))
			fprintf(stderr, "error writing %s\n", indexfile);
	}
	return(0);
}
#endif /* TEST */
