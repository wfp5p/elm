static char rcsid[] = "@(#)$Id: tail_of.c,v 1.4 1996/03/14 17:27:44 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
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
 * $Log: tail_of.c,v $
 * Revision 1.4  1996/03/14  17:27:44  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:41:45  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:00  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"


int
tail_of(from, buffer, to)
char *from, *buffer, *to;
{
	/** Return last two words of 'from'.  This is to allow
	    painless display of long return addresses as simply the
	    machine!username. 
	    Or if the first word of the 'from' address is username or
	    full_username and 'to' is not NULL, then use the 'to' line
	    instead of the 'from' line.
	    If the 'to' line is used, return 1, else return 0.

	    Also modified to know about X.400 addresses (sigh) and
	    that when we ask for the tail of an address similar to
	    a%b@c we want to get back a@b ...
	**/

	/** Note: '!' delimits Usenet nodes, '@' delimits ARPA nodes,
	          ':' delimits CSNet & Bitnet nodes, '%' delimits multi-
		  stage ARPA hops, and '/' delimits X.400 addresses...
	          (it is fortunate that the ASCII character set only has
	   	  so many metacharacters, as I think we're probably using
		  them all!!) **/

	register int loc, i = 0, cnt = 0, using_to = 0;

#ifndef INTERNET
	
	/** let's see if we have an address appropriate for hacking: 
	    what this actually does is remove the spuriously added
	    local bogus Internet header if we have one and the message
	    has some sort of UUCP component too...
	**/

	sprintf(buffer, "@%s", host_fullname); 
	if (chloc(from,'!') != -1 && strstr(from, buffer) != NULL)
	   from[strlen(from)-strlen(buffer)] = '\0';

#endif

	/**
	    Produce a simplified version of the from into buffer.  If the
	    from is just "username" or "Full Username" it will be preserved.
	    If it is an address, the rightmost "stuff!stuff", "stuff@stuff",
	    or "stuff:stuff" will be used.
	**/
	for (loc = strlen(from)-1; loc >= 0 && cnt < 2; loc--) {
	  if (from[loc] == '!' || from[loc] == '@' ||
	      from[loc] == ':') cnt++;
	  if (cnt < 2) buffer[i++] = from[loc];
	}
	buffer[i] = '\0';
	reverse(buffer);

	if ( strcmp(buffer, user_fullname) == 0 ||
	  addr_matches_user(buffer, user_name) ) {

	  /* This message is from the user, so use the "to" header instead
	   * if possible, to be more informative. Otherwise be nice and
	   * use user_fullname rather than the bare user_name even if
	   * we've only matched on the bare user_name.
	   */

	  if(to && *to != '\0' && !addr_matches_user(to, user_name)) {
	    tail_of(to, buffer, (char *)0);
	    using_to = 1;
	  } else
	    strcpy(buffer, user_fullname);

	} else {					/* user%host@host? */

	  /** The logic here is that we're going to use 'loc' as a handy
	      flag to indicate if we've hit a '%' or not.  If we have,
	      we'll rewrite it as an '@' sign and then when we hit the
	      REAL at sign (we must have one) we'll simply replace it
	      with a NULL character, thereby ending the string there.
	  **/

	  loc = 0;

	  for (i=0; buffer[i] != '\0'; i++)
	    if (buffer[i] == '%') {
	      buffer[i] = '@';
	      loc++;
	    }
	    else if (buffer[i] == '@' && loc)
	      buffer[i] = '\0';
	}
	return(using_to);

}
