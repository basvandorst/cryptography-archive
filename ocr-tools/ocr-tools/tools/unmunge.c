/*
 * unmunge.c -- Program to convert a munged file to original form
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Designed by Colin Plumb, Mark H. Weaver, and Philip R. Zimmermann
 * Written by Mark H. Weaver
 *
 * $Id: unmunge.c,v 1.13 1997/11/13 23:27:08 mhw Exp $
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

/*#include <direct.h>   teun: MS VC wants direct.h for mkdir */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include "util.h"

typedef struct UnMungeState
{
	char const *	mungedFileName;
	char			dirName[128];
	char			fileName[128];
	char *			fileNameTail;
	int				binaryMode, tabWidth;
	long			productNumber, fileNumber, pageNumber, lineNumber;
	long			manifestLineNumber;
	word16			hdrFlags;
	CRC				pageCRC, seenPageCRC;
	FILE *			manifest;
	FILE *			file;
	FILE *			out;
} UnMungeState;


/* Returns number of characters decoded, or -1 on error */
static int
Decode4(char const src[4], byte dest[3])
{
	int		i, length;
	byte	srcVal[4];

	for (i = 0; i < 4 && src[i] != RADIX64_END_CHAR; i++)
		if ((srcVal[i] = Radix64DigitValue(src[i])) == (byte) -1)
			return 1;

	length = i - 1;
	if (length < 1)
		return -1;

	for (; i < 4; i++)
		srcVal[0] = 0;

	dest[0] = (srcVal[0] << 2) | (srcVal[1] >> 4);
	dest[1] = (srcVal[1] << 4) | (srcVal[2] >> 2);
	dest[2] = (srcVal[2] << 6) | (srcVal[3]);

	return length;
}

/*
 * Return number of characters decoded, or -1 on error
 */
static int
DecodeLine(char const *src, char *dest, int srclength)
{
	int destlength = 0;
	int result;

	if (srclength % 4 || !srclength)
		return -1;	/* Must be a multiple of 4 */

	while (srclength -= 4) {
		if (Decode4(src, dest + destlength) != 3)
			return -1;
		src += 4;
		destlength += 3;
	}
	result = Decode4(src, dest + destlength);
	if (result < 1)
		return -1;
	return destlength + result;
}

int PrintFileError(UnMungeState *state, char const *message)
{
	fprintf(stderr, "%s, %s line %ld\n", message,
			state->mungedFileName, state->lineNumber);
	return 1;
}

int ReadManifest(UnMungeState *state, long fileNumberWanted,
				 char const *fileTailPrefix, long prefixLen)
{
	long		fileNumber = 0;
	long		firstMissingFileNum = 0, lastMissingFileNum = 0;
	char		buffer[512];
	char *		p;

	if (state->manifest == NULL)
	{
		if (fileNumberWanted != 0)
		{
			assert(fileTailPrefix != NULL);
			strncpy(state->fileName, fileTailPrefix, sizeof(state->fileName));
			state->fileName[sizeof(state->fileName) - 1] = '\0';
			state->fileNameTail = state->fileName;
		}
		return 0;
	}
	while (fgets(buffer, sizeof(buffer), state->manifest))
	{
		if ((p = strchr(buffer, '\n')) != NULL)
			*p = '\0';
		state->manifestLineNumber++;
		if (buffer[0] == 'D')
		{
			if (buffer[1] != ' ')
				goto invalidManifest;
			strncpy(state->dirName, buffer + 2, sizeof(state->dirName));
			if (state->dirName[sizeof(state->dirName) - 1] != '\0')
				goto invalidManifest;
		}
		else
		{
			fileNumber = strtol(buffer, &p, 10);
			if (p == buffer || *p != ' ')
				goto invalidManifest;
			p++;

			if (fileNumberWanted == 0 || fileNumber < fileNumberWanted)
			{
				if (firstMissingFileNum == 0)
					firstMissingFileNum = fileNumber;
				lastMissingFileNum = fileNumber;
				continue;
			}
			else if (fileNumber > fileNumberWanted)
				break;
			else
			{
				size_t		len;

				len = strlen(state->dirName);
				assert(sizeof(state->fileName) >= sizeof(state->dirName));
				memcpy(state->fileName, state->dirName, len);
				strncpy(state->fileName + len, p,
						sizeof(state->fileName) - len);
				if (strncmp(p, fileTailPrefix, prefixLen) != 0)
				{
					fprintf(stderr, "Mismatched filename, headers say '%s',\n"
							"  manifest says '%s'\n",
							fileTailPrefix, p);
					return 1;
				}
				p = state->dirName;
				while ((p = strchr(p, '/')) != NULL)
				{
					*p = '\0';
					mkdir(state->dirName, 0777);
					*p++ = '/';
				}
				state->fileNameTail = state->fileName + len;
				break;
			}
		}
	}
	if (firstMissingFileNum != 0)
	{
		fprintf(stderr, "Missing files %ld-%ld\n",
				firstMissingFileNum, lastMissingFileNum);
	}
	if (fileNumberWanted != 0 && fileNumber != fileNumberWanted)
	{
		fprintf(stderr, "Can't find file %ld in manifest file\n",
				fileNumberWanted);
		return 1;
	}
	return 0;

invalidManifest:
	fprintf(stderr, "Error parsing manifest file, line %ld\n",
			state->manifestLineNumber);
	return 1;
}

