/*
 *  Functions to aide manipulation of MIME RFC822 headers.
 *
 *  Filename: RFC822.h
 *
 *  Last Edited: July 15, 1996
 *
 *  Author: Scott Manjourides
 *
 *  Portions adopted from code originally written by Stever Dorner.
 *  Copyright 1995, 1996 QUALCOMM Inc.
 *
 *  Send comments and questions to <emsapi-info@qualcomm.com>
 *
 */

#ifndef EMS_RFC822_H
#define EMS_RFC822_H

#include <istream.h> // ifstream class
#include <fstream.h> // ofstream class

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Find next RFC822 token in given string, copying it into a newly created
 *  string. Advance pointer past token and any following whitespace.
 *   
 *  NOTE: The user of this function is responsible for freeing returned
 *        string.
 *
 *  Args:
 *   cpp [IN/OUT] Handle (pointer-to-pointer) of RFC822 string to extract from
 *
 *  Returns: String containing next token, NULL if error.
 *         Moves cpp to first non-whitespace character AFTER extracted token
 */
char *rfc822_extract_token(char **cpp);

/*
 *  Advances to next non-whitespace character in string. This includes
 *  ignoring RFC822 comments.
 *   
 *  Args:
 *   cp [IN] RFC822 string
 *
 *  Returns: Pointer to next non-whitespace character.
 */
char *rfc822_skipws(char *cp);

/*
 *  Advances to next character in string directly after the current token.
 *  The first character must be the beginning of a valid token or end of
 *  string (ie. all whitespace must be skipped BEFORE calling this function).
 *
 *  Args:
 *   cp [IN] RFC822 string
 *
 *  Returns: Pointer to next valid whitespace character.
 */
char *rfc822_skipword(char *cp);

/*
 *  Calculates the length of the given text string if it were converted
 *  to an RFC822 string.
 *
 *  Args:
 *   cp [IN] Text string
 *
 *  Returns: Equivalent RFC822 length of given text.
 */
unsigned int rfc822_quoted_strlen(char *cp);

/*
 *  Copies and converts the source text string to a destination
 *  RFC822 string. The source must be NULL terminated, and the
 *  destination will be NULL terminated.
 *
 *  Args:
 *   dst [OUT] RFC822 string
 *   src [IN]  Text string
 *
 *  Returns: Pointer to destination NULL termination.
 */
char *rfc822_quote_strcpy(char *dst, const char *src);

/*
 *  Copies and converts the source RFC822 string to a destination
 *  text string. The source must be NULL terminated, and the
 *  destination will be NULL terminated.
 *
 *  Args:
 *   dst [OUT] Text string
 *   src [IN]  RFC822 string
 *   len [IN]  Maximum charcters to copy; Zero implies whole string
 *
 *  Returns: Pointer to destination NULL termination.
 */
char *rfc822_unquote_strcpy(char *dst, const char *src, unsigned int len);

/*
 *  Finds and extracts a header line from a full multi-lined header. All
 *  unfolding (removing newlines) is done before header line is returned.
 *
 *  NOTE: The user of this function is responsible for freeing the
 *        returned string.
 *
 *  Args:
 *   pFullHeader [IN] Pointer to a full RFC822 header, including newlines
 *   pLinePrefix [IN] Prefix of header line to extract
 *
 *  Returns: Extracted header line string; dynamically allocated.
 */
char *rfc822_extract_header(const char *pFullHeader, const char *pLinePrefix);

/*
 *  Reads and returns the RFC822 header from input file. This is defined
 *  here as reading until the first blank line from current file
 *  position. The blank line is discarded.
 *
 *  NOTE: The user of this function is responsible for freeing the
 *        returned string.
 *
 *  Args:
 *   inFile [IN] Input file stream; already open
 *
 *  Returns: Complete multi-lined RFC822 header; dynamically allocated.
 *     The file pointer is left immediately after the first read blank line.
 */
char *rfc822_read_header(ifstream& inFile);


#ifdef __cplusplus
}
#endif

#endif /* EMS_RFC822_H */
