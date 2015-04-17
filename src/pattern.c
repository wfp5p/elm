

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.7 $   $State: Exp $
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
 * $Log: pattern.c,v $
 * Revision 1.7  1998/02/11  22:02:15  wfp5p
 * Beta 2
 *
 * Revision 1.6  1996/05/09  15:51:24  wfp5p
 * Alpha 10
 *
 * Revision 1.5  1996/03/14  17:29:45  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1995/09/29  17:42:21  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/09/11  15:19:23  wfp5p
 * Alpha 7
 *
 * Revision 1.2  1995/06/12  20:33:35  wfp5p
 * Alpha 2 clean up
 *
 * Revision 1.1.1.1  1995/04/19  20:38:37  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**    General pattern matching for the ELM mailer.

**/


#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"
#include <assert.h>

/* local procedures */
static void ask_clear_existing_tags P_((void));
static int from_matches();
static int subject_matches();
static int name_matches();
static int alias_matches();
static int comment_matches();
static int address_matches();
static int match_in_message();

int meta_match(int function)
{
    /** Perform specific function based on whether an entered string
	matches either the From or Subject lines..
	Return TRUE if the current message was matched, else FALSE.
    **/

    int i, count, curtag;
    char *word_Action, *word_Actioned, *word_actioned;
    static char pat[SLEN];

    switch (function) {
    case MATCH_TAG:
	word_Action = catgets(elm_msg_cat, ElmSet, ElmTag,
		    "Tag");
	word_Actioned = catgets(elm_msg_cat, ElmSet, ElmCapTagged,
		    "Tagged");
	word_actioned = catgets(elm_msg_cat, ElmSet, ElmTagged,
		    "tagged");
	break;
    case MATCH_DELETE:
	word_Action = catgets(elm_msg_cat, ElmSet, ElmDelete,
		    "Delete");
	word_Actioned = catgets(elm_msg_cat, ElmSet, ElmCapMarkDelete,
		    "Marked for deletion");
	word_actioned = catgets(elm_msg_cat, ElmSet, ElmMarkDelete,
		    "marked for deletion");
	break;
    case MATCH_UNDELETE:
	word_Action = catgets(elm_msg_cat, ElmSet, ElmUndelete,
		    "Undelete");
	word_Actioned = catgets(elm_msg_cat, ElmSet, ElmCapUndeleted,
		    "Undeleted");
	word_actioned = catgets(elm_msg_cat, ElmSet, ElmUndeleted,
		    "undeleted");
	break;
    default:
	assert(function == MATCH_TAG || function == MATCH_DELETE
	    || function == MATCH_UNDELETE);
	return FALSE;
    }

    PutLine2(LINES-3, strlen(nls_Prompt), catgets(elm_msg_cat, ElmSet,
	    ElmMessagesMatchPattern, "%s %s that match pattern..."),
	    word_Action, nls_items);

    /* clear any existing tags? */
    if (function == MATCH_TAG)
	ask_clear_existing_tags();

    PutLine0(LINES-2, 0, catgets(elm_msg_cat, ElmSet, ElmEnterPattern,
	    "Enter pattern: "));
    if (enter_string(pat, sizeof(pat), -1, -1, ESTR_REPLACE) < 0
		|| pat[0] == '\0') {
      ClearLine(LINES-3);
      ClearLine(LINES-2);
      return FALSE;
    }
    strcpy(pat, shift_lower(pat));

    count = 0;
    curtag = FALSE;
    if (inalias) {

	for (i = 0; i < num_aliases; i++) {

	    if (!name_matches(i, pat) && !alias_matches(i, pat))
		continue;
	    if (selected && !(aliases[i]->status & VISIBLE))
		continue;

	    switch (function) {
	    case MATCH_DELETE:
		if (aliases[i]->type & SYSTEM)
		    continue;		/* don't delete these!!! */
		aliases[i]->status |= DELETED;
		break;
	    case MATCH_UNDELETE:
		aliases[i]->status &= ~DELETED;
		break;
	    case MATCH_TAG:
		aliases[i]->status |= TAGGED;
		break;
	    }

	    show_new_status(i);
	    if (curr_alias == i+1)
		curtag = TRUE;
	    count++;

	}


    } else {

	for (i = 0; i < curr_folder.num_mssgs; i++) {

	    if (!from_matches(i, pat) && !subject_matches(i, pat))
		continue;
	    if (selected && !(curr_folder.headers[i]->status & VISIBLE))
		continue;

	    switch (function) {
	    case MATCH_DELETE:
		curr_folder.headers[i]->status |= DELETED;
		break;
	    case MATCH_UNDELETE:
		curr_folder.headers[i]->status &= ~DELETED;
		break;
	    case MATCH_TAG:
		curr_folder.headers[i]->status |= TAGGED;
		break;
	    }

	    show_new_status(i);
	    if (curr_folder.curr_mssg == i+1)
		curtag++;
	    count++;

	}

    }

    MoveCursor(LINES-3, 0);
    CleartoEOS();
    switch (count) {
    case 0:
	error2(catgets(elm_msg_cat, ElmSet, ElmNoMatchesNoTags,
		    "No matches. No %s %s."), nls_items, word_actioned);
	break;
    case 1:
	error2(catgets(elm_msg_cat, ElmSet, ElmTaggedMessage,
		    "%s 1 %s."),  word_Actioned, nls_item);
	break;
    default:
	error3(catgets(elm_msg_cat, ElmSet, ElmTaggedMessages,
		    "%s %d %s."), word_Actioned, count, nls_items);
	break;
    }

    return(curtag);
}

