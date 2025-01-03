/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Map file name extension to mac creator/type codes
	Map file data to mac creator/type codes

	$Id: pgpMacFileMapping.c,v 1.33 1997/10/22 22:01:08 lloyd Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include <string.h>
#include "pgpStrings.h"

#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpMacFileMapping.h"
#include "pgpUtilitiesPriv.h"
#include "pgpHex.h"


#include <stdio.h>
#include <ctype.h>
#include "pgpFileUtilities.h"
#include "pgpMemoryIO.h"



/*____________________________________________________________________________
	Run-time data structure used for mapping.
____________________________________________________________________________*/	
typedef struct CreatorTypeToExtensionEntry
{
	OSType	creator;
	OSType	type;
	char	extension[ 3 + 1 ];
} CreatorTypeToExtensionEntry;

typedef struct CreatorTypeToExtensionTable
{
	PGPUInt32					numEntries;
	PGPUInt32					numAllocated;
	CreatorTypeToExtensionEntry	entries[ 1 ];	/* dynamic */
} CreatorTypeToExtensionTable;


/* global table; one for whole SDK */
static CreatorTypeToExtensionTable	*sMappingTable	= NULL;


/*____________________________________________________________________________
	CreatorTypeToExtensionBuiltInEntry maps a mac creator/type code to a
	suggested file name extension.
	
	Used for the following:
	1. when de-macbinarizing a file on a non-Mac to map to a useful
		file name extension.
	2.  To map a file name extension on a Mac to a creator type.
	3.	To decide which files "Smart" MacBinary applies to
	
IMPORTANT:
	Because this table can map many creator/type pairs to a single extension,
	and an extension to a single creator/type pair,
	the preferred mapping from a suffix to a creator/type pair should be
	listed FIRST in the table before all other mappings for that extension.
	
	A similar principle applies to wildcards.  They should be listed after
	any specific mappings.
____________________________________________________________________________*/	
#define kWildCard	0x2A2A2A2A	/* '****' */

#if PGP_MACINTOSH
#define kEndOfLineString	"\r"
#elif PGP_WIN32
#define kEndOfLineString	"\r\n"
#elif PGP_UNIX
#define kEndOfLineString	"\n"
#else
#error unknown OS
#endif


