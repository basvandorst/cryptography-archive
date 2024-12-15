/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	A test program is also available in libs/pgpcdk/win32/test.  If you build 
	and run this, it puts up a command prompt.
	
	Mac project to run this is at: pgpcdk/mac/lib/test/Test.prj
	
	Suitable command lines include:

	-l
		Print out all the keys in the key ring (by userid).

	-e <userid> <infile> <outfile>

		Encrypt using the key for <userid> from <infile> to <outfile>.
		Can have multiple "-e <userid>" switches to encrypt to multiple
		recipients.

	-s <userid> -z <pphrase> <infile> <outfile>

		Sign using thekey for <userid> and the specified <pphrase> (which
		must not have any spaces).  If you leave off the -z <pphrase> it
		will prompt you for the passphrase when the callback event occurs.

	-e <userid> -s <userid> <infile> <outfile>

		Encrypt and sign.

	-d <infile> <outfile>

		Decrypt or verify the <infile>, output to <outfile>.  Can include
		"-z <pphrase>" to specify decryption passphrase (no spaces allowed)
		or else it will prompt you for one.

	-v <sigfile> <datafile>

		Verify detached signature from the <sigfile> against data in the
		<datafile>.

	-c <pphrase> <infile> <outfile>

		Conventionally encrypt <infile> to <outfile> using the specified
		<pphrase>, which must not have any spaces.


	Some optional switches, which should go before <infile>:

	-a
		Ascii armor output

	-t
		Treat input file as text

	-m
		Use memory-buffer versions of functions rather than file versions

	-b
		Output detached signature (signing only)

	-u
		Warn on untrusted keys
	
	-i	<force | auto [nonvolatileonly]>		Encode
	-i	[nonvolatileonly | nocrcok]				Decode
		extended file info; e.g. MacBinary for Mac. Suitable
		for reconstructing a file exactly the same as the original.


	$Id: pgpTestEncode.c,v 1.42 1997/10/08 02:27:27 mhw Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>


#if PGP_MACINTOSH
#include <stat.h>
#else
#include <sys/stat.h>
#endif

#if PGP_INTERNAL_TESTING
#include "pgpLeaks.h"
#endif

#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpKeys.h"
#include "pgpUtilities.h"

#include "pgpTest.h"


#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif


#ifndef IsNull
#define IsntNull( p )	( (p) != NULL )
#define IsNull( p )		( (p) == NULL )
#endif



#include <string.h>

static const char *pgpoptarg;
static int pgpoptind;

static int
pgpgetopt (int argc, char const *argv[], char const *opts)
{
	int c;
	const char *s;

	(void)argc;
	
	if (!pgpoptind)
		pgpoptind = 1;

	if (!argv[pgpoptind] || argv[pgpoptind][0] != '-') {
		return -1;
	}
	c = argv[pgpoptind][1];
	if (!(s = strchr (opts, c))) {
		pgpoptind += 1;
		return '?';
	}
	if (s[1] == ':') {
		pgpoptarg = argv[++pgpoptind];
	} else
		pgpoptarg = 0;
	pgpoptind += 1;
	return c;
}

static PGPByte *
fileread (char const *filename, size_t *bufsize)
{
	PGPByte *buf;
	struct stat statbuf;
	FILE *f;

	if (stat (filename, &statbuf) < 0) {
		perror ("stat");
		exit (2);
	}
	*bufsize = statbuf.st_size;
	buf = (PGPByte *)malloc (statbuf.st_size);
	if (!(f = fopen (filename, "rb"))) {
		perror ("fopen");
		exit (2);
	}
	if (fread (buf, 1, statbuf.st_size, f) != (size_t)statbuf.st_size) {
		perror ("fread");
		exit (2);
	}
	fclose (f);
	return buf;
}

static void
filewrite (char const *filename, PGPByte *buf, size_t bufsize)
{
	FILE *f;

	if (!(f = fopen (filename, "wb"))) {
		perror ("fopen");
		exit (3);
	}
	if (fwrite (buf, 1, bufsize, f) != bufsize) {
		perror ("fwrite");
		exit (3);
	}
	fclose (f);
}