static void ask_clear_existing_tags(void)
{
    int tagged, i;
    char tagmsg[SLEN], msg[SLEN];

    tagged = 0;
    if (inalias) {
	for (i=0; i < num_aliases; i++) {
	    if (aliases[i]->status & TAGGED)
		tagged++;
	}
    } else {
	for (i=0; i < curr_folder.num_mssgs; i++) {
	    if (curr_folder.headers[i]->status & TAGGED)
		tagged++;
	}
    }

    if (tagged == 0)
	return;

    if (tagged > 1) {
	MCsprintf(tagmsg, catgets(elm_msg_cat, ElmSet, ElmSomeMessagesATagged,
		    "Some %s are already tagged."), nls_items);
	MCsprintf(msg, catgets(elm_msg_cat, ElmSet, ElmRemoveTags,
		    "%s Remove Tags?"), tagmsg);
    } else {
	MCsprintf(tagmsg, catgets(elm_msg_cat, ElmSet, ElmAMessageATagged,
		    "One %s is already tagged."), nls_item);
	MCsprintf(msg, catgets(elm_msg_cat, ElmSet, ElmRemoveTag,
		    "%s Remove Tag?"), tagmsg);
    }

    if (enter_yn(msg, TRUE, LINES-2, FALSE)) {
	if (inalias) {
	    for (i=0; i < num_aliases; i++) {
		aliases[i]->status &= ~TAGGED;
		show_new_status(i);
	    }
	} else {
	    for (i=0; i < curr_folder.num_mssgs; i++) {
		curr_folder.headers[i]->status &= ~TAGGED;
		show_new_status(i);
	    }
	}
    }

}


/*
 * Select message based upon pattern supplied by user.
 *
 * Normally, matching is performed on the from and subject lines.
 * If the user strikes "/" then an alternate pattern is selected,
 * and matching is performed against the entire message (headers
 * and body).  This makes things a bit twisty, because we need
 * to get that first character and then decide what to do from
 * there.
 *
 * If a match is found, the selection is updated and TRUE is
 * displayed.  If matching fails then an error is displayed
 * and FALSE is returned.  If the user aborts the match then
 * we silently return FALSE.
 */
int pattern_match(void)
{
    char inpbuf[SLEN], *sel_pat;
    int inp_line, inp_col, anywhere, matched, ch, i;
    static char hdr_pattern[SLEN];
    static char body_pattern[SLEN];

    PutLine1(LINES-3, 40, catgets(elm_msg_cat, ElmSet, ElmMatchAnywhere,
		"/ = Match anywhere in %s."), nls_items);
    PutLine0(LINES-1, 0, catgets(elm_msg_cat, ElmSet, ElmMatchPattern,
		"Match pattern: "));
    GetCursorPos(&inp_line, &inp_col);
    PutLine0(-1, -1, hdr_pattern);
    CleartoEOLN();

    ch = ReadCh();
    switch (ch) {

    case ctrl('D'):	/* abort */
	ClearLine(LINES-1);
	return FALSE;

    case '\n':		/* accept current pattern */
    case '\r':
	if (hdr_pattern[0] == '\0') {
	    ClearLine(LINES-1);
	    return FALSE;
	}
	sel_pat = hdr_pattern;
	anywhere = FALSE;
	break;

    case '/':		/* switch patterns and match against entire message */
	MoveCursor(LINES-3, 40);
	CleartoEOLN();
	PutLine1(LINES-1, 0, catgets(elm_msg_cat, ElmSet,
		    ElmMatchPatternInEntire, "Match pattern (in entire %s): "),
		    nls_item);
	(void) strcpy(inpbuf, body_pattern);
	if (enter_string(inpbuf, sizeof(inpbuf), -1, -1, ESTR_UPDATE) < 0
		    || inpbuf[0] == '\0') {
	    ClearLine(LINES-1);
	    return FALSE;
	}
	sel_pat = strcpy(body_pattern, inpbuf);
	anywhere = TRUE;
	break;

    default:		/* edit the pattern */
	UnreadCh(ch); /* push back - let enter_string() deal with it */
	(void) strfcpy(inpbuf, hdr_pattern, sizeof(inpbuf));
	if (enter_string(inpbuf, sizeof(inpbuf), inp_line, inp_col,
		    ESTR_UPDATE) < 0 || inpbuf[0] == '\0') {
	    ClearLine(LINES-1);
	    return FALSE;
	}
	sel_pat = strcpy(hdr_pattern, inpbuf);
	anywhere = FALSE;
	break;

    }

    if (inalias) {
	for (i = curr_alias; i < num_aliases; i++) {
	    if (!selected || aliases[i]->status & VISIBLE) {
		matched = name_matches(i, sel_pat) || alias_matches(i, sel_pat);
		if (! matched && anywhere) {	/* Look only if no match yet */
		    matched = comment_matches(i, sel_pat) ||
			    address_matches(i, sel_pat);
		}
		if (matched) {
		    curr_alias = i+1;
		    return TRUE;
		}
	    }
	}
	goto not_matched;
    }

    if (anywhere) {
	if (match_in_message(sel_pat))
	    return TRUE;
	goto not_matched;
    }

    for (i = curr_folder.curr_mssg; i < curr_folder.num_mssgs; i++) {
	if (!selected || curr_folder.headers[i]->status & VISIBLE) {
	    if (from_matches(i, sel_pat) || subject_matches(i, sel_pat)) {
		curr_folder.curr_mssg = i+1;
		return TRUE;
	    }
	}
    }

not_matched:
    error(catgets(elm_msg_cat, ElmSet, ElmPatternNotFound,
		"pattern not found!"));
    return FALSE;
}


