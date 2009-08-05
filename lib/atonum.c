

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: atonum.c,v $
 * Revision 1.2  1995/09/29  17:41:03  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

/*
 * This is similar to atoi(), but it complains if the string
 * contains any non-numeric characters.  Returns the numeric
 * value on success, -1 on error.
 */
int atonum(str)
register const char *str;
{
    register int value;

    if (*str == '\0')
	return -1;
    value = 0;
    while (isdigit(*str))
	value = (value*10) + (*str++ - '0');
    return (*str == '\0' ? value : -1);
}


#ifdef _TEST
main()
{
	char buf[1024];
	while (gets(buf) != NULL)
		printf("atonum(%s) = %d\n", buf, atonum(buf));
	putchar('\n');
	exit(0);
}
#endif

