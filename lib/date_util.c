

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: date_util.c,v $
 * Revision 1.4  1999/03/24  14:03:51  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.3  1995/09/29  17:41:07  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:18:52  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

/*
 * Date processing functions:
 *
 * cvt_dayname_to_daynum() - Convert day of week name to a number.
 * cvt_monthname_to_monthnum() - Convert month name to a number.
 * cvt_yearstr_to_yearnum() - Convert year from string to a number.
 * cvt_mmddyy_to_dayofyear() - Convert numeric day/month/year to day of year.
 * cvt_timezone_to_offset() - Convert timezone string to an offset in mins.
 * cvt_timestr_to_hhmmss() - Convert an HH:MM:SS str to numeric hours/mins/secs.
 * make_gmttime() - Calculate number of seconds since the epoch.
 */


#define IsLeapYear(yr)	((yr % 4 == 0) && ((yr % 100 != 0) || (yr % 400 == 0)))


/*
 * The following time zones are taken from a variety of sources.  They
 * are by no means exhaustive, but seem to include most of those in
 * common usage.  A comprehensive list is impossible, since the same
 * abbreviation is sometimes used to mean different things in different
 * parts of the world.
 */
static struct tzone {
    char *str;		/* time zone name */
    int offset;		/* offset, in minutes, EAST of GMT */
} tzone_info[] = {

    /* the following are from RFC-822 */
    { "ut", 0 },
    { "gmt", 0 },
    { "est", -5*60 },	{ "edt", -4*60 },	/* USA eastern standard */
    { "cst", -6*60 },	{ "cdt", -5*60 },	/* USA central standard */
    { "mst", -7*60 },	{ "mdt", -6*60 },	/* USA mountain standard */
    { "pst", -8*60 },	{ "pdt", -7*60 },	/* USA pacific standard */
    { "z", 0 }, /* zulu time (the rest of the military codes are bogus) */

    /* popular European timezones */
    { "wet", 0*60 },				/* western european */
    { "met", 1*60 },				/* middle european */
    { "eet", 2*60 },				/* eastern european */
    { "bst", 1*60 },				/* ??? british summer time */

    /* Canadian timezones */
    { "ast", -4*60 },	{ "adt", -3*60 },	/* atlantic */
    { "nst", -3*60-30 },{ "ndt", -2*60-30 },	/* newfoundland */
    { "yst", -9*60 },	{ "ydt", -8*60 },	/* yukon */
    { "hst", -10*60 },				/* hawaii (not really canada) */

    /* Asian timezones */
    { "jst", 9*60 },				/* japan */
    { "sst", 8*60 },				/* singapore */

    /* South-Pacific timezones */
    { "nzst", 12*60 },	{ "nzdt", 13*60 },	/* new zealand */
    { "wst", 8*60 },	{ "wdt", 9*60 },	/* western australia */

    /*
     * Daylight savings modifiers.  These are not real timezones.
     * They are used for things like "met dst".  The "met" timezone
     * is 1*60, and applying the "dst" modifier makes it 2*60.
     */
    { "dst", 1*60 },
    { "dt", 1*60 },
    { "st", 1*60 },

    /*
     * There's also central and eastern australia, but they insist on using
     * cst, est, etc., which would be indistinguishable for the USA zones.
     */

     { NULL, 0 },
};

static char *month_name[13] = {
    "jan", "feb", "mar", "apr", "may", "jun",
    "jul", "aug", "sep", "oct", "nov", "dec", NULL
};

static char *day_name[8] = {
    "sun", "mon", "tue", "wed", "thu", "fri", "sat", 0
};

static int month_len[13] = {
    31, 99, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31, 0
};


int cvt_dayname_to_daynum(const char *str, int *day_p)
{
    /*
     * Convert a day name to number (Sun = 1).  Only the first three
     * characters are significant and comparison is case insensitive.
     * That is, "Saturday", "sat", and "SATxyzfoobar" all return 7.
     * Returns TRUE if a valid day name is found, otherwise FALSE.
     */

    int i;

    for (i = 0 ; day_name[i] != NULL ; i++) {
	if (strncasecmp(day_name[i], str, 3) == 0) {
	    *day_p = i+1;
	    return TRUE;
	}
    }

    dprint(4, (debugfile, "cvt_dayname_to_daynum failed at \"%s\"\n", str));
    return FALSE;
}



int cvt_monthname_to_monthnum(const char *str, int *month_p)
{
    /*
     * Convert a month name to number (Jan = 1).  Only the first three
     * characters are significant and comparison is case insensitive.
     * That is, "December", "dec", and "DECxyzfoobar" all return 12.
     * Returns TRUE if a valid month name is found, otherwise FALSE.
     */

    int i;

    for (i = 0 ; month_name[i] != NULL ; i++) {
	if (strncasecmp(month_name[i], str, 3) == 0) {
	    *month_p = i+1;
	    return TRUE;
	}
    }

    dprint(4, (debugfile, "cvt_monthname_to_monthnum failed at \"%s\"\n", str));
    return FALSE;
}


