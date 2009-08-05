
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
 * $Log: okay_addr.c,v $
 * Revision 1.5  1996/03/14  17:27:42  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1995/09/29  17:41:23  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/09/11  15:18:56  wfp5p
 * Alpha 7
 *
 * Revision 1.2  1995/06/22  14:48:37  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

#define before_okay(c)	\
	((c) == 0 || (c) == '<' || (c) == '!' || (c) == ':' || (c) == '%' || \
	 (c) == ' ' || (c) == ',')
#define after_okay(c)	\
	((c) == 0 || (c) == '>' || (c) == ':' || (c) == '%' || (c) == '@' || \
	 (c) == ' ' || (c) == ',' || (c) == '\r' || (c) == '\n')

int
okay_address(address, return_address)
char *address, *return_address;
{
	/** This routine checks to ensure that the address we just got
	    from the "To:" or "Cc:" line isn't us AND isn't the person	
	    who sent the message.  Returns true iff neither is the case **/

	static int first_time = TRUE;
	static int host_equal_hostfull;
	static int userlen, hostlen, hostfulllen;

	struct addr_rec  *alternatives;

	char *usrp;

	if (first_time) {
	  /* username and hostname don't change, so do this only once */
	  host_equal_hostfull = (istrcmp(host_name, host_fullname) == 0);
	  userlen = strlen(user_name);
	  hostlen = strlen(host_name);
	  hostfulllen = strlen(host_fullname);
	  first_time = FALSE;
	}

	/* check for return_address */

	if (return_address && in_list(address, return_address))
	  return(FALSE);

	/* check for username, optionally combined with hostname(s) */

	if ((usrp = strstr(address, user_name)) != NULL) {
	  char pre, post;
	  pre = (usrp == address)? 0: usrp[-1];
	  post = usrp[userlen];
	  if (before_okay(pre) && after_okay(post)) {
	    /* we have a valid username */
	    /* look for user@host or user%host */
	    if (post == '@' || post == '%') {
	      char *maybehost = usrp+userlen+1;
	      if (strincmp(maybehost, host_name, hostlen) == 0 &&
		  after_okay(maybehost[hostlen])) {
		/* got user@host */
		return FALSE;
	      }
	      if (! host_equal_hostfull &&
		  strincmp(maybehost, host_fullname, hostfulllen) == 0 &&
		  after_okay(maybehost[hostfulllen])) {
		/* got user@fullhost */
		return FALSE;
	      }
	    }
	    /* look for host!user */
	    else if (pre == '!') {
	      char *maybehost = usrp-hostlen-1;
	      if ((maybehost == address ||
		   (maybehost > address && before_okay(maybehost[-1]))) &&
	          strincmp(maybehost, host_name, hostlen) == 0) {
		/* got host!user */
		return FALSE;
	      }
	      maybehost = usrp-hostfulllen-1;
	      if (! host_equal_hostfull &&
		  (maybehost == address ||
		   (maybehost > address && before_okay(maybehost[-1]))) &&
	          strincmp(maybehost, host_fullname, hostfulllen) == 0) {
		/* got fullhost!user */
		return FALSE;
	      }
	    }
	    else {
	      /* not preceded by ! or followed by [@%] -- just username */
	      return FALSE;
	    }
	  }
	}

	/* didn't find valid username, look for alternates */

	alternatives = alternative_addresses;

	while (alternatives != NULL) {
	  if (in_list(address, alternatives->address))
	    return(FALSE);
	  alternatives = alternatives->next;
	}

	return(TRUE);
}