int UnMungeFile(char const *mungedFileName, char const *manifestFileName,
				int forceOverwrite, int forcePartialFiles)
{
	UnMungeState *	state;
	EncodeFormat const *	fmt = NULL;
	char			buffer[512];
	char			outbuf[BYTES_PER_LINE+1];
	char *			line;
	char *			lineData;
	char *			p;
	int				length;
	int				result = 0;
	int				skipPage = 0;
	CRC				lineCRC;
	word32			num;

	state = (UnMungeState *)calloc(1, sizeof(*state));
	state->mungedFileName = mungedFileName;

	if (manifestFileName != NULL)
	{
		if ((state->manifest = fopen(manifestFileName, "r")) == NULL)
			goto errnoError;
	}

	if ((state->file = fopen(state->mungedFileName, "r")) == NULL)
		goto errnoError;

	while (!feof(state->file))
	{
		if (fgets(buffer, sizeof(buffer), state->file) == NULL)
		{
			if (feof(state->file))
				break;
			goto fileError;
		}

		state->lineNumber++;

		line = buffer;
		/* Strip leading whitespace */
		while (isspace(*line))
			line++;
		if (*line == '\0')
			continue;

		/* Strip trailing whitespace */
		p = line + strlen(line);
		while (p > line && (byte)p[-1] < 128 && isspace(p[-1]))
			p--;

		lineData = line + PREFIX_LENGTH;

		/* Pad up to at least PREFIX_LENGTH */
		while (p < lineData)
			*p++ = ' ';
		*p++ = '\n';
		*p = '\0';
		length = p - lineData;

		if (line[0] == HDR_PREFIX_CHAR)
		{
			fmt = FindFormat(line[1]);
			if (!fmt)
			{
				result = PrintFileError(state, "ERROR: Invalid header type");
				goto error;
			}
		}

		lineCRC = CalculateCRC(fmt->lineCRC, 0, (byte const *)lineData, length);

		p = line + EncodedLength(fmt, fmt->runningCRCBits);
		if (DecodeCheckDigits(fmt, p, NULL, fmt->lineCRC->bits, &num)
				|| lineCRC != num)
		{
			result = PrintFileError(state, "ERROR: Line CRC failed");
			goto error;
		}

		if (line[0] == HDR_PREFIX_CHAR)
		{
			int			formatVersion;
			int			flags;
			CRC			seenPageCRC;
			int			tabWidth;
			long		productNumber;
			long		fileNumber;
			long		pageNumber;
			char *		fileNameTail;
			int			skipNextPage = 0;
			char *		p;
			EncodeFormat const *	hFmt = &hexFormat;

			/* Parse header line */
			p = lineData;

			if (DecodeCheckDigits(hFmt, p, &p, HDR_VERSION_BITS, &num))
			{
			invalidHeader:
				result = PrintFileError(state, "ERROR: Invalid header");
				goto error;
			}
			formatVersion = num;

			if (DecodeCheckDigits(hFmt, p, &p, HDR_FLAG_BITS, &num))
				goto invalidHeader;
			flags = num;

			if (DecodeCheckDigits(hFmt, p, &p, fmt->pageCRC->bits, &num))
				goto invalidHeader;
			seenPageCRC = num;

			if (DecodeCheckDigits(hFmt, p, &p, HDR_TABWIDTH_BITS, &num))
				goto invalidHeader;
			tabWidth = num;

			if (DecodeCheckDigits(hFmt, p, &p, HDR_PRODNUM_BITS, &num))
				goto invalidHeader;
			productNumber = num;

			if (DecodeCheckDigits(hFmt, p, &p, HDR_FILENUM_BITS, &num))
				goto invalidHeader;
			fileNumber = num;

			if (sscanf(p, " Page %ld of ", &pageNumber) < 1)
				goto invalidHeader;

			if (formatVersion > 0)
			{
				result = PrintFileError(state,
										"ERROR: Format too new for "
											"this version of unmunge");
				goto error;
			}

			p = strstr(p, " of ");
			if (p == NULL)
				goto invalidHeader;

			fileNameTail = p + 4;
			p = fileNameTail + strlen(fileNameTail);
			if (p < fileNameTail + 3 || p[-1] != '\n')
				goto invalidHeader;
			else
				p[-1] = '\0';

			if (state->out != NULL && state->pageCRC != state->seenPageCRC)
			{
				result = PrintFileError(state,
								"ERROR: Page CRC mismatch on page before");
				goto error;
			}

			if ((state->hdrFlags & HDR_FLAG_LASTPAGE) && state->out != NULL)
			{
				fclose(state->out);
				state->out = NULL;
			}

			if (state->out != NULL)
			{
				if (pageNumber != state->pageNumber + 1 ||
						fileNumber != state->fileNumber ||
						productNumber != state->productNumber ||
						tabWidth != state->tabWidth ||
						strcmp(fileNameTail, state->fileNameTail) != 0)
				{
					if (fileNumber == state->fileNumber &&
							pageNumber > state->pageNumber + 1)
					{
						(void)PrintFileError(state,
									"ERROR: Missing pages of this file");
						if (forcePartialFiles && !state->binaryMode)
						{
							fputs("\n\n@@@@@@ Missing pages here! @@@@@@\n\n",
								  state->out);
						}
						else
						{
							skipNextPage = 1;
							fclose(state->out);
							state->out = NULL;
							remove(state->fileName);
						}
					}
					else
					{
						(void)PrintFileError(state,
									"ERROR: Missing pages of previous file");
						if (forcePartialFiles && !state->binaryMode)
						{
							fputs("\n\n@@@@@@ Missing pages here! @@@@@@\n\n",
								  state->out);
							/* Make it non-fatal, though... */
							fclose(state->out);
							state->out = NULL;
						}
						else
						{
							fclose(state->out);
							state->out = NULL;
							remove(state->fileName);
						}
					}
				}
			}
			if (state->out == NULL)
			{
				if (pageNumber != 1 && !skipPage)
					(void)PrintFileError(state,
							 "ERROR: File doesn't begin with page 1");

				state->binaryMode = (tabWidth == 0);

				if (pageNumber != 1 && (state->binaryMode
										|| !forcePartialFiles))
				{
					skipNextPage = 1;
				}
				else
				{
					/* TODO: Use global filelist to get pathname */
					result = ReadManifest(state, fileNumber, fileNameTail,
										  strlen(fileNameTail));
					if (result != 0)
						goto error;

					if (!forceOverwrite)
					{
						FILE *	file;

						/* Make sure file doesn't already exist */
						file = fopen(state->fileName, "r");
						if (file != NULL)
						{
							fclose(file);
							fprintf(stderr, "ERROR: %s already exists\n",
									state->fileName);
							result = 1;
							goto error;
						}
					}

					state->out = fopen(state->fileName,
									   state->binaryMode ? "wb" : "w");
					if (state->out == NULL)
						goto errnoError;

					if (pageNumber != 1)
						fputs("\n\n@@@@@@ Missing pages here! @@@@@@\n\n",
							  state->out);
				}
			}

			state->pageCRC = 0;
			state->seenPageCRC = seenPageCRC;
			state->hdrFlags = (word16)flags;
			state->pageNumber = pageNumber;
			state->fileNumber = fileNumber;
			state->productNumber = productNumber;
			state->tabWidth = tabWidth;
			skipPage = skipNextPage;
		}
		else if (!skipPage)
		{
			if (state->out == NULL)
			{
				result = PrintFileError(state, "ERROR: Missing header line");
				goto error;
			}

			/* Normal data line */
			state->pageCRC = CalculateCRC(fmt->pageCRC, state->pageCRC,
											   (byte const *)lineData,
											   length);
			line[2] = '\0';
			if (DecodeCheckDigits(fmt, line, NULL, fmt->runningCRCBits, &num)
				|| RunningCRCFromPageCRC(fmt, state->pageCRC) != num)
			{
				result = PrintFileError(state, "ERROR: Running CRC failed");
				goto error;
			}

			if (state->binaryMode)
			{
				length = DecodeLine(lineData, outbuf, length-1);
				if (length < 0 || length > BYTES_PER_LINE) {
					result = PrintFileError(state,
									"ERROR: Corrupt radix-64 data");
					goto error;
				}
				fwrite(outbuf, 1, length, state->out);
			}
			else
			{
				p = lineData;
				while (*p != '\0')
				{
					if (*p == TAB_CHAR)
					{
						p++;
						putc('\t', state->out);
						while ((p - lineData) % state->tabWidth)
						{
							if (*p == '\n')
								break;
							else if (*p == ' ')
								p++;
							else
							{
								result = PrintFileError(state,
												"ERROR: Not enough spaces "
												"after a tab character");
								goto error;
							}
						}
					}
					else if (*p == FORMFEED_CHAR)
					{
						p++;
						if (*p != '\n')
						{
							result = PrintFileError(state,
											"ERROR: Formfeed character "
											"not at end of line");
							goto error;
						}
						p++;	/* Skip newline */
						putc('\f', state->out);
					}
					else if (*p == CONTIN_CHAR)
					{
						p++;
						if (*p != '\n')
						{
							result = PrintFileError(state,
											"ERROR: Continuation character "
											"not at end of line");
							goto error;
						}
						p++;	/* Skip newline */
					}
					else if (*p == SPACE_CHAR)
					{
						putc(' ', state->out);
						p++;
					}
					else
					{
						putc(*p, state->out);
						p++;
					}
				}
			}
		}
	}
	if (state->out != NULL)
	{
		if (!(state->hdrFlags & HDR_FLAG_LASTPAGE))
		{
			result = PrintFileError(state, "ERROR: Missing pages");
			goto error;
		}
		if (state->pageCRC != state->seenPageCRC)
		{
			result = PrintFileError(state,
							"ERROR: Page CRC failed on previous page");
			goto error;
		}
	}

	/* Check for missing files at the end */
	result = ReadManifest(state, 0, NULL, 0);
	goto done;

errnoError:
	result = errno;
	goto printError;

fileError:
	result = ferror(state->file);

printError:
	fprintf(stderr, "ERROR: %s\n", strerror(result));

error:
done:
	if (state != NULL)
	{
		if (state->out != NULL)
			fclose(state->out);
		if (state->file != NULL)
			fclose(state->file);
		if (state->manifest != NULL)
			fclose(state->manifest);
		free(state);
	}
	return result;
}

