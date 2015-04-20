

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
 * $Log: sort.c,v $
 * Revision 1.3  1996/03/14  17:29:58  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:33  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:39  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** Sort folder header table by the field specified in the global
    variable "sortby"...if we're sorting by something other than
    the default SENT_DATE, also put some sort of indicator on the
    screen.

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

static char *skip_re(char *string);

void find_old_current(int iindex)
{
	/** Set current to the message that has "index" as it's
	    index number.  This is to track the current message
	    when we resync... **/

	register int i;

	dprint(4, (debugfile, "find-old-current(%d)\n", iindex));

	for (i = 0; i < curr_folder.num_mssgs; i++)
	  if (curr_folder.headers[i]->index_number == iindex) {
	    dprint(4, (debugfile, "\tset current to %d!\n", i+1));
	    if (inalias)
	      curr_alias = i+1;
	    else
	      curr_folder.curr_mssg = i+1;
	    return;
	  }

	return;		/* can't be found.  Leave it alone, then */
}

int sort_mailbox(int entries, int visible)
{
	/** Sort the header_table definitions... If 'visible', then
	    put the status lines etc **/

	int last_index = -1;
	int compare_headers();	/* for sorting */

	dprint(2, (debugfile, "\n** sorting folder by %s **\n\n",
		sort_name(TRUE)));

	/* Don't get last_index if no entries or no current. */
	/* There would be no current if we are sorting a new mail file. */
	if (entries > 0 && curr_folder.curr_mssg > 0)
	  last_index = curr_folder.headers[curr_folder.curr_mssg-1]->index_number;

	if (entries > 30 && visible)
	  show_error(catgets(elm_msg_cat, ElmSet, ElmSortingMessagesBy,
		"Sorting messages by %s..."), sort_name(TRUE));

	if (entries > 1)
	  qsort((char *) curr_folder.headers, (unsigned) entries,
	      sizeof (struct header_rec *) , compare_headers);

	if (last_index > -1)
	  find_old_current(last_index);

	clear_error();
}

int compare_headers(struct header_rec **p1, struct header_rec **p2)
{
	/** compare two headers according to the sortby value.

	    Sent Date uses a routine to compare two dates,
	    Received date is keyed on the file offsets (think about it)
	    Sender uses the truncated from line, same as "build headers",
	    and size and subject are trivially obvious!!
	    (actually, subject has been modified to ignore any leading
	    patterns [rR][eE]*:[ \t] so that replies to messages are
	    sorted with the message (though a reply will always sort to
	    be 'greater' than the basenote)
	 **/

	char from1[SLEN], from2[SLEN];	/* sorting buffers... */
	struct header_rec *first, *second;
	int ret;
	long diff;

	first = *p1;
	second = *p2;

	switch (abs(sortby)) {
	case SENT_DATE:
 		diff = first->time_sent - second->time_sent;
 		if ( diff < 0 )	ret = -1;
 		else if ( diff > 0 ) ret = 1;
 		else ret = 0;
  		break;

	case RECEIVED_DATE:
 		diff = first->received_time - second->received_time;
 		if ( diff < 0 )	ret = -1;
 		else if ( diff > 0 ) ret = 1;
 		else ret = 0;
  		break;

	case SENDER:
		tail_of(first->from, from1, first->to);
		tail_of(second->from, from2, second->to);
		ret = strcmp(from1, from2);
		break;

	case SIZE:
		ret = (first->lines - second->lines);
		break;

	case MAILBOX_ORDER:
		ret = (first->index_number - second->index_number);
		break;

	case SUBJECT:
		/* need some extra work 'cause of STATIC buffers */
		strcpy(from1, skip_re(shift_lower(first->subject)));
		ret = strcmp(from1, skip_re(shift_lower(second->subject)));
		break;

	case STATUS:
		ret = (first->status - second->status);
		break;

	default:
		/* never get this! */
		ret = 0;
		break;
	}

	/* on equal status, use sent date as second sort param. */
	if (ret == 0) {
		diff = first->time_sent - second->time_sent;
		if ( diff < 0 )	ret = -1;
		else if ( diff > 0 ) ret = 1;
		else ret = 0;
	}

	if (sortby < 0)
	  ret = -ret;

	return ret;
}

char *sort_name(int longname)
{
    if (sortby < 0) {
	switch (-sortby) {
	case SENT_DATE: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongRevDateMailSent,
		"Reverse Date Mail Sent")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrtRevDateMailSent,
		"Reverse-Sent"));
	case RECEIVED_DATE: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongRevRecv,
		"Reverse Date Mail Rec'vd")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrRevRecv,
		"Reverse-Received"));
	case MAILBOX_ORDER: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongRevMailbox,
		"Reverse Mailbox Order")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrRevMailbox,
		"Reverse-Mailbox"));
	case SENDER: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongRevSender,
		"Reverse Message Sender")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrRevSender,
		"Reverse-From"));
	case SIZE: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongRevLines,
		"Reverse Lines in Message")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrRevLines,
		"Reverse-Lines"));
	case SUBJECT: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongRevSubject,
		"Reverse Message Subject")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrRevSubject,
		"Reverse-Subject"));
	case STATUS: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongRevStatus,
		"Reverse Message Status")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrRevStatus,
		"Reverse-Status"));
	}
    } else {
	switch (sortby) {
	case SENT_DATE: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongMailSent,
		"Date Mail Sent")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrMailSent,
		"Sent"));
	case RECEIVED_DATE: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongMailRecv,
		"Date Mail Rec'vd")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrMailRecv,
		"Received"));
	case MAILBOX_ORDER: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongMailbox,
		"Mailbox Order")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrMailbox,
		"Mailbox"));
	case SENDER: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongSender,
		"Message Sender")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrSender,
		"From"));
	case SIZE: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongLines,
		"Lines in Message")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrLines,
		"Lines"));
	case SUBJECT: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongSubject,
		"Message Subject")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrSubject,
		"Subject"));
	case STATUS: return (longname
	    ? catgets(elm_msg_cat, ElmSet, ElmLongStatus,
		"Message Status")
	    : catgets(elm_msg_cat, ElmSet, ElmAbrStatus,
		"Status"));
	}
    }

    return catgets(elm_msg_cat, ElmSet, ElmSortUnknown,
		"*UNKNOWN-SORT-PARAMETER*");
}

static char *skip_re(char *string)
{
	/** this routine returns the given string minus any sort of
	    "re:" prefix.  specifically, it looks for, and will
	    remove, any of the pattern:

		( [Rr][Ee][^:]:[ ] ) *

	    If it doesn't find a ':' in the line it will return it
	    intact, just in case!
	**/

	static char buffer[SLEN];
	register int i=0;

	while (whitespace(string[i])) i++;

	/* Initialize buffer */
	strcpy(buffer, (char *) string);

	do {
	  if (string[i] == '\0') return( (char *) buffer);	/* forget it */

	  if (string[i] != 'r' || string[i+1] != 'e')
	    return( (char *) buffer);				/*   ditto   */

	  i += 2;	/* skip the "re" */

	  while (string[i] != ':')
	    if (string[i] == '\0')
	      return( (char *) buffer);		      /* no colon in string! */
	    else
	      i++;

	  /* now we've gotten to the colon, skip to the next non-whitespace  */

	  i++;	/* past the colon */

	  while (whitespace(string[i])) i++;

	  /* Now save the resulting subject for return purposes */
	  strcpy(buffer, (char *) string + i);

	} while (string[i] == 'r' && string[i+1] == 'e');

	return( (char *) buffer);
}
