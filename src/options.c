
static char rcsid[] = "@(#)$Id: options.c,v 1.5 1996/03/14 17:29:43 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.5 $   $State: Exp $
 *
 * 			Copyright (c) 1986,1987 Dave Taylor
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: options.c,v $
 * Revision 1.5  1996/03/14  17:29:43  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1995/09/29  17:42:19  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/07/18  19:00:00  wfp5p
 * Alpha 6
 *
 * Revision 1.2  1995/04/26  15:23:17  wfp5p
 * If DONT_ADD_FROM is defined the Your name option doesn't do anything so it
 * now doesn't show up (from Larry Schwimmer <rosebud@cyclone.Stanford.EDU>).
 *
 * There were a couple places in mailmsg2 that strincmp was called with only 2
 * parameters.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:37  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This set of routines allows the alteration of a number of paramaters
    in the Elm mailer, including the following;


    a)  arrow-cursor	<on or off>
    b)  border for copy <string for left margin>
    c)  calendar-file	<where to put calendar entries>
    d)  display pager	<how to page messages>
    e)  editor		<name of primary editor or "builtin">
    f)  folder-dir	<folder directory>
    h)  hold sent msgs  <on or off>
  * i) (internal: quit to index screen)
    j)  reply editor    <name of editor for replies>
    k)  pause pager     <on or off>
    l)  alias sort      <how to sort the alias display>
    m)  menu-display    <on or off>
    n)  names-only      <on or off>
    o)  sent-mail	<file to save outbound message copies to>
    p)  printmail	<how to print messages>
  * q) (internal: quit to previous screen)
    r)  reply copy      <on or off>
    s)  sort-by		<how to sort folders>
    t)  text editor     <name of ~e editor>
    u)  user-level	<BEGINNER|INTERMEDIATE|EXPERT>
    v)  visual editor	<name of ~v editor>
    w)  want CC prompt  <on or off>
  * x) (internal: exit ELM now)
    y)  full_username	<your full user name for outgoing mail>
    z)  zap sign dashes <on or off>
	
    And others as they seem useful.

    Formatting parameters:

    _)		Add a blank line
    ^)		Put the title on this line

    A subset of the above parameters may be selected as the
    o)ptions screen using the parameter "configoptions" which is
    a string of letters representing the "key" letter for each
    item as listed above.  The current default is "^_cdefsopyv_am_un".
    The order of the options displayed is as listed in the string.

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "save_opts.h"
#include "s_elm.h"

#undef onoff
#define   onoff(n)	(n == 1? on_name : off_name)

#ifdef ENABLE_CALENDAR
# ifdef DONT_ADD_FROM
#define CONFIG_OPTIONS "^_cdefsopv_am_un"
# else
#define CONFIG_OPTIONS "^_cdefsopyv_am_un"
# endif
#else
# ifdef DONT_ADD_FROM
#define CONFIG_OPTIONS "^_defsopv_am_un"
# else
#define CONFIG_OPTIONS "^_defsopyv_am_un"
# endif
#endif


extern char *alias_sort_name(), *sort_name();

char *level_name();
static char *on_name = NULL;
static char *off_name = NULL;
static int resort = 0;


static post_cal(f)
int f;
{
    (void) expand_env(calendar_file, raw_calendar_file, sizeof(calendar_file));
}

static post_editor(f)
int f;
{
    (void) expand_env(editor, raw_editor, sizeof(editor));
}

static post_pager(f)
int f;
{
    (void) expand_env(pager, raw_pager, sizeof(pager));
    clear_pages = (streq(pager, "builtin+") || streq(pager, "internal+"));
}

static post_folder(f)
int f;
{
    (void) expand_env(folders, raw_folders, sizeof(folders));
}

static post_sort(f)
int f;
{
    if (f)
	resort++;
}

static post_sent(f)
int f;
{
    (void) expand_env(sent_mail, raw_sentmail, sizeof(sent_mail));
}

static post_print(f)
int f;
{
    (void) expand_env(printout, raw_printout, sizeof(printout));
}

static post_menu(f)
int f;
{
    headers_per_page = LINES - (mini_menu ? 13 : 8);
}

