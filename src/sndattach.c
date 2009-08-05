#include "elm_defs.h"
#include "elm_globals.h"
#include "sndparts.h"
#include "s_attach.h"
#include <assert.h>

#define AT_DUMMYMODE	(user_level == 0)
#define AT_MAX_ATTACH	64

/* WARNING - side effects */
#define PutRight(line, str) PutLine0((line), COLS-(strlen(str)+2), (str))

#define S_(sel, str)	catgets(elm_msg_cat, AttachSet, (sel), (str))

/*
 * Locations within the screen display.
 */
#define ATLINE_TITLE		0		/* page title		*/
#define ATLINE_LTOP		3		/* top line of att list	*/
#define ATLINE_LBOT		(LINES-14)	/* bot line of att list	*/
#define ATLINE_CURR_TITLE	(LINES-12)	/* current sel title	*/
#define ATLINE_CURR_FILE	(LINES-11)	/* curr filename	*/
#define ATLINE_CURR_TYPE	(LINES-10)	/* curr Content-Type:	*/
#define ATLINE_CURR_ENCOD	(LINES-9)	/* curr C-T-Encoding:	*/
#define ATLINE_CURR_DESCR	(LINES-8)	/* curr C-Description:	*/
#define ATLINE_CURR_DISP	(LINES-7)	/* curr C-Disposition:	*/
#define ATLINE_INSTR		(LINES-4)	/* instructions display	*/
#define ATLINE_PROMPT		(LINES-2)	/* user data entry line	*/

#define ATCOL_CURR_COLON	24	/* end of curr selection title	*/
#define ATCOL_CURR_DATA		26	/* start of curr selection data	*/

/*
 * Display redraw requests.
 */
#define ATDRAW_NONE		(0)	/* nothing to redisplay		*/
#define ATDRAW_HEADER		(1<<0)	/* redisplay the screen header	*/
#define ATDRAW_LIST		(1<<1)	/* redisplay list of attachments*/
#define ATDRAW_LIST_SEL		(1<<2)	/*  just sel change of _LIST	*/
#define ATDRAW_CURR_FILE	(1<<3)	/* redisplay curr filename	*/
#define ATDRAW_CURR_TYPE	(1<<4)	/* redisplay curr Content-Type:	*/
#define ATDRAW_CURR_ENCOD	(1<<5)	/* redisplay curr C-T-Encoding:	*/
#define ATDRAW_CURR_DESCR	(1<<6)	/* redisplay curr C-Descrip:	*/
#define ATDRAW_CURR_DISP	(1<<7)	/* redisplay curr C-Disposition:*/
#define ATDRAW_CURR_ALL		(ATDRAW_CURR_FILE|ATDRAW_CURR_TYPE|ATDRAW_CURR_ENCOD|ATDRAW_CURR_DESCR|ATDRAW_CURR_DISP)
#define ATDRAW_INSTR		(1<<8)
#define ATDRAW_PROMPT		(1<<9)
#define ATDRAW_ALL		(~0)	/* redisplay everything		*/


/*
 * Entry selection/sizing/placment values.
 */
#define AT_lines_per_page	((ATLINE_LBOT-ATLINE_LTOP) + 1)
#define AT_pagenum(sel)		((sel) / AT_lines_per_page)
#define AT_line(sel)		((sel) % AT_lines_per_page)
#define AT_first_sel_of_page	(AT_pagenum(curr_sel)*AT_lines_per_page)
#define AT_last_sel_of_page	((AT_pagenum(curr_sel)+1)*AT_lines_per_page - 1)
#define AT_first_sel_of_list	(0)
#define AT_last_sel_of_list	(at_attachmenu_count - 1)
#define AT_num_pages		(AT_last_sel_of_list/AT_lines_per_page + 1)

#define AT_FL_NOTOUCH	(1<<0)
#define AT_FL_DUMMY	(1<<1)
#define AT_FL_TAGGED	(1<<2)

struct at_attachmenu_entry {
    SEND_BODYPART *att;
    int flags;
};

static struct at_attachmenu_entry at_attachmenu_list[AT_MAX_ATTACH];
static int at_attachmenu_count;	/* num entries in at_attachmenu_list[]	*/
static SEND_BODYPART at_bogus;	/* marker used in assertions		*/

static char at_acursor_on[]  = "->";	/* arrow cursor on		*/
static char at_acursor_off[] = "  ";	/* arrow cursor blanked		*/

static void at_disp_entry P_((int, int, int));
static void at_disp_currline P_((int, const char *, const char *, int));
static void at_disp_instr P_((const char *, const char *, const char *, int));
static SEND_BODYPART *at_do_change_file P_((SEND_BODYPART *, int *, int *));
static int at_do_change_type P_((SEND_BODYPART *, int *));
static int at_do_change_encoding P_((SEND_BODYPART *, int *));
static int at_do_change_descrip P_((SEND_BODYPART *, int *));
static int at_do_change_disposition P_((SEND_BODYPART *, int *));

static char *strtruncate P_((char *, int));

/* manipulation of the internal attachment list */
static void atlist_initialize P_((void));
static int atlist_full P_((void));
static void atlist_insert P_((int, SEND_BODYPART *, int));
static void atlist_remove P_((int));
static void atlist_replace P_((int, SEND_BODYPART *, int));
static SEND_BODYPART *atlist_getbodypart P_((int));
static int atlist_getflags P_((int));
static int atlist_tagged_clrcnt P_((int));
static int atlist_tagged_move P_((int));

