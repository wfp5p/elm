
static char rcsid[] = "@(#)$Id: add_site.c,v 1.2 1995/09/29 17:41:02 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
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
 * $Log: add_site.c,v $
 * Revision 1.2  1995/09/29  17:41:02  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"

void
add_site(buffer, site, lastsite)
char *buffer;
const char *site;
char *lastsite;
{
	/** add site to buffer, unless site is 'uucp' or site is
	    the same as lastsite.   If not, set lastsite to site.
	**/

	char local_buffer[SLEN], *stripped;
	char *strip_parens();

	stripped = strip_parens(site);

	if (istrcmp(stripped, "uucp") != 0)
	  if (strcmp(stripped, lastsite) != 0) {
	    if (buffer[0] == '\0')
	      strcpy(buffer, stripped);         /* first in list! */
	    else {
	      sprintf(local_buffer,"%s!%s", buffer, stripped);
	      strcpy(buffer, local_buffer);
	    }
	    strcpy(lastsite, stripped); /* don't want THIS twice! */
	  }
}
