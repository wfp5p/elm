

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.5 $   $State: Exp $
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
 * $Log: strincmp.c,v $
 * Revision 1.5  1995/09/29  17:41:42  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.4  1995/09/11  15:19:00  wfp5p
 * Alpha 7
 *
 * Revision 1.3  1995/07/18  18:59:51  wfp5p
 * Alpha 6
 *
 * Revision 1.2  1995/06/22  14:48:38  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** compare strings ignoring case
**/

#include "elm_defs.h"

static char lwtab[1<<(8*sizeof(char))];
static int first_time = TRUE;

static void setup_lwtab()
{
	int c;
	for (c = 1<<(8*sizeof(char)) - 1; c >= 0; c--)
	    lwtab[c] = tolower(c);
	first_time = FALSE;
}


int
strincmp(s1,s2,n)
register const char *s1, *s2;
register int n;
{
	register int d;

	if (first_time)
	  setup_lwtab();

#define CHECKSTR \
    if ((d = (lwtab[*s1] - lwtab[*s2])) != 0 || *s1 == '\0' || *s2 == '\0' ) \
	return d; \
    else \
	++s1, ++s2

	switch (n & 07) {
	    case 7:	CHECKSTR;
	    case 6:	CHECKSTR;
	    case 5:	CHECKSTR;
	    case 4:	CHECKSTR;
	    case 3:	CHECKSTR;
	    case 2:	CHECKSTR;
	    case 1:	CHECKSTR;
	}
	for (n >>= 3 ; n > 0 ; --n) {
	  CHECKSTR;
	  CHECKSTR;
	  CHECKSTR;
	  CHECKSTR;
	  CHECKSTR;
	  CHECKSTR;
	  CHECKSTR;
	  CHECKSTR;
	}

	return 0;
}


int
istrcmp(s1,s2)
register const char *s1, *s2;
{
	register int d;

	if (first_time)
	  setup_lwtab();

	for (;;) {
	  d = (lwtab[*s1] - lwtab[*s2]);
	  if ( d != 0 || *s1 == '\0' || *s2 == '\0' )
	    return d;
	  ++s1;
	  ++s2;
	}
	/*NOTREACHED*/
}