static void
usage (const char *pname)
{
	fprintf (stderr,
"Usage: %s [options] infile outfile\n"
" * Options:\n"
" * -d				Decrypt\n"
" * -s <username>	Sign it specified user name\n"
" * -b				Break off as separate signature\n"
" * -a				Ascii armor the output\n"
" * -t				Treat as text mode\n"
" * -c <passphrase>	Use pass phrase for encrypting/decrypting\n"
" * -u				Warn on untrusted keys\n"
" * -z <passphrase>	Key passphrase for signing/decrypting\n"
" * -e <username>	Encrypt to username (may have multiple -e switches)\n"
" * -m				Use memory buffer functions rather than file functions\n"
" * -v				Do verify, infile is sig, outfile is text (no output)\n"
" * -l				List keyring contents.  Filenames not used.\n"
" *\n"
" * Following are not implemented\n"
" * -y				AnalYze to see what the file is.  Output file not used.\n"
" * -k               Add keys from infile into outfile\n"
" * -x               Extract key (\"infile\" param) to outfile\n"
" \n", pname);
}

static void
userr (const char *pname)
{
	usage (pname);
	exit (1);
}



/********************* Callback Function Support ************************/

struct codename {
	PGPUInt32	code;
	const char	*name;
};

#define EV(x)	{ (PGPUInt32)kPGPEvent_##x##Event, #x " event" }

struct codename eventnames[] =
{
	EV(Null),
	EV(Initial),
	EV(Final),
	EV(Error),
	EV(Warning),
	EV(Entropy),
	EV(Passphrase),
	EV(InsertKey),
	EV(Analyze),
	EV(Recipients),
	EV(KeyFound),
	EV(Output),
	EV(Signature),
	EV(BeginLex),
	EV(EndLex),
	EV(Recursion),
	EV(DetachedSignature)
};

#undef EV

static char const *
eventName( PGPEventType evt )
{
	unsigned i;
	for (i=0; i<sizeof(eventnames)/sizeof(eventnames[0]); ++i) {
		if (eventnames[i].code == (PGPUInt32)evt) {
			return eventnames[i].name;
		}
	}
	return "Unknown event";
}

#define EV(x)	{ (PGPUInt32)kPGPAnalyze_##x, #x }

struct codename analyzenames[] =
{
	EV(Encrypted),
	EV(Signed),
	EV(DetachedSignature),
	EV(Key),
	EV(Unknown)
};

static char const *
analyzeName( PGPAnalyzeType anl )
{
	unsigned i;
	for (i=0; i<sizeof(analyzenames)/sizeof(analyzenames[0]); ++i) {
		if (analyzenames[i].code == (PGPUInt32)anl) {
			return analyzenames[i].name;
		}
	}
	return "Unknown analyze code";
}


static void
printKeys( PGPKeySetRef keyset, const char *prefix, FILE *fout )
{
	PGPKeyListRef	klist;
	PGPKeyIterRef	kiter;
	PGPKeyRef		key;

	PGPOrderKeySet( keyset, kPGPAnyOrdering, &klist );
	PGPNewKeyIter( klist, &kiter );
	while( IsntPGPError( PGPKeyIterNext( kiter, &key ) ) ) {
		char name[256];
		PGPSize namelen = sizeof(name);
		PGPGetPrimaryUserIDNameBuffer( key, sizeof( name ), name, &namelen );
		fprintf(fout, "%s%s\n", prefix, name);
	}
	PGPFreeKeyIter( kiter );
	PGPFreeKeyList( klist );
}

typedef struct MyState {
	PGPFileSpecRef	 fileRef;
	char const		*fileName;
	PGPByte			*dataBuf;
	PGPLocalEncodingFlags fLocalEncode;
	PGPSize			 dataBufSize;
	PGPUInt32		 sectionCount;
} MyState;


/* Generic event handler */
	static PGPError
