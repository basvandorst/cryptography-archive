/*
 * munge.c -- Program to convert a text file into "munged" form,
 *            suitable for reconstruction from printed form.  Tabs are
 *            made visible and checksums are added to each line and each
 *            page to protect against transcription errors.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Designed by Colin Plumb, Mark H. Weaver, and Philip R. Zimmermann
 * Written by Mark H. Weaver
 *
 * $Id: munge.c,v 1.32 1997/11/12 23:28:53 mhw Exp $
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "util.h"

/*
 * The file is divided into pages, and the format of each page is
 *
--f414 000b2dc79af40010002 Page 1 of munge.c

bc38e5 /*
40a838  * munge.c -- Program to convert a text file into munged form
647222  *
193f28  * Copyright (C) 1997 Pretty Good Privacy, Inc.
827222  *
699025  * Designed by Colin Plumb, Mark H. Weaver, and Philip R. Zimmermann
0d050c  * Written by Mark H. Weaver
 *
 * Where the first 2 columns are the high 8 bits (in hex) of a running
 * CRC-32 of the page (the string "--", unlikely to be confused with
 * any digits, indicates a page header line) and the next 4 columns
 * are a CRC-16 of the rest of the line.  Then a space (not counted in
 * the CRC), and the line of text.  Tabs are printed as the currency
 * symbol (ISO Latin 1 character 164) followed by the appropriate number
 * of spaces, and any form feeds are printed as a yen symbol (Latin 1 165).
 * The CRC is computed on the transformed line, including the trailing
 * newline.  No trailing whitespace is permitted.
 *
 * The header line contains a (hex) number of the form 0ffcccccccctpppnnnn,
 * where the digit 0 is a version number, ff are flags, ccccccc is the CRC-32
 * of the page, t is the tab size (usually 4 or 8; 0 for binary files that
 * are sent in radix-64), ppp is the product number (usually 1, different
 * for different books), and nnnn is the file number (sequential from 1).
 *
 * This is followed by " Page %u of " and the file name.
 */

typedef struct MungeState
{
	EncodeFormat const *	fmt;
	EncodeFormat const *	hFmt;
	int				binaryMode, tabWidth;
	long			origLineNumber;
	long			productNumber, fileNumber, pageNumber, lineNumber;
	unsigned long	fileOffset;
	CRC				pageCRC;
	char const *	fileName;
	char const *	fileNameTail;
	char *			pageBuffer;	/* Buffer large enough to hold one page */
	char *			pagePos;	/* Current position in pageBuffer */
	word16			hdrFlags;
	FILE *			file;
	FILE *			out;
} MungeState;


void ChecksumLine(EncodeFormat const *fmt, char const *line, size_t length,
				  char *prefix, CRC *pageCRC)
{
	CRC			lineCRC;
	CRC			runCRCPart = 0;

	lineCRC = CalculateCRC(fmt->lineCRC, 0, (byte const *)line, length);
	if (pageCRC != NULL)
	{
		*pageCRC = CalculateCRC(fmt->pageCRC, *pageCRC,
								(byte const *)line, length);
		runCRCPart = RunningCRCFromPageCRC(fmt, *pageCRC);
	}

	prefix += EncodeCheckDigits(fmt, runCRCPart, fmt->runningCRCBits, prefix);
	prefix += EncodeCheckDigits(fmt, lineCRC, fmt->lineCRC->bits, prefix);

	*prefix++ = ' ';	/* Write a space over the null byte */
}

/* Returns 1 for convenience */
int PrintFileError(MungeState *state, char const *message)
{
	fprintf(stderr, "%s in %s %s %lu\n", message, state->fileName,
			state->binaryMode ? "offset" : "line",
			state->binaryMode ? state->fileOffset : state->origLineNumber);
	return 1;
}

