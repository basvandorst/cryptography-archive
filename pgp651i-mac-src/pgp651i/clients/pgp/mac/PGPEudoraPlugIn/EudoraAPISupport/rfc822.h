/* =========================================================================	Functions for very basic RFC-822 header manipulation	         	Filename:			RFC822.h	Last Edited:		March 7, 1997	Authors:			Laurence Lundblade, Myra Callen, Bob Fronabarger	Copyright:			1995, 1996 QUALCOMM Inc.	Technical support:	<emsapi-info@qualcomm.com>	Some of this code is from the c-client and is 		Copyright University of Washington*/char			*RFC822_ExtractToken(char **cpp);char			*RFC822_SkipWS(char *cp);char			*RFC822_SkipWord(char *cp);unsigned short	RFC822_QuotedStrLen(StringPtr theStr);char			*RFC822_QuoteStrCpy(char *dst, StringPtr theStr);char			*RFC822_ExtractHeader(const char *pFullHeader, const char *pLinePrefix);extern const char	*hSpecials, *wSpecials, *ptSpecials;