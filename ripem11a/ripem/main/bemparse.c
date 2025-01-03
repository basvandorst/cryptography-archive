/*-- bemparse.c -- Routines to parse a data structure
 *  in OSI's Basic Encoding Rules format.
 *
 *  Basic Encoding Rules (BEM) specify a means of encoding
 *  a complex data structure as a string of "octets" (bytes).
 *  The routines here display a data structure represented
 *  via BEM, breaking down the data structure into its
 *  constituent parts.  (Abstract Syntax Notation refers
 *  to these data structures as "types".)
 *
 *  These routines were written to facilitate experimentation/
 *  development of a Privacy Enhanced Mailer, and by no
 *  means can parse all possible BEM data structures.
 *
 *  Information used to develop these routines was gleaned
 *  from the Public-Key Cryptography Standards documents
 *  and other documents available via FTP to RSA.COM.
 *
 *  Placed in the public domain.
 *
 *  Mark Riordan   7 May 1992
 */

#include <stdio.h>
#include <errno.h>
#include "bemparse.h"

#ifdef mips
extern int errno;
#endif

/* Symbols for the Tag field of a type. */

#define TAG_INTEGER             0x02
#define TAG_BIT_STRING          0x03
#define TAG_OCTET_STRING        0x04
#define TAG_NULL                0x05
#define TAG_OBJECT_IDENTIFIER   0x06
#define TAG_SEQUENCE            0x10
#define TAG_SET                 0x11
#define TAG_PrintableString     0x13
#define TAG_IA5String           0x16
#define TAG_UTCTime             0x17

#define TAG_MASK                0x1f
#define TAG_HIGH_BASE           0x80

/* Bit mask to isolate the "constructed" bit. */

#define CONSTRUCTED_MASK        0x20

/* Symbols for the Class field of a type. */

#define CLASS_MASK             0x03
#define CLASS_Universal        0x00
#define CLASS_Application      0x01
#define CLASS_Context_Specific 0x02
#define CLASS_Private          0x03
#define CLASS_SHIFT            6

#define LENGTH_MASK            0x80

#define OBJECT_BASE            128


/* Here are some data structures used by the program
 * to identify the various types and classes, and to map
 * the binary representation to ASCII strings that
 * describe them.
 */

#if defined(IBMRT) || defined(SUNCC) || defined(mips) \
  || (defined(_AIX370) && !defined(_IBMESA)) \
  || (defined(ps2) && !defined(__GNUC__)) \
  || (defined(HP) && !defined(__GNUC__))
#define mktag(str) { TAG_/**/str, "str" }
#else
#define mktag(str) { TAG_##str, #str }
#endif

struct struct_tag {
  TypeTag      tagcode;
  char        *tagname;
} tagCodes[] = {
 mktag(INTEGER), mktag(BIT_STRING), mktag(OCTET_STRING),
 mktag(NULL), mktag(OBJECT_IDENTIFIER), mktag(SEQUENCE), mktag(SET),
 mktag(PrintableString), mktag(IA5String), mktag(UTCTime) };

#if defined(IBMRT) || defined(SUNCC) || defined(mips) \
   || (defined(_AIX370) && !defined(_IBMESA)) \
   || (defined(ps2) && !defined(__GNUC__)) \
   || (defined(HP) && !defined(__GNUC__))
#define mkcl(str) {CLASS_/**/str, "str" }
#else
#define mkcl(str) {CLASS_##str, #str }
#endif

struct struct_class {
	TypeClass    classcode;
   char        *classname;
} classCodes[] = {
	mkcl(Universal), mkcl(Application), mkcl(Context_Specific),
   mkcl(Private) };

int indent_level;
FILE *bstream=stderr;


/*--- function BEMParse ---------------------------------
 *
 * Parse a BEM type, writing a description to standard output.
 *
 * Entry: 	octstr   is a "binary" string of bytes representing
 *                   a BEM type.
 *  			stream	is the stream to write to.
 *
 * Exit:    A human-readable description has been written
 *            to output.
 *				Returns the number of bytes in the type.
 */

