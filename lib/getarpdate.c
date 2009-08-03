static char rcsid[] = "@(#)$Id: getarpdate.c,v 1.3 1999/03/24 14:03:52 wfp5p Exp $";

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
 *******************************************************************************
 * $Log: getarpdate.c,v $
 * Revision 1.3  1999/03/24  14:03:52  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.2  1995/09/29  17:41:11  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

#ifdef I_TIME
#  include <time.h>
#endif
#ifdef I_SYSTIME
#  include <sys/time.h>
#endif

#ifndef	_POSIX_SOURCE
extern struct tm *localtime();
extern time_t	  time();
#endif

static char *arpa_dayname[] = { "Sun", "Mon", "Tue", "Wed", "Thu",
		  "Fri", "Sat", "" };

static char *arpa_monname[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", ""};

extern int get_tz_mins();
extern char *get_tz_name();

char *
get_arpa_date()
{
	/** returns an ARPA standard date.  The format for the date
	    according to DARPA document RFC-822 is exemplified by;

	       	      Mon, 12 Aug 85 6:29:08 MST

	**/

	static char buffer[SLEN];	/* static character buffer       */
	time_t	  curr_time;		/* time in seconds....		 */
	struct tm curr_tm;		/* Time structure, see CTIME(3C) */
	long      tzmin;		/* number of minutes off gmt 	 */
	int	  tzsign;		/* + or - gmt 			 */
	int	  year;			/* current year - with century	 */

	/*
	 * The get_tz_mins() routine steps on the static data returned
	 * by localtime(), so we need to save off the value obtained here.
	 */
	(void) time(&curr_time);
	curr_tm = *localtime(&curr_time);

/* 	if ((year = curr_tm.tm_year) < 100)
		year += 1900;
 */
        year = curr_tm.tm_year + 1900;
   
	if ((tzmin = -get_tz_mins()) >= 0) {
		tzsign = '+';
	} else {
		tzsign = '-';
		tzmin = -tzmin;
	}

	sprintf(buffer, "%s, %d %s %d %02d:%02d:%02d %c%02d%02d (%s)",
	  arpa_dayname[curr_tm.tm_wday],
	  curr_tm.tm_mday, arpa_monname[curr_tm.tm_mon], year,
	  curr_tm.tm_hour, curr_tm.tm_min, curr_tm.tm_sec,
	  tzsign, tzmin / 60, tzmin % 60, get_tz_name(&curr_tm));
	
	return buffer;
}


#ifdef _TEST
int debug = 1;
FILE *debugfile = stderr;
main()
{
	printf("system(\"date\") says:   ");
	fflush(stdout);
	system("date");
	fflush(stdout);
	printf("get_arpa_date() says:  %s\n", get_arpa_date());
	exit(0);
}
#endif

