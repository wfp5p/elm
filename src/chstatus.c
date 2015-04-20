
/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
 *
 * This file and all associated files and documentation:
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: chstatus.c,v $
 * Revision 1.4  1996/05/09  15:51:17  wfp5p
 * Alpha 10
 *
 * Revision 1.3  1996/03/14  17:27:55  wfp5p
 * Alpha 9
 *
 *
 */

#include "elm_defs.h"
#ifdef ALLOW_STATUS_CHANGING /*{*/
#include "elm_globals.h"
#include "s_elm.h"

#undef onoff
#define   onoff(n)	(n ? on_name : off_name)
#undef yesno
#define   yesno(n)	(n ? yes_name : no_name)

static char *on_name = NULL, *off_name = NULL;
static char *yes_name = NULL, *no_name = NULL;


static void status_help();


#define MENU_NEW 0
#define MENU_REPLIED 1

#define SLINE_HDR 5
#define SLINE_NEW 7
#define SLINE_REPLIED 9

#define SCOL_CUR 20
#define SCOL_ORG 35

static char *oneline_new = NULL, *oneline_replied = NULL;

#define CYCLE_NEW 0
#define CYCLE_OLD 1
#define CYCLE_READ 2
static char *cycle_str[] = { NULL, NULL, NULL };
#define CYCLE_STRS (3)

static int new, undo_new, org_new, replied, undo_replied, org_replied;


static void display_status(void)
{
    /*  Display all the available status items */

    char buf[SLEN];
    static char *scurrent = NULL, *soriginal = NULL;

    if (scurrent == NULL) {
	scurrent = catgets(elm_msg_cat, ElmSet, ElmStatusCurrent, "Current");
	soriginal = catgets(elm_msg_cat, ElmSet, ElmStatusOriginal, "Original");
    }

    ClearScreen();

    CenterLine(0, catgets(elm_msg_cat, ElmSet, ElmStatusEditor,
	    "-- ELM Status Editor --"));

    sprintf(buf, "%*s%s%*s%s", SCOL_CUR, "", scurrent,
	    SCOL_ORG - (SCOL_CUR + strlen(scurrent)), "", soriginal);
    PutLine(SLINE_HDR, 0, buf);

    /*  not a catalog entry because it determines the command character */
    sprintf(buf,"%-20.20s%*s%%-5s%*s%%-5s", "N)ew/Old/Read", SCOL_CUR - 20, "",
	    SCOL_ORG - (SCOL_CUR + 5), "");
    PutLine(SLINE_NEW, 0, buf, cycle_str[new], cycle_str[org_new]);

    /*  not a catalog entry because it determines the command character */
    sprintf(buf,"%-20.20s%*s%%-5s%*s%%-5s", "R)eplied-to", SCOL_CUR - 20, "",
	    SCOL_ORG - (SCOL_CUR + 5), "");
    PutLine(SLINE_REPLIED, 0, buf, yesno(replied), yesno(org_replied));
}


static void status_help(void)
{
    /*  help menu for the status screen... */

    char c, *prompt;

    ClearLine(LINES-2);		/* clear option prompt message */
    CenterLine(LINES-3, catgets(elm_msg_cat, ElmSet, ElmPressKeyHelp,
"Press the key you want help for, '?' for a key list, or '.' to exit help"));

    lower_prompt(prompt = catgets(elm_msg_cat, ElmSet, ElmKeyPrompt, "Key : "));

    while ((c = ReadCh()) != '.') {
	c = tolower(c);
	if (c == '?') {
	    display_helpfile("status");
	    display_status();
	    return;
	}
	switch (c) {
	case ctrl('L'):
	    show_error(catgets(elm_msg_cat, ElmSet, ElmOptionCtrlLHelp,
		    "^L = Rewrite the screen."));
	    break;

	case 'n':
	    show_error(catgets(elm_msg_cat, ElmSet, ElmStatusNewHelp,
  "Whether the message is new and unread, old and unread, or already read."));
	    break;

	case 'o':
	    show_error(catgets(elm_msg_cat, ElmSet, ElmStatusOriginalHelp,
    "Restore original status of message as of entry to elm or last resync."));
	    break;

	case 'r':
	    show_error(catgets(elm_msg_cat, ElmSet, ElmStatusRepliedHelp,
		    "Whether you've replied to the message."));
	    break;

	case 'i':
	case 'q':
	case RETURN:
	case LINE_FEED:
	    show_error(catgets(elm_msg_cat, ElmSet, ElmStatusReturnHelp,
		    "q,<return> = Return from status menu."));
	    break;

	case 'u':
	    show_error(catgets(elm_msg_cat, ElmSet, ElmStatusUndoHelp,
		    "Undo changes made this time in the status editor."));
	    break;

	case 'x':
	    show_error(catgets(elm_msg_cat, ElmSet, ElmHelpQuickExit,
		    "X = Exit leaving the folder untouched, unconditionally."));
	    break;

	default:
	    show_error(catgets(elm_msg_cat, ElmSet, ElmKeyIsntUsed,
		    "That key isn't used in this section."));
	}
	lower_prompt(prompt);
    }
    ClearLine(LINES-3);		/* clear Press help key message */
    ClearLine(LINES-1);		/* clear lower prompt message */
}