myEvents(
	PGPContextRef context,
	PGPEvent *event,
	PGPUserValue userValue
	)
{
	static PGPByte buf[256];
	size_t len;
	MyState	*s;

	(void) context;

	s = (MyState *)userValue;
	printf ("%s: (callback code %d)\n", eventName(event->type), event->type);
	switch( event->type ) {
	case kPGPEvent_NullEvent:
		{	PGPEventNullData *d = &event->data.nullData;
			printf (" written %ld / %ld bytes\n", (long)d->bytesWritten,
					(long)d->bytesTotal);
		}
		break;
	case kPGPEvent_ErrorEvent:
		{
			PGPEventErrorData const *	d = &event->data.errorData;
			pgpTestAssertNoErr( d->error );
		}
		break;
	case kPGPEvent_WarningEvent:
		{	PGPEventWarningData *d = &event->data.warningData;
			printf (" warning code %d\n", d->warning);
		}
		break;
	case kPGPEvent_PassphraseEvent:
		{
			PGPEventPassphraseData *d = &event->data.passphraseData;
			if( d->fConventional ) {
				printf ("Enter conventional-decryption pass phrase: ");
			} else {
				PGPUInt32	numKeys;
				
				PGPCountKeys(d->keyset, &numKeys );
				if (numKeys == 0) {
					/* Can happen if don't have RSA support */
					printf (" NO KEYS CAN DECRYPT MESSAGE, SKIPPING\n");
					return kPGPError_NoErr;
				}
				printf (" decryption key%s:\n",
						numKeys >1?"s":"");
				printKeys( d->keyset, "  ", stdout );
				printf (" enter passphrase: ");
			}
			fgets( (char *)buf, sizeof(buf), stdin);
			len = strlen( (char *)buf) - 1;
			/* Test abort using zero length passphrase */
			if( len == 0 )
				return kPGPError_NoErr;
			PGPAddJobOptions( event->job, 
							PGPOPassphraseBuffer( context, buf, len),
							PGPOLastOption( context ) );
		}
		break;
	case kPGPEvent_SignatureEvent:
		{	PGPEventSignatureData *d = &event->data.signatureData;
			printf (" signature data:");
			if( IsNull( d->signingKey ) ) {
				printf (" (unknown signer)");
			} else {
				char	name[256];
				PGPSize	namelen = sizeof(name);
				PGPGetPrimaryUserIDNameBuffer( d->signingKey,
					sizeof( name ), name, &namelen );
				printf (" by %s", name);
				if ( ! d->verified )
					printf( "\n!!!!!  WARNING: did not verify  !!!!!!!" );
				printf ("\n %schecked, %sverified, %skeyDisabled, "
						"%skeyRevoked, %skeyExpired, "
						"%skeyMeetsValidityThreshold, validity=%d",
						(d->checked?"":"!"),(d->verified?"":"!"),
						(d->keyDisabled?"":"!"), (d->keyRevoked?"":"!"),
						(d->keyExpired?"":"!"),
						(d->keyMeetsValidityThreshold?"":"!"),
						d->keyValidity);
			}
			printf (", created: %s", ctime((time_t *)&d->creationTime));
		}
		break;
	case kPGPEvent_RecipientsEvent:
		{
			PGPEventRecipientsData *d = &event->data.recipientsData;
			PGPUInt32	numKeys;
			
			if( d->keyCount > 0 ) {
				PGPUInt32 i;
				printf (" encrypted to %d key%s total:\n", d->keyCount,
											(d->keyCount>1?"s":" "));
				for (i=0; i<d->keyCount; ++i) {
					PGPKeyID	keyid = d->keyIDArray[i];
					char		keyidstr[ kPGPMaxKeyIDStringSize ];

					PGPGetKeyIDString( &keyid, kPGPKeyIDString_Abbreviated,
									   keyidstr );
					printf ("  %s\n", keyidstr );
				}
			}

			PGPCountKeys(d->recipientSet, &numKeys );
			if( numKeys > 0 ) {
				printf (" encrypted to keys which we have:\n");
				printKeys( d->recipientSet, "  ", stdout );
			}
			if( d->conventionalPassphraseCount > 0 ) {
				printf (" encrypted with %d conventional passphrase%s\n",
					 d->conventionalPassphraseCount,
					 (d->conventionalPassphraseCount > 1 ? "s" : ""));
			}
		}
		break;
	case kPGPEvent_AnalyzeEvent:
		{	PGPEventAnalyzeData *d = &event->data.analyzeData;
			static int skipper = 0;
			printf (" %s (analyze code %d)\n",
					analyzeName(d->sectionType), d->sectionType);
			if( skipper ) {
				printf (" (Returning SkipSection error)\n");
				return kPGPError_SkipSection;
			}
		}
		break;
	case kPGPEvent_DetachedSignatureEvent:
		if( IsntNull( s->dataBuf ) ) {
			printf(" (using memory buffer)\n");
			PGPAddJobOptions( event->job,
				  PGPODetachedSig( context,
				  		PGPOInputBuffer( context, s->dataBuf, s->dataBufSize ),
						PGPOLocalEncoding( context, s->fLocalEncode ),
						PGPOLastOption ( context ) ),
				  PGPOLastOption ( context ) );
		} else {
			printf( " (using file %s)\n", s->fileName );
			PGPAddJobOptions( event->job,
				  PGPODetachedSig( context,
					  PGPOInputFile( context, s->fileRef ),
					  PGPOLocalEncoding( context, s->fLocalEncode ),
					  PGPOLastOption ( context ) ),
				  PGPOLastOption ( context ) );
		}
		break;
	case kPGPEvent_OutputEvent:
		{	PGPEventOutputData *d = &event->data.outputData;
			printf (" type = %d, suggested name = %s, %sfyeo\n",
					d->messageType, d->suggestedName,
					d->forYourEyesOnly?"":"!");
#if 0
			if( IsntNull( s ) ) {
				char *outname;
				PGPFileSpecRef outref;

				outname = pgpMemAlloc( strlen(s->outFile)+5 );
				strcpy(outname, s->outFile);
				sprintf(outname+strlen(outname), "%d", s->count++);
				pgpNewFileRefFromFullPath( outname, &outref);
				PGPAddJobOptions( event->job,
								  PGPOOutputFile(outref),
								  PGPOLastOption( context ));
				printf(" setting output file as %s\n", outname);
				pgpFree( outname );
			}
#endif
		}
		break;
	case kPGPEvent_BeginLexEvent:
		{	PGPEventBeginLexData *d = &event->data.beginLexData;
			printf (" section number = %d\n", d->sectionNumber);
			s->sectionCount = d->sectionNumber;
		}
		break;
	case kPGPEvent_EndLexEvent:
		{	PGPEventEndLexData *d = &event->data.endLexData;
			printf (" section number = %d\n", d->sectionNumber);
		}
		break;
		
	default:
		break;
	}
				
	return kPGPError_NoErr;
}