TypeLength
BEMParse(octstr,stream)
unsigned char *octstr;
FILE *stream;
{
   indent_level=0;
	bstream = stream;
	return BEMParse2(&octstr);
}

/*--- function BEMParse2 --------------------------------
 *
 * Do the real work of BEMParse.  It's a separate routine
 * to allow recursion.
 *
 * Entry:	octstr	points to a string of octets.
 *
 * Exit:    octstr   points to the end of the string.
 *				Returns the number of bytes in the type.
 *          A description has been written to output.
 */

TypeLength
BEMParse2(octstr)
unsigned char **octstr;
{
	octet *octptr = *octstr;
	TypeClass  class;
	TypeTag    tag;
	TypeLength length;
	TypeLength idlen, lenlen, totlen, val;
	int j, found, constructed, retval;
	int idx = 0;

   idlen = ParseID(octstr,&class,&constructed,&tag);

	lenlen = ParseLength(octstr,&length);

	totlen = idlen + lenlen + length;

	PutIndent();
	if(constructed) {
		fprintf(bstream,"Constructed ");
	}

	/* Describe the class. */

	for(j=0, found=0; !found && j<sizeof(classCodes) /
    sizeof(classCodes[0]); j++) {
		if(class == classCodes[j].classcode) {
         found = 1;
         idx = j;
		}
	}

	retval = fprintf(bstream,"%s ",found ? classCodes[idx].classname : "Bad class");
	if(EOF == retval) {
		fprintf(stderr,"Printf returned error %u.\n",errno);
	}

	/* Describe the type.  If it's not Universal class, I
    * won't know much about it.
    */

	for(j=0, found=0; !found && j<sizeof(tagCodes) /
    sizeof(tagCodes[0]); j++) {
		if(tag == tagCodes[j].tagcode) {
         found = 1;
         idx = j;
		}
	}

	fprintf(bstream,"%s ",found ? tagCodes[idx].tagname : "Unknown tag");

	if(constructed) {
		fprintf(bstream,":\n");
		ParseSequence(octstr,length);
	} else {

	switch(tag) {
		case TAG_SET:
		case TAG_SEQUENCE:
			fprintf(bstream,"\n");
			ParseSequence(octstr,length);
			break;

		case TAG_INTEGER:
		case TAG_OCTET_STRING:
		case TAG_BIT_STRING:
			if(length > 3 || 1) {
				int chperline=0;

				putc('\n',bstream);
				for(j=0; j<length; j++) {
					if(chperline == 0) {
						PutIndent(); fputs("   ",bstream);
					}
					fprintf(bstream,"%-2.2x ",*(*octstr+j));
					if(++chperline >= 16) {
						putc('\n',bstream);
						chperline = 0;
					}
				}
				if(chperline) putc('\n',bstream);
			} else {
				val = 0;
				for(j=0; j<length; j++) {
					val = val*256 + *(*octstr+j);
				}
				fprintf(bstream,"%ld\n",val);
			}
			*octstr += length;
			break;

		case TAG_IA5String:
		case TAG_PrintableString:
			for(j=0; j<length; j++) putc(*(*octstr+j),bstream);
			putc('\n',bstream);
			*octstr += length;
			break;

      case TAG_OBJECT_IDENTIFIER:
			octptr = *octstr;
			val = *octptr / 40;
			fprintf(bstream,"%ld ",val);
			val = *octptr % 40;
			fprintf(bstream,"%ld ",val);
			octptr++;
			while(octptr < *octstr+length) {
				for(val=0; *octptr & OBJECT_BASE; octptr++) {
					val = val*OBJECT_BASE + ((OBJECT_BASE-1) & *octptr);
				}
				val = val*OBJECT_BASE + *(octptr++);
				fprintf(bstream,"%ld ",val);
			}
			putc('\n',bstream);
			*octstr += length;
			break;

		default:

			*octstr += length;
			break;
	}
	}

	return totlen;
}

