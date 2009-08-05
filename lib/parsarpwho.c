

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: parsarpwho.c,v $
 * Revision 1.4  1995/09/29  17:41:25  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/09/11  15:18:57  wfp5p
 * Alpha 7
 *
 * Revision 1.2  1995/07/18  18:59:51  wfp5p
 * Alpha 6
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"

/*
 * parse_arpa_who() - Extract a sanitized fullname from a header line.
 *
 * We assume the "fullname" result buffer is at least SLEN chars long.
 */

int
parse_arpa_who(buffer, fullname)
const char *buffer;
char *fullname;
{
    char addrbuf[SLEN];

    /* extract the address and fullname */
    if (parse_arpa_mailbox(buffer, addrbuf, sizeof(addrbuf),
		fullname, SLEN, (char **)NULL) != 0) {
	/* yoicks! */
	fullname[0] = '\0';
	return -1;
    }

    /* if there wasn't a fullname then return the address instead */
    if (fullname[0] == '\0')
	(void) strncpy(fullname, addrbuf,SLEN);

    return 0;
}