#if PGP_MACINTOSH /* [ */

	static void
CToPString(
	const char	*cString,
	StringPtr	pString)
	{
	UInt32		length;
	UInt8		*curOut;
	
	pgpTestAssert( IsntNull( cString ) );
	pgpTestAssert( IsntNull( pString ) );
	
	length		= 0;
	curOut		= &pString[1];
	#define kMaxPascalStringLength	255
	while ( length < kMaxPascalStringLength )
		{
		if ( (curOut[length] = cString[length]) == 0)
			break;
		++length;
		}
	
	pString[0]	= length;
	}
	
	static OSStatus
GetSpecFromRefNum(
	short		refNum,
	FSSpec *	spec)
	{
	OSStatus	err	= noErr;
	FCBPBRec	pb;
	
	pb.ioFCBIndx	= 0;
	pb.ioRefNum		= refNum;
	pb.ioNamePtr	= spec->name;
	err	= PBGetFCBInfoSync( &pb );
	if ( IsntPGPError( err ) )
		{
		spec->vRefNum	= pb.ioFCBVRefNum;
		spec->parID		= pb.ioFCBParID;
		}
	
	return( err );
	}



/*____________________________________________________________________________
	Create a file ref from the specified path.
	
	For macintosh, the path is assumed to be just a file name in the local
	directory.
____________________________________________________________________________*/
	static PGPError
sGetFileRef(
	PGPContextRef		context,
	char const *		fileName,
	PGPFileSpecRef *	outRef )
{
	PGPError	err;
	FSSpec		spec;
	
	err	= GetSpecFromRefNum( CurResFile(), &spec );
	if ( IsntPGPError( err ) )
	{
		CToPString( fileName, spec.name ); 
		err	= PGPNewFileSpecFromFSSpec( context, &spec, outRef );
	}
	return( err );
}

#else	/* ] PGP_MACINTOSH [ */

	static PGPError