static const char
sCreatorTypeToExtensionDefaults[] =
{
	"# Mapping table for MacBinary and file name extension conversion"
		kEndOfLineString
	"# Format: " kEndOfLineString
	"#     <4 char Mac creator><tab>"
	"<4 char Mac type><tab><1-3 letter DOS extension>"
		kEndOfLineString
		kEndOfLineString
	"# The special code **** indicates a wild card for creator or type"
		kEndOfLineString
	"# Hex may be used for creator & type in the format 0xHHHHHHHH"
		kEndOfLineString
	"# Comment lines begin with '#'"
		kEndOfLineString
		kEndOfLineString
	


#define MappingLine( creator,	 type,	 ext )	 \
	creator "\t" type "\t" ext kEndOfLineString
	
/* BEGIN preferred mappings */
/* IMPORTANT: list preferred mappings first */
	MappingLine( "CWIE",	"TEXT",	"h")	/* MetroWerks CodeWarrior */
	MappingLine( "CWIE",	"TEXT",	"c")	/* MetroWerks CodeWarrior */
	MappingLine( "CWIE",	"TEXT",	"cp")	/* MetroWerks CodeWarrior */
	MappingLine( "CWIE",	"TEXT",	"cpp")	/* MetroWerks CodeWarrior */
	MappingLine( "ttxt",	"TEXT",	"txt")	/* SimpleText */
	MappingLine( "MOSS",	"TEXT",	"htm")	/* Navigator */
	MappingLine( "SITx",	"TEXT",	"hqx")	/* StuffIt Expander */
	
	/* graphics */
	MappingLine( "JVWR",	"JPEG",	"jpg")	
	MappingLine( "JVWR",	"GIFf",	"gif")	
	
/* types that StuffIt can expand: */
	MappingLine( "SIT!",	"BINA",	"zip")	/* StuffIt */
	MappingLine( "SIT!",	"BINA",	"gz")	/* StuffIt */
	MappingLine( "SIT!",	"BINA",	"tar")	/* StuffIt */
	MappingLine( "SIT!",	"BINA",	"uu")	/* StuffIt */
	MappingLine( "SIT!",	"BINA",	"bin")	/* StuffIt */
	MappingLine( "SIT!",	"BINA",	"arc")	/* StuffIt */
	MappingLine( "SIT!",	"BINA",	"z")		/* StuffIt */

/* END preferred mappings */
	
	/* NOTE: creator wild cards should usually come last in the last */
	/* that's so a more specific mapping can be found first */
	
	MappingLine( "8BIM",	"GIFf",	"gif")	/* Photoshop gif */
	MappingLine( "8BIM",	"8BPS",	"psd")	/* Photoshop  */
	MappingLine( "8BIM",	"TPIC",	"tga")	/* Photoshop  */
	MappingLine( "8BIM",	"JPEG",	"jpg")	/* Photoshop  */
	
	MappingLine( "ALD3",	"ALB3",	"pm3")	/* PageMaker */
	MappingLine( "ALD4",	"ALB4",	"pm4")	
	MappingLine( "ALD5",	"ALB5",	"pm5")	
	MappingLine( "ALD6",	"ALB6",	"pm6")	
	
	MappingLine( "MSPJ",	"MPP ",	"mpp")	/* Microsoft Project */
	MappingLine( "MSPJ",	"MPX ",	"mpp")	
	
	MappingLine( "AnVw",	"****",	"gl")	
	MappingLine( "arc*",	"mArc",	"arc")	
	MappingLine( "BnHq",	"TEXT",	"hqx")	
	MappingLine( "Booz",	"Zoo ",	"zoo")	
	MappingLine( "CARO",	"PDF ",	"pdf")	/* Acrobat */
	MappingLine( "CPCT",	"PACT",	"cpt")	
	MappingLine( "DAD2",	"drw2",	"cvs")	
	MappingLine( "DArj",	"BINA",	"arj")	
	MappingLine( "dCpy",	"dlmg",	"ima")	
	MappingLine( "dPro",	"EPSF",	"eps")	
	MappingLine( "dPro",	"PICT",	"mac")	
	MappingLine( "FMPR",	"FMPR",	"fm")	/* FileMaker Pro */
	
	MappingLine( "GKON",	"CGMm",	"cgm")	
	MappingLine( "GKON",	"DLdI",	"dl")	
	MappingLine( "GKON",	"FLI ",	"fli")	
	MappingLine( "GKON",	"ICO ",	"ico")	
	MappingLine( "GKON",	"ILBM",	"iff")	
	MappingLine( "GKON",	"IMGg",	"img")	
	MappingLine( "GKON",	"ILBM",	"lbm")	
	MappingLine( "GKON",	"MSPp",	"msp")	
	MappingLine( "GKON",	"STAD",	"PAC")	
	MappingLine( "GKON",	"PPGM",	"pbm")	
	MappingLine( "GKON",	"PICS",	"pcs")	
	MappingLine( "GKON",	"PCXx",	"pcx")	
	MappingLine( "GKON",	"PPGM",	"pgm")	
	MappingLine( "GKON",	"HPGL",	"plt")	
	MappingLine( "GKON",	"PMpm",	"pm")	
	MappingLine( "GKON",	"PPGM",	"ppm")	
	MappingLine( "GKON",	"RIFF",	"rif")	
	MappingLine( "GKON",	"RLE ",	"rle")	
	MappingLine( "GKON",	"SHPp",	"shp")	
	MappingLine( "GKON",	"Spec",	"spc")	
	MappingLine( "GKON",	"SUNn",	"sr")	
	MappingLine( "GKON",	"SUNn",	"sun")	
	MappingLine( "GKON",	"SCRN",	"sup")	
	MappingLine( "GKON",	"TARG",	"tga")	
	MappingLine( "GKON",	"WMF ",	"wmf")	
	MappingLine( "GKON",	"FLI ",	"flc")	
	
	MappingLine( "Gzip",	"Gzip",	"gz")	
	MappingLine( "JVWR",	"JPEG",	"jpg")	
	MappingLine( "JVWR",	"BMPp",	"vga")	
	MappingLine( "L123",	"WKS ",	"wks")	/* Lotus 123 */
	MappingLine( "LARC",	"LHA ",	"lha")	
	MappingLine( "LARC",	"LHA ",	"lzh")	
	MappingLine( "LZIV",	"ZIVU",	"z")	
	MappingLine( "mdos",	"BINA",	"exe")	
	MappingLine( "mMPG",	"MPEG",	"mpg")	
	
	
	/* NOTE: wildcards should generally be at the bottom of the list */
	MappingLine( "MSWD",	"WDBN",	"doc")	/* MS Word  */
	MappingLine( "MSWD",	"W6BN",	"doc")	
	MappingLine( "MSWD",	"WTBN",	"dot")	
	MappingLine( "MSWD",	"TEXT",	"txt")	
	MappingLine( "MSWD",	"RTF ",	"rtf")	
	MappingLine( "MSWD",	"****",	"doc")	/* MS Word (all")	 */
	
	MappingLine( "OTEX",	"ODVI",	"dvi")	
	MappingLine( "OTEX",	"TEXT",	"tex")	
	
	/* PGP */
	MappingLine( "pgpM",	"pgDS",	"sig")	/* detached sig */
	MappingLine( "pgpM",	"pgEF",	"pgp")	/* encrypted file */
	MappingLine( "pgpM",	"pgSF",	"pgp")	/* signed file */
	MappingLine( "pgpM",	"TEXT",	"asc")	/* message */
	MappingLine( "pgpK",	"pgPR",	"pkr") 	/* public ring */
	MappingLine( "pgpK",	"pgRR",	"skr")	/* secret ring */
	MappingLine( "pgpK",	"pgRS",	"rnd")	/* random seed */
	MappingLine( "pgpK",	"TEXT",	"asc")	/* data */
	
	MappingLine( "PPT3",	"SLD3",	"ppt")	/* PowerPoint */
	MappingLine( "pZIP",	"pZIP",	"zip")	
	MappingLine( "SCPL",	"AIFF",	"aif")	
	MappingLine( "SCPL",	"ULAW",	"au")	
	MappingLine( "SCPL",	"STrk",	"mod")	
	MappingLine( "SCPL",	"8SVX",	"svx")	
	MappingLine( "SCPL",	"WAVE",	"wav")	
	MappingLine( "SITx",	"SIT!",	"sit")	
	MappingLine( "SITx",	"SITD",	"sit")	
	MappingLine( "TAR ",	"TARF",	"tar")	
	MappingLine( "ttxt",	"PICT",	"pct")	
	MappingLine( "ttxt",	"PICT",	"pic")	
	MappingLine( "TVOD",	"MooV",	"qt")	
	MappingLine( "UPIT",	"PIT ",	"pit")	
	MappingLine( "WPC2",	".WP5",	"wp")		/* Word Perfect */
	MappingLine( "WPC2",	".WP5",	"wp5")	/* Word Perfect */
	
	MappingLine( "XCEL",	"XLS5",	"xls")	/* MS Excel */
	MappingLine( "XCEL",	"XLS4",	"xls")	/* MS Excel */
	MappingLine( "XCEL",	"XLC3",	"xlc")	/* MS Excel */
	MappingLine( "XCEL",	"XLM3",	"xlm")	/* MS Excel */
	MappingLine( "XCEL",	"XLW ",	"xlw")	/* MS Excel */
	MappingLine( "XCEL",	"TEXT",	"slk")	/* MS Excel */
	MappingLine( "XCEL",	"****",	"xls")	/* MS Excel */
	
	MappingLine( "XPRS",	"XDOC",	"qxd")	/* Quark */
	
	
	/* general wildcards should be *last* in the list so they are found last */
	MappingLine( "****",	"TEXT",	"txt")	
	MappingLine( "****",	"pZIP",	"zip")	
	MappingLine( "****",	"ZIVU",	"zip")	
	MappingLine( "****",	"Midi",	"mdi")	
	MappingLine( "****",	"PAK ",	"pak")	
	MappingLine( "****",	"mArc",	"arc")	
	MappingLine( "****",	"ZOO ",	"zoo")	
	MappingLine( "****",	"GIFf",	"gif")	
	MappingLine( "****",	"JPEG",	"jpg")	
	MappingLine( "****",	"TIFF",	"tif")	
	MappingLine( "****",	"EPSF",	"eps")	

};