int MungeLine(MungeState *state, char *buffer, int length,
			  char *line, int *bufferUsed)
{
	int		i = 0, j = 0, jOld = 0;
	char	ch;

	for (i = 0; i < length && j < LINE_LENGTH; i++)
	{
		jOld = j;
		ch = buffer[i];
		if (ch == '\t')
		{
			line[j++] = TAB_CHAR;
			if (state->tabWidth < 1)
				return PrintFileError(state,
									  "ERROR: Tab found in radix64 stream");
			else
				while (j % state->tabWidth && j < LINE_LENGTH)
					line[j++] = TAB_PAD_CHAR;
		}
		else if (ch == '\n')
		{
			if (i + 1 < length)
				return PrintFileError(state,
								"UNEXPECTED ERROR: fgets read past newline!?");
			break;
		}
		else if (ch == '\f')
		{
			break;
		}
		else if (ch == ' ' && (j <= 0 || line[j-1] == ' ' ||
							   line[j-1] == SPACE_CHAR ||
							   i+1 >= length || buffer[i+1] == '\n'))
		{
			line[j++] = SPACE_CHAR;
		}	
		else if (ch >= ' ' && ch <= '~')
			line[j++] = ch;
		else
			return PrintFileError(state, "ERROR: Non-ASCII char");
	}

	if (i < length && buffer[i] == '\n')
	{
		i++;
		state->origLineNumber++;
	}
	else if (i < length && buffer[i] == '\f' && j < LINE_LENGTH)
	{
		i++;
		line[j++] = FORMFEED_CHAR;
	}
	else
	{
		/* If there's no newline, we need to add the continuation marker */
		if (i > 0 && j >= LINE_LENGTH)
		{
			/* Remove the last character if we're out of room */
			i--;
			j = jOld;
		}
		line[j++] = CONTIN_CHAR;
	}

	/* Strip trailing spaces */
	while (j > 0 && isspace((unsigned char)line[j - 1]))
		j--;

	if (j > LINE_LENGTH)	/* This should never happen */
		return PrintFileError(state, "ERROR: Internal error, line too long");

	/* Add trailing newline and NULL */
	line[j++] = '\n';
	line[j++] = '\0';

	/* Return number of chars used from buffer */
	*bufferUsed = i;

	return 0;
}

static void
Encode3(byte const src[3], char dest[4])
{
	dest[0] = radix64Digits[                     (src[0]>>2 & 0x3f)];
	dest[1] = radix64Digits[(src[0]<<4 & 0x30) | (src[1]>>4 & 0x0f)];
	dest[2] = radix64Digits[(src[1]<<2 & 0x3c) | (src[2]>>6 & 0x03)];
	dest[3] = radix64Digits[(src[2]    & 0x3f)];
}

static int
EncodeLine(byte const *src, int srcLen, char *dest)
{
	char *	destp = dest;
	byte	tempSrc[3];

	for (; srcLen >= 3; srcLen -= 3)
	{
		Encode3(src, destp);
		src += 3; destp += 4;
	}

	if (srcLen > 0)
	{
		memset(tempSrc, 0, sizeof(tempSrc));
		memcpy(tempSrc, src, srcLen);
		Encode3(src, destp);
		src += 3; destp += 4; srcLen -= 3;
		while (srcLen < 0)
			destp[srcLen++] = RADIX64_END_CHAR;
	}

	return destp - dest;
}

static int
MungeBinaryLine(MungeState *state, byte const *buffer, int length, char *line)
{
	char	binLine[128];
	int		binLength;			/* Destination length */
	int		used;

	binLength = EncodeLine(buffer, length, binLine);

	/* Append newline */
	binLine[binLength++] = '\n';
	binLine[binLength] = '\0';

	return MungeLine(state, binLine, binLength, line, &used);
}

