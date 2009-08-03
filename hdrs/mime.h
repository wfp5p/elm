/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 ******************************************************************************
 * $Log: mime.h,v $
 * Revision 1.2  1995/09/29  17:40:51  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:30  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#if defined(MIME_SEND) || defined (MIME_RECV) /*{*/

#define MIME_HDR_MIMEVERSION	"MIME-Version"
#define MIME_VERSION_10		"1.0"
#define MIME_VERSION_OLD	"RFCXXXX"
#define MIME_HDR_CONTENTTYPE	"Content-Type"
#define MIME_HDR_CONTENTENCOD	"Content-Transfer-Encoding"

/* Encoding types */

#define ENCODING_ILLEGAL	-1
#define ENCODING_NONE		0
#define ENCODING_7BIT		1
#define ENCODING_8BIT		2
#define ENCODING_BINARY		3
#define ENCODING_QUOTED		4
#define ENCODING_BASE64		5
#define ENCODING_UUENCODE	6
#define ENCODING_EXPERIMENTAL	7

#define ENC_NAME_7BIT		"7bit"
#define ENC_NAME_8BIT		"8bit"
#define ENC_NAME_BINARY		"binary"
#define ENC_NAME_QUOTED		"quoted-printable"
#define ENC_NAME_BASE64		"base64"
#define ENC_NAME_UUENCODE	"x-uuencode"

/* default charsets, which are a superset of US-ASCII, so we did not
   have to go out to metamail for us-ascii */

#define COMPAT_CHARSETS "ISO-8859-1 ISO-8859-2 ISO-8859-3 ISO-8859-4 ISO-8859-5 ISO-8859-7 ISO-8859-8 ISO-8859-9"

#endif /*} defined(MIME_SEND) || defined (MIME_RECV) */


#if defined(MIME_SEND) || defined(MIME_RECV)
int mime_encoding_type P_((const char *));
#endif

#ifdef MIME_RECV
int needs_mmdecode P_((char *));
int notplain P_((char *));
int charset_ok P_((char *));
#endif