#if PGP_MACINTOSH	/* [ */



/*____________________________________________________________________________
	DataMappingEntry is used to look at actual file data at the front of a file
	being decrypted/verified and to assign it a Mac creator/type.
	
	It is generally consulted last after looking at the file extension because
	the file data could match just by chance, thus possibly giving a file a
	type which is inappropriate.
____________________________________________________________________________*/
typedef struct DataMappingEntry
{
	const OSType			creator;
	const OSType			type;
	const int				length;
	PGPByte const *	const	ident;
} DataMappingEntry;

static const DataMappingEntry  sDataMappingEntries[] =
{
#define DmEntry( cr, ty, len, id ) \
	{ cr, ty, len, (PGPByte const *)(id) }
	
	DmEntry('pZIP', 'pZIP',  2, "PK" ),
	DmEntry('LZIV', 'ZIVU',  3, "\x1F\x9D\x90" ),
	DmEntry('MIDI', 'Midi',  4, "MThd" ),
	DmEntry('PAK ', 'PAK ',  2, "\x1A\x0A" ),
	DmEntry('BOOZ', 'ZOO ',  4, "ZOO " ),
	DmEntry('JVWR', 'GIFf',  4, "GIF8" ),
	DmEntry('SIT!', 'SIT!',  4, "SIT!" ),
	DmEntry('CPCT', 'PACT',  2, "\x01\x01" ),
	DmEntry('arc*', 'mArc',  2, "\x1A\x08" ),
	DmEntry('arc*', 'mArc',  2, "\x1A\x09" ),
	DmEntry('JVWR', 'JPEG', 10, "\xFF\xD8\xFF\xE0\0\x10JFIF" ),
	DmEntry(0, 0, 0, NULL )
};



