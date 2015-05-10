

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
 *******************************************************************************
 * $Log: date.c,v $
 * Revision 1.2  1995/09/29  17:42:02  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** return the current date and time in a readable format! **/
/** also returns an ARPA RFC-822 format date...            **/


#include "elm_defs.h"
#include "elm_globals.h"

#ifdef I_TIME
#  include <time.h>
#endif
#ifdef I_SYSTIME
#  include <sys/time.h>
#endif
#ifdef BSD
#  include <sys/timeb.h>
#endif

char *elm_date_str(char *buf, struct header_rec *entry)
{
	time_t secs = (entry->time_sent + entry->tz_offset);
	strftime(buf, SLEN, "%b %d, %Y %r", gmtime(&secs));
	return buf;
}

void make_menu_date(struct header_rec *entry)
{
	time_t secs = (entry->time_sent + entry->tz_offset);
	strftime(entry->time_menu, SLEN, "%b %d", gmtime(&secs));
}