/*--- function ParseSequence ------------------------------
 *
 * Parse a structured type.
 *
 * Entry: 	octstr 	points to a string of bytes.  It is
 *							positioned at the first member of a
 *							sequence or constructed type.
 *				length	is the number of bytes in the sequence,
 *							not including the header after which
 *							octstr is positioned.
 *				indent_level specifies how deep we are down in
 *							the data structure.
 *
 *	Exit:    octstr   points right after the sequence.
 *				indent_level is unchanged.
 *				A description of the entire sequence has been
 *				  written out.
 */
void
ParseSequence(octstr,length)
unsigned char **octstr; 
TypeLength length;
{
	TypeLength mylen;

	indent_level++;
	do {
		mylen = BEMParse2(octstr);
		length -= mylen;
	} while(length > 0);
	indent_level--;
}

/*--- function ParseID ----------------------------------
 *
 * Parse the ID field of a BEM type.
 *
 * Entry:	octstr	points to the beginning of a BEM type,
 *							which is where the ID bytes are.
 *
 *	Exit:		octstr	points to just beyond the ID bytes.
 *          class		is the class of the type.
 *				constructed is non-zero if the type is constructed.
 *				tag		is the tag of the type.
 *				Returns the number of bytes in the ID field.
 */
TypeLength
ParseID(octstr,class,constructed,tag)
unsigned char **octstr; 
TypeClass *class;
TypeConstructed *constructed;
TypeTag *tag;
{
	octet *octptr = *octstr;
	TypeLength IDlen;

   *class = (*octptr >> CLASS_SHIFT) & CLASS_MASK;
	IDlen = 1;

	*constructed = *octptr & CONSTRUCTED_MASK;

   if(TAG_MASK & *octptr == TAG_MASK) {
		/* This is High-tag-number form */
		*tag = 0;
		for(octptr++; TAG_HIGH_BASE & *octptr; octptr++) {
			*tag *= TAG_HIGH_BASE;
			*tag += *octptr - TAG_HIGH_BASE;
			IDlen++;
		}
		*tag *= TAG_HIGH_BASE;
		*tag += *octptr;
		IDlen++;
   } else {
      *tag = *octptr & TAG_MASK;
	}

   *octstr = octptr+1;
	return IDlen;
}

/*--- function ParseLength ----------------------------------
 *
 *	Parse the Length field of a BEM type.
 *
 *	Entry:	octstr	points to the beginning of the length
 *							field of a BEM type.
 *
 *	Exit:    length   is the extracted value of the length field.
 *				Returns the number of bytes in the length field.
 */

TypeLength
ParseLength(octstr,length)
octet **octstr; 
TypeLength *length;
{
	octet *octptr = *octstr;
   int lenbytes;
	TypeLength Lenlen;

	if(*octptr & LENGTH_MASK) {
		/* Long form of length. */
		lenbytes = *octptr & (LENGTH_MASK-1);
		Lenlen = lenbytes+1;
		for(*length=0,octptr++; lenbytes; lenbytes--,octptr++) {
         *length = *length*256 + *octptr;
		}
	} else {
		*length = *octptr;
		Lenlen = 1;
		octptr++;
	}

	*octstr = octptr;
	return Lenlen;
}

/*--- function PutIndent ----------------------------------
 *
 *	Indent the output to reflect how deeply nested the
 * BEM type is at this point.
 *
 *	Entry:	indent_level is the number of levels of nesting
 *				 at the current point in the BEM string.
 *				 0 means no nesting (e.g., the whole BEM string
 *				 is just a simple type).
 *
 *	Exit:		Spaces have been written to output to indent
 *				 the line about to be written.
 */

void
PutIndent()
{
	int j;

	for(j=0; j<indent_level; j++) fprintf(bstream,"   ");
}