sGetFileRef(
	PGPContextRef		context,
	char const *		path,
	PGPFileSpecRef *	outRef )
{
	return( PGPNewFileSpecFromFullPath( context, path, outRef ) );
}
#endif	/* ] PGP_MACINTOSH */




/*
 * Options:
 * -d				Decrypt
 * -s <username>	Sign it specified user name
 * -b				Break off as separate signature
 * -a				Ascii armor the output
 * -t				Treat as text mode
 * -c <passphrase>	Use pass phrase for encrypting/decrypting
 * -u				Warn on untrusted keys
 * -z <passphrase>	Key passphrase for signing/decrypting
 * -e <username>	Encrypt to username (may have multiple -e switches)
 * -m				Use memory buffer functions rather than file functions
 * -v				Do verify, infile is sig, outfile is text (no output)
 * -l				List keyring contents.  Filenames not used.
 *
 * Following are not implemented
 * -y				AnalYze to see what the file is.  Output file not used.
 * -k               Add keys from infile into outfile
 * -x               Extract key ("infile" param) to outfile
 */
	static void
RunTest(
	PGPContextRef	context,
	int				argc,
	const char **	argv )
{
	char const *	infile	= NULL;
	char const *	outfile	= NULL;
	PGPByte fdecrypt=0, fkeypass=0,
		 fsign=0, fencrypt=0, fconv=0,
		 fmem=0, fverify=0, fanalyze=0, faddkey=0, fextkey=0;
	PGPLocalEncodingFlags	fLocalEncode	= kPGPLocalEncoding_None;
	char const*		skeypass=0;
	int				c;
	PGPByte *		inbuf;
	PGPByte *		outbuf;
	size_t			inbufsize, outbufsize;
	extern const char *	pgpoptarg;
	extern int			pgpoptind;
	PGPKeySetRef	keySet = NULL, encryptKeySet = NULL;
	PGPKeySetRef	keys = NULL;
	PGPKeyRef		signKey = NULL;
	PGPOptionListRef opts = NULL;
	PGPError		err = kPGPError_NoErr;
	PGPFileSpecRef	inref=NULL, outref=NULL;
	MyState			myState;

	/* Set up and initialize context */
	err = PGPOpenDefaultKeyRings( context,
		kPGPKeyRingOpenFlags_Mutable, &keys );
	if( IsPGPError( err ) )
		goto error;

#if PGP_INTERNAL_TESTING
	pgpLeaksBeginSession ("main");
#endif

	memset( &myState, 0, sizeof(myState) );
	PGPBuildOptionList( context, &opts,
				PGPOEventHandler(context, myEvents, &myState),
				PGPOLastOption( context ) );
	PGPNewKeySet( context, &encryptKeySet );

	pgpoptind = 0;
	while ((c = pgpgetopt(argc,
			argv, "daykxls:btc:uz:e:mvi:")) != EOF) {
		switch (c) {
		case 'd':	fdecrypt = TRUE;
					break;
		case 'a':	PGPAppendOptionList( context, opts,
						   PGPOArmorOutput(context, TRUE),
						   PGPOLastOption( context ) );
					break;
		case 'b':	PGPAppendOptionList( context, opts,
						   PGPODetachedSig( context,
						   	PGPOLastOption ( context ) ),
						   PGPOLastOption( context ) );
					break;
		case 't':	PGPAppendOptionList( context, opts,
						   PGPODataIsASCII(context, TRUE),
						   PGPOLastOption( context ) );
					break;
		case 'u':	PGPAppendOptionList( context, opts,
							   PGPOWarnBelowValidity( context,
							   kPGPValidity_Complete ),
							   PGPOLastOption ( context ) );
					break;
		case 'm':	fmem = TRUE;
					break;
		case 'c':	PGPAppendOptionList( context, opts,
						   PGPOConventionalEncrypt( context,
						   	PGPOPassphrase( context, pgpoptarg ),
							  PGPOLastOption ( context ) ),
						   PGPOLastOption( context ) );
					fconv = TRUE;
					break;
		case 'z':	fkeypass = TRUE;
					skeypass = pgpoptarg;
					break;
		case 's':
			{
					PGPUInt32		numKeys;
					PGPFilterRef	filter;
					
					err	= PGPNewUserIDStringFilter( context, pgpoptarg,
								kPGPMatchSubString, &filter );
					if ( IsntPGPError( err ) ) {
						err	= PGPFilterKeySet( keys, filter, &keySet);
						if ( IsPGPError( err ) )
							goto error;
							
						PGPFreeFilter( filter );
						filter	= NULL;
					}
					if ( IsPGPError( err ) )
						goto error;
					
					PGPCountKeys( keySet, &numKeys );
					if( numKeys < 1 ) {
						fprintf (stderr, "Unable to find a key for %s\n",
								 pgpoptarg);
					} else if( numKeys > 1 ) {
						fprintf (stderr, "Ambiguous key choice: %s\n",
								 pgpoptarg);
					} else {
						PGPKeyListRef	klist;
						PGPKeyIterRef	kiter;
						
						PGPOrderKeySet( keySet, kPGPAnyOrdering, &klist );
						PGPNewKeyIter( klist, &kiter );
						PGPKeyIterNext( kiter, &signKey );
						PGPFreeKeyIter( kiter );
						PGPFreeKeyList( klist );
						fsign = TRUE;
					}
					PGPFreeKeySet( keySet );
					break;
				}
		case 'e':
			{
					PGPUInt32		numKeys;
					PGPFilterRef	filter;
					
					err	= PGPNewUserIDStringFilter( context, pgpoptarg,
								kPGPMatchSubString, &filter );
					if ( IsntPGPError( err ) ) {
						err	= PGPFilterKeySet( keys, filter, &keySet);
						if ( IsPGPError( err ) )
							goto error;
							
						PGPFreeFilter( filter );
						filter	= NULL;
					}
					if ( IsPGPError( err ) )
						goto error;
						
					PGPCountKeys( keySet, &numKeys );
					
					if( numKeys < 1 ) {
						fprintf (stderr, "Unable to find a key for %s\n",
								 pgpoptarg);
					} else if( numKeys > 1 ) {
						fprintf (stderr, "Ambiguous key choice: %s\n",
								 pgpoptarg);
					} else {
						PGPAddKeys( keySet, encryptKeySet );
						fencrypt = TRUE;
					}
					PGPFreeKeySet( keySet );
					break;
			}
		case 'i':	/* local encode (MacBinary for Mac) */
					{
					fLocalEncode	= kPGPLocalEncoding_None;
					if ( strstr( pgpoptarg, "force" ) != NULL )
						fLocalEncode	|= kPGPLocalEncoding_Force;
					else if ( strstr( pgpoptarg, "auto" ) != NULL )
						fLocalEncode	|= kPGPLocalEncoding_Auto;
					else
						fLocalEncode	|= kPGPLocalEncoding_Auto;
						
					if ( strstr( pgpoptarg, "nocrcok" ) != NULL )
						fLocalEncode	|= kPGPLocalEncoding_NoMacBinCRCOkay;
						
					PGPAppendOptionList( context, opts,
						   PGPOLocalEncoding( context, fLocalEncode ),
						   PGPOLastOption( context ) );
					break;
					}
		case 'v':	fverify = TRUE;
					break;
		case 'y':	fanalyze = TRUE;
					break;
		case 'k':	faddkey = TRUE;
					break;
		case 'x':	fextkey = TRUE;
					break;
		case 'l':	printf ("Keyring contents:\n");
					printKeys (keys, "  ", stdout);
					printf ("\n");
					goto done;
					break;
		case ':':
		case '?':	userr ("");
		default:	userr ("");
		}
	}

	/* Check for some incompatible options and that we have something to do */
	if (fdecrypt + fencrypt + fconv + fverify + fanalyze
		+ faddkey + fextkey > 1)
		userr ("");
	if (fdecrypt + fencrypt + fconv + fverify + fsign 
		+ fanalyze + faddkey + fextkey < 1)
		userr ("");
	if (fsign && (fdecrypt || fverify || fanalyze || faddkey || fextkey))
		userr ("");

	/* Check that we have an input and an output file spec as needed */
	if (!(infile = argv[pgpoptind++]))
		userr ("");
	if( fmem ) {
		inbuf = fileread (infile, &inbufsize);
		PGPAppendOptionList( context, opts,
					  PGPOInputBuffer( context, inbuf, inbufsize ),
					  PGPOLastOption ( context ) );
	} else {
		if( IsPGPError( sGetFileRef( context, infile, &inref ) ) )
			goto error;
		PGPAppendOptionList( context, opts, PGPOInputFile( context, inref ),
								   PGPOLastOption ( context ) );
	}
	if (!fanalyze) {
		if (!(outfile = argv[pgpoptind++]))
			userr ("");
		if( fverify ) {
			/* Don't set up an output file, handle 2nd input below */
		} else if( fmem ) {
			PGPAppendOptionList( context, opts,
				PGPOAllocatedOutputBuffer( context,
					(void **)&outbuf, MAX_PGPUInt32, &outbufsize ),
				PGPOLastOption ( context ) );
		} else {
			if( IsPGPError( sGetFileRef( context, outfile, &outref ) ) )
				goto error;
			PGPAppendOptionList( context, opts,
							PGPOOutputFile( context, outref ),
							PGPOLastOption ( context ) );
		}
	}
	if ( pgpoptind < argc )
		userr ("");

	/* Handle signing option */
	if( fsign ) {
		if( fkeypass ) {
			PGPAppendOptionList( context, opts,
						 PGPOSignWithKey( context, signKey,
							 PGPOPassphrase( context, skeypass ),
							 PGPOLastOption ( context ) ),
						 PGPOLastOption ( context ) );
		} else {
			PGPAppendOptionList( context, opts,
						 PGPOSignWithKey( context, signKey,
						 	PGPOLastOption ( context ) ),
						 PGPOLastOption ( context ) );
		}
	}

	if (fencrypt || fconv || fsign) {
		if( fencrypt ) {
			PGPAppendOptionList( context, opts,
						  PGPOEncryptToKeySet( context, encryptKeySet ),
						  PGPOLastOption ( context ) );
		}
		if( IsPGPError( err = PGPEncode( context, opts,
				PGPOLastOption ( context ) ) ) )
			goto error;
		if( fmem )
			filewrite (outfile, outbuf, outbufsize);
	} else if (fdecrypt || fverify) {
		if (fverify) {
			/* infile is signature, outfile should hold data */
			if (fmem) {
				myState.dataBuf = fileread (outfile, &myState.dataBufSize);
			} else {
				if( IsPGPError( sGetFileRef( context, outfile, &outref ) ) )
					goto error;
				myState.fileRef = outref;
				myState.fileName = outfile;
			}
			myState.fLocalEncode = fLocalEncode;
		}
		if( fkeypass ) {
			PGPAppendOptionList( context, opts,
						  PGPOPassphrase( context, skeypass ),
						  PGPOLastOption ( context ) );
		}
		if( IsPGPError( err = PGPDecode( context, opts,
										 PGPOKeySetRef( context, keys ),
										 PGPOLastOption ( context ) ) ) )
			goto error;
		if( fmem && !fverify )
			filewrite (outfile, outbuf, outbufsize);
#if 0
	} else if (fanalyze) {
		if (fmem) {
			inbuf = fileread (infile, &inbufsize);
			rslt = SimplePGPAnalyzeBuffer (inbuf, inbufsize);
		} else {
			rslt = SimplePGPAnalyzeFile (infile);
		}
	} else if (faddkey) {
		char *keyfile = outfile;
		if (fmem) {
			inbuf = fileread (infile, &inbufsize);
			rslt = SimplePGPAddKeyBuffer (NULL, inbuf, inbufsize, keyfile);
		} else {
			rslt = SimplePGPAddKey (NULL, infile, keyfile);
		}
	} else if (fextkey) {
		char *userid = infile;
		if (fmem) {
			outbufsize = 1;
			outbuf = (PGPByte *)malloc(outbufsize);
			rslt = SimplePGPExtractKeyBuffer (NULL, userid, (char *)outbuf,
				&outbufsize, NULL);
			outbuf = (PGPByte *)malloc(outbufsize);
			rslt = SimplePGPExtractKeyBuffer (NULL, userid, outbuf,
				&outbufsize, NULL);
			if (!rslt)
				filewrite (outfile, outbuf, outbufsize);
		} else {
			rslt = SimplePGPExtractKey (NULL, userid, outfile, NULL);
		}
#endif
	}
error:
done:
	if( IsntNull( opts ) ) {
		PGPFreeOptionList( opts );
		opts = NULL;
	}
	if( IsntNull( inref ) ) {
		PGPFreeFileSpec( inref );
		inref = NULL;
	}
	if( IsntNull( outref ) ) {
		PGPFreeFileSpec( outref );
		outref = NULL;
	}
	if( IsntNull( encryptKeySet ) ) {
		PGPFreeKeySet( encryptKeySet );
		encryptKeySet = NULL;
	}
	
	if ( IsntPGPError( err ) )
		printf ("Finished, no errors.\n");
	else
	{
		printf ("!!!!!!!!!!!!!!\n");
		pgpTestAssertNoErr( err );
		printf ("!!!!!!!!!!!!!!\n");
	}

	PGPFreeKeySet( keys );

	pgpTestAssertNoErr( err );

#if PGP_INTERNAL_TESTING
	printf ("(Checking for leaks...)\n");
	pgpLeaksEndSession ();
	printf ("done.\n");
#endif
}