void UsageAndExit(int result)
{
	fprintf(stderr,
			"Usage: unmunge [-fp] <file> [<manifest>]\n"
			"  -f  Force overwrites of existing files\n"
			"  -p  Force unmunge of partial files\n");
	exit(result);
}

int main(int argc, char *argv[])
{
	int		result = 0;
	int		forceOverwrite = 0;
	int		forcePartialFiles = 0;
	char *	fileName = NULL;
	char *	manifestFileName = NULL;
	int		i, j;

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
			if (argv[i][j] == 'h')
				UsageAndExit(0);
			else if (argv[i][j] == 'f')
				forceOverwrite = 1;
			else if (argv[i][j] == 'p')
				forcePartialFiles = 1;
			else
			{
				fprintf(stderr, "ERROR: Unrecognized option -%c\n", argv[i][j]);
				UsageAndExit(1);
			}
		}
	}

	if (i < argc)
		fileName = argv[i++];
	if (i < argc)
		manifestFileName = argv[i++];
	if (fileName == NULL || i < argc)
		UsageAndExit(1);

	if ((result = UnMungeFile(fileName, manifestFileName,
							  forceOverwrite, forcePartialFiles)) != 0)
	{
		/* If result > 0, message should have already been printed */
		if (result < 0)
			fprintf(stderr, "ERROR: %s\n", strerror(result));
		exit(1);
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