#endif	/* ] PGP_MACINTOSH */






/* convert 4 bytes into a PGPUInt32 assuming the first char is the
	high-order byte */
	static OSType
BytesToOSType( char const * str )
{
	PGPUInt32		type;
	unsigned char const *	cur	= (unsigned char const *)str;
	
	/* first byte becomes high-order byte */
	type	= cur[ 0 ];		type	<<= 8;
	type	|= cur[ 1 ];	type	<<= 8;
	type	|= cur[ 2 ];	type	<<= 8;
	type	|= cur[ 3 ];
	
	return( type );
}



	static CreatorTypeToExtensionEntry const *
sMatchMacBinaryCreatorAndType(
	OSType	creator,
	OSType	type )
{
	PGPUInt32	index;

	for( index = 0; index < sMappingTable->numEntries; ++index )
	{
		CreatorTypeToExtensionEntry const *	entry;
		
		entry	= &sMappingTable->entries[ index ];
		
		if ( entry->creator == creator ||
			entry->creator == kWildCard )
		{
			if ( entry->type == type ||
				entry->type == kWildCard )
			{
				return( entry );
			}
		}
	}
	
	return( NULL );
}



/*____________________________________________________________________________
	map a creator/type pair to a file name extension
	
	extension is of the form "doc", "bin", etc + null char.
	The period is *not* included
____________________________________________________________________________*/
	PGPError
pgpMapMacCreatorTypeToExtension(
	OSType	creator,
	OSType	type,
	char	extension[ 3 + 1 ]
	)
{
	PGPError	err	= kPGPError_NoErr;
	CreatorTypeToExtensionEntry const *	entry	= NULL;
	
	PGPValidatePtr( sMappingTable );
	
	extension[ 0 ]	= '\0';

	entry	= sMatchMacBinaryCreatorAndType( creator, type );
	if ( IsntNull( entry ) )
	{
		strcpy( extension, entry->extension );
		pgpAssert( extension[ 0 ] != '.' );
		err	= kPGPError_NoErr;
	}
	else
	{
		static const char	kDefaultExtension[] = "bin";
		
		strcpy( extension, kDefaultExtension );
		err	= kPGPError_NoMacBinaryTranslationAvailable;
	}
	
	return( err );
}



/*____________________________________________________________________________
	Return TRUE if the extension (without the ".") is a suitable file name
	extension for the specified creator and type.
	
	Only ambiguous types need be added here; TEXT is the best example.
	Other mac creator/type pairs need not be added since they generally only
	map to one possible extension.
____________________________________________________________________________*/
	PGPBoolean
pgpIsValidExtensionForMacType(
	OSType			creator,
	OSType			type,
	char const *	extension )
{
	PGPBoolean		isSuitable	= FALSE;
	char const **	extensionList	= NULL;
	PGPUInt32		numExtensions	= 0;
	
	(void)creator;
#define kFileTypeText	((OSType)0x54455854)	/* 'TEXT' */
	if ( type == kFileTypeText )
	{
		/* array of pointers to strings */
		static const char *	sSuitableExtensions[] =
		{
			"c", "h", "cpp", "cp", "cc", "cxx", "asm", "txt",
			"slk", "tex", "tx8", "rtf", "htm", "asc", "bas", "bat",
			"cmd", "csv", "dif", "faq", "hqx", "ini",
			"mak", "me", "p", "pas", "prn",
		};
		numExtensions	= sizeof( sSuitableExtensions ) /
						sizeof( sSuitableExtensions[ 0 ] );
		extensionList	= sSuitableExtensions;
	}
	
	if ( numExtensions != 0 )
	{
		PGPUInt32	index;
		
		pgpAssert( IsntNull( extensionList ) );
		for( index = 0; index < numExtensions; ++index )
		{
			if ( pgpCompareStringsIgnoreCase( extensionList[ index ],
					extension ) == 0 )
			{
				isSuitable	= TRUE;
				break;
			}
		}
	}
	
	return( isSuitable );
}