opts_menu cfg_opts[] = {

{'a', "A)rrow cursor",	ElmOptionArrowMenu,  "arrow",	NULL,
"This defines whether the ELM cursor is an arrow or a highlight bar.", ElmOptionArrowHelp},

{'b', "B)order on copy", ElmOptionBorderMenu,	"prefix",	NULL,
"Left margin string (prefix) for reply and forward.", ElmOptionBorderHelp},

#ifdef ENABLE_CALENDAR
{'c', "C)alendar file", ElmOptionCalendarMenu,	"calendar",	post_cal,
"This is the file where calendar entries from messages are saved.", ElmOptionCalendarHelp},
#endif

{'d', "D)isplay mail using", ElmOptionDisplayMenu,	"pager",	post_pager,
"This is the program invoked to display individual messages (try 'builtin').", ElmOptionDisplayHelp},

{'e', "E)ditor (primary)", ElmOptionEditorMenu,	"editor",	post_editor,
"This is the editor used for all outbound messages (may be 'builtin').", ElmOptionEditorHelp},

{'f', "F)older directory", ElmOptionFolderMenu,	"maildir",	post_folder,
"This is the folders directory used when '=' (etc) is used in filenames.", ElmOptionFolderHelp},

{'h', "H)old sent message", ElmOptionHoldMenu,	"copy",		NULL,
"Whether to save a copy of outbound messages in O)utbound folder.", ElmOptionHoldHelp},

{'j', "J) reply editor", ElmOptionJreplyMenu,	"alteditor",	NULL,
"This is the editor that will be used for replies (Editor=\"builtin\").", ElmOptionJreplyHelp},

{'k', "K) pause after pager", ElmOptionKpauseMenu, "promptafter",	NULL,
"Whether to pause (prompt) at end of message with display message pager.", ElmOptionKpauseHelp},

{'l', "A(l)ias Sorting", ElmOptionaLiasMenu,	"aliassortby",	post_sort,
"This is used to specify the sorting criteria for the alias display.", ElmOptionaLiasHelp},

{'m', "M)enu display", ElmOptionMenuMenu,	"menu",		post_menu,
"This determines if you have the mini-menu displayed or not.", ElmOptionMenuHelp},

{'n', "N)ames only", ElmOptionNameMenu,		"names",	NULL,
"Whether to display the names and addresses on mail, or names only.", ElmOptionNameHelp},

{'o', "O)utbound mail saved", ElmOptionOutboundMenu, "sentmail",	post_sent,
"This is where copies of outbound messages are saved automatically.", ElmOptionOutboundHelp},

{'p', "P)rint mail using", ElmOptionPrintMenu,	"print",	post_print,
"This is the command used to print mail messages.", ElmOptionPrintHelp},

{'r', "R)eply copies msg", ElmOptionReplyMenu,	"replycopy",	NULL,
"Whether to copy the message being replied to into the reply.", ElmOptionReplyHelp},

{'s', "S)orting criteria", ElmOptionSortMenu,	"sortby",	post_sort,
"This is used to specify the sorting criteria for the folders.", ElmOptionSortHelp},

{'t', "T)ext editor (~e)", ElmOptionTextMenu,	"easyeditor",	NULL,
"This is the editor that will be used by the ~e command (Editor=\"builtin\").", ElmOptionTextHelp},

{'u', "U)ser level", ElmOptionUserMenu,		"userlevel",	NULL,
"The level of knowledge you have about the ELM mail system.", ElmOptionUserHelp},

{'v', "V)isual Editor (~v)", ElmOptionVisualMenu,	"visualeditor",	NULL,
"This is the editor that will be used by the ~v command (Editor=\"builtin\").", ElmOptionVisualHelp},

{'w', "W)ant Cc: prompt", ElmOptionWantMenu,	"askcc",	NULL,
"Whether to ask for \"Copies to:\" list when sending messages.", ElmOptionWantHelp},

{'y', "Y)our full name", ElmOptionYourMenu,	"fullname",	NULL,
"When mail is sent out, this is what your full name will be recorded as.", ElmOptionYourHelp},

{'z', "Z) signature dashes", ElmOptionZdashMenu,	"sigdashes",	NULL,
"Whether to print dashes before including signature on outbound messages.", ElmOptionZdashHelp},

{ 0 } };

void init_opts_menu()
{
	register char *c;
	register opts_menu *o;

	/* FOO - this is breaking my catgets scanner */
	for (o = cfg_opts; o->parm; o++) {
		c = catgets(elm_msg_cat, ElmSet, o->menu_msg, "~");
		if (*c != '~')
			o->menu = c;

		c = catgets(elm_msg_cat, ElmSet, o->one_liner_msg, "~");
		if (*c != '~')
			o->one_liner = c;
	}
}