PUBLIC int attachment_menu(user_attachments_p)
SEND_MULTIPART **user_attachments_p;
{
    SEND_MULTIPART *mp;
    SEND_BODYPART *att;
    int done;			/* TRUE when it's time to exit		*/
    int inp_line, inp_col;	/* cursor position for user input	*/
    int do_redraw;		/* what parts of screen need redrawing	*/
    int bad_cmd;		/* TRUE if last command was bad		*/
    int cmd;			/* command from user			*/
    int next_cmd;		/* force a "cmd" next time through loop	*/
    int curr_sel;		/* currently selected entry		*/
    int prev_sel;		/* selection at previous iteration	*/
    int mssg_ok;
    int n, i;
    char tmp_buf[SLEN], *s;

    /* initialize our list of attachments */
    atlist_initialize();
    atlist_insert(0, bodypart_new(S_(AttachMainMessage,
		"[main message]"), BP_IS_DUMMY), AT_FL_NOTOUCH|AT_FL_DUMMY);

    /* slurp all of the existing attachments from an (SEND_BODYPART *) */
    if (*user_attachments_p != NULL) {
	for (;;) {
	    mp = multipart_next(*user_attachments_p, (SEND_MULTIPART *) NULL);
	    if (mp == NULL)
		break;
	    if (atlist_full()) {
		error(S_(AttachTooManyAttachments,
			    "Too many attachments for menu to handle!"));
		return 0;
	    }
	    att = multipart_deletepart(*user_attachments_p, mp);
	    atlist_insert(at_attachmenu_count, att, 0);
	}
	multipart_destroy(*user_attachments_p);
	*user_attachments_p = NULL;
    }

    done = FALSE;
    do_redraw = ATDRAW_ALL;
    bad_cmd = FALSE;
    next_cmd = '\0';
    prev_sel = curr_sel = 0;

    while (!done) {

	/* complain if last entry was bad */
	if (bad_cmd) {
	    Beep();
	    bad_cmd = FALSE;
	}

	/* see if the selection moved */
	if (prev_sel != curr_sel) {
	    if (curr_sel < AT_first_sel_of_list) {
		/* adjust for movement beyond start of list */
		if ((curr_sel = AT_first_sel_of_list) == prev_sel) {
		    bad_cmd = TRUE;
		    continue;
		}
	    }
	    if (curr_sel > AT_last_sel_of_list) {
		/* adjust for movement beyond end of list */
		if ((curr_sel = AT_last_sel_of_list) == prev_sel) {
		    bad_cmd = TRUE;
		    continue;
		}
	    }
	    if (AT_pagenum(curr_sel) != AT_pagenum(prev_sel)) {
		/* page changed */
		do_redraw = ATDRAW_ALL;
	    } else {
		/* moved to a selection on this page */
		do_redraw |= (ATDRAW_LIST_SEL|ATDRAW_CURR_ALL);
	    }
	}

	/* do screen updates */
	if (do_redraw != ATDRAW_NONE) {

	    if (do_redraw == ATDRAW_ALL)
		ClearScreen();

	    /* redraw the title and selection header lines */
	    if (do_redraw & ATDRAW_HEADER) {
		if (do_redraw != ATDRAW_ALL)
		    ClearLine(ATLINE_TITLE);
		CenterLine(ATLINE_TITLE, S_(AttachScreenTitle,
			    "Message Attachments Screen"));
		sprintf(tmp_buf, S_(AttachScreenPage, "[page %d/%d]"),
			    AT_pagenum(curr_sel)+1, AT_num_pages);
		PutLine0(ATLINE_TITLE, COLS-(strlen(tmp_buf)+2), tmp_buf);
	    }

	    /* redraw the entire list of entries */
	    if (do_redraw & ATDRAW_LIST) {
		n = AT_first_sel_of_page;
		for (i = 0 ; i < AT_lines_per_page ; ++i) {
		    if (n <= AT_last_sel_of_list) {
			at_disp_entry(ATLINE_LTOP+i, n, (n == curr_sel));
		    } else if (do_redraw != ATDRAW_ALL) {
			ClearLine(ATLINE_LTOP+i);
		    }
		    ++n;
		}
	    }

	    /* redraw just the entries with selection changes */
	    if ((do_redraw & ATDRAW_LIST_SEL) && !(do_redraw & ATDRAW_LIST)) {
		if (prev_sel >= AT_first_sel_of_page
			&& prev_sel <= AT_last_sel_of_page) {
		    if (arrow_cursor) {
			PutLine0(ATLINE_LTOP+AT_line(prev_sel), 0,
				    at_acursor_off);
		    } else {
			at_disp_entry(ATLINE_LTOP+AT_line(prev_sel),
				    prev_sel, FALSE);
		    }
		}
		if (arrow_cursor) {
		    PutLine0(ATLINE_LTOP+AT_line(curr_sel), 0, at_acursor_on);
		} else {
		    at_disp_entry(ATLINE_LTOP+AT_line(curr_sel),
				curr_sel, TRUE);
		}
	    }

	    /* display details on the selected attachment */
	    if (do_redraw == ATDRAW_ALL) {
		CenterLine(ATLINE_CURR_TITLE, S_(AttachCurrTitle,
			    "---------- Current Attachment ----------"));
	    }
	    att = atlist_getbodypart(curr_sel);
	    if (do_redraw & ATDRAW_CURR_FILE)
		at_disp_currline(ATLINE_CURR_FILE,
			S_(AttachCurrFileName, /*(*/ "F)ile Name"),
			bodypart_get_filename(att),
			(do_redraw != ATDRAW_ALL));
	    if (do_redraw & ATDRAW_CURR_TYPE)
		at_disp_currline(ATLINE_CURR_TYPE,
			S_(AttachCurrContType, /*(*/ "C)ontent Type"),
			bodypart_get_content(att, BP_CONT_TYPE),
			(do_redraw != ATDRAW_ALL));
	    if (do_redraw & ATDRAW_CURR_ENCOD)
		at_disp_currline(ATLINE_CURR_ENCOD,
			S_(AttachCurrContEncoding, /*(*/ "Content E)ncoding"),
			bodypart_get_content(att, BP_CONT_ENCODING),
			(do_redraw != ATDRAW_ALL));
	    if (do_redraw & ATDRAW_CURR_DESCR)
		at_disp_currline(ATLINE_CURR_DESCR,
			S_(AttachCurrContDescription, "Content De(s)cription"),
			bodypart_get_content(att, BP_CONT_DESCRIPTION),
			(do_redraw != ATDRAW_ALL));
	    if (do_redraw & ATDRAW_CURR_DISP)
		at_disp_currline(ATLINE_CURR_DISP,
			S_(AttachCurrContDisposition, "Content Dis(p)osition"),
			bodypart_get_content(att, BP_CONT_DISPOSITION),
			(do_redraw != ATDRAW_ALL));

	    if (do_redraw & ATDRAW_INSTR) {
		at_disp_instr(
			    S_(AttachMainInstrNorm,
/*(((*/ "Use \"jk+-\" to move; \"?\" = help; a)dd, d)elete, or q)uit."),
			    S_(AttachMainInstrDummy1,
"Make selection:  j/k = down/up, +/- = down/up page, or enter selection num."),
			    S_(AttachMainInstrDummy2, /*(((*/
"Then a)dd after or d)elete the selection.  ? = help.  Then q)uit when done."),
			    (do_redraw != ATDRAW_ALL));
	    }

	    if (do_redraw & ATDRAW_PROMPT) {
		PutLine0(ATLINE_PROMPT, 0, S_(AttachMainPrompt, "Command: "));
		GetCursorPos(&inp_line, &inp_col);
		if (do_redraw != ATDRAW_ALL) {
		    /* erase down to (but EXCLUDING) the error line */
		    for (i = ATLINE_PROMPT+1 ; i < LINES ; ++i)
			ClearLine(i);
		}
	    }

	    if (do_redraw == ATDRAW_ALL)
		show_last_error();
	    do_redraw = ATDRAW_NONE;

	}

	prev_sel = curr_sel;

	/* prompt for command */
	if (next_cmd != '\0') {
	    cmd = next_cmd;
	    next_cmd = '\0';
	} else {
	    MoveCursor(inp_line, inp_col);
	    CleartoEOLN();
	    PutLine0(-1, -1, "q\b");
	    if ((cmd = GetKey(0)) == KEY_REDRAW) {
		do_redraw = ATDRAW_ALL;
		continue;
	    }
	    if (clear_error())
		MoveCursor(inp_line, inp_col);
	}

	switch (cmd) {

	case '?':			/* help */
	    display_helpfile("attach");
	    do_redraw = ATDRAW_ALL;
	    break;

	case ctrl('L'):			/* redraw display */
	    do_redraw = ATDRAW_ALL;
	    break;

#ifdef ALLOW_SUBSHELL
	case '!':			/* subshell */
	    at_disp_instr((char *)NULL, (char *)NULL, (char *)NULL, TRUE);
	    ClearLine(ATLINE_PROMPT);
	    do_redraw = (subshell()
			? ATDRAW_ALL : (ATDRAW_INSTR|ATDRAW_PROMPT));
	    break;
#endif

	case 'q':			/* quit menu */
	case '\r':
	case '\n':
	    done = TRUE;
	    break;

	case KEY_DOWN:			/* down entry */
	case 'j':
	    ++curr_sel;
	    break;

	case KEY_UP:			/* up entry */
	case 'k':
	    --curr_sel;
	    break;

	case KEY_NPAGE:			/* down page */
	case '+':
	    curr_sel += AT_lines_per_page;
	    break;

	case KEY_PPAGE:			/* up page */
	case '-':
	    curr_sel -= AT_lines_per_page;
	    break;

	case KEY_HOME:			/* first entry */
	    curr_sel = 0;
	    break;

	case KEY_END:			/* last entry */
	case '*':
	    curr_sel = AT_last_sel_of_list;
	    break;

	case '1': case '2': case '3':	/* numeric selection */
	case '4': case '5': case '6':
	case '7': case '8': case '9':
	    UnreadCh(cmd);
	    i = enter_number(ATLINE_PROMPT, curr_sel+1,
			S_(AttachAttachment, "attachment")) - 1;
	    if (i < AT_first_sel_of_list || i > AT_last_sel_of_list) {
		error(S_(AttachNoAttachmentThere,
			    "There isn't an attachment there!"));
	    } else if (i == curr_sel) {
		error(S_(AttachSelectionNotChanged,
			    "Selection not changed."));
	    } else {
		curr_sel = i;
	    }
	    do_redraw = ATDRAW_PROMPT;
	    break;

	case 'a':			/* add attachment */
	    if (atlist_full()) {
		error(S_(AttachNoRoom,
			    "Sorry - no room for any more attachments."));
		break;
	    }
	    att = bodypart_new(S_(AttachNewAttachment, "[new attachment]"),
			BP_IS_DUMMY);
	    atlist_insert(++curr_sel, att, AT_FL_DUMMY);
	    do_redraw = ATDRAW_LIST;
	    /* we need to do a display update ... then continue with add */
	    next_cmd = (0x100|'a');
	    break;

	case (0x100|'a'):		/* add attachment ... continued */
	    att = at_do_change_file((SEND_BODYPART *)NULL, &do_redraw, &mssg_ok);
	    if (att == NULL) {
		atlist_remove(curr_sel--);
		if (mssg_ok) {
			error(S_(AttachAttachmentNotAdded,
				    "Attachment not added."));
		}
		do_redraw |= ATDRAW_LIST;
		break;
	    }
	    atlist_replace(curr_sel, att, 0);
	    if (mssg_ok) {
		error(S_(AttachAttachmentHasBeenAdded,
			    "Attachment has been added to this message."));
	    }
	    do_redraw |= ATDRAW_LIST_SEL;
	    break;

	case 'd':			/* delete attachment */
	    if (atlist_getflags(curr_sel) & AT_FL_NOTOUCH) {
		error(S_(AttachYouCantDelete, "Hey!  You can't delete that!"));
		break;
	    }
	    do_redraw = (ATDRAW_INSTR|ATDRAW_PROMPT);
	    if (!enter_yn(S_(AttachReallyDelete, "Really delete attachment?"),
			FALSE, ATLINE_PROMPT, FALSE)) {
		error(S_(AttachAttachmentNotDeleted,
			    "Attachment not deleted."));
		break;
	    }
	    atlist_remove(curr_sel);
	    if (curr_sel > AT_last_sel_of_list)
		--curr_sel;
	    error(S_(AttachAttachmentHasBeenDeleted,
			"Attachment has been deleted from this message."));
	    do_redraw |= (ATDRAW_LIST|ATDRAW_CURR_ALL);
	    break;

	case 'f':			/* change "f)ile name" */
	    if (atlist_getflags(curr_sel) & AT_FL_NOTOUCH) {
		error(S_(AttachYouCantChange, "Hey!  You can't change that!"));
		break;
	    }
	    att = at_do_change_file(atlist_getbodypart(curr_sel),
			&do_redraw, &mssg_ok);
	    if (att == NULL) {
		if (mssg_ok)
			error(S_(AttachNotChanged, "Attachment not changed."));
	    } else {
		atlist_replace(curr_sel, att, ~0);
		if (mssg_ok) {
		    error(S_(AttachAttachmentFileChanged,
				"Attachment file has been changed."));
		}
		do_redraw |= (ATDRAW_LIST|ATDRAW_CURR_ALL);
	    }
	    break;


	case 'c':			/* change "c)ontent type" */
	    if (atlist_getflags(curr_sel) & AT_FL_NOTOUCH) {
		error(S_(AttachYouCantChange, "Hey!  You can't change that!"));
		break;
	    }
	    if (!at_do_change_type(atlist_getbodypart(curr_sel), &do_redraw))
		error(S_(AttachNotChanged, "Attachment not changed."));
	    else {
		error(S_(AttachAttachmentContTypeChanged,
			"Attachment content type has been changed."));
		do_redraw |= (ATDRAW_LIST_SEL|ATDRAW_CURR_ALL);
	    }
	    break;

	case 'e':			/* change "content e)ncoding" */
	    if (atlist_getflags(curr_sel) & AT_FL_NOTOUCH) {
		error(S_(AttachYouCantChange, "Hey!  You can't change that!"));
		break;
	    }
	    if (!at_do_change_encoding(atlist_getbodypart(curr_sel), &do_redraw))
		error(S_(AttachNotChanged, "Attachment not changed."));
	    else {
		error(S_(AttachAttachmentContEncodingChanged,
			    "Attachment content encoding has been changed."));
		do_redraw |= (ATDRAW_LIST_SEL|ATDRAW_CURR_ALL);
	    }
	    break;

	case 's':			/* change "content de(s)cription" */
	    if (atlist_getflags(curr_sel) & AT_FL_NOTOUCH) {
		error(S_(AttachYouCantChange, "Hey!  You can't change that!"));
		break;
	    }
	    if (!at_do_change_descrip(atlist_getbodypart(curr_sel), &do_redraw))
		error(S_(AttachNotChanged, "Attachment not changed."));
	    else {
		error(S_(AttachAttachmentContDescriptionChanged,
			"Attachment content description has been changed."));
		do_redraw |= (ATDRAW_LIST_SEL|ATDRAW_CURR_ALL);
	    }
	    break;

	case 'p':			/* change "content dis(p)osition" */
	    if (atlist_getflags(curr_sel) & AT_FL_NOTOUCH) {
		error(S_(AttachYouCantChange, "Hey!  You can't change that!"));
		break;
	    }
	    if (!at_do_change_disposition(atlist_getbodypart(curr_sel), &do_redraw))
		error(S_(AttachNotChanged, "Attachment not changed."));
	    else {
		error(S_(AttachAttachmentContDispositionChanged,
			"Attachment content disposition has been changed."));
		do_redraw |= (ATDRAW_LIST_SEL|ATDRAW_CURR_ALL);
	    }
	    break;

	case 't':			/* tag attachment */
	    if (atlist_getflags(curr_sel) & AT_FL_NOTOUCH) {
		error(S_(AttachYouCantTag, "Hey!  You can't tag that!"));
		break;
	    }
	    i = (atlist_getflags(curr_sel) ^ AT_FL_TAGGED);
	    atlist_replace(curr_sel, (SEND_BODYPART *)NULL, i);
	    if (i & AT_FL_TAGGED) {
		error(S_(AttachAttachmentHasBeenTagged,
			    "Current attachment has been tagged."));
	    } else {
		error(S_(AttachTagHasBeenRemoved,
			    "Tag has been removed from current attachment."));
	    }
	    do_redraw |= ATDRAW_LIST_SEL;
	    break;

	case ctrl('T'):			/* remove all tags */
	    if (atlist_tagged_clrcnt(FALSE) == 0) {
		error(S_(AttachTagsAlreadyClear,
			    "All tags already are cleared."));
		break;
	    }
	    do_redraw = ATDRAW_PROMPT;
	    if (!enter_yn(S_(AttachReallyClearTags, "Really clear all tags?"),
			FALSE, ATLINE_PROMPT, FALSE)) {
		error(S_(AttachTagsNotChanged, "Tags not changed."));
		break;
	    }
	    atlist_tagged_clrcnt(TRUE);
	    error(S_(AttachTagsHaveBeenCleared, "All tags have been cleared."));
	    do_redraw |= ATDRAW_LIST;
	    break;

	case 'm':			/* move tagged attachments */
	    if ((i = atlist_tagged_clrcnt(FALSE)) == 0) {
		error(S_(AttachMustTagAttachments,
			    "You must tag the attachments you want to move."));
		break;
	    }
	    do_redraw = ATDRAW_PROMPT;
	    s = (i > 1
			? S_(AttachMoveTaggedAttachments,
			    "Move tagged attachments after selection?")
			: S_(AttachMoveTaggedAttachment,
			    "Move tagged attachment after selection?"));
	    if (!enter_yn(s, FALSE, ATLINE_PROMPT, FALSE)) {
		if (i > 1) {
		    error(S_(AttachAttachmentsNotMoved,
				"Attachments not moved."));
		} else {
		    error(S_(AttachAttachmentNotMoved,
				"Attachment not moved."));
		}
		break;
	    }
	    curr_sel = atlist_tagged_move(curr_sel);
	    if (i > 1) {
		error(S_(AttachAttachmentsHaveBeenMoved,
			    "Attachments have been moved."));
	    } else {
		error(S_(AttachAttachmentHaveBeenMoved,
			    "Attachment has been moved."));
	    }
	    do_redraw |= ATDRAW_LIST;
	    break;

	default:
	    bad_cmd = TRUE;
	    break;

	}

    }

    /* clear out entire bottom but the error line */
    for (i = ATLINE_INSTR-1 ; i < LINES ; ++i)
	ClearLine(i);

    /* rebuild the user attachments list */
    assert(*user_attachments_p == NULL);
    for (i = 0 ; i < at_attachmenu_count ; ++i) {
	if (*user_attachments_p == NULL)
	    *user_attachments_p = multipart_new((SEND_BODYPART *)NULL, 0L);
	if ((atlist_getflags(i) & AT_FL_DUMMY)) {
	    bodypart_destroy(atlist_getbodypart(i));
	} else {
	    multipart_appendpart(*user_attachments_p,
			MULTIPART_TAIL(*user_attachments_p),
			atlist_getbodypart(i), MP_ID_ATTACHMENT);
	}
    }

    return 0;
}


