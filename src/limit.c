

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
 * $Log: limit.c,v $
 * Revision 1.5  1996/03/14  17:29:40  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1995/09/29  17:42:16  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/09/11  15:19:12  wfp5p
 * Alpha 7
 *
 * Revision 1.2  1995/05/10  13:34:50  wfp5p
 * Added mailing list stuff by Paul Close <pdc@sgi.com>
 *
 * Revision 1.1.1.1  1995/04/19  20:38:37  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This stuff is inspired by MH and dmail and is used to 'select'
    a subset of the existing mail in the folder based on one of a
    number of criteria.  The basic tricks are pretty easy - we have
    as status of VISIBLE associated with each header stored in the
    (er) mind of the computer (!) and simply modify the commands to
    check that flag...the global variable `selected' is set to the
    number of messages currently selected, or ZERO if no select.
**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"
#include "s_aliases.h"

#define TO		1
#define FROM		2

static int limit_selection(int based_on, char *pattern,
			   int additional_criteria);
static int limit_alias_selection(int based_on, char *pattern,
				 int additional_criteria);

int limit(void)
{
	/** returns non-zero if we changed selection criteria = need redraw **/

	char criteria[STRING], first[STRING], rest[STRING], msg[STRING];
	static char *prompt = NULL;
	int  last_selected, all;

	last_selected = selected;
	all = 0;
	if (prompt == NULL) {
	  prompt = catgets(elm_msg_cat, ElmSet, ElmLimitEnterCriteria,
		"Enter criteria or '?' for help: ");
	}

	if (selected) {
	  MCsprintf(msg, catgets(elm_msg_cat, ElmSet, ElmLimitAlreadyHave,
		"Already have selection criteria - add more? (%c/%c) %c%c"),
		*def_ans_yes, *def_ans_no, *def_ans_no, BACKSPACE);
	  PutLine(LINES-2, 0, msg);
	  criteria[0] = ReadCh();
	  if (tolower(criteria[0]) == *def_ans_yes) {
	    PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmYesWord, "Yes."));
	    PutLine(LINES-3, COLS-30, catgets(elm_msg_cat, ElmSet, ElmLimitAdding,
		"Adding criteria..."));
	  } else {
	    PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmNoWord, "No."));
	    selected = 0;
	    PutLine(LINES-3, COLS-30, catgets(elm_msg_cat, ElmSet, ElmLimitChanging,
		"Change criteria..."));
	  }
	}

	while(1) {
	  PutLine(LINES-2, 0, prompt);
	  if (enter_string(criteria, sizeof(criteria), -1, -1, ESTR_ENTER) < 0
			|| criteria[0] == '\0') {
	    /* no change */
	    selected = last_selected;
	    return(FALSE);
	  }
	  clear_error();

	  split_word(criteria, first, rest);

	  if (inalias) {
	    if (streq(first, "?")) {
	      if (last_selected)
	        show_error(catgets(elm_msg_cat, AliasesSet, AliasesEnterLastSelected,
	          "Enter:{\"name\",\"alias\"} [pattern] OR {\"person\",\"group\",\"user\",\"system\"} OR \"all\""));
	      else
	        show_error(catgets(elm_msg_cat, AliasesSet, AliasesEnterSelected,
	          "Enter: {\"name\",\"alias\"} [pattern] OR {\"person\",\"group\",\"user\",\"system\"}"));
	      continue;
	    }
	    else if (streq(first, "all")) {
	      all++;
	      selected = 0;
	    }
	    else if (streq(first, "name"))
	      selected = limit_alias_selection(BY_NAME, rest, selected);
	    else if (streq(first, "alias"))
	      selected = limit_alias_selection(BY_ALIAS, rest, selected);
	    else if (streq(first, "person"))
	      selected = limit_alias_selection(PERSON, rest, selected);
	    else if (streq(first, "group"))
	      selected = limit_alias_selection(GROUP, rest, selected);
	    else if (streq(first, "user"))
	      selected = limit_alias_selection(USER, rest, selected);
	    else if (streq(first, "system"))
	      selected = limit_alias_selection(SYSTEM, rest, selected);
	    else {
	      show_error(catgets(elm_msg_cat, ElmSet, ElmLimitNotValidCriterion,
		"\"%s\" not a valid criterion."), first);
	      continue;
	    }
	    break;
	  }
	  else {
	    if (streq(first, "?")) {
	      if (last_selected)
	        show_error(catgets(elm_msg_cat, ElmSet, ElmEnterLastSelected,
	          "Enter: {\"subject\",\"to\",\"from\"} [pattern] OR \"all\""));
	      else
	        show_error(catgets(elm_msg_cat, ElmSet, ElmEnterSelected,
		  "Enter: {\"subject\",\"to\",\"from\"} [pattern]"));
	      continue;
	    }
	    else if (streq(first, "all")) {
	      all++;
	      selected = 0;
	    }
	    else if (streq(first, "subj") || streq(first, "subject"))
	      selected = limit_selection(SUBJECT, rest, selected);
	    else if (streq(first, "to"))
	      selected = limit_selection(TO, rest, selected);
	    else if (streq(first, "from"))
	      selected = limit_selection(FROM, rest, selected);
	    else {
	      show_error(catgets(elm_msg_cat, ElmSet, ElmLimitNotValidCriterion,
		"\"%s\" not a valid criterion."), first);
	      continue;
	    }
	    break;
	  }
	}

	if (all && last_selected)
	  strcpy(msg, catgets(elm_msg_cat, ElmSet, ElmLimitReturnToUnlimited,
		"Returned to unlimited display."));
	else {
	  if (inalias) {
	    if (selected > 1)
	      sprintf(msg, catgets(elm_msg_cat, AliasesSet,
		AliasesLimitMessagesSelected, "%d aliases selected."),
		selected);
	    else if (selected == 1)
	      strcpy(msg, catgets(elm_msg_cat, AliasesSet,
		AliasesLimitMessageSelected, "1 alias selected."));
	    else
	      strcpy(msg, catgets(elm_msg_cat, AliasesSet,
		AliasesLimitNoMessagesSelected, "No aliases selected."));
	  }
	  else {
	    if (selected > 1)
	      sprintf(msg, catgets(elm_msg_cat, ElmSet,
		ElmLimitMessagesSelected, "%d messages selected."), selected);
	    else if (selected == 1)
	      strcpy(msg, catgets(elm_msg_cat, ElmSet, ElmLimitMessageSelected,
		"1 message selected."));
	    else
	      strcpy(msg, catgets(elm_msg_cat, ElmSet,
		ElmLimitNoMessagesSelected, "No messages selected."));
	  }
	}
	set_error(msg);

	/* we need a redraw if there had been a selection or there is now. */
	if (last_selected || selected) {
	  /* if current message won't be on new display, go to first message */
	  if (inalias) {
	    if (selected && !(aliases[curr_alias-1]->status & VISIBLE))
	      curr_alias = visible_to_index(1)+1;
	  } else {
	    if (selected && !(curr_folder.headers[curr_folder.curr_mssg-1]->status & VISIBLE))
	      curr_folder.curr_mssg = visible_to_index(1)+1;
	  }
	  return(TRUE);
	} else {
	  return(FALSE);
	}
}

static int limit_selection(int based_on, char *pattern, int additional_criteria)
{
	/** Given the type of criteria, and the pattern, mark all
	    non-matching curr_folder.headers as ! VISIBLE.  If additional_criteria,
	    don't mark as visible something that isn't currently!
	**/

	register int iindex;
	register char *hdr_value;
	int count = 0;

	dprint(2, (debugfile, "\n\n\n**limit on %d - '%s' - (%s) **\n\n",
		   based_on, pattern, additional_criteria?"add'tl":"base"));

	for (iindex = 0 ; iindex < curr_folder.num_mssgs ; iindex++) {

	    switch (based_on) {
	    case FROM:
		hdr_value = curr_folder.headers[iindex]->allfrom;
		break;
	    case TO:
		hdr_value = curr_folder.headers[iindex]->allto;
		break;
	    case SUBJECT:
	    default:
		hdr_value = curr_folder.headers[iindex]->subject;
		break;
	    }

	    if (!patmatch(pattern, hdr_value, PM_NOCASE|PM_WSFOLD))
		curr_folder.headers[iindex]->status &= ~VISIBLE;
	    else if (additional_criteria && !(curr_folder.headers[iindex]->status&VISIBLE))
		; /* leave this marked not visible */
	    else {
		curr_folder.headers[iindex]->status |= VISIBLE;
		count++;
		dprint(5, (debugfile,
		    "  Message %d (%s from %s) marked as visible\n",
		    iindex, curr_folder.headers[iindex]->subject, curr_folder.headers[iindex]->from));
	    }

	}

	dprint(4, (debugfile, "\n** returning %d selected **\n\n\n", count));
	return(count);
}

static int limit_alias_selection(int based_on, char *pattern,
				 int additional_criteria)
{
	/** Given the type of criteria, and the pattern, mark all
	    non-matching aliases as ! VISIBLE.  If additional_criteria,
	    don't mark as visible something that isn't currently!
	**/

	register int iindex, count = 0;

	dprint(2, (debugfile, "\n\n\n**limit on %d - '%s' - (%s) **\n\n",
		   based_on, pattern, additional_criteria?"add'tl":"base"));

	if (based_on == BY_NAME) {
	  for (iindex = 0; iindex < num_aliases; iindex++)
	    if (! patmatch(pattern, aliases[iindex]->name, PM_NOCASE|PM_WSFOLD))
	      clearit(aliases[iindex]->status, VISIBLE);
	    else if (additional_criteria &&
		     !(aliases[iindex]->status & VISIBLE))
	      clearit(aliases[iindex]->status, VISIBLE);	/* shut down! */
	    else { /* mark it as readable */
	      setit(aliases[iindex]->status, VISIBLE);
	      count++;
	      dprint(5, (debugfile,
		     "  Alias %d (%s, %s) marked as visible\n",
			iindex, aliases[iindex]->alias,
			aliases[iindex]->name));
	    }
	}
	else if (based_on == BY_ALIAS) {
	  for (iindex = 0; iindex < num_aliases; iindex++)
	    if (!patmatch(pattern, aliases[iindex]->alias, PM_NOCASE|PM_WSFOLD))
	      clearit(aliases[iindex]->status, VISIBLE);
	    else if (additional_criteria &&
		     !(aliases[iindex]->status & VISIBLE))
	      clearit(aliases[iindex]->status, VISIBLE);	/* shut down! */
	    else { /* mark it as readable */
	      setit(aliases[iindex]->status, VISIBLE);
	      count++;
	      dprint(5, (debugfile,
			"  Alias %d (%s, %s) marked as visible\n",
			iindex, aliases[iindex]->alias,
			aliases[iindex]->name));
	    }
	}
	else {
	  for (iindex = 0; iindex < num_aliases; iindex++)
	    if (! (based_on & aliases[iindex]->type))
	      clearit(aliases[iindex]->status, VISIBLE);
	    else if (additional_criteria &&
		     !(aliases[iindex]->status & VISIBLE))
	      clearit(aliases[iindex]->status, VISIBLE);	/* shut down! */
	    else { /* mark it as readable */
	      setit(aliases[iindex]->status, VISIBLE);
	      count++;
	      dprint(5, (debugfile,
			"  Alias %d (%s, %s) marked as visible\n",
			iindex, aliases[iindex]->alias,
			aliases[iindex]->name));
	    }
	}

	dprint(4, (debugfile, "\n** returning %d selected **\n\n\n", count));

	return(count);
}

int next_message(register int iindex, register int skipdel)
{
	/** Given 'iindex', this routine will return the actual iindex into the
	    array of the NEXT message, or '-1' iindex is the last.
	    If skipdel, return the iindex for the NEXT undeleted message.
	    If selected, return the iindex for the NEXT message marked VISIBLE.
	**/

	register int remember_for_debug, stat;
	int item_count;

	if (iindex < 0) return(-1);	/* invalid argument value! */

	remember_for_debug = iindex;
	item_count = (inalias ? num_aliases : curr_folder.num_mssgs);

	for(iindex++;iindex < item_count; iindex++) {
	  stat = (inalias ?  aliases[iindex]->status : curr_folder.headers[iindex]->status);
	  if (((stat & VISIBLE) || (!selected))
	    && (!(stat & DELETED) || (!skipdel))) {
	      dprint(9, (debugfile, "[Next%s%s: given %d returning %d]\n",
		  (skipdel ? " undeleted" : ""),
		  (selected ? " visible" : ""),
		  remember_for_debug+1, iindex+1));
	      return(iindex);
	  }
	}

	return(-1);
}

int prev_message(register int iindex, register int skipdel)
{
	/** Like next_message, but the PREVIOUS message. **/

	register int remember_for_debug, stat;
	int item_count;

	item_count = (inalias ? num_aliases : curr_folder.num_mssgs);
	if (iindex >= item_count) return(-1);	/* invalid argument value! */

	remember_for_debug = iindex;
	for(iindex--; iindex >= 0; iindex--) {
	  stat = (inalias ? aliases[iindex]->status : curr_folder.headers[iindex]->status);
	  if (((stat & VISIBLE) || (!selected))
	    && (!(stat & DELETED) || (!skipdel))) {
	      dprint(9, (debugfile, "[Previous%s%s: given %d returning %d]\n",
		  (skipdel ? " undeleted" : ""),
		  (selected ? " visible" : ""),
		  remember_for_debug+1, iindex+1));
	      return(iindex);
	  }
	}
	return(-1);
}

int compute_visible(int message)
{
	/** return the 'virtual' iindex of the specified message in the
	    set of messages - that is, if we have the 25th message as
	    the current one, but it's #2 based on our limit criteria,
	    this routine, given 25, will return 2.
	**/

	register int iindex, count = 0;

	if (! selected) return(message);

	if (message < 1) message = 1;	/* normalize */

	if (inalias) {
	    for (iindex = 0; iindex < message; iindex++)
	       if (aliases[iindex]->status & VISIBLE)
		 count++;
	} else {
	    for (iindex = 0; iindex < message; iindex++)
	       if (curr_folder.headers[iindex]->status & VISIBLE)
		 count++;
	}

	dprint(4, (debugfile,
		"[compute-visible: displayed message %d is actually %d]\n",
		count, message));

	return(count);
}

int visible_to_index(int message)
{
	/** Given a 'virtual' iindex, return a real one.  This is the
	    flip-side of the routine above, and returns (count+1)
	    if it cannot map the virtual iindex requested (too big)
	**/

	register int iindex = 0, count = 0;
	int item_count;

	item_count = (inalias ? num_aliases : curr_folder.num_mssgs);

	for (iindex = 0; iindex < item_count; iindex++) {
	   if ((inalias ? aliases[iindex]->status : curr_folder.headers[iindex]->status)
		      & VISIBLE)
	     count++;
	   if (count == message) {
	     dprint(4, (debugfile,
		     "visible-to-index: (up) index %d is displayed as %d\n",
		     message, iindex));
	     return(iindex);
	   }
	}

	dprint(4, (debugfile, "index %d is NOT displayed!\n", message));

	return(item_count+1);
}