#if PGP_MACINTOSH	/* [ */


	
#include "MacStrings.h"
#include "MacFiles.h"



	PGPError
pgpMapFileDataToMacCreatorType(
	const void *	dataIn,
	OSType *		creator,
	OSType *		type )
{
	PGPError				err	= kPGPError_ItemNotFound;
	PGPByte const *			data	= (PGPByte const *)dataIn;
	DataMappingEntry const *	rec;
	
	rec	= &sDataMappingEntries[ 0 ];
	while ( IsntNull( rec->ident ) )
	{
		if ( rec->ident[ 0 ] == data[ 0 ] &&
			pgpMemoryEqual( data, rec->ident, rec->length))
		{
			*creator	= rec->creator;
			*type		= rec->type;
			err	= noErr;
			break;
		}
		++rec;
	}
		
	return( err );
}



/*____________________________________________________________________________
	Map a filename (generally just the extension) to a mac creator and type.
	
	Two tables are consulted here.
	
	The first, 'sSuffixMap', contains mapping from a suffix to a creator/type.
	This table allows us to give a preferred mapping from a suffix to a
	mac creator type. If we find an entry in this table, then we'll use it.
	The mapping in this table is one-to-one; a single suffix can map to
	only one creator/type pair.
	
	The second table is the one used on non-Mac systems to map the
	creator/type code to an extension.
	Since multiple creator/type pairs can map to the same suffix, and
	since the entries in this table will be searched in order, the first
	matching suffix found will match.  

	A third fact is important: mappings in the second table may be
	wildcarded.  Without a mapping in the sSuffixMap, we wouldn't be able
	to generate an appropriate creator/type pair.
____________________________________________________________________________*/
	PGPError
pgpMapFileNameToMacCreatorType(
	const unsigned char *	name,
	OSType *				creator,
	OSType *				type )
{
	PGPError		err	= kPGPError_ItemNotFound;
	PGPUInt32		index;
	char			cName[ 256 ];
	const char *	extension;
	
	/* not really a param, but does desired thing */
	PGPValidatePtr( sMappingTable );
	
	PToCString( name, cName );
	extension	= PGPGetFileNameExtension( cName );
	
	for( index = 0; index < sMappingTable->numEntries; ++index )
	{
		CreatorTypeToExtensionEntry const *	entry;
		
		entry	= &sMappingTable->entries[ index ];
		
		if ( pgpCompareStringsIgnoreCase( entry->extension,
				extension ) == 0 )
		{
			/* extension matches.  If the type and creator are
			not wild cards, the use it */
			if (	entry->creator != kWildCard &&
					entry->type != kWildCard )
			{
				err			= noErr;
				*creator	= entry->creator;
				*type		= entry->type;
				break;
			}
		}
	}
	
	return( err );
}


	PGPBoolean
pgpOKToEncodeFSSpecWithoutMacBinary( const FSSpec *spec )
{
	OSStatus	err;
	CInfoPBRec	cpb;
	PGPBoolean	encodeAsMacBinary	= TRUE;
	
	pgpAssert( IsntNull( sMappingTable ) );
	if ( IsNull( sMappingTable ) )
		return( FALSE );
	
	err	= FSpGetCatInfo( spec, &cpb );
	pgpAssertNoErr( err );
	if ( IsntPGPError( err ) )
	{
		CreatorTypeToExtensionEntry const *	entry;
		
		entry	= sMatchMacBinaryCreatorAndType( cpbFileCreator( &cpb ),
				cpbFileType( &cpb ) );
				
		if ( IsntNull( entry ) )
		{
			encodeAsMacBinary	= FALSE;
		}
		else
		{
			#define kMaxResForkSize		400
			/* programatically determine */
			/* should we allow files without resource forks? */
			if ( cpbResForkSize( &cpb ) < kMaxResForkSize )
			{
				encodeAsMacBinary	= FALSE;
			}
		}
	}
	
	return( ! encodeAsMacBinary );
}


#endif	/* ] PGP_MACINTOSH */




#if PGP_MACINTOSH
const char	kMappingFileName[]	= "PGP MacBinary Mappings";
#else
const char	kMappingFileName[]	= "PGPMacBinaryMappings.txt";
#endif

	static PGPError
