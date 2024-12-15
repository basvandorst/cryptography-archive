/*
* pgpAnnotate.c -- convert an annotation type to a string.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpAnnotate.c,v 1.3.2.1 1997/06/07 09:49:59 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpAnnotate.h"

const char *
pgpScopeName (int type)
	{
		if (!PGP_IS_BEGIN_SCOPE (type))
		 return "Not a Begin Scope type";

		switch (type) {
		case PGPANN_UNKNOWN_BEGIN:
		 return "Unknown";
		case PGPANN_LITERAL_BEGIN:
		 return "Literal";
		case PGPANN_CIPHER_BEGIN:
		 return "Encrypted";
		case PGPANN_COMPRESSED_BEGIN:
		 return "Compressed";
		case PGPANN_COMMENT_BEGIN:
		 return "Comment";
		case PGPANN_SIGNED_BEGIN:
		 return "Signed";
		case PGPANN_NONPACKET_BEGIN:
		 return "Non";
		case PGPANN_NONPGP_BEGIN:
		 return "Unprotected";
		case PGPANN_ARMOR_BEGIN:
		 return "Ascii Armored";
		case PGPANN_CLEARSIG_BEGIN:
		 return "Clearsigned";
		case PGPANN_PGPKEY_BEGIN:
		 return "PGP Key";
		case PGPANN_INPUT_BEGIN:
		 return "Input";
		case PGPANN_FILE_BEGIN:
		 return "File";
		default:
		 return "HUH? Weird Packet";
		}
	}
