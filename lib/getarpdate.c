#include "elm_defs.h"

#include <time.h>

char *get_arpa_date(void)
{
	/** returns an ARPA standard date.  The format for the date
	    according to DARPA document RFC-822 is exemplified by;

	       	      Mon, 12 Aug 85 6:29:08 MST

	**/

	static char buffer[SLEN];	/* static character buffer       */
	time_t	  curr_time;		/* time in seconds....		 */

	time(&curr_time);

	strftime(buffer, SLEN, "%a, %d %b %Y %T %z", localtime(&curr_time));

	return buffer;
}