opts_menu *
find_cfg_opts(c)
int c;
{
	register opts_menu *o;

	for (o = cfg_opts; o->parm; o++) {
	if (c == o->letter)
	    break;
    }

    return(o->parm ? o : NULL);
}


char *
one_liner_for(c)
char c;
{
    opts_menu *o;

    o = find_cfg_opts(c);

    if (o)
	return(o->one_liner);
    else
	return(NULL);
}


void
display_options()
{
	/** Display all the available options.. **/
	
	int printed_title = FALSE;
	register int y;
	register opts_menu *o;
	register char *s;
	char buf[SLEN];
	extern char *str_opt_nam();
	
	ClearScreen();

	for (s = config_options, y = 0; *s; s++, y++) {
	    if (*s == '^') {
		printed_title = TRUE;
		CenterLine(y, catgets(elm_msg_cat, ElmSet, ElmOptionsEditor,
		    "-- ELM Options Editor --"));
		continue;
	    }
	    if (*s == '_') {
		continue;
	    }
            o = find_cfg_opts(*s);
            if (o != NULL && y<LINES-5) {
		sprintf(buf,"%-20.20s : %%s", o->menu);
		PutLine1(y, 0, buf, str_opt_nam(o->parm, TRUE));
	    }
	}
	if (!printed_title) {
	    CenterLine(LINES-5, catgets(elm_msg_cat, ElmSet, ElmOptionsEditor,
		"-- ELM Options Editor --"));
	}
}


void
options_help()
{
	/** help menu for the options screen... **/

	char	*ptr, *prompt;
	int	c;

	ClearLine(LINES-2);		/* clear option prompt message */
	CenterLine(LINES-3, catgets(elm_msg_cat, ElmSet, ElmPressKeyHelp,
  "Press the key you want help for, '?' for a key list, or '.' to exit help"));

	lower_prompt(prompt = catgets(elm_msg_cat, ElmSet, ElmKeyPrompt, "Key : "));

	while ((c = ReadCh()) != '.') {

	  c = tolower(c);

	  if (c == '?') {
	     display_helpfile("options");
	     display_options();
	     return;
	  }

	  if ((ptr = one_liner_for(c)) != NULL)
	    error2("%c = %s", c, ptr);
	  else
	    switch (c) {
	      case ctrl('L'): error(catgets(elm_msg_cat, ElmSet,
				  ElmOptionCtrlLHelp,
				  "^L = Rewrite the screen."));		break;

	      case '>' : error(catgets(elm_msg_cat, ElmSet,ElmOptionGreaterHelp,
				"> = Save to elmrc file."));		break;

	      case 'q' :
	      case 'i' : error(catgets(elm_msg_cat, ElmSet, ElmOptionReturnHelp,
				"i,q = Return from option menu."));	break;

	      case 'x' : error(catgets(elm_msg_cat, ElmSet, ElmHelpQuickExit,
				"X = Exit leaving the folder untouched, unconditionally."));
				break;

	      default: error(catgets(elm_msg_cat, ElmSet, ElmKeyIsntUsed,
				"That key isn't used in this section."));
	    }
	  lower_prompt(prompt);
	}
	ClearLine(LINES-3);		/* clear Press help key message */
	ClearLine(LINES-1);		/* clear lower prompt message */
}


char *
level_name(n)
int n;
{
	/** return the 'name' of the level... **/

	switch (n) {
	  case 0 : return(catgets(elm_msg_cat, ElmSet, ElmBeginningUser,
			"Beginning User   "));
	  case 1 : return(catgets(elm_msg_cat, ElmSet, ElmIntermediateUser,
			"Intermediate User"));
	  default: return(catgets(elm_msg_cat, ElmSet, ElmExpertUser,
			"Expert User      "));
	}
}