static void at_disp_entry(line, n, selected)
int line, n, selected;
{
    int flags, len, i;
    char out_buf[SLEN], trunc_buf[SLEN], *bp;
    SEND_BODYPART *att;

    att = atlist_getbodypart(n);
    flags = atlist_getflags(n);

    /* truncate parameters off displayed Content-Type */
    (void) strfcpy(trunc_buf, bodypart_get_content(att, BP_CONT_TYPE), sizeof(trunc_buf));
    for (bp = trunc_buf ; *bp != '\0' && *bp != ';' && !isspace(*bp) ; ++bp)
	;
    *bp = '\0';

    /* format up the line (sans Content-Description) */
    (void) sprintf(out_buf, "%2.2s%1.1s%2d  %-16.16s  %-6.6s  %-14.14s  ",
		(arrow_cursor && selected ? at_acursor_on : at_acursor_off),
		((flags & AT_FL_TAGGED) ? "+" : " "),
		n+1,
		basename(bodypart_get_filename(att)),
		bodypart_get_content(att, BP_CONT_ENCODING),
		trunc_buf);

    /* truncate the Content-Description to fit the remainder of the line */
    (void) strfcpy(trunc_buf, bodypart_get_content(att, BP_CONT_DESCRIPTION), sizeof(trunc_buf));
    len = strlen(out_buf);
    (void) strcpy(out_buf+len, strtruncate(trunc_buf, COLS - (len+2)));

    MoveCursor(line, 0);
    if (selected && !arrow_cursor)
	StartStandout();
    PutLine0(-1, -1, out_buf);
    if (selected && !arrow_cursor) {
	for (i = (COLS-2) - strlen(out_buf) ; i > 0 ; --i)
	    WriteChar(' ');
	EndStandout();
    } else {
	CleartoEOLN();
    }
}


