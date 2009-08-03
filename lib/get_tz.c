
static char rcsid[] = "@(#)$Id: get_tz.c,v 1.3 1995/09/29 17:41:11 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: get_tz.c,v $
 * Revision 1.3  1995/09/29  17:41:11  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:18:54  wfp5p
 * Alpha 7
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
extern struct tm *gmtime();
extern time_t	  time();
#endif


#ifdef _CONFIGURE
/*
 * The "Configure" program will try to determine the proper setting to make
 * "get_tz_name()" work.  It will compile the program with _CONFIGURE enabled.
 * We do not want to build "get_tz_mins()" when doing the config tests.
 */
#define get_tz_mins() 0
main()
{
	char *get_tz_name();
	puts(get_tz_name((struct tm *)0));
	exit(0);
}
#endif


#ifndef _CONFIGURE /*{*/
/*
 * get_tz_mins() - Return the local timezone offset in minutes west of GMT.
 *
 * WARNING -- This routine will step on the static data returned by
 * localtime() and gmtime().  Precautions must be taken in the calling
 * routine to avoid trouncing the time information being used.
 *
 * An earlier version of Elm had a more complicated routine of the same
 * name.  This implementation is more limited in that it calculates only
 * the local TZ offset.  The old routine was able to calculate any TZ
 * offset given a (struct tm *).
 */
int get_tz_mins()
{
	time_t tval;
	struct tm *tm;
	long t2, t1;
	extern long make_gmttime(); /* from date_util.c */

	time(&tval);

	tm = localtime(&tval);
	t1 = make_gmttime(1900+tm->tm_year, 1+tm->tm_mon, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	tm = gmtime(&tval);
	t2 = make_gmttime(1900+tm->tm_year, 1+tm->tm_mon, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return (int) ((t2-t1)/60);
}
#endif /*}!_CONFIGURE*/


/*
 * get_tz_name(tm) - Return timezone name.
 *
 * Try to return the timezone name associated with the time specified by
 * "tm", or the local timezone name if "tm" is NULL.  On some systems, you
 * will get the local timezone name regardless of the "tm" value.
 *
 * Exactly one of the following definitions must be enabled to indicate
 * the system-specific method for timezone name handling.
 *
 *	TZNAME_USE_TM_NAME     use (struct tm *)->tm_name
 *	TZNAME_USE_TM_ZONE     use (struct tm *)->tm_zone
 *	TZNAME_USE_TZNAME      use "tzname[]" external
 *	TZNAME_USE_TIMEZONE    use timezone() function
 *
 * The TZNAME_HANDLING definition is just used to verify the configurations
 * was setup correctly.  It will force a compiler warning or error if there
 * is a configuration problem.
 */

char *get_tz_name(tm)
struct tm *tm;
{

	if (tm == 0) {
		time_t t;
		(void) time(&t);
		tm = localtime(&t);
	}

#ifdef TZNAME_USE_TM_NAME
#define TZNAME_HANDLING 1
	/*
	 * This system maintains the timezone name in the (struct tm).
	 */
	return tm->tm_name;
#endif

#ifdef TZNAME_USE_TM_ZONE
#define TZNAME_HANDLING 2
	/*
	 * This system maintains the timezone name in the (struct tm).
	 */
	return tm->tm_zone;
#endif

#ifdef TZNAME_USE_TZNAME
#define TZNAME_HANDLING 3
	/*
	 * This system maintains a global array that contains two timezone
	 * names, one for when DST is in effect and one for when it is not.
	 * We simply need to pick the right one.
	 */
	{
		extern char *tzname[];
		return tzname[tm->tm_isdst];
	}
#endif

#ifdef TZNAME_USE_TIMEZONE
#define TZNAME_HANDLING 4
	/*
	 * This system provides a timezone() procedure to get a timezone
	 * name.  Be careful -- some systems have this procedure but
	 * depreciate its use, and in some cases it is outright broke.
	 * WARNING!!!  The "get_tz_mins()" routine is destructive
	 * to any (struct tm *) value that was obtained by gmtime() or
	 * localtime().
	 */
	{
		extern char *timezone();
		int isdst = tm->tm_isdst;
		return timezone(get_tz_mins(), isdst);
	}
#endif

#ifndef TZNAME_HANDLING
	/* Force a compile error if the timezone config is wrong. */
	no_tzname_handling_defined(TZNAME_HANDLING);
#endif
}


#ifdef _TEST

/*
 * This routine tests the timezone procedures by forcing a TZ value
 * and checking the results.  This test routine is *not* portable.  It
 * will work only on systems that (1) use the TZ environment parameter,
 * (2) have a putenv() procedure, and (3) putenv() takes a single argument.
 */

int debug = 1;
FILE *debugfile = stderr;

struct {
	char *tz_setting;
	char *expected_ans;
} trytable[] = {
	{ "",			"local timezone setting" },
	{ "TZ=GMT",		"always GMT/0" },
	{ "TZ=CST6CDT",		"either CDT/300 or CST/360" },
	{ "TZ=EST5EDT",		"either EDT/240 or EST/300" },
	{ "TZ=EST5EDT;0,364",	"always EDT/240" },
	{ "TZ=EST5EDT;0,0",	"always EST/300" },
	{ NULL,			NULL }
};

main()
{
	time_t t;
	int i;
	extern char *getenv();

	puts("Notes:");
	puts("\"get_tz_name(gmtime)\" trial should always show GMT.");
	puts("\"get_tz_name(NULL)\" should match \"get_tz_name(localtime)\".");
	puts("Results marked \"either/or\" depend whether DST in effect now.");

	for (i = 0 ; trytable[i].tz_setting != NULL ; ++i) {
		if (trytable[i].tz_setting[0] != '\0')
			putenv(trytable[i].tz_setting);
		putchar('\n');
		printf("expected result: %s\n", trytable[i].expected_ans);
		printf("getenv(\"TZ\") = \"%s\"\n", getenv("TZ"));
		printf("get_tz_mins() = %d\n", get_tz_mins());
		(void) time(&t);
		printf("get_tz_name(NULL) = \"%s\"\n",
			get_tz_name((struct tm *)0));
		printf("get_tz_name(localtime) = \"%s\"\n",
			get_tz_name(localtime(&t)));
		printf("get_tz_name(gmtime) = \"%s\"\n",
			get_tz_name(gmtime(&t)));
	}
	exit(0);
}

#endif /*_TEST*/