/*____________________________________________________________________________
 	Decode a command line and put it into argc, argv form.
 	argv[ 0 ] is set to a dummy arg.
 	
 	Caller should deallocate argv with free()
____________________________________________________________________________*/
	static void
DecodeCommandLine(
	char *		buffer,
	int *		argcOut,
	char ***	argvOut)
{
	int				argc;
	char **			argv;
	int				lastlen;
	char *			nextArg;
	char *			curBuffer;
	char			programName[ 64 ];
	#define kMaxArgs	100
	
	*argvOut	= NULL;
	*argcOut	= 0;
	
	argv		= (char **)malloc( sizeof( char *) * kMaxArgs );
	if ( IsNull( argv ) )
		return;
	
	strcpy( programName, "testEncode" );
	argv[ 0 ]	= programName;	/* simulate program name */
	argc		= 1;
	curBuffer	= buffer;
	while ( argc < kMaxArgs &&
		IsntNull(nextArg = strtok( curBuffer, " ")))
	{
		curBuffer	= 0;	/* strange, but required for strok */
		argv[ argc ]	= nextArg;
		++argc;
	}

	/* get last arg and turn EOL into null */
	if ( argc > 1 )
	{
		char *	lastArg	= argv[ argc - 1 ];
		
		lastlen		= strlen( lastArg );
		if ( isspace( lastArg[ lastlen-1 ] ) )
			lastArg[ lastlen-1 ] = '\0';
		
		/* why check this? */
		if ( argc == 2 && argv[1][0]=='\0')
			argc	= 1;
	}
	
	*argcOut	= argc;
	*argvOut	= argv;
}