sGetMacBinaryFileSpec(
	PFLContextRef		context,
	PFLFileSpecRef *	spec )
{
	PGPError	err	= kPGPError_NoErr;
	
	err	= pgpGetPrefsSpec( context, spec );
	if ( IsntPGPError( err ) )
	{
		err	= PFLSetFileSpecName( *spec, kMappingFileName );
		if ( IsPGPError( err ) )
		{
			PFLFreeFileSpec( *spec );
			*spec	= NULL;
		}
	}
	
	return( err );
}



#define IsEndOfLine( c ) 		((c) == '\r' || (c) == '\n' )

	static const char *
sSkipWhite(
	const char *	cur,
	const char *	last )
{
	while ( cur <= last )
	{
		if ( isspace( *cur ) )
			++cur;
		else
			break;
	}
	return( cur );
}

	static const char *
sSkipToNextLine(
	const char *	cur,
	const char *	last )
{
	while ( cur <= last )
	{
		if ( ! IsEndOfLine( *cur ) )
			++cur;
		else
			break;
	}
	
	while ( cur <= last )
	{
		if ( IsEndOfLine( *cur ) )
			++cur;
		else
			break;
	}
	
	return( cur );
}


	static PGPUInt32
sGetLineLength(
	const char *	cur,
	const char *	last )
{
	PGPUInt32	length	= 0;
	
	while ( cur <= last )
	{
		if ( IsEndOfLine( *cur ) )
			break;
		++length;
		++cur;
	}
	return( length );
}



	


	static const char *
sParseOSType(
	const char *	cur,
	const char *	last,
	OSType *		osType,
	PGPBoolean *	valid )
{
	PGPUInt32	lineLength;
	PGPByte		buffer[ 4 ];
	PGPUInt32	result;
	
	*valid	= FALSE;
	
	lineLength	= sGetLineLength( cur, last );
	
	/* does it start with "0x"? */
	if ( ( *cur == '0' && *(cur + 1) == 'x' )  )
	{
		cur	+= 2;
		if ( lineLength >= 8 )
		{
			pgpHexToBytes( cur, 4, buffer );
			cur	+= 8;
			*valid	= TRUE;
		}
	}
	else if ( lineLength >= 4 )
	{
		buffer[ 0 ]	= *cur++;
		buffer[ 1 ]	= *cur++;
		buffer[ 2 ]	= *cur++;
		buffer[ 3 ]	= *cur++;

		*valid	= TRUE;
	}
	
	result	= buffer[ 0 ];
	result	= ( result << 8 ) | buffer[ 1 ];
	result	= ( result << 8 ) | buffer[ 2 ];
	result	= ( result << 8 ) | buffer[ 3 ];
	
	*osType	= result;
		
	return( cur );
}


	static const char *
sParseExtension(
	const char *	cur,
	const char *	last,
	char			extension[ 3 + 1 ],
	PGPBoolean *	valid )
{
	PGPUInt32	length	= 0;
	
	*valid	= FALSE;
	
	pgpFillMemory( extension, 3 + 1, '\0' );
	while ( cur <= last )
	{
		if ( isspace( *cur ) )
			break;
		
		extension[ length ]	= *cur++;
		++length;
		*valid	= TRUE;
		if ( length == 3 )
			break;
	}
	extension[ length ]	= '\0';
	
	if ( cur <= last &&
		! (isspace( *cur ) || IsEndOfLine( *cur ) ) )
	{
		pgpDebugMsg( "malformed line" );
		*valid	= FALSE;
	}
		
	return( cur );
}


/*____________________________________________________________________________
	Return TRUE if a valid line was found
____________________________________________________________________________*/
	static const char *
sParseLine(
	const char *					cur,
	const char *					last,
	CreatorTypeToExtensionEntry *	entry,
	PGPBoolean *					foundEntry )
{
	*foundEntry	= FALSE;
	
	pgpClearMemory( entry, sizeof( *entry ) );
	#define kCommentChar		'#'
	
	/* skip comment and blank lines */
	if ( *cur == kCommentChar || IsEndOfLine( *cur ))
	{
		cur	= sSkipToNextLine( cur, last );
	}
	else
	{
		/* potentially a valid line */
		PGPUInt32	lineLength;
		
		lineLength	= sGetLineLength( cur, last );
		
		/* CCCCTTTTExx with no space */
		/* or hex variants of such of form 0xHHHHHHHH */
		#define kMinLineLength (4 + 4 + 3)
		if ( lineLength >= kMinLineLength )
		{
			PGPBoolean		valid;
			
			cur	= sParseOSType( cur, last, &entry->creator, &valid );
			cur	= sSkipWhite( cur, last );
			
			if ( valid )
			{
				cur	= sParseOSType( cur, last, &entry->type, &valid );
				cur	= sSkipWhite( cur, last );

				if ( valid )
				{
					cur	= sParseExtension( cur,
							last, entry->extension, &valid );
				}
			}
			
			pgpDebugMsgIf( ! valid, "malformed line" );
			*foundEntry	= valid;
		}
		else
		{
			pgpDebugMsgIf( lineLength != 0, "malformed line" );
		}
		
		cur	= sSkipToNextLine( cur, last );
	}
	
	return( cur );
}


	static PGPError
