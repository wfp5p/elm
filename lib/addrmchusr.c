

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
 * $Log: addrmchusr.c,v $
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

int
addr_matches_user(addr,user)
register char *addr;
register const char *user;
{
	int len = strlen(user);
	static char c_before[] = "!:%";	/* these can appear before a username */
	static char c_after[] = ":%@";	/* these can appear after a username  */

	do {
	  if ( strncmp(addr,user,len) == 0 ) {
	    if ( addr[len] == '\0' || index(c_after,addr[len]) != NULL )
	      return TRUE;
	  }
	} while ( (addr=qstrpbrk(addr,c_before)) != NULL && *++addr != '\0' ) ;
	return FALSE;
}
