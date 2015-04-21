

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
 * $Log: screen.c,v $
 * Revision 1.7  1996/03/14  17:29:50  wfp5p
 * Alpha 9
 *
 * Revision 1.6  1995/09/29  17:42:26  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.5  1995/09/11  15:19:30  wfp5p
 * Alpha 7
 *
 * Revision 1.4  1995/06/15  13:09:33  wfp5p
 * Changed so the local mlist files adds to the global one instead of
 * overriding it. (Paul Close <pdc@sgi.com>)
 *
 * Revision 1.3  1995/05/10  13:34:53  wfp5p
 * Added mailing list stuff by Paul Close <pdc@sgi.com>
 *
 * Revision 1.2  1995/04/20  21:01:50  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**  screen display routines for ELM program

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

extern char version_buff[];

static int fix_header_page(void);
static int build_header_line(char *buffer, struct header_rec *entry,
			     int message_number, int highlight, char *from,
			     int really_to);

int showscreen(void)
{

	ClearScreen();

	update_title();

	last_header_page = -1;	 	/* force a redraw regardless */
	show_headers();

	if (mini_menu)
	  show_menu();

	show_last_error();
}

int update_title(void)
{
	/** display a new title line, probably due to new mail arriving **/

	char buffer[SLEN], folder_string[SLEN];
	static char *folder = NULL, *mailbox = NULL;

	if (folder == NULL) {
		folder = catgets(elm_msg_cat, ElmSet, ElmFolder, "Folder");
		mailbox = catgets(elm_msg_cat, ElmSet, ElmMailbox, "Mailbox");
	}

/* FIXME make this user configureable
/* #ifdef DISP_HOST */
/* 	sprintf(folder_string, "%s:%s", host_name, nameof(curr_folder.filename)); */
/* #else */

	strcpy(folder_string, nameof(curr_folder.filename));


	if (selected)
	  MCsprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmShownWithSelect,
	      "%s is '%s' with %d shown out of %d [ELM %s]"),
	      ((curr_folder.flags & FOLDER_IS_SPOOL) ? mailbox : folder),
	      folder_string, selected, curr_folder.num_mssgs, version_buff);
	else if (curr_folder.num_mssgs == 1)
	  MCsprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmShownNoSelect,
	      "%s is '%s' with 1 message [ELM %s]"),
	      ((curr_folder.flags & FOLDER_IS_SPOOL) ? mailbox : folder),
	      folder_string, version_buff);
	else
	  MCsprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmShownNoSelectPlural,
	      "%s is '%s' with %d messages [ELM %s]"),
	      ((curr_folder.flags & FOLDER_IS_SPOOL) ? mailbox : folder),
	      folder_string, curr_folder.num_mssgs, version_buff);

	ClearLine(1);
	CenterLine(1, buffer);
}

int show_menu(void)
{
	/** write main system menu... **/

	if (user_level == 0) {	/* a rank beginner.  Give less options  */
	  CenterLine(LINES-7, catgets(elm_msg_cat, ElmSet, ElmLevel0MenuLine1,
  "You can use any of the following commands by pressing the first character;"));
          CenterLine(LINES-6, catgets(elm_msg_cat, ElmSet, ElmLevel0MenuLine2,
"d)elete or u)ndelete mail,  m)ail a message,  r)eply or f)orward mail,  q)uit"));
	  CenterLine(LINES-5, catgets(elm_msg_cat, ElmSet, ElmLevel0MenuLine3,
  "To read a message, press <return>.  j = move down, k = move up, ? = help"));
	} else {
	CenterLine(LINES-7, catgets(elm_msg_cat, ElmSet, ElmLevel1MenuLine1,
  "|=pipe, !=shell, ?=help, <n>=set current to n, /=search pattern"));
        CenterLine(LINES-6, catgets(elm_msg_cat, ElmSet, ElmLevel1MenuLine2,
"a)lias, C)opy, c)hange folder, d)elete, e)dit, f)orward, g)roup reply, m)ail,"));
	CenterLine(LINES-5, catgets(elm_msg_cat, ElmSet, ElmLevel1MenuLine3,
  "n)ext, o)ptions, p)rint, q)uit, r)eply, s)ave, t)ag, u)ndelete, or e(x)it"));
	}
}

int show_headers(void)
{
	/** Display page of headers (10) if present.  First check to
	    ensure that header_page is in bounds, fixing silently if not.
	    If out of bounds, return zero, else return non-zero
	    Modified to only show headers that are "visible" to ze human
	    person using ze program, eh?
	**/

	register int this_msg = 0, line = 4, last = 0, last_line,
		     displayed = 0, using_to;
	int max, do_standout;
	char newfrom[SLEN], buffer[SLEN];

	max = (inalias ? num_aliases : curr_folder.num_mssgs);
        headers_per_page = LINES - (mini_menu ? 13 : 8);
        if (headers_per_page < 1)
	    headers_per_page = 1;

	if (fix_header_page())
	  return(FALSE);

	if (selected) {
	  if ((header_page*headers_per_page) > selected)
	    return(FALSE); 	/* too far! too far! */

	  this_msg = visible_to_index(header_page * headers_per_page + 1);
	  displayed = header_page * headers_per_page;

	  last = displayed+headers_per_page;

	}
	else {
	  if (header_page == last_header_page) 	/* nothing to do! */
	    return(FALSE);

	  /** compute last header to display **/

	  this_msg = header_page * headers_per_page;
	  last = this_msg + (headers_per_page - 1);
	}

	if (last >= max)
	    last = max-1;

	/** Okay, now let's show the header page! **/

	ClearLine(line);	/* Clear the top line... */

	MoveCursor(line, 0);	/* and move back to the top of the page... */

	while ((selected && displayed < last) || this_msg <= last) {
	  if (inalias) {
	    if (this_msg == curr_alias-1)
	      build_alias_line(buffer, aliases[this_msg], this_msg+1,
			       TRUE);
	    else
	      build_alias_line(buffer, aliases[this_msg], this_msg+1,
			       FALSE);
	  }
	  else {
	  using_to = tail_of(curr_folder.headers[this_msg]->from, newfrom,
	    curr_folder.headers[this_msg]->to);

	  if (this_msg == curr_folder.curr_mssg-1)
	    build_header_line(buffer, curr_folder.headers[this_msg], this_msg+1,
			    TRUE, newfrom, using_to);
	  else
	    build_header_line(buffer, curr_folder.headers[this_msg],
			    this_msg+1, FALSE, newfrom, using_to);
	  }
	  if (selected)
	    displayed++;

	  if (inalias)
	    do_standout = (this_msg == curr_alias-1 && !arrow_cursor);
	  else
	    do_standout = (this_msg == curr_folder.curr_mssg-1 && !arrow_cursor);

	  if (do_standout)
	      StartStandout();
	  PutLine(-1, -1, buffer);
	  if (do_standout)
	      EndStandout();
	  NewLine();

	  CleartoEOLN();
	  line++;		/* for clearing up in a sec... */

	  if (selected) {
	    if ((this_msg = next_message(this_msg, FALSE)) < 0)
	      break;	/* GET OUTTA HERE! */

	    /* the preceeding looks gross because we're using an INDEX
	       variable to pretend to be a "current" counter, and the
	       current counter is always 1 greater than the actual
	       index.  Does that make sense??
	     */
	  }
	  else
	    this_msg++;					/* even dumber...  */
	}

	/* clear unused lines */

	if (mini_menu)
	  last_line = LINES-8;
	else
	  last_line = LINES-4;

	while (line < last_line) {
	  CleartoEOLN();
	  NewLine();
	  line++;
	}

	display_central_message();

	last_current = (inalias ? curr_alias : curr_folder.curr_mssg);
	last_header_page = header_page;

	return(TRUE);
}

void show_current(void)
{
	/** Show the new header, with all the usual checks **/

	register int first = 0, last = 0, last_line, new_line, using_to;
	int curr, max;
	char     newfrom[SLEN], old_buffer[SLEN], new_buffer[SLEN];

	if (inalias) {
	    curr = curr_alias;
	    max = num_aliases;
	} else {
	    curr = curr_folder.curr_mssg;
	    max = curr_folder.num_mssgs;
	}
	(void) fix_header_page();	/* Who cares what it does? ;-) */

	/** compute the first and last header on this page **/
	first = header_page * headers_per_page + 1;
	last  = first + (headers_per_page - 1);

	/* if not a full page adjust last to be the real last */
	if (selected && last > selected)
	  last = selected;
	if (!selected && last > max)
	  last = max;

	/** okay, now let's show the pointers... **/

	/** have we changed??? **/
	if (curr == last_current)
	  return;

	if (selected) {
	  last_line = ((compute_visible(last_current)-1) %
			 headers_per_page)+4;
	  new_line  = ((compute_visible(curr)-1) % headers_per_page)+4;
	} else {
	  last_line = ((last_current-1) % headers_per_page)+4;
	  new_line  = ((curr-1) % headers_per_page)+4;
	}

	if (! arrow_cursor) {

	  if (inalias)
	    build_alias_line(new_buffer, aliases[curr_alias-1], curr_alias,
		   TRUE);
	  else {
	    using_to = tail_of(curr_folder.headers[curr_folder.curr_mssg-1]->from, newfrom,
		    curr_folder.headers[curr_folder.curr_mssg-1]->to);
	    build_header_line(new_buffer, curr_folder.headers[curr_folder.curr_mssg-1],
		    curr_folder.curr_mssg, TRUE, newfrom, using_to);
	  }

	  /* clear last current if it's in proper range */
	  if (last_current > 0		/* not a dummy value */
	      && compute_visible(last_current) <= last
	      && compute_visible(last_current) >= first) {

	    dprint(5, (debugfile,
		  "\nlast_current = %d ... clearing [1] before we add [2]\n",
		   last_current));
	    dprint(5, (debugfile, "first = %d, and last = %d\n\n",
		  first, last));

	    if (inalias)
	      build_alias_line(old_buffer, aliases[last_current-1],
	                       last_current, FALSE);
	    else {
	    using_to = tail_of(curr_folder.headers[last_current-1]->from, newfrom,
	      curr_folder.headers[last_current-1]->to);
	    build_header_line(old_buffer, curr_folder.headers[last_current-1],
		 last_current, FALSE, newfrom, using_to);
	    }

	    ClearLine(last_line);
	    PutLine(last_line, 0, old_buffer);
	  }
	  MoveCursor(new_line, 0);
	  if (Term.status & TERM_CAN_SO)
	      StartStandout();
	  PutLine(-1, -1, new_buffer);
	  if (Term.status & TERM_CAN_SO)
	      EndStandout();
	}
	else {
	  if (on_page(last_current-1))
	    PutLine(last_line,0,"  ");	/* remove old pointer... */
	  if (on_page(curr-1))
	    PutLine(new_line, 0,"->");
	}

	last_current = curr;
}

