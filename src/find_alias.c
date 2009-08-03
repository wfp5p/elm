
static char rcsid[] = "@(#)$Id: find_alias.c,v 1.3 1996/03/14 17:29:35 wfp5p Exp $";

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
 * $Log: find_alias.c,v $
 * Revision 1.3  1996/03/14  17:29:35  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:11  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:36  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 
	Search the list of aliases for a specific address.  Search
	is limited to either SYSTEM or USER alias types....
**/

#include "elm_defs.h"
#include "elm_globals.h"

extern int num_duplicates;

int
find_alias(word, alias_type)
char *word;
int alias_type;
{
	/** find word and return loc, or -1 **/
	register int loc = -1;

	/** cannot be an alias if its longer than NLEN chars **/
	if (strlen(word) > NLEN)
	    return(-1);

	while (++loc < (num_aliases+num_duplicates)) {
	    if ( aliases[loc]->type & alias_type ) {
	        if (istrcmp(word, aliases[loc]->alias) == 0)
	            return(loc);
	    }
	}

	return(-1);				/* Not found */
}
