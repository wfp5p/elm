

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: a_screen.c,v $
 * Revision 1.3  1996/03/14  17:27:48  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:55  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**  alias screen display routines for ELM program

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_aliases.h"

void alias_screen(int modified)
{
	/* Stolen from showscreen() */

	ClearScreen();

	alias_title(modified);

	last_header_page = -1;	 	/* force a redraw regardless */
	show_headers();

	if (mini_menu)
	  show_alias_menu();

	show_last_error();

}

void alias_title(int modified)
{
	/** display a new title line, due to re-sync'ing the aliases **/
	/* Stolen from update_title() */

	char buffer[SLEN];
	char modmsg[SLEN];

	if (modified) {
	    strcpy(modmsg, catgets(elm_msg_cat, AliasesSet, AliasesModified,
		"(modified, resync needed) "));
	}
	else {
	    modmsg[0] = '\0';
	}

	if (selected)
	  sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesSelect,
	      "Alias mode: %d shown out of %d %s[ELM %s]"),
	      selected, num_aliases, modmsg, version_buff);
	else if (num_aliases == 1)
	  sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesSingle,
	      "Alias mode: 1 alias %s[ELM %s]"), modmsg, version_buff);
	else
	  sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesPlural,
	      "Alias mode: %d aliases %s[ELM %s]"),
	      num_aliases, modmsg, version_buff);

	ClearLine(1);
	CenterLine(1, buffer);
}

void show_alias_menu(void)
{
	/** write alias menu... **/
	/* Moved from alias.c */

	if (user_level == 0) {	/* Give less options  */
	  CenterLine(LINES-7, catgets(elm_msg_cat, AliasesSet, AliasesRMenuLn1,
"You can use any of the following commands by pressing the first character;"));
	  CenterLine(LINES-6, catgets(elm_msg_cat, AliasesSet, AliasesRMenuLn2,
"a)lias current message, n)ew alias, d)elete or u)ndelete an alias,"));
	  CenterLine(LINES-5, catgets(elm_msg_cat, AliasesSet, AliasesRMenuLn3,
"m)ail to alias, or r)eturn to main menu.  To view an alias, press <return>."));
	  CenterLine(LINES-4, catgets(elm_msg_cat, AliasesSet, AliasesRMenuLn4,
"j = move down, k = move up, ? = help"));
	}
	else {
	    CenterLine(LINES-7, catgets(elm_msg_cat, AliasesSet, AliasesMenuLn1,
"Alias commands:  ?=help, <n>=set current to n, /=search pattern"));
	    CenterLine(LINES-6, catgets(elm_msg_cat, AliasesSet, AliasesMenuLn2,
"a)lias current message, c)hange, d)elete, e)dit aliases.text, f)ully expand,"));
	    CenterLine(LINES-5, catgets(elm_msg_cat, AliasesSet, AliasesMenuLn3,
"l)imit display, m)ail, n)ew alias, r)eturn, t)ag, u)ndelete, or e(x)it"));
	}

}

void build_alias_line(char *buffer, struct alias_rec *entry,
		      int message_number, int highlight)
{
	/** Build in buffer the alias header ... entry is the current
	    message entry, 'highlight' is either TRUE or FALSE,
	    and 'message_number' is the number of the message.
	**/

	char mybuffer[SLEN];

	/** Note: using 'strncpy' allows us to output as much of the
	    subject line as possible given the dimensions of the screen.
	    The key is that 'strncpy' returns a 'char *' to the string
	    that it is handing to the dummy variable!  Neat, eh? **/
	/* Stolen from build_header_line() */

	int name_width;

	/* Note that one huge sprintf() is too hard for some compilers. */

	sprintf(buffer, "%s%s%c%-3d ",
		(highlight && arrow_cursor)? "->" : "  ",
		show_status(entry->status),
		(entry->status & TAGGED?  '+' : ' '),
		message_number);

	/* Set the name display width. */
	name_width = COLS-40;

	/* Put the name and associated comment in local buffer */
	if (strlen(entry->comment))
	  sprintf(mybuffer, "%s, %s", entry->name, entry->comment);
	else
	  sprintf(mybuffer, "%s", entry->name);

	/* complete line with name, type and alias. */
	sprintf(buffer + strlen(buffer), "%-*.*s %s %-18.18s",
		/* give max and min width parameters for 'name' */
		name_width, name_width, mybuffer,
		alias_type(entry->type),
		entry->alias);
}

char *alias_type(int type)
{
	/** This routine returns a string showing the alias type,
	    'Person' or 'Group' aliases.  Additionally, a '(S)'
	    is appended if this is a system alias.
	**/

	static char mybuffer[10];

	if (type & GROUP) {
	    strcpy(mybuffer, catgets(elm_msg_cat, AliasesSet,
	    		AliasesGroup, " Group"));
	} else {
	    strcpy(mybuffer, catgets(elm_msg_cat, AliasesSet,
			AliasesPerson, "Person"));
	}

	if (type & SYSTEM) {
	    strcat(mybuffer, catgets(elm_msg_cat, AliasesSet,
	    		AliasesSystemFlag, "(S)"));
	} else {
	    strcat(mybuffer, "   ");
	}

	return mybuffer;
}
