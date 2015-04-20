

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.6 $   $State: Exp $
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
 * $Log: showmsg_c.c,v $
 * Revision 1.6  1999/03/24  14:04:05  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.5  1996/03/14  17:29:52  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1995/09/29  17:42:28  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/07/18  19:00:09  wfp5p
 * Alpha 6
 *
 * Revision 1.2  1995/05/24  15:34:38  wfp5p
 * Change to deal with parenthesized comments in when eliminating members from
 * an alias. (from Keith Neufeld <neufeld@pvi.org>)
 *
 * Allow a shell escape from the alias screen (just like from
 * the index screen).  It does not put the shell escape onto the alias
 * screen menu. (from Keith Neufeld <neufeld@pvi.org>)
 *
 * Allow the use of "T" from the builtin pager. (from Keith Neufeld
 * <neufeld@pvi.org>)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This is an interface for the showmsg command line.  The possible
    functions that could be invoked from the showmsg command line are
    almost as numerous as those from the main command line and include
    the following;

	   |    = pipe this message to command...
	   !    = call Unix command
	   <    = scan message for calendar info
	   b    = bounce (remail) message
	   d    = mark message for deletion
	   f    = forward message
	   g    = group reply
	   h    = redisplay this message from line #1, showing headers
	   <CR> = redisplay this message from line #1, weeding out headers
	   i,q  = move back to the index page (simply returns from function)
	   J    = move to body of next message
	   j,n  = move to body of next undeleted message
	   K    = move to body of previous message
	   k    = move to body of previous undeleted message
	   m    = mail a message out to someone
	   p    = print this (all tagged) message
	   r    = reply to this message
	   s    = save this message to a maibox/folder
	   t    = tag this message
	   u    = undelete message
	   x    = Exit Elm NOW

    all commands not explicitly listed here are beeped at.  Use i)ndex
    to get back to the main index page, please.

    This function returns when it is ready to go back to the index
    page.
**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

static int screen_mangled = 0;
static char msg_line[SLEN];
static char *put_help_prompt = NULL;
#define store_msg(a)	(void)strcpy(msg_line,a)
#define put_prompt()	PutLine(LINES-3, 0, nls_Prompt)
#define put_help()	PutLine(LINES-3, 45, put_help_prompt)
#define POST_PROMPT_COL	strlen(nls_Prompt)


int process_showmsg_cmd(int command)
{
	int ch, i;

	if (put_help_prompt == NULL) {
		put_help_prompt = catgets(elm_msg_cat, ElmSet, ElmUseIToReturnIndex,
				"(Use 'i' to return to index.)");
	}
	Raw(ON);

	for (;;) {

	  ch = (command == 0 ? GetKey(0) : command);
	  command = 0;

	  clear_error();
	  switch (ch) {
	    case '?' : if (help(TRUE)) {
			 ClearScreen();
			 build_bottom();
		       } else screen_mangled = TRUE;
		       break;

	    case '|' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmPipe, "Pipe"), TRUE);
		       (void) do_pipe();     /* do pipe - ignore return val */
		       ClearScreen();
		       build_bottom();
		       break;

#ifdef ALLOW_SUBSHELL
	    case '!' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmSystemCall,
				"System call"), TRUE);
		       (void) subshell();
		       ClearScreen();
		       build_bottom();
		       break;
#endif

       /* this was scan for calendar entries */
	    case '<' : break;

	    case '%' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmDisplayReturnAdd,
				"Display return address"), TRUE);
		       get_return(msg_line, curr_folder.curr_mssg-1);
		       break;

	    case 'b' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmBounceMessage,
				"Bounce message"), TRUE);
		       remail();
		       break;

	    case 'd' : delete_msg(TRUE, FALSE); /* really delete it, silent */
		       if (! resolve_mode)
			 store_msg(catgets(elm_msg_cat, ElmSet, ElmMessageMarkedForDeleteion,
				"Message marked for deletion."));
		       else
			 goto next_undel_msg;
		       break;

	    case 'f' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmForwardMessage,
				"Forward message"), TRUE);
		       if(forward()) put_border();
		       break;

	    case 'g' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmGroupReply,
				"Group reply"), TRUE);
		       (void) reply_to_everyone();
		       break;

	    case 'h' : screen_mangled = 0;
		       if (filter) {
		         filter = 0;
		         i = show_msg(curr_folder.curr_mssg);
		         filter = 1;
			 return(i);
		       } else
		         return(show_msg(curr_folder.curr_mssg));

	    case 'q' :
	    case 'i' : (void) get_page(curr_folder.curr_mssg);
		       clear_error();		/* zero out pending msg   */
		       EnableFkeys(ON);
		       screen_mangled = 0;
		       return(0);		/* avoid <return> looping */