int ch_status(void)
{
    /** change status... **/
    /* return:
     *	> 0	if change was done - to indicate we need to redraw
     *	 	the screen with the new status on it
     *	0	otherwise
     */

	/* FOO - the above comment is totally wrong */

    int	ch;
    char *prompt;
    struct header_rec *hdr;

    hdr = curr_folder.headers[curr_folder.curr_mssg-1];


    if (on_name == NULL) {
	on_name = catgets(elm_msg_cat, ElmSet, ElmOn, "ON ");
	off_name = catgets(elm_msg_cat, ElmSet, ElmOff, "OFF");
    }
    if (yes_name == NULL) {
	yes_name = catgets(elm_msg_cat, ElmSet, ElmToggleYes, "YES");
	no_name = catgets(elm_msg_cat, ElmSet, ElmToggleNo, "NO ");
    }
    if (oneline_new == NULL) {
	oneline_new = catgets(elm_msg_cat, ElmSet, ElmStatusNewHelp,
"Whether the message is new and unread, old and unread, or already read.");
	oneline_replied = catgets(elm_msg_cat, ElmSet, ElmStatusRepliedHelp,
		"Whether you've replied to the message.");
    }
    if (cycle_str[0] == NULL) {
	cycle_str[0] = catgets(elm_msg_cat, ElmSet, ElmStatusNew, "New ");
	cycle_str[1] = catgets(elm_msg_cat, ElmSet, ElmStatusOld, "Old ");
	cycle_str[2] = catgets(elm_msg_cat, ElmSet, ElmStatusRead, "Read");
    };

    prompt = catgets(elm_msg_cat, ElmSet, ElmPrompt, "Command: ");

    /*  set ``original'' values as they were when elm was invoked */
    if (ison(hdr->org_status, UNREAD))
	if (ison(hdr->org_status, NEW))
	    org_new = CYCLE_NEW;
	else
	    org_new = CYCLE_OLD;
    else
	org_new = CYCLE_READ;
    org_replied = ison(hdr->org_status, REPLIED_TO);

    /*  and current and ``undo'' values as they are now */
    if (ison(hdr->status, UNREAD))
	if (ison(hdr->status, NEW))
	    new = CYCLE_NEW;
	else
	    new = CYCLE_OLD;
    else
	new = CYCLE_READ;
    undo_new = new;
    replied = undo_replied = ison(hdr->status, REPLIED_TO);

    display_status();

    clearerr(stdin);

    while(1) {

	ClearLine(LINES-5);
	CenterLine(LINES-5, catgets(elm_msg_cat, ElmSet, ElmStatusMenuLine1,
	    "Select letter of status line, 'o' to restore original status,"));
	ClearLine(LINES-4);
	CenterLine(LINES-4, catgets(elm_msg_cat, ElmSet, ElmStatusMenuLine2,
		"'u' to undo changes, or 'i' to return to index."));

	PutLine(LINES-2, 0, prompt);

	ch = ReadCh();

	clear_error();	/*  remove possible message etc... */

	ClearLine(LINES-5);
	ClearLine(LINES-4);

	switch (ch) {
	case 'n':
	    /*  don't put up one-liner 'cause it'll just disappear anyway */
	    new = (new >= CYCLE_STRS-1 ? 0 : new + 1);
	    PutLine(SLINE_NEW,SCOL_CUR, "%s", cycle_str[new]);
	    break;

	case 'r':
	    /*  don't put up one-liner 'cause it'll just disappear anyway */
	    replied = ! replied;
	    PutLine(SLINE_REPLIED, SCOL_CUR, yesno(replied));
	    break;

	case 'u':
	    new = undo_new;
	    replied = undo_replied;
	    display_status();
	    break;

	case 'o':
	    new = org_new;
	    replied = org_replied;
	    display_status();
	    break;

	case '?':
	    status_help();
	    PutLine(LINES-2,0, prompt);
	    break;

	case 'X':
	    quit_abandon(FALSE);
	    break;

	case 'x':
	    quit_abandon(TRUE);
	    break;

	case 'q':/*  pop back up to previous level, in this case == <return> */
	case 'i':
	case RETURN: 			/*  return to index screen */
	case LINE_FEED:
	    switch (new) {
	    case CYCLE_NEW:
		setit(hdr->status, NEW);
		setit(hdr->status, UNREAD);
		break;
	    case CYCLE_OLD:
		clearit(hdr->status, NEW);
		setit(hdr->status, UNREAD);
		break;
	    case CYCLE_READ:
		clearit(hdr->status, NEW);
		clearit(hdr->status, UNREAD);
		break;
	    }

	    if (replied)
		setit(hdr->status, REPLIED_TO);
	    else
		clearit(hdr->status, REPLIED_TO);

	    hdr->status_chgd = (new != org_new || replied != org_replied);
	    return(0);
	    break;

	case ctrl('L'):		/*  redraw */
	    display_status();
	    break;

	default:
	    show_error(catgets(elm_msg_cat, ElmSet, ElmCommandUnknown,
		    "Command unknown!"));
	    break;
	}
    }
}

#endif /*}ALLOW_STATUS_CHANGING*/