static void at_disp_currline(line, title, value, do_erase)
int line;
const char *title, *value;
int do_erase;
{
    char trunc_buf[SLEN];

    if (do_erase)
	ClearLine(line);

    PutLine0(line, ATCOL_CURR_COLON-(strlen(title)+1), title);
    MoveCursor(line, ATCOL_CURR_COLON);
    WriteChar(':');
    (void) strfcpy(trunc_buf, value, sizeof(trunc_buf));
    PutLine0(line, ATCOL_CURR_DATA,
		    strtruncate(trunc_buf, COLS-(ATCOL_CURR_DATA+2)));
}


static void at_disp_instr(instr_normal, instr_dummy1, instr_dummy2, do_erase)
const char *instr_normal, *instr_dummy1, *instr_dummy2;
int do_erase;
{
    if (AT_DUMMYMODE) {
	if (do_erase)
	    ClearLine(ATLINE_INSTR-1);
	if (instr_dummy1 != NULL && *instr_dummy1)
	    CenterLine(ATLINE_INSTR-1, instr_dummy1);
	if (do_erase)
	    ClearLine(ATLINE_INSTR);
	if (instr_dummy2 != NULL && *instr_dummy2)
	    CenterLine(ATLINE_INSTR, instr_dummy2);
    } else {
	if (do_erase) {
	    ClearLine(ATLINE_INSTR-1);
	    ClearLine(ATLINE_INSTR);
	}
	if (instr_normal != NULL && *instr_normal)
	    CenterLine(ATLINE_INSTR, instr_normal);
    }
}