int
options()
{
	/** change options... **/
	/* return:
	 *	> 0	if resort was done - to indicate we might need to
	 *	 	change the page of our headers as a consequence
	 *		of the new sort order
	 *	0	otherwise
	 */

	int ch;
	char *opos;

	if (on_name == NULL) {
		on_name = catgets(elm_msg_cat, ElmSet, ElmOn, "ON ");
		off_name = catgets(elm_msg_cat, ElmSet, ElmOff, "OFF");
	}
	if (!config_options[0]) {
	    strcpy(config_options,CONFIG_OPTIONS);
	}

	resort = 0;
	display_options();

	while(1) {

	  ClearLine(LINES-4);
	  CenterLine(LINES-4, catgets(elm_msg_cat, ElmSet, ElmSelectOptionLetter,
 "Select letter of option line, '>' to save, or 'i' to return to index."));

	  ClearLine(LINES-2);
	  PutLine0(LINES-2, 0, catgets(elm_msg_cat, ElmSet, ElmPrompt,
		"Command: "));
	  ch = ReadCh();
	  ch = tolower(ch);
	  clear_error();

	  if ((opos = index(config_options, ch)) != NULL) {
	      int y = opos - config_options;
	      opts_menu *o = find_cfg_opts(ch);
              if (o != NULL && y < LINES-5) {
		  one_liner(o->one_liner);
		  y = info_enter(o->parm, y, 23);
		  if (o->post)
		      (o->post)(y);
	      } else {
	          error(catgets(elm_msg_cat, ElmSet, ElmCommandUnknown,
			"Command unknown!"));
	      }
	      continue;
	  }

	  one_liner(NULL);
	  switch (ch) {

	  case '?':
	    options_help();
	    break;

	  case '>':
	    PutLine0(-1, -1, catgets(elm_msg_cat, ElmSet, ElmSaveOptions,
		  "Save options in .elm/elmrc..."));
	    FlushOutput();
	    save_options();
	    break;

	  case 'x':
	    quit_abandon(TRUE);
	    break;

	  case 'q':
	  case 'i' :
	    return (resort ? 1 : 0);

	  case ctrl('L'):
	    display_options();
	    break;

	  default:
	    error(catgets(elm_msg_cat, ElmSet, ElmCommandUnknown,
		  "Command unknown!"));
	  }

	}

	/*NOTREACHED*/
}



on_or_off(var, x, y)
int *var, x,y;
{
	/** 'var' field at x.y toggles between on and off... **/

	int ch;

     	PutLine0(x, y+6, catgets(elm_msg_cat, ElmSet, ElmUseSpaceToToggle,
		"(use <space> to toggle, any other key to leave)"));

	MoveCursor(x,y+3);	/* at end of value... */

	do {
	  ch = ReadCh();

	  if (ch == ' ') {
	    *var = ! *var;
	    PutLine0(x,y, onoff(*var));
	  }
	} while (ch == ' ');

	MoveCursor(x,y+4); 	CleartoEOLN();	/* remove help prompt */
}


switch_user_level(ulevel, x, y)
int *ulevel, x, y;
{
	/** step through possible user levels... **/

     	PutLine0(x, y+20, catgets(elm_msg_cat, ElmSet, ElmSpaceToChange,
		"<space> to change"));

	MoveCursor(x,y);	/* at end of value... */

	while (ReadCh() == ' ') {
	  *ulevel = (*ulevel >= 2? 0 : *ulevel + 1);
	  PutLine1(x,y, "%s", level_name(*ulevel));
	}

	MoveCursor(x,y+20); 	CleartoEOLN();	/* remove help prompt */
}


change_sort(var, x, y)
int *var;
int x,y;
{
	/** change the sorting scheme... **/
	/** return !0 if new sort order, else 0 **/
	
	int last_sortby,	/* so we know if it changes... */
	    sign = 1;		/* are we reverse sorting??    */
	int ch;			/* character typed in ...      */

/*	*var = sortby; 	or...	*var == sortby; 	*/
	last_sortby = sortby;	/* remember current ordering   */

	PutLine0(x, COLS-29, catgets(elm_msg_cat, ElmSet, ElmSpaceForNext,
		"(SPACE for next, or R)everse)"));
	sort_one_liner(sortby);
	MoveCursor(x, y);

	do {
	  ch = ReadCh();
	  ch = tolower(ch);
	  switch (ch) {
	    case ' ' : if (sortby < 0) { 
	    		   sign = -1; 
	    		   sortby = - sortby; 
	  		 }
			 else sign = 1;		/* insurance! */
	  		 sortby = sign * ((sortby + 1) % (STATUS+2));
			 if (sortby == 0) sortby = sign;  /* snicker */
	  		 PutLine1(x, y, "%-24s", sort_name(TRUE));
			 sort_one_liner(sortby);
	  		 MoveCursor(x, y);
			 break;

	    case 'r'   : sortby = - sortby;
	  		 PutLine1(x, y, "%-24s", sort_name(TRUE));
			 sort_one_liner(sortby);
	  		 MoveCursor(x, y);
	 }
        } while (ch == ' ' || ch == 'r');

	MoveCursor(x, COLS-30);	CleartoEOLN();

	if (sortby != last_sortby) {
	  error(catgets(elm_msg_cat, ElmSet, ElmResortingFolder,
		"Resorting folder..."));
	  if (sleepmsg > 0)
		sleep((sleepmsg + 1) / 2);
	  sort_mailbox(curr_folder.num_mssgs, 0);
	}
	ClearLine(LINES-2);		/* clear sort_one_liner()! */
	*var=sortby;
	sortby=last_sortby;
	return(*var-sortby);
}


one_liner(string)
char *string;
{
	/** A single-line description of the selected item... **/

	ClearLine(LINES-4);
	if (string)
		CenterLine(LINES-4, string);
}


sort_one_liner(sorting_by)
int sorting_by;
{
	/** A one line summary of the particular sorting scheme... **/

	ClearLine(LINES-2);

	switch (sorting_by) {
	  
	  case REVERSE SENT_DATE:	CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortRSentDate,
"This sort will order most-recently-sent to least-recently-sent"));
				break;
	  case REVERSE RECEIVED_DATE:	CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortRRecvDate,
"This sort will order most-recently-received to least-recently-received"));
				break;
	  case REVERSE MAILBOX_ORDER:	CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortRMailbox,
"This sort will order most-recently-added-to-folder to least-recently"));
				break;
	  case REVERSE SENDER:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortRSender,
"This sort will order by sender name, in reverse alphabetical order"));
				break;
	  case REVERSE SIZE:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortRSize,
"This sort will order messages by longest to shortest"));
				break;
	  case REVERSE SUBJECT:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortRSubject,
"This sort will order by subject, in reverse alphabetical order"));
				break;
	  case REVERSE STATUS:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortRStatus,
"This sort will order by reverse status - Deleted through Tagged..."));
				break;
	  case SENT_DATE:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortSentDate,
"This sort will order least-recently-sent to most-recently-sent"));
				break;
	  case RECEIVED_DATE:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortRecvDate,
"This sort will order least-recently-received to most-recently-received"));
				break;
	  case MAILBOX_ORDER:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortMailbox,
"This sort will order least-recently-added-to-folder to most-recently"));
				break;
	  case SENDER:			CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortSender,
"This sort will order by sender name"));
				break;
	  case SIZE:			CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortSize,
"This sort will order messages by shortest to longest"));
				break;
	  case SUBJECT:			CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortSubject,
"This sort will order messages by subject"));
				break;
	  case STATUS:			CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmSortStatus,
"This sort will order by status - Tagged through Deleted..."));
				break;
	}
}



change_alias_sort(var, x, y)
int *var;
int x,y;
{
	/** change the sorting scheme... **/
	/** return !0 if new sort order, else 0 **/
	
	int last_sortby,	/* so we know if it changes... */
	    sign = 1;		/* are we reverse sorting??    */
	int ch;			/* character typed in ...      */

/*	*var = alias_sortby; 	or...	*var == alias_sortby; 	*/
	last_sortby = alias_sortby;	/* remember current ordering   */

	PutLine0(x, COLS-29, catgets(elm_msg_cat, ElmSet, ElmSpaceForNext,
		"(SPACE for next, or R)everse)"));
	alias_sort_one_liner(alias_sortby);
	MoveCursor(x, y);

	do {
	  ch = ReadCh();
	  ch = tolower(ch);
	  switch (ch) {
	    case ' ' : if (alias_sortby < 0) { 
	    		   sign = -1; 
	    		   alias_sortby = - alias_sortby; 
	  		 }
			 else sign = 1;		/* insurance! */
	  		 alias_sortby = sign * ((alias_sortby + 1)
	                                % (LAST_ALIAS_SORT+1));
			 if (alias_sortby == 0) alias_sortby = sign;
	  		 PutLine1(x, y, "%-24s", alias_sort_name(TRUE));
			 alias_sort_one_liner(alias_sortby);
	  		 MoveCursor(x, y);
			 break;

	    case 'r'   : alias_sortby = - alias_sortby;
	  		 PutLine1(x, y, "%-24s", alias_sort_name(TRUE));
			 alias_sort_one_liner(alias_sortby);
	  		 MoveCursor(x, y);
	 }
        } while (ch == ' ' || ch == 'r');

	MoveCursor(x, COLS-30);	CleartoEOLN();

	if (alias_sortby != last_sortby) {
	  error(catgets(elm_msg_cat, ElmSet, ElmResortingAliases,
			"Resorting aliases..."));
	  if (sleepmsg > 0)
		sleep((sleepmsg + 1) / 2);
	  main_state();
	  sort_aliases(num_aliases, FALSE, FALSE);
	  main_state();
	}
	ClearLine(LINES-2);		/* clear alias_sort_one_liner()! */
	*var=alias_sortby;
	alias_sortby=last_sortby;
	return(*var-alias_sortby);
}