/*____________________________________________________________________________
 	Loop, accepting a command line, until "q" or "Q" is entered.
____________________________________________________________________________*/
	void
TestEncodeInteractive( PGPContextRef context )
{
	for(;;)
	{
		char	buffer[ 1024 ];
		
		fputs( "Command -> ", stdout);
		fgets( buffer, sizeof(buffer), stdin);
		
		/* enable a quit command */
		if ( buffer[ 0 ] == 'q' || buffer[ 0 ] =='Q' )
			return;
			
		TestEncodeCmdLine( context, buffer );
	}
}

/*____________________________________________________________________________
 	Run an encode test given an argc, argv setup as gotten from a Unix main()
 	routine.
____________________________________________________________________________*/
	void
TestEncodeArgs(
	PGPContextRef	context,
	int				argc,
	const char **	argv )
{
	RunTest( context, argc, argv );
}


/*____________________________________________________________________________
 	Run an encode test given a command line string.  The string will be parsed
 	for options.
____________________________________________________________________________*/
	void
TestEncodeCmdLine(
	PGPContextRef	context,
	const char *	commandLine )
{
	int		argc;
	char **	argv;
	char	temp[ 1024 ];
	
	printf( "***** Command: %s *****\n", commandLine  );
	
	/* make copy so that we won't modify original */
	if ( strlen( commandLine ) >= sizeof( temp ) )
		return;
	strcpy( temp, commandLine );
	
	DecodeCommandLine( temp, &argc, &argv );
	
	/* first arg is always program name, so we require two or more  */
	if ( argc >= 2 )
	{
		TestEncodeArgs( context, argc, (const char **)argv );
		
		free( argv );
	}

}



/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */

