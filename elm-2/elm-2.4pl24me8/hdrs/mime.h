/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 5.3 $   $State: Exp $
 *
 * 			Copyright (c) 1988-1992 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Syd Weinstein, Elm Coordinator
 *	elm@DSI.COM			dsinc!elm
 *
 ******************************************************************************
 * $Log: mime.h,v $
 * Revision 5.3  1992/11/07  20:50:08  syd
 * add some new names to make header_cmp use easier
 * From: Syd
 *
 * Revision 5.2  1992/10/25  01:47:45  syd
 * fixed a bug were elm didn't call metamail on messages with a characterset,
 * which could be displayed by elm itself, but message is encoded with QP
 * or BASE64
 * From: Klaus Steinberger <Klaus.Steinberger@Physik.Uni-Muenchen.DE>
 *
 * Revision 5.1  1992/10/03  22:34:39  syd
 * Initial checkin as of 2.4 Release at PL0
 *
 *
 ******************************************************************************/

#define	MIME_HEADER	"MIME-Version: 1.0"
#define	MIME_HEADER_NAME	"MIME-Version"
#define	MIME_HEADER_VERSION	"1.0"
#define	MIME_INCLUDE	"[include"
#define	MIME_CONTENTTYPE	"Content-Type:"
#define	MIME_CONTENTTYPE_LEN	13
#define	MIME_HEADER_CONTENTTYPE	"Content-Type"
#define	MIME_CONTENTENCOD	"Content-Transfer-Encoding:"
#define	MIME_CONTENTENCOD_LEN	26
#define	MIME_HEADER_CONTENTENCOD	"Content-Transfer-Encoding"

/* Encoding types */

#define	ENCODING_ILLEGAL	-1
#define	ENCODING_NONE	0
#define	ENCODING_7BIT	1
#define	ENCODING_8BIT	2
#define	ENCODING_BINARY	3
#define	ENCODING_QUOTED	4
#define	ENCODING_BASE64	5
#define	ENCODING_EXPERIMENTAL	6

#define	ENC_NAME_7BIT	"7bit"
#define	ENC_NAME_8BIT	"8bit"
#define	ENC_NAME_BINARY	"binary"
#define	ENC_NAME_QUOTED "quoted-printable"
#define	ENC_NAME_BASE64	"base64"

/* Default charsets, which are a superset of US-ASCII, so we do not
   have to go out to metamail for us-ascii */

#define COMPAT_CHARSETS "ISO-8859-1 ISO-8859-2 ISO-8859-3 ISO-8859-4 ISO-8859-5 ISO-8859-7 ISO-8859-8 ISO-8859-9 KOI8-R"

/* These are for figuring out what the encoding on outgoing messages should
 * be.
 */
#define HAVE_8BIT 1
#define HAVE_CTRL 4
#define HAVE_BINARY 8

/* Possible major types in Content-Type field. */
#define MIME_TYPE_UNKNOWN	0
#define MIME_TYPE_APPLICATION	1
#define MIME_TYPE_AUDIO		2
#define MIME_TYPE_IMAGE		3
#define MIME_TYPE_MESSAGE	4
#define MIME_TYPE_MULTIPART	5
#define MIME_TYPE_TEXT		6
#define MIME_TYPE_VIDEO		7

/* Values for the "flag" field in "mimeinfo" */
#define MIME_RFC822 1
#define MIME_MIXED 2
#define MIME_DIGEST 4

/* Content-Disposition */
#define DISP_INLINE	0
#define DISP_ATTACH	1

#define DISPOSITION(x) (x == DISP_INLINE ? "inline" : "attachment")
#define TYPE(x) (mime_types[(x)])
#define NONULL(x) (x ? x : "")
#define ENCODING(x) (x >= 0 ? mime_encode_names[x] : "<ILLEGAL>")

typedef struct mime_send {    /* Mime parameters for this mail */
  int encoding_top; /* Encoding for top multipart type.
                     * ENCODING_8BIT:   add content_transfer_encoding: 8bit
                     *                  pass -B8BITMIME to mailer
                     * ENCODING_BINARY: add content_transfer_encoding: binary
		     *                  pass -BBINARYMIME to mailer
                     */
  int encoding_text;/* Encoding of text parts (nt attachments)
		     * ENCODING_QUOTED: 
		     *         add content_transfer_encoding: quoted-printable
		     * ENCODING_8BIT:
		     *         add content_transfer_encoding: 8bit
		     */
  char * Charset;   /* Charset of text */
  int need_enc;     /* Bitmask: HAVE_8BIT:   have 8-bit data
		     *          HAVE_BINARY: have 'binary' data
		     *          HAVE_CTRL:   have control characters
		     */
  int msg_is_multipart;
  char mime_boundary[STRING];
  unsigned int type : 3;
  char subtype[STRING];
  char type_opts[STRING];
} mime_send_t;