int MaybePageBreak(MungeState *state)
{
	EncodeFormat const *	fmt = state->fmt;
	EncodeFormat const *	hFmt = state->hFmt;

	if (state->lineNumber >= LINES_PER_PAGE)
	{
		char	line[512];
		char *	lineData	= line + PREFIX_LENGTH;
		char *	p			= lineData;
		
		p += EncodeCheckDigits(hFmt, 0, HDR_VERSION_BITS, p);
		p += EncodeCheckDigits(hFmt, state->hdrFlags, HDR_FLAG_BITS, p);
		p += EncodeCheckDigits(hFmt, state->pageCRC, fmt->pageCRC->bits, p);
		p += EncodeCheckDigits(hFmt, state->tabWidth, HDR_TABWIDTH_BITS, p);
		p += EncodeCheckDigits(hFmt, state->productNumber, HDR_PRODNUM_BITS, p);
		p += EncodeCheckDigits(hFmt, state->fileNumber, HDR_FILENUM_BITS, p);

		sprintf(p, " Page %ld of %s\n", state->pageNumber + 1,
				state->fileNameTail);

		if (strlen(lineData) > LINE_LENGTH + 1)
		{
			PrintFileError(state, "ERROR: Header line too long");
			fprintf(stderr, "> %s", lineData);
			return -1;
		}

		/* Compute checksums and prefix them to line */
		ChecksumLine(fmt, lineData, strlen(lineData), line, NULL);

		fprintf(state->out, "%c%c%s\n%s\f", HDR_PREFIX_CHAR,
				fmt->headerTypeChar, line + 2, state->pageBuffer);

		state->pageNumber++;
		state->lineNumber = 0;
		state->pageCRC = 0;
		state->pagePos = state->pageBuffer;		/* Clear page buffer */
	}
	return 0;
}

/*
 * Search for Emacs "tab-width: " maker in file.
 * Emacs is stricter about the format, but this will do.
 */
int FindTabWidth(MungeState *state)
{
	char const * const	tabWidthMarker = " tab-width: ";
	char				buffer[512];
	char *				p;
	int					length;
	int					tabWidth = 0;

	fseek(state->file, -(sizeof(buffer) - 1), SEEK_END);
	length = fread(buffer, 1, sizeof(buffer) - 1, state->file);
	buffer[length] = '\0';
	p = strstr(buffer, tabWidthMarker);
	if (p != NULL)
	{
		p += strlen(tabWidthMarker);
		while (*p != '\0' && *p != '\n' && isspace(*p))
			p++;
		tabWidth = strtol(p, &p, 10);
		while (*p != '\0' && *p != '\n' && isspace(*p))
			p++;
		if (*p != '\n' || tabWidth < 2)
			tabWidth = 0;
		else if (tabWidth > 16)
			fprintf(stderr, "WARNING: Weird tab-width (%d), %s\n",
							tabWidth, state->fileName);
	}
	return tabWidth;
}

/*
 * Open the given source file and send the munged output to the
 * FILE *, with the given options.
 */
int MungeFile(char const *fileName, FILE *out, EncodeFormat const *fmt,
			  int binaryMode, int defaultTabWidth,
			  long productNumber, long fileNumber)
{
	MungeState *	state;
	int				length, used;
	char			line[PREFIX_LENGTH + LINE_LENGTH + 10];
	char *			lineData = line + PREFIX_LENGTH;
	char			buffer[128];
	int				result = 0;

	state = (MungeState *)calloc(1, sizeof(*state));
	state->fmt = fmt;
	state->hFmt = &hexFormat;
	state->origLineNumber = 1;
	state->fileName = fileName;
	state->pageCRC = 0;
	state->productNumber = productNumber;
	state->fileNumber = fileNumber;
	state->pageNumber = 0;
	state->lineNumber = 0;
	state->fileOffset = 0;
	state->binaryMode = binaryMode;
	state->pageBuffer = malloc(PAGE_BUFFER_SIZE);
	state->pageBuffer[0] = '\0';
	state->pagePos = state->pageBuffer;
	state->hdrFlags = 0;
	state->out = out;

	state->fileNameTail = strrchr(state->fileName, '/');
	if (state->fileNameTail == NULL)
		state->fileNameTail = state->fileName;
	else
		state->fileNameTail++;

	state->file = fopen(state->fileName, binaryMode ? "rb" : "r");
	if (state->file == NULL)
	{
		result = errno;
		fprintf(stderr, "ERROR opening %s: %s\n",
				state->fileName, strerror(result));
		goto error;
	}
	
	if (state->binaryMode)
	{
		state->tabWidth = 0;
	}
	else
	{
		state->tabWidth = FindTabWidth(state);
		if (state->tabWidth == 0)
			state->tabWidth = defaultTabWidth;
		rewind(state->file);
	}

	while (!feof(state->file))
	{
		if (state->binaryMode)
		{
			length = fread(buffer, 1, BYTES_PER_LINE, state->file);
			if (length < 1)
			{
				if (feof(state->file))
					break;
				goto fileError;
			}
			if ((result = MaybePageBreak(state)))
				goto error;
			if ((result = MungeBinaryLine(state, buffer, length, lineData)))
				goto error;
			state->fileOffset += length;
		}
		else
		{
			if (fgets(buffer, sizeof(buffer), state->file) == NULL)
			{
				if (feof(state->file))
					break;
				goto fileError;
			}
			length = strlen(buffer);
			if ((result = MaybePageBreak(state)))
				goto error;
			if ((result = MungeLine(state, buffer, length, lineData, &used)))
				goto error;

			if (used < length)
				if (fseek(state->file, used - length, SEEK_CUR))
					goto fileError;
		}

		/* Compute checksums and prefix them to the line */
		ChecksumLine(fmt, lineData, strlen(lineData), line, &state->pageCRC);

		strcpy(state->pagePos, line);
		length = strlen(state->pagePos);
		/* Suppress trailing whitespace on blank lines */
		if (length == PREFIX_LENGTH+1 && state->pagePos[length-1] == '\n') {
			state->pagePos[--length-1] = '\n';
			state->pagePos[length] = '\0';
		}
		state->pagePos += length;

		state->lineNumber++;
	}

	if (state->lineNumber > 0)
	{
		/* Force a final page break */
		state->lineNumber = LINES_PER_PAGE;
		state->hdrFlags |= HDR_FLAG_LASTPAGE;
		if ((result = MaybePageBreak(state)))
			goto error;
	}

	result = 0;
	goto done;

fileError:
	result = ferror(state->file);

error:
done:
	if (state != NULL)
	{
		if (state->file != NULL)
			fclose(state->file);
		free(state);
	}
	return result;
}