next_undel_msg :	/* a target for resolve mode actions */

	    case KEY_DOWN:
	    case ' ' :
	    case 'j' :
	    case 'n' : screen_mangled = 0;
		       if((i=next_message(curr_folder.curr_mssg-1, TRUE)) != -1)
			 return(show_msg(curr_folder.curr_mssg = i+1));
		       else return(0);

next_msg:
	    case 'J' : screen_mangled = 0;
		       if((i=next_message(curr_folder.curr_mssg-1, FALSE)) != -1)
			 return(show_msg(curr_folder.curr_mssg = i+1));
		       else return(0);

	    case KEY_UP:
	    case 'k' : screen_mangled = 0;
		       if((i=prev_message(curr_folder.curr_mssg-1, TRUE)) != -1)
			 return(show_msg(curr_folder.curr_mssg = i+1));
		       else return(0);

	    case 'K' : screen_mangled = 0;
		       if((i=prev_message(curr_folder.curr_mssg-1, FALSE)) != -1)
			 return(show_msg(curr_folder.curr_mssg = i+1));
		       else return(0);

	    case 'm' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmMailMessage,
				"Mail message"), TRUE);
		       if (send_message((char *)NULL, (char *)NULL,
			         (char *)NULL, SM_ORIGINAL))
			 put_border();
		       break;

	    case 'p' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmPrintMessage,
				"Print message"), FALSE);
		       (void) print_msg(FALSE);
		       break;

	    case 'r' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmReplyToMessage,
				"Reply to message"), TRUE);
		       if(reply()) put_border();
		       break;

	    case '>' :
	    case 'C' :
	    case 's' :
		       put_cmd_name("", TRUE);
		       if (save(&i, TRUE, (ch != 'C')) &&
			   resolve_mode && ch != 'C')
			 goto next_undel_msg;
		       break;

	    case 't' :
 	    case 'T' :
		       if(tag_message(FALSE))
			 store_msg(catgets(elm_msg_cat, ElmSet, ElmMessageTagged,
				"Message tagged."));
		       else
			 store_msg(catgets(elm_msg_cat, ElmSet, ElmMessageUntagged,
				"Message untagged."));
		       if (ch == 'T')
			 goto next_undel_msg;
		       break;

	    case 'u' : undelete_msg(FALSE); /* undelete it, silently */
		       if (! resolve_mode)
			 store_msg(catgets(elm_msg_cat, ElmSet, ElmMessageUndeleted,
				"Message undeleted."));
		       else {
/******************************************************************************
 ** We're special casing the U)ndelete command here *not* to move to the next
 ** undeleted message ; instead it'll blindly move to the next message in the
 ** list.  See 'elm.c' and the command by "case 'u'" for further information.
 ** The old code was:
			 goto next_undel_msg;
*******************************************************************************/
			 goto next_msg;
		       }
		       break;

	    case 'X' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmQuickExit,
				"Quick Exit"), TRUE);
		       quit_abandon(FALSE);
		       break;

	    case 'x' : put_cmd_name(catgets(elm_msg_cat, ElmSet, ElmExit, "Exit"), TRUE);
		       quit_abandon(TRUE);
		       break;

	    case ctrl('J'):
	    case ctrl('M'):  screen_mangled = 0;
			     return(show_msg(curr_folder.curr_mssg));

	    default  : Beep();
	  }

	  /* display prompt */
	  if (screen_mangled) {
	    /* clear what was left over from previous command
	     * and display last generated message.
	     */
	    put_prompt();
	    CleartoEOS();
	    put_help();
	    CenterLine(LINES, msg_line);
	    MoveCursor(LINES-3, POST_PROMPT_COL);
	  } else {
	    /* display bottom line prompt with last generated message */
	    MoveCursor(LINES, 0);
	    CleartoEOS();
	    if (Term.status & TERM_CAN_SO)
	      StartStandout();
	    PutLine(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmCommandLine,
		"%s Command ('i' to return to index): "), msg_line);
	    if (Term.status & TERM_CAN_SO)
	      EndStandout();
	  }
	  *msg_line = '\0';	/* null last generated message */

	}
}

int put_cmd_name(char *command, int will_mangle)
{

	/* If screen is or will be mangled display the command name
	 * and erase the bottom of the screen.
	 * But first if the border line hasn't yet been drawn, draw it.
	 */
	if(will_mangle && !screen_mangled) {
	  build_bottom();
	  screen_mangled = TRUE;
	}
	if(screen_mangled) {
	  PutLine(LINES-3, POST_PROMPT_COL, command);
	  CleartoEOS();
	}
}

int put_border(void)
{
	 PutLine(LINES-4, 0,
"--------------------------------------------------------------------------\n");
}

int build_bottom(void)
{
	 MoveCursor(LINES-4, 0);
	 CleartoEOS();
	 put_border();
	 put_prompt();
	 put_help();
}