alias_sort_one_liner(sorting_by)
int sorting_by;
{
	/** A one line summary of the particular sorting scheme... **/

	ClearLine(LINES-2);

	switch (sorting_by) {
	  
	  case REVERSE ALIAS_SORT:	CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmASortRAlias,
"This sort will order by alias name, in reverse alphabetical order"));
					break;
	  case REVERSE NAME_SORT:	CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmASortRName,
"This sort will order by user (real) name, in reverse alphabetical order"));
					break;
	  case REVERSE TEXT_SORT:	CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmASortRText,
"This sort will order aliases in the reverse order as listed in aliases.text"));
					break;

	  case ALIAS_SORT:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmASortAlias,
"This sort will order by alias name"));
					break;
	  case NAME_SORT:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmASortName,
"This sort will order by user (real) name"));
					break;
	  case TEXT_SORT:		CenterLine(LINES-2,
				catgets(elm_msg_cat, ElmSet, ElmASortText,
"This sort will order aliases in the order as listed in aliases.text"));
					break;

	}
}

/*
 * This routine is a "shell" to enter_string(), etc. so that
 * we can use save_info structure and tag the param as being changed
 * locally (so we know to save it to the .elm/elmrc file).
 */
info_enter(name, ypos, xpos)
char *name;
int ypos, xpos;
{
	register int x,q;
	char buffer[SLEN];
	int number;

	for (x = 0; x < NUMBER_OF_SAVEABLE_OPTIONS; x++) {
		q = strcmp(name, save_info[x].name);
		if (q <= 0)
			break;
	}

	if (q)
	    return(1);

	switch(save_info[x].flags & DT_MASK) {

	case DT_STR:
	    strcpy(buffer, SAVE_INFO_STR(x));
	    if (save_info[x].flags & FL_NOSPC) {
		register char *s;
		for (s = buffer; *s; ++s)
		    if (*s==' ')
			*s='_';
	    }

	    /* FOO - this is not handling enter_string() abort properly */
	    enter_string(buffer, sizeof(buffer), ypos, xpos, ESTR_REPLACE);

	    if (save_info[x].flags & FL_NOSPC) {
		register char *s;
		for (s=buffer; *s; ++s)
		    if (*s=='_')
			*s=' ';
	    }

	    if (streq(buffer, SAVE_INFO_STR(x)))
		break;

	    strcpy(SAVE_INFO_STR(x), buffer);
	    q++;
	    break;

	case DT_SRT:
	    number = *SAVE_INFO_SRT(x);
	    if (change_sort(&number, ypos, xpos) != 0) {
		*SAVE_INFO_SRT(x) = number;
		++q;
	    }
	    break;

	case DT_ASR:
	    number = *SAVE_INFO_ASR(x);
	    if (change_alias_sort(&number, ypos, xpos) != 0) {
		*SAVE_INFO_ASR(x) = number;
		++q;
	    }
	    break;

	case DT_NUM:
	    number = *SAVE_INFO_NUM(x);
	    if (streq(name, "userlevel")) {
		switch_user_level(&number, ypos, xpos);
	    }

	    if (number == *SAVE_INFO_NUM(x))
		break;

	    *SAVE_INFO_NUM(x) = number;
	    ++q;
	    break;

	case DT_BOL:
	    number = *SAVE_INFO_BOL(x);
	    on_or_off(&number, ypos, xpos);
	    if (number == *SAVE_INFO_BOL(x))
		break;

	    *SAVE_INFO_BOL(x) = number;
	    ++q;
	    break;

	default:
	    break;

	}

	if (q)
	    save_info[x].flags |= FL_LOCAL;

	return(q != 0);
}

