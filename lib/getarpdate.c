#include "elm_defs.h"

#include <time.h>

char *get_arpa_date()
{
	/** returns an ARPA standard date.  The format for the date
	    according to DARPA document RFC-822 is exemplified by;

	       	      Mon, 12 Aug 85 6:29:08 MST

	**/

	static char buffer[SLEN];	/* static character buffer       */
	time_t	  curr_time;		/* time in seconds....		 */
	struct tm curr_tm;		/* Time structure, see CTIME(3C) */

	time(&curr_time);
	curr_tm = *localtime(&curr_time);

	strftime(buffer, SLEN, "%a, %d %b %Y %T %z (%Z)", &curr_tm);

	return buffer;
}