/* set "att" NULL to create a new attachment */
static SEND_BODYPART *at_do_change_file(att, do_redraw_p, mssg_ok_p)
SEND_BODYPART *att;
int *do_redraw_p, *mssg_ok_p;
{
    SEND_BODYPART *att_new;
    char fname[SLEN], fb_dir[SLEN], fb_pat[SLEN], *s;
    int i;
    static char save_fname[SLEN];

    if (att == NULL)
	fname[0] = '\0';
    else
	strfcpy(fname, bodypart_get_filename(att), sizeof(fname));

    /*
     * We may leave this routine with important messages on the display
     * (e.g. from encoding failure).  This flag tells the calling routine
     * it is ok to write a message.
     */
    *mssg_ok_p = TRUE;

    *do_redraw_p |= (ATDRAW_INSTR|ATDRAW_PROMPT|ATDRAW_CURR_FILE);
    at_disp_instr(
		S_(AttachChgFileInstrNorm,
"Enter attachment filename.  \"~\", \"=\", and patterns ok, CTRL/D to abort."),
		S_(AttachChgFileInstrDummy1,
"Enter name of file to attach to your mail message, or CTRL/D to abort entry."),
		S_(AttachChgFileInstrDummy2,
"Say \".\" to select a file in the current directory."),
		TRUE);
    ClearLine(ATLINE_PROMPT);

    for (att_new = NULL ; att_new == NULL ; ) {

	if (*do_redraw_p == ATDRAW_ALL) {
	    /*
	     * We are in a bit of a jam here.  We've been through this loop
	     * once before, and the display is garbage (probably from the
	     * browser).  About all we can do is abort the add.  There
	     * should be a message on the display explaining the problem,
	     * so be sure not to trounce it.
	     */
	    *mssg_ok_p = FALSE;
	    return (SEND_BODYPART *) NULL;
	}

	if (enter_string(fname, sizeof(fname),
		    ATLINE_CURR_FILE, ATCOL_CURR_DATA, ESTR_UPDATE) < 0)
	    return (SEND_BODYPART *) NULL;
	clear_error();

	/* FOO - I'd rather have "^R)ecall last" and ^B)rowse options above */
	if (fname[0] == '\0') {
	    if (att != NULL)
		(void) strfcpy(fname,
			    bodypart_get_filename(att), sizeof(fname));
	    else if (save_fname[0] != '\0')
		(void) strfcpy(fname, save_fname, sizeof(fname));
	    if ((s = strrchr(fname, '/')) != NULL)
		s[1] = '\0';
	}

	if (fname[0] == '\0') {
	    error("Please enter a filename for the attachment.");
	    continue;
	}

	if (!expand_filename(fname))
	    continue;

	if (att != NULL && streq(fname, bodypart_get_filename(att)))
	    return (SEND_BODYPART *) NULL;

	if (fbrowser_analyze_spec(fname, fb_dir, fb_pat)) {
	    *do_redraw_p = ATDRAW_ALL;
	    if (!fbrowser(fname, sizeof(fname), fb_dir, fb_pat,
			FB_READ, "Select File to Attach")) {
		return (SEND_BODYPART *) NULL;
	    }
	}

	if ((att_new = newpart_mimepart(fname)) != NULL) {
	    for (i = 0 ; i < BP_NUM_CONT_HEADERS ; ++i)
		bodypart_guess_content(att_new, i);
	}

    }

    (void) strfcpy(save_fname, fname, sizeof(save_fname));
    *do_redraw_p |= ATDRAW_CURR_ALL;
    return att_new;
}