int cvt_yearstr_to_yearnum(const char *str, int *year_p)
{
    /*
     * Convert a year from a string to a number.  We will add the century
     * into two-digit strings, e.g. "91" becomes "1991".  Returns TRUE
     * if a reasonable year is specified, else FALSE;
     */

    int year;

    if ((year = atonum(str)) >= 0) {
	if (year < 70) {
	    *year_p = 2000 + year;
	    return TRUE;
	}
	if (year < 100) {
	    *year_p = 1900 + year;
	    return TRUE;
	}
	if (year >= 1900 && year <= 2099) {
	    *year_p = year;
	    return TRUE;
	}
    }

    dprint(4, (debugfile, "cvt_yearstr_to_yearnum failed at \"%s\"\n", str));
    return FALSE;
}


int cvt_mmddyy_to_dayofyear(int month, int dayofmon, int year,
			    int *dayofyear_p)
{
    /*
     * Convert numeric month (1-12), day of month (1-31), and year (with
     * century) to day of year (Jan 1 = 0).  Always returns TRUE.
     */

    int dayofyear, i;

    dayofyear = dayofmon-1;
    for (i = 0 ; i < month-1 ; ++i)
	dayofyear += (i != 1 ? month_len[i] : (IsLeapYear(year) ? 29 : 28));
    *dayofyear_p = dayofyear;
    return TRUE;
}


int cvt_timezone_to_offset(char *str, int *mins_p)
{
    /*
     * Convert a timezone to a number of minutes *east* of gmt.  The
     * timezone can either be a name or an offset, e.g. "+0600".  We also
     * handle two-digit numeric timezones, e.g. "+06", even though they
     * are bogus.  IMPORTANT:  If we are given a two-digit numeric timezone
     * we will rewrite the string into a legal timezone by appending
     * "00".  Returns TRUE if a valid timezone is found, otherwise FALSE.
     */

    struct tzone *p; 
    int tz;

    /*
     * Check for two-digit or four-digit numeric timezone.
     */
    if ((*str == '+' || *str == '-') && (tz = cvt_numtz_to_mins(str+1)) >= 0) {
	switch (strlen(str)) {
	case 3:					/* +NN		*/
	    (void) strcat(str, "00");		/*  make +NN00	*/
	    tz *= 60;
	    break;
	case 5:					/* +NNNN	*/
	    break;
	default:				/* eh?		*/
	    goto failed;
	}
	*mins_p = (*str == '-' ? -tz : tz);
	return TRUE;
    }

    /*
     * Check for timezone name.  I'm told some brain damaged systems
     * can put a "-" before a tz name.
     */
    if (*str == '-') {
	tz = -1;
	++str;
    } else {
	tz = 1;
    }
    for (p = tzone_info; p->str; p++) {
	if (strcasecmp(p->str, str) == 0) {
	    *mins_p = tz * p->offset;
	    return TRUE;
	}
    }

failed:
    /*
     * We parse a lot of stuff where the timezone is optional, and this
     * routine gets a lot of fields that are actually year numbers.  The
     * debug message is an annoying distraction in these cases.
     */
    if (!isdigit(*str)) {
	dprint(4, (debugfile,
		"cvt_timezone_to_offset failed at \"%s\"\n", str));
    }
    return FALSE;
}


int cvt_numtz_to_mins(const char *str)
{
    /*
     * Convert an HHMM string to minutes.  Check to make sure that the
     * string is exactly 4 characters long, and contains all digits.
     * Return -1 if it is not a valid string.
     */
    register int tz;

    /* Process the first 2 characters, ie. the HH part */
    if (!isdigit(str[0]))
	goto bad_tz_str;
    tz = (str[0] - '0') * 10;
    if (!isdigit(str[1]))
	goto bad_tz_str;
    tz += (str[1] - '0');

    /* That takes care of the hours, multiple by 60 to get minutes */
    tz *= 60;

    /* Process the second 2 characters, ie. the MM part */
    if (!isdigit(str[2]))
	goto bad_tz_str;
    tz += (str[2] - '0') * 10;
    if (!isdigit(str[3]))
	goto bad_tz_str;
    tz += (str[3] - '0');

    /* Conversion succeeded */
    if (str[4] == '\0')
	return tz;

bad_tz_str:
    dprint(7,(debugfile,"ridiculous numeric timezone: %s\n",str));
    return -1;
}