static int build_header_line(char *buffer, struct header_rec *entry,
			     int message_number, int highlight, char *from,
			     int really_to)
{
	/** Build in buffer the message header ... entry is the current
	    message entry, 'from' is a modified (displayable) from line,
	    'highlight' is either TRUE or FALSE, and 'message_number'
	    is the number of the message.
	**/

	/** Note: using 'strncpy' allows us to output as much of the
	    subject line as possible given the dimensions of the screen.
	    The key is that 'strncpy' returns a 'char *' to the string
	    that it is handing to the dummy variable!  Neat, eh? **/

	int match;
	int who_width = 18, subj_width, subj_field_width;
	static int initialized = 0;
	static int mlist_justf, mlist_width = 0;
	extern struct addrs patterns;
	extern struct addrs mlnames;
	static char *to_me = NULL;
	static char *to_many = NULL;
	static char *cc_me = NULL;
	char *dot = strchr(from, '.');
	char *bang = strchr(from, '!');

	/*if (show_mlists && !initialized) {*/
	if (!initialized) {
	  int i;
	  mlist_init();
	  mlist_width = 12;
	  mlist_justf = -mlist_width;	/* positive for right-justified */
	  /* search for to_me/to_many override */
	  for (i=0; i < patterns.len; i++) {
	    if (patterns.str[i] == NULL)
	      continue;
	    if (strcmp(patterns.str[i], TO_ME_TOKEN) == 0) {
	      to_me = mlnames.str[i];
	    }
	    else if (strcmp(patterns.str[i], TO_MANY_TOKEN) == 0) {
	      to_many = mlnames.str[i];
	    }
	    else if (strcmp(patterns.str[i], CC_ME_TOKEN) == 0) {
	      cc_me = mlnames.str[i];
	    }
	  }
	  if (to_me == NULL)   to_me   = TO_ME_DEFAULT;
	  if (to_many == NULL) to_many = TO_MANY_DEFAULT;
	  if (cc_me == NULL)   cc_me   = CC_ME_DEFAULT;
	  initialized = 1;
	}

	subj_field_width = COLS - 46 - (show_mlists? mlist_width: 0);

	/* truncate 'from' to 18 characters -
	 * this includes the leading "To" if really_to is true.
	 * Note:
	 *	'from' is going to be of three forms
	 *		- full name (truncate on the right for readability)
	 *		- logname@machine (truncate on the right to preserve
	 *			logname over machine name
	 *		- machine!logname -- a more complex situation
	 *			If this form doesn't fit, either machine
	 *			or logname are long. If logname is long,
	 *			we can stand to loose part of it, so we
	 *			truncate on the right. If machine name is
	 *			long, we'd better truncate on the left,
	 *			to insure we get the logname. Now if the
	 *			machine name is long, it will have "." in
	 *			it.
	 *	Therfore, we truncate on the left if there is a "." and a "!"
	 *	in 'from', else we truncate on the right.
	 */

	/* Note that one huge sprintf() is too hard for some compilers. */

        make_menu_date(entry);

	sprintf(buffer, "%s%s%c%-3d %s ",
		(highlight && arrow_cursor)? "->" : "  ",
		show_status(entry->status),
		(entry->status & TAGGED?  '+' : ' '),
	        message_number,
	        entry->time_menu);

	if (show_mlists) {
	  mlist_parse_header_rec(entry);
	  match = mlist_match_user(entry);
	  if (match >= 0) {
	    if (entry->ml_to.len == 1 ||	/* just to me ... */
		(entry->ml_to.len == 2 &&	/* ... or me and sender */
		 mlist_match_address(entry, entry->allfrom) >= 0))
	      sprintf(buffer + strlen(buffer), "%*.*s/ ",
	        mlist_justf, mlist_width, to_me);
	    else {
	      if (match < entry->ml_cc_index)	/* to me and others */
		sprintf(buffer + strlen(buffer), "%*.*s/ ",
		  mlist_justf, mlist_width, to_many);
	      else				/* cc'd to me (implies others)*/
		sprintf(buffer + strlen(buffer), "%*.*s/ ",
		  mlist_justf, mlist_width, cc_me);
	    }
	  } else {				/* not to me at all */
	    match = addrmatch(&entry->ml_to, &patterns);
	    if (match >= 0) {			/* ... found a mlist entry */
	      sprintf(buffer + strlen(buffer), "%*.*s/ ",
	        mlist_justf, mlist_width, mlnames.str[match]);
	    }
	    else {
	      if (entry->ml_to.len > 0) {	/* ... no mlist, try 'to' hdr */
		sprintf(buffer + strlen(buffer), "(%*.*s) ",
		  (mlist_justf<0)?mlist_justf+1:mlist_justf-1, mlist_width-1,
		  entry->ml_to.str[0]);
	      }
	      else {				/* ... no mlist/'to'?  punt! */
		sprintf(buffer + strlen(buffer), "%*.*s/ ",
		  mlist_justf, mlist_width, "***");
	      }
	    }
	  }
	}
	else {
	  char *bufend = buffer+strlen(buffer);
	  mlist_parse_header_rec(entry);
	  match = mlist_match_user(entry);
	  if (match >= 0) {
	    if (entry->ml_to.len == 1 ||	/* just to me ... */
		(entry->ml_to.len == 2 &&	/* ... or me and sender */
		 mlist_match_address(entry, entry->allfrom) >= 0))
	      *bufend++ = to_chars[0];
	    else {
	      if (match < entry->ml_cc_index)	/* to me and others */
		*bufend++ = to_chars[1];
	      else				/* cc'd to me */
		*bufend++ = to_chars[2];
	    }
	  } else				/* not to me at all */
	    *bufend++ = to_chars[3];
	  *bufend++ = ' ';
	  *bufend++ = '\0';
	}

	/* show "To " in a way that it can never be truncated. */
	if (really_to) {
	  strcat(buffer, "To ");
	  who_width -= 3;
	}

	/* truncate 'from' on left if needed.
	 * sprintf will truncate on right afterward if needed. */
	if ((strlen(from) > who_width) && dot && bang && (dot < bang)) {
	  from += (strlen(from) - who_width);
	}

	/* Set the subject display width.
	 * If it is too long, truncate it to fit.
	 * If it is highlighted but not with the arrow  cursor,
	 * expand it to fit so that the reverse video bar extends
	 * aesthetically the full length of the line.
	 */
	if ((highlight && !arrow_cursor)
		|| (subj_field_width < (subj_width = strlen(entry->subject))))
	    subj_width = subj_field_width;

	/* complete line with sender, length and subject. */
	sprintf(buffer + strlen(buffer), "%-*.*s (%d) %s%-*.*s",
		/* give max and min width parameters for 'from' */
		who_width,
		who_width,
		from,

		entry->lines,
		(entry->lines / 1000   > 0? ""   :	/* spacing the  */
		  entry->lines / 100   > 0? " "  :	/* same for the */
		    entry->lines / 10  > 0? "  " :	/* lines in ()  */
		                            "   "),     /*   [wierd]    */

		subj_width, subj_width, entry->subject);
}