static int at_do_change_type(att, do_redraw_p)
SEND_BODYPART *att;
int *do_redraw_p;
{
    char cont_type[SLEN];

    (void) strfcpy(cont_type, bodypart_get_content(att, BP_CONT_TYPE),
		sizeof(cont_type));
    *do_redraw_p |= (ATDRAW_INSTR|ATDRAW_PROMPT|ATDRAW_CURR_TYPE);
    at_disp_instr(
		S_(AttachChgTypeInstrNorm,
"Enter attachment content type, CTRL/D to abort."),
		S_(AttachChgTypeInstrDummy1,
"Enter the \"content type\" of this file, or CTRL/D to abort entry."),
		S_(AttachChgTypeInstrDummy2,
"You DO know what a Content-Type is...don't you???"), /*FOO*/
		TRUE);
    ClearLine(ATLINE_PROMPT);

    for (;;) {
	if (enter_string(cont_type, sizeof(cont_type),
		    ATLINE_CURR_TYPE, ATCOL_CURR_DATA, ESTR_UPDATE) < 0)
	    return FALSE;
	if (streq(cont_type, bodypart_get_content(att, BP_CONT_TYPE)))
	    return FALSE;
	clear_error();
	if (cont_type[0] == '\0') {
	    error(S_(AttachChgTypeEnterType, "Please enter a content type."));
	    continue;
	}
	bodypart_set_content(att, BP_CONT_TYPE, cont_type);
	return TRUE;
    }
    /*NOTREACHED*/
}


static int at_do_change_encoding(att, do_redraw_p)
SEND_BODYPART *att;
int *do_redraw_p;
{
    char cont_encoding[SLEN];

    (void) strfcpy(cont_encoding, bodypart_get_content(att, BP_CONT_ENCODING),
		sizeof(cont_encoding));
    *do_redraw_p |= (ATDRAW_INSTR|ATDRAW_PROMPT|ATDRAW_CURR_ENCOD);
    at_disp_instr(
		S_(AttachChgEncodingInstrNorm,
"Enter attachment content encoding, CTRL/D to abort."),
		S_(AttachChgEncodingInstrDummy1,
"Enter the \"content encoding\" of this file, or CTRL/D to abort entry."),
		S_(AttachChgEncodingInstrDummy2,
"Common values are \"7bit\", \"8bit\", \"base64\", and \"x-uuencode\"."),
		TRUE);
    ClearLine(ATLINE_PROMPT);

    for (;;) {
	if (enter_string(cont_encoding, sizeof(cont_encoding),
		    ATLINE_CURR_ENCOD, ATCOL_CURR_DATA, ESTR_UPDATE) < 0)
	    return FALSE;
	if (streq(cont_encoding, bodypart_get_content(att, BP_CONT_ENCODING)))
	    return FALSE;
	clear_error();
	if (!encoding_is_reasonable(cont_encoding))
	    continue;
	bodypart_set_content(att, BP_CONT_ENCODING, cont_encoding);
	return TRUE;
    }
    /*NOTREACHED*/
}