sAddEntryToMappingTable(
	CreatorTypeToExtensionEntry const *	entry)
{
	PGPError	err	= kPGPError_NoErr;
	CreatorTypeToExtensionTable *		table;
	
	table	= sMappingTable;
	
	if ( table->numEntries + 1 > table->numAllocated )
	{
		PGPUInt32	newNumAllocated;
		PGPSize		newSize;
		
		/* allocate them 10 at a time to save space */
		newNumAllocated	= table->numAllocated + 10;
		
		newSize	= sizeof( CreatorTypeToExtensionTable ) +
					newNumAllocated * sizeof( table->entries[ 0 ] );
		err	= pgpRealloc( &table, newSize );
		if ( IsntPGPError( err ) )
		{
			table->numAllocated	= newNumAllocated;
		}
	}
	
	if ( IsntPGPError( err ) )
	{
		pgpAssert( table->numAllocated > table->numEntries );
		table->entries[ table->numEntries ]	= *entry;
		table->numEntries++;
	}
	
	sMappingTable	= table;
	
	return( err );
}



	static PGPError
sParseMappings(
	void const *	memIn,
	PGPSize			length )
{
	PGPError		err	= kPGPError_NoErr;
	
	pgpAssert( IsntNull( memIn ) );
	pgpAssert( IsNull( sMappingTable ) );
	
	/* special case: use pgpAlloc() since we don't have a user context */
	sMappingTable	= (CreatorTypeToExtensionTable *)
						pgpAlloc( sizeof( *sMappingTable ) );
	if ( IsntNull( sMappingTable ) )
	{
		CreatorTypeToExtensionEntry	entry;
		const char *	cur;
		const char *	last;
		
		pgpClearMemory( sMappingTable, sizeof( *sMappingTable) );
		sMappingTable->numEntries	= 0;
		sMappingTable->numAllocated	= 0;

		cur		= (char const *)memIn;
		last	= cur + (length -1);
		
		while ( cur <= last )
		{
			PGPBoolean	foundEntry;
			
			cur	= sParseLine( cur, last, &entry, &foundEntry );
			if ( foundEntry )
			{
				err	= sAddEntryToMappingTable( &entry );
				if ( IsPGPError( err ) )
					break;
			}
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


	static PGPError
sReadMappingsFromIO( PGPIORef	io)
{
	PGPError		err	= kPGPError_NoErr;
	PGPFileOffset	eof;
	
	err	= PGPIOGetEOF( io, &eof );
	if ( IsntPGPError( err ) )
	{
		void *			mem;
		PFLContextRef	context;
		PGPSize			bufferSize;
		
		bufferSize	= (PGPSize)eof;
		context	= PGPIOGetContext( io );
		mem		= PFLContextMemAlloc( context, bufferSize, 0 );
		if ( IsntNull( mem ) )
		{
			err	= PGPIORead( io, bufferSize, mem, NULL );
			if ( IsntPGPError( err ) )
			{
				err	= sParseMappings( mem, bufferSize );
			}
			PFLContextMemFree( context, mem );
		}
		else
		{
			err	= kPGPError_OutOfMemory;
		}
	}
	
	return( err );
}

	static PGPError
pgpReadMacBinaryMappings( PFLFileSpecRef spec )
{
	PGPError	err	= kPGPError_NoErr;
	PGPIORef	io;
	
	err	= PGPOpenFileSpec( spec,
		kPFLFileOpenFlags_ReadOnly, (PGPFileIORef *)&io );
	if ( IsntPGPError( err ) )
	{
		err	= sReadMappingsFromIO( io );
		PGPFreeIO( io );
		io	= NULL;
	}
	
	return( err );
}


	
	static PGPError
sWriteBuiltInMappingsToIO( PGPIORef	io )
{
	PGPError	err	= kPGPError_NoErr;
	
	err	= PGPIOWrite( io,
			strlen( sCreatorTypeToExtensionDefaults ),
			sCreatorTypeToExtensionDefaults );
	
	return( err );
}


/*____________________________________________________________________________
	Initialize the mappings from the built-in table.  Generally called only if
	(a) the mapping file can't be found and
	(b) we couldn't create a mapping file.
	
	Here we use a trick: first write them into a memory io, then init them
	from this memory IO.
____________________________________________________________________________*/
	static PGPError
sInitFromBuiltIn( PFLContextRef	context)
{
	PGPError	err	= kPGPError_NoErr;
	PGPIORef	io;
	
	err	= PGPNewMemoryIO( context, (PGPMemoryIORef *)&io );
	if ( IsntPGPError( err ) )
	{
		err	= sWriteBuiltInMappingsToIO( io );
		if ( IsntPGPError( err ) )
		{
			err	= PGPIOSetPos( io, 0 );
			if ( IsntPGPError( err ) )
				err	= sReadMappingsFromIO( io );
		}
		
		PGPFreeIO( io );
	}
	
	return( err );
}




	static PGPError
sWriteBuiltInMappingsToFile( PFLFileSpecRef	spec )
{
	PGPError		err	= kPGPError_NoErr;
	PGPIORef		io	= NULL;
	
	err	= PFLFileSpecCreate( spec );
	if ( IsntPGPError( err ) )
	{
		err	= PGPOpenFileSpec( spec,
			kPFLFileOpenFlags_ReadWrite, (PGPFileIORef *)&io );
		if ( IsntPGPError( err ) )
		{
			err	= sWriteBuiltInMappingsToIO( io );
			PGPFreeIO( io );
			io	= NULL;
		}
	}
	
	return( err );
}


#if PGP_MACINTOSH
	static PGPError
sAddFileInfoToSpec( PFLFileSpecRef spec )
{
	PFLFileSpecMacMetaInfo	metaInfo;
	PGPError				err	= kPGPError_NoErr;
	
	metaInfo.fInfo.fdCreator	= 'ttxt';
	metaInfo.fInfo.fdType		= 'TEXT';
	metaInfo.fInfo.fdFlags		= 0;
	metaInfo.fInfo.fdLocation.h	= -1;
	metaInfo.fInfo.fdLocation.v	= -1;
	
	err	= PFLSetFileSpecMetaInfo( spec, &metaInfo );
	return( err );
}
#else
#define sAddFileInfoToSpec( spec )	kPGPError_NoErr
#endif

	static PGPError
pgpWriteMacBinaryMappings( PFLFileSpecRef toFile )
{
	PGPError		err	= kPGPError_NoErr;
	
	err	= sAddFileInfoToSpec( toFile );
	if ( IsntPGPError( err ) )
	{
		err	= sWriteBuiltInMappingsToFile( toFile );
	}

	return( err );
}


	PGPError
pgpInitMacBinaryMappings( void )
{
	PGPError		err			= kPGPError_NoErr;
	PFLFileSpecRef	spec		= NULL;
	PFLContextRef	pflContext	= NULL;
	
	err	= PFLNewContext( &pflContext );
	if ( IsntPGPError( err ) )
	{
		err	= sGetMacBinaryFileSpec( pflContext, &spec );
		if ( IsntPGPError( err ) )
		{
			PGPBoolean		exists;
			
			err	= PFLFileSpecExists( spec, &exists );
			/* if file doesn't exist, then create it */
			if ( IsntPGPError( err ) )
			{
				if ( ! exists )
				{
					err	= pgpWriteMacBinaryMappings( spec );
				}
				if ( IsntPGPError( err ) )
				{
					err	= pgpReadMacBinaryMappings( spec );
				}
				pgpAssertNoErr( err );
			}
				
			/* "soft" failure: init from built-in if we have to */
			if ( IsPGPError( err ) )
			{
				err	= sInitFromBuiltIn( pflContext );
				pgpAssertNoErr( err );
			}
			
			PFLFreeFileSpec( spec );
			spec	= NULL;
		}
		
		PFLFreeContext( pflContext );
	}
	
	pgpAssertNoErr( err );
	return( err );
}


	PGPError
pgpDisposeMacBinaryMappings( void )
{
	PGPError		err	= kPGPError_NoErr;
	
	if ( IsntNull( sMappingTable ) )
	{
		pgpFree( sMappingTable );
		sMappingTable	= NULL;
	}
	
	return( err );
}






















/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
