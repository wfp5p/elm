
static char rcsid[] = "@(#)$Id: mime.c,v 1.3 1996/03/14 17:29:41 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 ******************************************************************************
 * $Log: mime.c,v $
 * Revision 1.3  1996/03/14  17:29:41  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:18  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:37  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/


#include "elm_defs.h"
#include "elm_globals.h"
#include "mime.h"
#include "s_elm.h"

int mime_encoding_type(Encoding)
const char *Encoding;
{
    if (Encoding == NULL || *Encoding == '\0')
	return ENCODING_NONE;
    if (istrcmp(Encoding, ENC_NAME_7BIT) == 0)
	return ENCODING_7BIT;
    if (istrcmp(Encoding, ENC_NAME_8BIT) == 0)
	return ENCODING_8BIT;
    if (istrcmp(Encoding, ENC_NAME_BINARY) == 0)
	return ENCODING_BINARY;
    if (istrcmp(Encoding, ENC_NAME_QUOTED) == 0)
	return ENCODING_QUOTED;
    if (istrcmp(Encoding, ENC_NAME_BASE64) == 0)
	return ENCODING_BASE64;
    if (istrcmp(Encoding, ENC_NAME_UUENCODE) == 0)
	return ENCODING_UUENCODE;
    if (strincmp(Encoding, "x-", 2) == 0)
	return ENCODING_EXPERIMENTAL;
    return ENCODING_ILLEGAL;
}

#ifdef MIME_RECV /*{*/

needs_mmdecode(s)
char *s;
{
	char buf[SLEN];
	char *t;
	int EncType;

	if (!s) return(1);
	while (*s && isspace(*s)) ++s;
	t = buf;
	while (*s && !isspace(*s) && ((t-buf) < (SLEN-1))) *t++ = *s++;
	*t = '\0';
	EncType = mime_encoding_type(buf);
	if ((EncType == ENCODING_NONE) ||
	    (EncType == ENCODING_7BIT) ||
	    (EncType == ENCODING_8BIT) ||
	    (EncType == ENCODING_BINARY)) {
	    /* We don't need to go out to mmdecode, return 0 */
	    return(0);
	} else {
	    return(1);
	}
}

notplain(s)
char *s;
{
	char *t;
	if (!s) return(1);
	while (*s && isspace(*s)) ++s;
	if (istrcmp(s, "text\n") == 0) {
		/* old MIME spec, subtype now obligat, accept it as
		   "text/plain; charset=us-ascii" for compatibility
		   reason */
		return(0);
	}
	if (strincmp(s, "text/plain", 10)) return(1);
	t = (char *) index(s, ';');
	while (t) {
		++t;
		while (*t && isspace(*t)) ++t;
		if (!strincmp(t, "charset", 7)) {
			s = (char *) index(t, '=');
			if (s) {
				++s;
				while (*s && (isspace(*s) || *s == '\"')) ++s;
				if (!strincmp(s, display_charset, strlen(display_charset)))
					return(0);
				if (!strincmp(s, "us-ascii", 8)) {
					/* check if configured charset could
					   display us-ascii */
					if(charset_ok(display_charset)) return(0);
				}
			}
			return(1);
		}
		t = (char *) index(t, ';');
	}
	return(0); /* no charset, was text/plain */
}

int charset_ok(s)
char *s;
{
    /* Return true if configured charset could display us-ascii too */
    char buf[SLEN];	/* assumes sizeof(charset_compatlist) <= SLEN */
    char *bp, *chset;

    /* the "charset_compatlist[]" format is: */
    /*   charset charset charset ... */
    bp = strcpy(buf, charset_compatlist);
    while ((chset = strtok(bp, " \t\n")) != NULL) {
	bp = NULL;
	if (istrcmp(chset, s) == 0)
	    break;
    }

    /* see if we reached the end of the list without a match */
    if (chset == NULL) {
	return(FALSE);
    }
    return(TRUE);
}

#endif /*} MIME_RECV */