static int at_do_change_descrip(att, do_redraw_p)
SEND_BODYPART *att;
int *do_redraw_p;
{
    char cont_descrip[SLEN];

    (void) strfcpy(cont_descrip, bodypart_get_content(att, BP_CONT_DESCRIPTION),
		sizeof(cont_descrip));
    *do_redraw_p |= (ATDRAW_INSTR|ATDRAW_PROMPT|ATDRAW_CURR_DESCR);
    at_disp_instr(
		S_(AttachChgDescripInstrNorm,
"Enter attachment content description, CTRL/D to abort."),
		S_(AttachChgDescripInstrDummy1,
"Enter the \"content description\", or CTRL/D to abort entry."),
		S_(AttachChgDescripInstrDummy2,
"This is just a brief comment to tell the recipient about the attachment."),
		TRUE);
    ClearLine(ATLINE_PROMPT);

    for (;;) {
	if (enter_string(cont_descrip, sizeof(cont_descrip),
		    ATLINE_CURR_DESCR, ATCOL_CURR_DATA, ESTR_UPDATE) < 0)
	    return FALSE;
	if (streq(cont_descrip, bodypart_get_content(att, BP_CONT_DESCRIPTION)))
	    return FALSE;
	clear_error();
	bodypart_set_content(att, BP_CONT_DESCRIPTION, cont_descrip);
	return TRUE;
    }
    /*NOTREACHED*/
}


static int at_do_change_disposition(att, do_redraw_p)
SEND_BODYPART *att;
int *do_redraw_p;
{
    const char *cp;
    char orig_fname[SLEN], new_fname[SLEN], *s;
    int len;

    *do_redraw_p |= (ATDRAW_INSTR|ATDRAW_PROMPT|ATDRAW_CURR_DISP);
    at_disp_instr(
		S_(AttachChgDispositionInstrNorm,
"Enter suggested filename (empty OK), CTRL/D to abort."),
		S_(AttachChgDispositionInstrDummy1,
"Enter a suggested filename under which the recipient might want to save"),
		S_(AttachChgDispositionInstrDummy2,
"this attachment (empty filename is OK), or CTRL/D to abort entry."),
		TRUE);
    ClearLine(ATLINE_PROMPT);

    /*
     * The Content-Disposition header is in a format:
     *     attachment; filename="/suggested/attachment/filename"
     * The user will be editing just the filename.  Here, we parse
     * the filename out of the existing header.
     */
    orig_fname[0] = '\0';
    cp = bodypart_get_content(att, BP_CONT_DISPOSITION);
    while ((cp = strchr(cp, ';')) != NULL) {
	++cp;					/* skip semicolon	*/
	while (isspace(*cp))			/* advance to name	*/
	    ++cp;
	if (!strbegConst(cp, "filename="))	/* "filename=" param?	*/
	    continue;
	cp += (sizeof("filename=")-1);		/* point to arg		*/
	if (*cp == '"') {
	    /* filename is in quotes */
	    len = len_next_part(cp) - 2;
	    if (len >= sizeof(orig_fname))
		len = sizeof(orig_fname)-1;
	    (void) strfcpy(orig_fname, cp+1, len+1);
	} else {
	    /* filename is unquoted */
	    (void) strfcpy(orig_fname, cp, sizeof(orig_fname));
	    for (s = orig_fname ; *s != '\0' && !isspace(*s) ; ++s)
		;
	    *s = '\0';
	}
    }
    (void) strcpy(new_fname, orig_fname);

    for (;;) {
	PutLine0(ATLINE_CURR_DISP, ATCOL_CURR_DATA, "attachment; filename=");
	if (enter_string(new_fname, sizeof(new_fname), -1, -1, ESTR_UPDATE) < 0)
	    return FALSE;
	if (streq(new_fname, orig_fname))
	    return FALSE;
	clear_error();
	if (new_fname[0] != '\0') {
	    sprintf(orig_fname, "attachment; filename=\"%s\"", new_fname);
	    bodypart_set_content(att, BP_CONT_DISPOSITION, orig_fname);
	} else {
	    bodypart_set_content(att, BP_CONT_DISPOSITION, "attachment");
	}
	return TRUE;
    }
    /*NOTREACHED*/
}


static char *strtruncate(str, len)
char *str;
int len;
{
    int i;

    if (strlen(str) <= len)
	return str;

    while (len > 0 && isspace(str[len-1]))
	--len;

    /* see if there is a word boundary near the end */
    for (i = 4 ; i < 10 ; ++i) {
	if (isspace(str[len-i])) {
		len = len-i+1;
		while (len > 0 && isspace(str[len-1]))
		    --len;
		(void) strcpy(str+len, " ...");
		return str;
	}
    }

    /* nope - just chop off the tail */
    (void) strcpy(str+len-4, " ...");
    return str;
}


static void atlist_initialize()
{
    int i;

    for (i = 0 ; i < AT_MAX_ATTACH ; ++i) {
	at_attachmenu_list[i].att = &at_bogus;
	at_attachmenu_list[i].flags = (~0);
    }
    at_attachmenu_count = 0;
}


static int atlist_full()
{
    return (at_attachmenu_count >= AT_MAX_ATTACH);
}


static void atlist_insert(sel, att, flags)
int sel;
SEND_BODYPART *att;
int flags;
{
    int i;

    assert(at_attachmenu_count >= 0 && at_attachmenu_count < AT_MAX_ATTACH);
    assert(sel >= 0 && sel <= at_attachmenu_count);

    for (i = at_attachmenu_count ; i > sel ; --i) {
	assert(at_attachmenu_list[i-1].att != &at_bogus);
	at_attachmenu_list[i].att = at_attachmenu_list[i-1].att;
	at_attachmenu_list[i].flags = at_attachmenu_list[i-1].flags;
    }
    at_attachmenu_list[sel].att = att;
    at_attachmenu_list[sel].flags = flags;
    ++at_attachmenu_count;
}