static int fix_header_page(void)
{
	/** this routine will check and ensure that the current header
	    page being displayed contains messages!  It will silently
	    fix 'header-page' if wrong.  Returns TRUE if changed.  **/

	int last_page, old_header, max;

	old_header = header_page;
	max = (inalias ? num_aliases : curr_folder.num_mssgs);

	last_page = (int) ((max-1) / headers_per_page);

	if (header_page > last_page)
	  header_page = last_page;
	else if (header_page < 0)
          header_page = 0;

	return(old_header != header_page);
}

int on_page(int message)
{
	/** Returns true iff the specified message is on the displayed page. **/

	if (selected)
	    message = compute_visible(message);

	return ((message / headers_per_page) == header_page);
}

char *show_status(int status)
{
	/** This routine returns a pair of characters indicative of
	    the status of this message.  The first character represents
	    the interim status of the message (e.g. the status within
	    the mail system):

		E = Expired message
		N = New message
		O = Unread old message	dsi mailx emulation addition
		D = Deleted message
		_ = (space) default

	    and the second represents the permanent attributes of the
	    message:

		C = Company Confidential message
	        U = Urgent (or Priority) message
		P = Private message
		A = Action associated with message
		F = Form letter
		M = MIME compliant Message
		    (only displayed, when metamail is needed)
		_ = (space) default
	**/

	static char mybuffer[3];

	/** the first character, please **/

	     if (status & DELETED)	mybuffer[0] = 'D';
	else if (status & EXPIRED)	mybuffer[0] = 'E';
	else if (status & NEW)		mybuffer[0] = 'N';
	else if (status & UNREAD)	mybuffer[0] = 'O';
        else if ( (show_reply) && (status & REPLIED_TO) )       mybuffer[0] = 'r';
	else                            mybuffer[0] = ' ';

	/** and the second... **/

	     if (status & CONFIDENTIAL) mybuffer[1] = 'C';
	else if (status & URGENT)       mybuffer[1] = 'U';
	else if (status & PRIVATE)      mybuffer[1] = 'P';
	else if (status & ACTION)       mybuffer[1] = 'A';
	else if (status & FORM_LETTER)  mybuffer[1] = 'F';
#ifdef MIME_RECV
	else if ((status & MIME_MESSAGE) &&
		 ((status & MIME_NOTPLAIN) ||
		  (status & MIME_NEEDDECOD))) mybuffer[1] = 'M';
#endif /* MIME_RECV */
	else 			        mybuffer[1] = ' ';

	mybuffer[2] = '\0';

	return( (char *) mybuffer);
}