int cvt_timestr_to_hhmmss(const char *timestr, int *hours_p, int *mins_p,
			  int *secs_p)
{
    /*
     * Convert a HH:MM[:SS] time specification to hours, minutes, seconds.
     * We will also handle a couple of (bogus) variations:  a simple "HHMM"
     * as well as an "am/pm" suffix (thank BITNET for the latter).
     */

    char tmp[STRING], *str, *s;
    int len, add_hrs, i;

    /*
     * Make a copy so we can step on it.
     */
    str = strfcpy(tmp, timestr, sizeof(tmp));
    len = strlen(str);

    /*
     * Yank any AM/PM off the end.
     */
    add_hrs = 0;
    if (len > 3) {
	if (strcasecmp(str+len-2, "am") == 0) {
		str[len -= 2] = '\0';
	} else if (strcasecmp(str+len-2, "pm") == 0) {
		str[len -= 2] = '\0';
		add_hrs = 12;
	}
    }

    /*
     * It ain't legal, but accept "HHMM".
     */
    if (len == 4 && (i = atonum(str)) > 0) {
	*hours_p = i/60 + add_hrs;
	*mins_p = i%60;
	*secs_p = 0;
	return TRUE;
    }

    /*
     * Break it up as HH:MM[:SS].
     */
    for (s = str ; isdigit(*s) ; ++s)		/* At end of loop: */
	;					/*    HH:MM:SS     */
    if (*s == ':') {				/* str^ ^s         */
	*s++ = '\0';
	*hours_p = atoi(str) + add_hrs;
	str = s;
	for (s = str ; isdigit(*s) ; ++s)	/* At end of loop: */
	    ;					/*    HH:MM:SS     */
	if (*s == '\0') {			/*    str^ ^s      */
	    *mins_p = atoi(str);
	    *secs_p = 0;
	    return TRUE;
	}
	if (*s == ':') {
	    *s++ = '\0';
	    *mins_p = atoi(str);
	    *secs_p = atoi(s);
	    return TRUE;
	}
    }

    dprint(4, (debugfile, "cvt_timestr_to_hhmmss failed at \"%s\"\n", str));
    return FALSE;
}


long make_gmttime(int year, int month, int day, int hours, int mins, int secs)
{
    /*
     * Convert date specification (year with century, month 1-12, day 1-31,
     * and HH:MM:SS) to seconds since epoch (1 Jan 1970 00:00).
     */

    long days_since_epoch, secs_since_midnight;
    int y1, d1;

    /*
     * Rationalize year to the epoch.
     */
    y1 = year - 1970;

    /*
     * Calculate number of days since the epoch.
     */
    (void) cvt_mmddyy_to_dayofyear(month, day, year, &d1);
    days_since_epoch = y1*365 + (y1+1)/4 + d1;

    /*
     * Calculate number of seconds since midnight.
     */
    secs_since_midnight = ((hours*60) + mins)*60 + secs;

    /*
     * Calculate seconds since epoch.
     */
    return days_since_epoch*(24*60*60) + secs_since_midnight;
}


#ifdef _TEST /*{*/

int getitem(prompt, item)
char *prompt, *item;
{
	char buf[1024];
	printf("%s [%s] > ", prompt, item);
	fflush(stdout);
	if (gets(buf) == NULL)
		return -1;
	if (buf[0] != '\0')
		(void) strcpy(item, buf);
	return 0;
}

main()
{
	char dowstr[256], domstr[256], monstr[256], yrstr[256],
		tzstr[256], timestr[256];
	int dow, dom, mon, yr, tzmins, dayofyr, hrs, mins, secs;
	long gmttime;

	(void) strcpy(dowstr, "Monday");
	(void) strcpy(domstr, "1");
	(void) strcpy(monstr, "January");
	(void) strcpy(yrstr, "1980");
	(void) strcpy(tzstr, "GMT");
	(void) strcpy(timestr, "00:00:00");

	for (;;) {
		if (getitem("day of week", dowstr) != 0)
			break;
		if (getitem("month", monstr) != 0)
			break;
		if (getitem("day", domstr) != 0)
			break;
		if (getitem("year", yrstr) != 0)
			break;
		if (getitem("timezone", tzstr) != 0)
			break;
		if (getitem("time", timestr) != 0)
			break;
		if (!cvt_dayname_to_daynum(dowstr, &dow))
			fputs("cvt_dayname_to_daynum failed\n", stderr);
		if (!cvt_monthname_to_monthnum(monstr, &mon))
			fputs("cvt_monthname_to_monthnum failed\n", stderr);
		dom = atoi(domstr);
		if (!cvt_yearstr_to_yearnum(yrstr, &yr))
			fputs("cvt_yearstr_to_yearnum failed\n", stderr);
		if (!cvt_timezone_to_offset(tzstr, &tzmins))
			fputs("cvt_timezone_to_offset failed\n", stderr);
		if (!cvt_mmddyy_to_dayofyear(mon, dom, yr, &dayofyr))
			fputs("cvt_mmddyy_to_dayofyear failed\n", stderr);
		if (!cvt_timestr_to_hhmmss(timestr, &hrs, &mins, &secs))
			fputs("cvt_timestr_to_hhmmss failed\n", stderr);
		gmttime = make_gmttime(yr, mon, dom, hrs, mins+tzmins, secs);
		printf("date=%04d/%02d/%02d  time=%02d:%02d:%02d  tzmins=%d\n",
			yr, mon, dom, hrs, mins, secs, tzmins);
		printf("day-of-week=%d day-of-year=%d gmttime=%ld gmtdate=%s",
			dow, dayofyr, gmttime, ctime(&gmttime));
		putchar('\n');
	}

	putchar('\n');
	exit(0);
}

#endif /*}_TEST*/

