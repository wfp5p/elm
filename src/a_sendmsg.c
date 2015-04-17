#include "elm_defs.h"
#include "elm_globals.h"

int a_sendmsg(void)
{
	/** Prompt for fields and then call send_message() to send the
	    specified message.  Return TRUE if the main part of the screen
	    has been changed and requires redraw.
	**/

	int rc, num_tagged, bufsiz, len, i;
	char *cp;
	char given_to[SLEN];

	num_tagged = 0;

	bufsiz = sizeof(given_to)-1; /* reserve space for '\0' */
	cp = given_to;
	for (i = 0 ; bufsiz > 1 && i < num_aliases ; ++i) {

	  if (!ison(aliases[i]->status, TAGGED))
	    continue;

	  if (num_tagged++ > 0) {
	      *cp++ = ' ';
	      --bufsiz;
	  }
	  strfcpy(cp, aliases[i]->alias, bufsiz);

	  len = strlen(aliases[i]->alias);
	  bufsiz -= len;
	  if (bufsiz > 0)
	      cp += len;

	}

	if (num_tagged == 0)
	  strfcpy(given_to, aliases[curr_alias-1]->alias, sizeof(given_to));

	dprint(4, (debugfile, "%d aliases tagged for mailing (a_sndmsg)\n",
	        num_tagged));

	main_state();
	rc = send_message(given_to, (char *)NULL, (char *)NULL, SM_ORIGINAL);
	main_state();

/*
 *	Since we got this far, it must be okay to clear the tags.
 */
	for (i = 0 ; num_tagged > 0 && i < num_aliases ; ++i) {
	    if (ison(aliases[i]->status, TAGGED)) {
	        clearit(aliases[i]->status, TAGGED);
	        show_msg_tag(i);
	        --num_tagged;
	    }
	}

	return rc;
}