/*
 * Local Procedures
 */
static int from_matches(int message_number, char *pat)
{
    return patmatch(pat, curr_folder.headers[message_number]->from, PM_NOCASE|PM_WSFOLD);
}

static int subject_matches(int message_number, char *pat)
{
    return patmatch(pat, curr_folder.headers[message_number]->subject, PM_NOCASE|PM_WSFOLD);
}

static int name_matches(int message_number, char *pat)
{
    return patmatch(pat, aliases[message_number]->name, PM_NOCASE|PM_WSFOLD);
}

static int alias_matches(int message_number, char *pat)
{
    return patmatch(pat, aliases[message_number]->alias, PM_NOCASE|PM_WSFOLD);
}

static int comment_matches(int message_number, char *pat)
{
    return patmatch(pat, aliases[message_number]->comment, PM_NOCASE|PM_WSFOLD);
}

static int address_matches(int message_number, char *pat)
{
    char *exp;
    int dummy;

    exp = get_alias_address(aliases[message_number]->alias, TRUE, &dummy);
    return (exp != NULL && patmatch(pat, exp, PM_NOCASE|PM_WSFOLD));
}

static int match_in_message(char *pat)
{
	/** Match a string INSIDE a message...starting at the current
	    message read each line and try to find the pattern.  As
	    soon as we do, set current and leave!
	    Returns 1 if found, 0 if not
	**/

	char buffer[VERY_LONG_STRING];
	int  message_number, lines, line, line_len, err;

	message_number = curr_folder.curr_mssg-1;

	error(catgets(elm_msg_cat, ElmSet, ElmSearchingFolderPattern,
		"Searching folder for pattern..."));

	for ( ; message_number < curr_folder.num_mssgs; message_number++) {

	  /*  if limited, search only selected messages */
	  if (selected &&
		  !(curr_folder.headers[message_number]->status & VISIBLE))
	    continue;

	  if (fseek(curr_folder.fp, curr_folder.headers[message_number]->offset, 0L) == -1) {

	    err = errno;
	    dprint(1, (debugfile,
		"Error: seek %ld bytes into file failed. errno %d (%s)\n",
		curr_folder.headers[message_number]->offset, err,
		"match_in_message"));
	    error2(catgets(elm_msg_cat, ElmSet, ElmMatchSeekFailed,
		   "ELM [match] failed looking %ld bytes into file (%s)."),
		   curr_folder.headers[message_number]->offset, strerror(err));
	    return TRUE; /* fake it out to avoid replacing error message */
	  }

	  line = 0;
	  lines = curr_folder.headers[message_number]->lines;

	  while ((line_len = mail_gets(buffer, VERY_LONG_STRING, curr_folder.fp)) &&
		line < lines) {

	    if(buffer[line_len - 1] == '\n') line++;

	    if (patmatch(pat, buffer, PM_NOCASE|PM_WSFOLD)) {
	      curr_folder.curr_mssg = message_number+1;
	      clear_error();
	      return TRUE;
	    }
	  }
	}

	return FALSE;
}