static void atlist_remove(sel)
int sel;
{
    int i;

    assert(at_attachmenu_count > 0 && at_attachmenu_count <= AT_MAX_ATTACH);
    assert(sel >= 0 && sel < at_attachmenu_count);
    assert(at_attachmenu_list[sel].att != &at_bogus);

    bodypart_destroy(at_attachmenu_list[sel].att);

    for (i = sel+1 ; i < at_attachmenu_count ; ++i) {
	assert(at_attachmenu_list[i].att != &at_bogus);
	at_attachmenu_list[i-1].att = at_attachmenu_list[i].att;
	at_attachmenu_list[i-1].flags = at_attachmenu_list[i].flags;
    }
    at_attachmenu_list[at_attachmenu_count].att = &at_bogus;
    at_attachmenu_list[at_attachmenu_count].flags = (~0);
    --at_attachmenu_count;
}


static void atlist_replace(sel, att, flags)
int sel;
SEND_BODYPART *att;
int flags;
{
    assert(at_attachmenu_count > 0 && at_attachmenu_count <= AT_MAX_ATTACH);
    assert(sel >= 0 && sel < at_attachmenu_count);
    assert(at_attachmenu_list[sel].att != &at_bogus);

    if (att != NULL && att != at_attachmenu_list[sel].att) {
	bodypart_destroy(at_attachmenu_list[sel].att);
	at_attachmenu_list[sel].att = att;
    }
    if (flags != ~0)
	at_attachmenu_list[sel].flags = flags;
}


static SEND_BODYPART *atlist_getbodypart(sel)
int sel;
{
    assert(at_attachmenu_count > 0 && at_attachmenu_count <= AT_MAX_ATTACH);
    assert(sel >= 0 && sel < at_attachmenu_count);
    assert(at_attachmenu_list[sel].att != &at_bogus);

    return at_attachmenu_list[sel].att;
}


static int atlist_getflags(sel)
int sel;
{
    assert(at_attachmenu_count > 0 && at_attachmenu_count <= AT_MAX_ATTACH);
    assert(sel >= 0 && sel < at_attachmenu_count);
    assert(at_attachmenu_list[sel].att != &at_bogus);

    return at_attachmenu_list[sel].flags;
}


static int atlist_tagged_clrcnt(do_clear)
int do_clear;	/* if TRUE clear all tags, if FALSE just return count */
{
    int count, i;

    assert(at_attachmenu_count > 0 && at_attachmenu_count <= AT_MAX_ATTACH);

    count = 0;
    for (i = 0 ; i < at_attachmenu_count ; ++i) {
	if (!(at_attachmenu_list[i].flags & AT_FL_NOTOUCH)
		    && (at_attachmenu_list[i].flags & AT_FL_TAGGED)) {
	    if (do_clear)
		at_attachmenu_list[i].flags &= ~AT_FL_TAGGED;
	    ++count;
	}
    }
    return count;
}


static int atlist_tagged_move(sel)
int sel;
{
    int nmoved, i, j;
    struct at_attachmenu_entry tmp_att;

    assert(at_attachmenu_count > 0 && at_attachmenu_count <= AT_MAX_ATTACH);
    assert(sel >= 0 && sel < at_attachmenu_count);

    nmoved = 0;

    /*
     * move attachments left of selection:
     *
     * before:		_a_ _b_ _c_ _d_ _e_ _f_ _g_ _h_
     *		                TAG         SEL
     *			         i
     *
     * after:		_a_ _b_ _d_ _e_ _f_ _c_ _g_ _h_
     *		                        SEL TAG
     *			         i
     *
     * Several important effects:
     *  - The move modifies the index of the selected attachment,
     *    so the "sel" value must be adjusted.
     *  - Adjustment is required or else the (++i) for the next
     *    loop iteration will skip "_d_".
     *  - The target of the next move should NOT be after "sel"
     *    but rather "sel+1".  Thus we need to track "nmoved".
     */

    /* move attachments left of selection (note this modifies the sel value) */
    for (i = 0 ; i < sel ; ++i) {
	if (at_attachmenu_list[i].flags & AT_FL_NOTOUCH)
	    continue;
	if (!(at_attachmenu_list[i].flags & AT_FL_TAGGED))
	    continue;
	tmp_att.att = at_attachmenu_list[i].att;
	tmp_att.flags = at_attachmenu_list[i].flags;
	for (j = i ; j < sel+nmoved ; ++j) {
	    at_attachmenu_list[j].att = at_attachmenu_list[j+1].att;
	    at_attachmenu_list[j].flags = at_attachmenu_list[j+1].flags;
	}
	at_attachmenu_list[sel+nmoved].att = tmp_att.att;
	at_attachmenu_list[sel+nmoved].flags = tmp_att.flags;
	--sel;
	--i;
	++nmoved;
    }

    /*
     * move attachments right of selection:
     *
     * before:		_a_ _b_ _c_ _d_ _e_ _f_ _g_ _h_
     *		                SEL         TAG
     *			                     i
     *
     * after:		_a_ _b_ _c_ _f_ _d_ _e_ _g_ _h_
     *		                SEL TAG     
     *			                     i
     *
     * This is much simpler than the previous case.  The only
     * side effect of concern is that making sure the target
     * index for subsequent moves is adjusted by "nmoved".
     */

    for (i = sel+nmoved+1 ; i < at_attachmenu_count ; ++i) {
	if (at_attachmenu_list[i].flags & AT_FL_NOTOUCH)
	    continue;
	if (!(at_attachmenu_list[i].flags & AT_FL_TAGGED))
	    continue;
	tmp_att.att = at_attachmenu_list[i].att;
	tmp_att.flags = at_attachmenu_list[i].flags;
	for (j = i ; j > sel+nmoved+1 ; --j) {
	    at_attachmenu_list[j].att = at_attachmenu_list[j-1].att;
	    at_attachmenu_list[j].flags = at_attachmenu_list[j-1].flags;
	}
	at_attachmenu_list[sel+nmoved+1].att = tmp_att.att;
	at_attachmenu_list[sel+nmoved+1].flags = tmp_att.flags;
	++nmoved;
    }

    return sel;
}