int main(int argc, char *argv[])
{
	int		result = 0;
	int		i, j;
	int		defaultTabWidth = 4;
	int		binaryMode = 0;
	long	productNumber = 1;
	long	fileNumber = 1;
	char *	endOfNumber;
	EncodeFormat const *	fmt = NULL;

	InitUtil();

	for (i = 1; i < argc && argv[i][0] == '-'; i++)
	{
		if (0 == strcmp(argv[i], "--"))
		{
			i++;
			break;
		}
		for (j = 1; argv[i][j] != '\0'; j++)
		{
			if (isdigit(argv[i][j]))
			{
				defaultTabWidth = argv[i][j] - '0';
				if (defaultTabWidth < 2 || defaultTabWidth > 9)
					fprintf(stderr, "WARNING: Weird default tab-width (%d)\n",
									defaultTabWidth);
			}
			else if (argv[i][j] == 'b')
			{
				binaryMode = 1;
			}
			else if (argv[i][j] == 'F')
			{
				fmt = FindFormat(argv[i][j+1]);
				if (!fmt || argv[i][j+2] != '\0')
				{
					fprintf(stderr, "ERROR: Invalid format char\n");
					exit(1);
				}
				break;
			}
			else if (argv[i][j] == 'p')
			{
				productNumber = strtol(&argv[i][j+1], &endOfNumber, 10);
				if (*endOfNumber != '\0')
				{
					fprintf(stderr, "ERROR: Invalid product number\n");
					exit(1);
				}
				break;
			}
			else if (argv[i][j] == 'f')
			{
				fileNumber = strtol(&argv[i][j+1], &endOfNumber, 10);
				if (*endOfNumber != '\0')
				{
					fprintf(stderr, "ERROR: Invalid file number\n");
					exit(1);
				}
				break;
			}
			else
			{
				fprintf(stderr, "ERROR: Unrecognized option -%c\n", argv[i][j]);
				exit(1);
			}
		}
	}
	if (!fmt)
		fmt = binaryMode ? &radix64Format : &hexFormat;

	for (; i < argc; i++)
	{
		if ((result = MungeFile(argv[i], stdout, fmt, binaryMode,
								defaultTabWidth, productNumber,
								fileNumber)) != 0)
		{
			/* If result > 0, message should have already been printed */
			if (result < 0)
				fprintf(stderr, "ERROR: %s\n", strerror(result));
			exit(1);
		}
		fileNumber++;
	}
	
	return 0;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
