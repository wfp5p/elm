
static char rcsid[] = "@(#)$Id: in_utils.c,v 1.5 1996/05/09 15:51:21 wfp5p Exp $";

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
 * $Log: in_utils.c,v $
 * Revision 1.5  1996/05/09  15:51:21  wfp5p
 * Alpha 10
 *
 * Revision 1.4  1996/03/14  17:29:38  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:14  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:10  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:36  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"
#include <setjmp.h>
#include "elm_globals.h"
#include "s_elm.h"
#include <assert.h>


struct edit_field {
    char *buf;		/* space for editing buffer			*/
    int max_len;	/* max chars to accept (excluding '\0')		*/
    int line;		/* screen line of editing field			*/
    int beg_col;	/* starting column of editing field		*/
    int disp_width;	/* number of columns in field display		*/
    int curr_len;	/* current number of chars in "buf"		*/
    int curr_pos;	/* cursor into "buf"				*/
    int curr_offset;	/* offset into "buf" at which display begins	*/
    int options;	/* option settings				*/
};

/* option flags */
#define ED_BEGFRONT	(1<<0)	/* start with cursor at front of field	*/
#define ED_PASSWORD	(1<<1)	/* password mode - no echo or output	*/

#define edfld_currlen(ed)	((ed)->curr_len)
#define edfld_currpos(ed)	((ed)->curr_pos)

#ifdef NDEBUG
# define edfld_integrity_check(ed)
#else
  void edfld_integrity_check P_((struct edit_field *));
#endif
static struct edit_field *edfld_new P_((char *, int, int, int, int));
void edfld_destroy P_((struct edit_field *));
static void edfld_output_field P_((struct edit_field *, int));
static void edfld_output_cursor P_((struct edit_field *));
static void edfld_redraw P_((struct edit_field *));
static int edfld_setpos P_((struct edit_field *, int));
static int edfld_clear P_((struct edit_field *));
static int edfld_delch P_((struct edit_field *, int));
static int edfld_insch P_((struct edit_field *, int));
static int edfld_nextword P_((struct edit_field *));
static int edfld_prevword P_((struct edit_field *));


/*
 * enter_yn() - Prompt for a yes/no response.
 *
 * Ask "question" and obtain a yes or no answer (dealing with NLSisms).
 * Returns TRUE if the user selects "yes", FALSE if "no" is selected.
 *
 * The "dflt" should be TRUE, FALSE, or -1 (no default).  The
 * default will be returned if the user strikes ENTER.
 *
 * The prompt will be made on the indicated "line".  If "clear_and_center"
 * set then the prompt will be centered, else it will be right justified.
 */

PUBLIC int enter_yn(question, dflt, line, clear_and_center)
char *question;
int dflt, line, clear_and_center;
{
    int yes_len, no_len, col, len, ch, ans, prev_keys;
    char *yes_display, *no_display;

    yes_display = catgets(elm_msg_cat, ElmSet, ElmYesWord, "Yes.");
    no_display = catgets(elm_msg_cat, ElmSet, ElmNoWord, "No.");
    yes_len = strlen(yes_display);
    no_len = strlen(no_display);

    prev_keys = define_softkeys(SOFTKEYS_YESNO);

    len = strlen(question) + (sizeof(" (y/n) ")-1)
		+ (yes_len > no_len ? yes_len : no_len);
    if (clear_and_center) {
	col = (COLS-len)/2;
	ClearLine(line);
    } else {
	col = COLS - len - 1;
    }

    PutLine1(line, col, question);
    PutLine2(-1, -1, " (%c/%c) ", *def_ans_yes, *def_ans_no);
    if (!clear_and_center)
	CleartoEOLN();
    switch (dflt) {
    case TRUE:
	WriteChar(*def_ans_yes);
	WriteChar(BACKSPACE);
	break;
    case FALSE:
	WriteChar(*def_ans_no);
	WriteChar(BACKSPACE);
	break;
    }

    for (;;) {
	ch = ReadCh();
	if ((ch == '\r' || ch == '\n') && dflt >= 0) {
	    ans = dflt;
	    break;
	}
	ch = tolower(ch);
	if (ch == *def_ans_yes) {
	    ans = TRUE;
	    break;
	}
	if (ch == *def_ans_no) {
	    ans = FALSE;
	    break;
	}
	Beep();
    }

    if (sleepmsg > 0) {
	PutLine0(-1, -1, (ans ? yes_display : no_display));
	FlushOutput();
	sleep((sleepmsg + 1) / 2);
    }
    MoveCursor(line, col);
    CleartoEOLN();

    (void) define_softkeys(prev_keys);
    return ans;
}


/*
 * enter_number() - Prompt for a numeric answer.
 *
 * Ask user to enter a number of "thing".  Return value entered.
 * Return "dfltval" if entry aborted or no value entered.
 */

PUBLIC int enter_number(line, dfltval, thing)
int line, dfltval;
char *thing;
{
#define MAXDIG 8
    char buf[MAXDIG+1], *prompt;
    int len;

    prompt = catgets(elm_msg_cat, ElmSet, ElmSetCurrentTo,
	    "Set current %s to : ");
    len = ((int)strlen(prompt)-(sizeof("%s")-1)) + (int)strlen(thing) + MAXDIG;

    PutLine1(line, COLS-(len+1), prompt, thing);
    if (enter_string(buf, sizeof(buf), -1, -1, ESTR_NUMBER) < 0
		|| buf[0] == '\0')
	return dfltval;

    return atoi(buf);
}


/*
 * enter_string() - Enter a string value into "buf" with editing.
 * Returns 0 normally, -1 on abort or input error.
 *
 * Modes are:
 *
 * ESTR_ENTER - A new value is entered.
 *
 * ESTR_REPLACE - The "buf" must be initialized to a value.  That
 * value is displayed.  If the user just strikes ENTER, that value
 * is maintained.  If the users presses the right arrow or delete,
 * the user may edit and update the value.  Otherwise the entered
 * value replaces it.
 *
 * ESTR_UPDATE - The "buf" must be initialized to a value.  That
 * value is displayed, and the cursor is placed at the end of
 * the string where the user may edit and update the value.
 * The updated value is returned.
 *
 * ESTR_NUMBER - Just like ESTR_ENTER, except only digits are accepted as input.
 *
 * ESTR_PASSWORD - Just like ESTR_ENTER, except don't echo.  Also, some
 * of the fancier functions (such as cursor movement) are inhibited.
 */

PUBLIC int enter_string(buf, bufsiz, line, col, mode)
char *buf;
int bufsiz, line, col, mode;
{
    int ch, rc, i;
    struct edit_field *ed;

    rc = -1;
    if (line < 0 || col < 0)
	GetCursorPos(&line, &col);

    switch (mode) {
    case ESTR_REPLACE:
	/* either accept current value or enter a new one */
	ed = edfld_new(buf, bufsiz, line, col, ED_BEGFRONT);
	break;
    case ESTR_UPDATE:
	/* edit current value */
	ed = edfld_new(buf, bufsiz, line, col, 0);
	break;
    case ESTR_PASSWORD:
	/* enter new value using passwd mode */
	buf[0] = '\0';
	ed = edfld_new(buf, bufsiz, line, col, ED_PASSWORD);
	break;
    case ESTR_ENTER:
    case ESTR_NUMBER:
	/* enter new value */
	buf[0] = '\0';
	ed = edfld_new(buf, bufsiz, line, col, 0);
	break;
    default:
	error1(catgets(elm_msg_cat, ElmSet, ElmEnterStringBadMode,
		    "INTERNAL ERROR - bad mode \"%d\" for enter_string"), mode);
	return -1;
    }

    for (;;) {

	ch = GetKey(0);

	/* accept entry */
	if (ch == '\r' || ch == '\n') {
	    rc = 0;
	    break;
	}

	/* abort entry */
	if (ch == ctrl('D') || ch == KEY_REDRAW)
	    break;

	/* if in replace mode and user isn't moving into default answer,
	 * must have decided to replace existing value, so erase old value */
	if (mode == ESTR_REPLACE && !(ch == KEY_RIGHT || ch == KEY_DC))
	    edfld_clear(ed);

	/* remap keys to fixed constants to allow switch() statement */
	if (ch == Term.erase_char)
	    ch = ctrl('H');
	if (ch == Term.werase_char)
	    ch = ctrl('W');
	if (ch == Term.kill_char)
	    ch = ctrl('U');

	switch (ch) {

	case ctrl('R'):				/* redraw */
	    edfld_redraw(ed);
	    break;

	case ctrl('H'):				/* delete character */
	    if (edfld_delch(ed, 1) < 0) {
		if (mode != ESTR_REPLACE)
		    Beep();
	    }
	    break;

	case KEY_DC:				/* delete char under cursor */
	case 0x7F:
	    if (edfld_delch(ed, 0) < 0) {
		if (mode != ESTR_REPLACE)
		    Beep();
	    }
	    break;

	case ctrl('W'):				/* delete word */
	    if ((i = edfld_prevword(ed)) < 0 || edfld_delch(ed, i) < 0) {
		if (mode != ESTR_REPLACE)
		    Beep();
	    }
	    break;

	case ctrl('U'):				/* delete line */
	    if (edfld_clear(ed) < 0) {
		if (mode != ESTR_REPLACE)
		    Beep();
	    }
	    break;

	case KEY_LEFT:				/* move left char */
	case ctrl('B'):
	    if (edfld_setpos(ed, edfld_currpos(ed)-1) < 0)
		Beep();
	    break;

	case KEY_RIGHT:				/* move right char */
	case ctrl('F'):
	    if (edfld_setpos(ed, edfld_currpos(ed)+1) < 0)
		Beep();
	    break;

	case KEY_BTAB:				/* move left word */
	case ctrl('P'):
	    if ((i = edfld_prevword(ed)) < 0
			|| edfld_setpos(ed, edfld_currpos(ed)-i) < 0)
		Beep();
	    break;

	case '\t':				/* move right word */
	    if ((i = edfld_nextword(ed)) < 0
			|| edfld_setpos(ed, edfld_currpos(ed)+i) < 0)
		Beep();
	    break;

	case KEY_HOME:				/* move beginning of line */
	case ctrl('A'):
	    if (edfld_setpos(ed, 0) < 0)
		Beep();
	    break;

	case KEY_END:				/* move end of line */
	case ctrl('E'):
	    if (edfld_setpos(ed, edfld_currlen(ed)) < 0)
		Beep();
	    break;

	default:				/* character input */
	    switch (mode) {
	    case ESTR_PASSWORD:
		if (ch > 0xFF)
		    ch = 0;
		break;
	    case ESTR_NUMBER:
		if (ch > 0xFF || !isdigit(ch))
		    ch = 0;
		break;
	    default:
		if (ch > 0xFF || !isprint(ch))
		    ch = 0;
		break;
	    }
	    if (ch == 0 || edfld_insch(ed, ch) < 0)
		Beep();
	    break;

	}

	/* from this point on, REPLACE acts like ENTER mode */
	if (mode == ESTR_REPLACE)
	    mode = ESTR_ENTER;

    }

    edfld_destroy(ed);
    return rc;
}


/*
 * GetKey() - Retrieve a single keystroke.
 *
 * If the "wait_time" is non-zero, it indicates a timeout in seconds.
 * If the timer expires before entry is made, the special "KEY_TIMEOUT"
 * is returned.
 *
 * If a SIGCONT or SIGWINCH occurs while waiting for input,
 * then KEY_REDRAW is returned.
 *
 * If the key appears to be the start of a special function key
 * sequence, the remaining characters in the sequence will be read,
 * and the corresponding code (such as KEY_UP) is returned.  If the
 * sequence cannot be mapped to a known special function key, then
 * KEY_UNKNOWN is returned.
 *
 * Otherwise, the character entered by the user is returned.
 */

PUBLIC int GetKey(wait_time)
int wait_time;
{

    int first_key, ch, rc;
#ifndef ANSI_C
    extern unsigned alarm();
#endif

    /*
     * On the whole, Elm is pretty dumb about handling screen changes.
     * Since something that calls GetKey() should be prepared to handle
     * it, now is a good opportunity to see if there has been a recent
     * change that hasn't been handled yet.
     */
    if (caught_screen_change_sig) {
	ResizeScreen();
	return KEY_REDRAW;
    }

    /* activate interrupt handler */
    if ((rc = SETJMP(GetKey_jmpbuf)) != 0) {
	/*
	 * If we are running this, it means a signal handler has
	 * triggered a longjmp() call.  The signal number is saved
	 * in "rc".  At this time, the only signals that (should!)
	 * trigger this are SIGWINCH, SIGCONT, and SIGALRM.
	 */
	GetKey_active = FALSE;
	if (wait_time > 0)
	    alarm((unsigned) 0);
#if defined(SIGWINCH) || defined(SIGCONT)
	if (rc != SIGALRM) {
	    ResizeScreen();
	    return KEY_REDRAW;
	}
#endif
	return KEY_TIMEOUT;
    }

    /* initialize special key parser */
    (void) knode_parse(0);

    /* read in the keystroke */
    GetKey_active = TRUE;
    first_key = TRUE;
    for (;;) {
	if (wait_time > 0)
	    alarm((unsigned) wait_time);
	if ((ch = ReadCh()) < 0 || (rc = knode_parse(ch)) != 0)
	    break;
	first_key = FALSE;
    }
    if (wait_time > 0)
	alarm((unsigned) 0);
    GetKey_active = FALSE;

    /* got a special function key */
    if (ch > 0 && rc > 0)
	return rc;

    if (!first_key) {
	/*
	 * The user has struck a multi-byte special function key that
	 * we do not understand.  Return a special code.
	 *
	 * There is a potential problem here.  If the special function
	 * key sequences are varying lengths, then there could be more
	 * bytes pending in this sequence.  Fortunately, in practice,
	 * terminals tend to use same length strings for all the special
	 * function keys.  (Exception is e.g. Wyse terminals, which have
	 * some single-byte keys, but those are not loaded into the
	 * knode_parse() tree.)
	 */
	 ch = KEY_UNKNOWN;
    }

    return ch;
}



#ifndef NDEBUG
void edfld_integrity_check(ed)
struct edit_field *ed;
{
    assert(ed->line >= 0 && ed->line <= LINES);
    assert(ed->beg_col >= 0 && ed->beg_col < COLS);
    assert(ed->disp_width == (COLS - ed->beg_col));
    assert(ed->curr_len >= 0 && ed->curr_len <= ed->max_len);
    assert(ed->curr_len == strlen(ed->buf));
    assert(ed->curr_pos >= 0 && ed->curr_pos <= ed->curr_len);
    assert(ed->curr_offset >= 0 && ed->curr_offset <= ed->curr_len);
    assert(ed->curr_pos >= ed->curr_offset);
    assert(ed->curr_pos <= ed->curr_offset+ed->disp_width);
}
#endif


/* create a new editing buffer */
static struct edit_field *edfld_new(buf, bufsiz, line, beg_col, options)
char *buf;
int bufsiz, line, beg_col, options;
{
    struct edit_field *ed;

    ed = (struct edit_field *) safe_malloc(sizeof(struct edit_field));
    ed->buf = buf;
    ed->max_len = bufsiz-1;	/* reserve space for '\0' */
    ed->line = line;
    ed->beg_col = beg_col;
    ed->disp_width = COLS - ed->beg_col;
    ed->curr_len = strlen(buf);
    if (options & ED_BEGFRONT) {
	ed->curr_pos = 0;
	ed->curr_offset = 0;
    } else {
	ed->curr_pos = ed->curr_len;
	if (ed->curr_len >= ed->disp_width)
	    ed->curr_offset = (ed->curr_len - ed->disp_width) + 1;
	else
	    ed->curr_offset = 0;
    }
    ed->options = options;

    edfld_output_field(ed, ed->curr_offset);

    return ed;
}


/* destroy an existing editing buffer */
void edfld_destroy(ed)
struct edit_field *ed;
{
    free((malloc_t)ed);
}


/* position the cursor */
static void edfld_output_cursor(ed)
struct edit_field *ed;
{
    edfld_integrity_check(ed);
    if (ed->options & ED_PASSWORD)
	MoveCursor(ed->line, ed->beg_col);
    else
	MoveCursor(ed->line, ed->beg_col+ed->curr_pos-ed->curr_offset);
}


/* redraw field starting at char position "start_idx" */
static void edfld_output_field(ed, start_idx)
struct edit_field *ed;
int start_idx;
{
    int stop_idx, nch;
    const char *s;

    edfld_integrity_check(ed);

    if (ed->options & ED_PASSWORD) {
	MoveCursor(ed->line, ed->beg_col);
	CleartoEOLN();
    }

    stop_idx = ed->curr_offset + (ed->disp_width-1);
    assert(start_idx >= ed->curr_offset && start_idx <= stop_idx);

    MoveCursor(ed->line, ed->beg_col+(start_idx-ed->curr_offset));
    if (start_idx < ed->curr_len) {
	/* display until end of string or end of field is reached */
	nch = (stop_idx - start_idx) + 1;
	for (s = ed->buf + start_idx ; *s && --nch >= 0 ; ++s)
	    WriteChar(*s);
    }
    if (stop_idx >= ed->curr_len) {
	/* stopped before end of field, erase to end of line */
	CleartoEOLN();
    }

    edfld_output_cursor(ed);
}


/* scroll the text "nch" (left < 0, right > 0) chars and update display */
static void edfld_output_scroll(ed, nch)
struct edit_field *ed;
int nch;
{
    char *s;

    assert(nch != 0);
    ed->curr_offset -= nch;
    edfld_integrity_check(ed);

    if (nch < 0) {
	/* scroll text left */
	nch = -nch;
	if (nch < ed->disp_width/4 && (Term.status & TERM_CAN_DC)) {
	    MoveCursor(ed->line, ed->beg_col);
	    DeleteChar(nch);
	    edfld_output_field(ed, ed->curr_offset+ed->disp_width-nch);
	} else {
	    edfld_output_field(ed, ed->curr_offset);
	}
    } else {
	/* scroll text right */
	if (nch < ed->disp_width/4 && (Term.status & TERM_CAN_IC)) {
	    MoveCursor(ed->line, ed->beg_col);
	    for (s = ed->buf + ed->curr_offset ; --nch >= 0 ; ++s)
		InsertChar(*s);
	    edfld_output_cursor(ed);
	} else {
	    edfld_output_field(ed, ed->curr_offset);
	}
    }

}


/* redisplay entire (visible portion of) editing buffer */
static void edfld_redraw(ed)
struct edit_field *ed;
{
    edfld_output_field(ed, ed->curr_offset);
}


/* set cursor position within editing buffer */
static int edfld_setpos(ed, pos)
struct edit_field *ed;
int pos;
{
    edfld_integrity_check(ed);

    if (pos < 0 || pos > ed->curr_len || ed->options & ED_PASSWORD)
	return -1;
    ed->curr_pos = pos;

    if (ed->curr_pos < ed->curr_offset) {
	/* cursor moved off left-end of field - scroll text right */
	edfld_output_scroll(ed, ed->curr_offset-ed->curr_pos);
    } else if (ed->curr_pos >= ed->curr_offset+ed->disp_width) {
	/* cursor moved off right-end of field - scroll text left */
	edfld_output_scroll(ed,
		    (ed->curr_offset+ed->disp_width-1) - ed->curr_pos);
    } else {
	/* cursor still within displayed field */
	edfld_output_cursor(ed);
    }
    return 0;
}


/* erase editing buffer */
static int edfld_clear(ed)
struct edit_field *ed;
{
    edfld_integrity_check(ed);

    if (ed->curr_len == 0)
	return -1;

    ed->buf[0] = '\0';
    ed->curr_len = 0;
    ed->curr_pos = 0;
    ed->curr_offset = 0;

    edfld_output_field(ed, 0);
    return 0;
}


/* delete number of chars to left of cursor (0 == char under cursor) */
static int edfld_delch(ed, nch)
struct edit_field *ed;
int nch;
{
    int old_pos, old_len, old_offset, i;
    char *s;

    edfld_integrity_check(ed);

    /* handle special case for delete char under cursor */
    if (nch == 0) {
	if (ed->curr_pos == ed->curr_len || ed->options & ED_PASSWORD)
	    return -1;
	for (s = ed->buf+ed->curr_pos ; s[1] != '\0' ; ++s)
	    s[0] = s[1];
	ed->buf[--ed->curr_len] = '\0';
	if (Term.status & TERM_CAN_DC) {
	    DeleteChar(1);
	    if (ed->curr_len >= ed->curr_offset+ed->disp_width) {
		/* fill in character position opened up by the deletion */
		edfld_output_field(ed, ed->curr_offset+ed->disp_width-1);
	    }
	} else {
	    edfld_output_field(ed, ed->curr_pos);
	}
	return 0;
    }

    if (nch < 0 || nch > ed->curr_pos)
	return -1;

    old_len = ed->curr_len;
    old_pos = ed->curr_pos;
    old_offset = ed->curr_offset;
    ed->curr_len -= nch;
    ed->curr_pos -= nch;

    for (i = old_len-old_pos, s = ed->buf+ed->curr_pos ; i > 0 ; --i, ++s)
	s[0] = s[nch];
    ed->buf[ed->curr_len] = '\0';

    if (ed->options & ED_PASSWORD) {
	; /* no output in password mode */
    } else if (Term.status & TERM_CAN_DC) {
	if (ed->curr_pos > old_offset) {
	    /* deletion totally within displayed region */
	    edfld_output_cursor(ed);
	    DeleteChar(nch);
	} else if (old_pos == old_offset) {
	    /* deletion totally beyond left side of displayed region */
/*	    ed->curr_offset = ed->curr_pos;*/
	    if (ed->curr_len >= ed->disp_width)
	     ed->curr_offset = (ed->curr_len - ed->disp_width) + 1;
	   else
	     ed->curr_offset = 0;
	   edfld_output_field(ed, ed->curr_offset);
	} else {
	    /* deletion moved off left side of displayed region */
	    ed->curr_offset = ed->curr_pos;
	    edfld_output_cursor(ed);
	    DeleteChar(old_pos-old_offset);
	}
	if (ed->curr_len >= ed->curr_offset+ed->disp_width)
	    edfld_output_field(ed, ed->curr_offset+ed->disp_width-nch);
    } else {
	if (ed->curr_pos < ed->curr_offset)
	    ed->curr_offset = ed->curr_pos;
	edfld_output_field(ed, ed->curr_pos);
    }

    return 0;
}


/* insert character at cursor then advance cursor right */
static int edfld_insch(ed, ch)
struct edit_field *ed;
int ch;
{
    int i;
    char *s;

    edfld_integrity_check(ed);

    if (ed->curr_len >= ed->max_len)
	return -1;
    for (s = ed->buf+ed->curr_len, i = ed->curr_len+1-ed->curr_pos
		; i > 0 ; --s, --i)
	s[0] = s[-1];
    ed->buf[ed->curr_pos++] = ch;
    ed->buf[++ed->curr_len] = '\0';

    if (ed->options & ED_PASSWORD) {
	; /* no output in password mode */
    } else if (Term.status & TERM_CAN_IC) {
	if (ed->curr_pos == ed->curr_offset+ed->disp_width) {
	    /* addition in rightmost position of displayed region */
	    ++ed->curr_offset;
	    MoveCursor(ed->line, ed->beg_col);
	    DeleteChar(1);
	    MoveCursor(ed->line, ed->beg_col+ed->disp_width-2);
	    WriteChar(ch);
	} else if (ed->curr_pos == ed->curr_len) {
	    /* addition within display, at end of buffer */
	    WriteChar(ch);
	} else {
	    /* addition within display, in middle of buffer */
	    InsertChar(ch);
	}
    } else {
	if (ed->curr_pos >= ed->curr_offset+ed->disp_width) {
	    /* addition in rightmost position of displayed region */
	    ++ed->curr_offset;
	    edfld_output_field(ed, ed->curr_offset);
	} else {
	    /* addition within displayed region */
	    edfld_output_field(ed, ed->curr_pos-1);
	}
    }

    return 0;
}


/* calculate number of chars right to start of next word */
static int edfld_nextword(ed)
struct edit_field *ed;
{
    int i;
    char *s;

    edfld_integrity_check(ed);

    i = 0;
    s = ed->buf+ed->curr_pos;
    for ( ; *s != '\0' && !isspace(*s) ; ++s, ++i)
	;
    for ( ; *s != '\0' && isspace(*s) ; ++s, ++i)
	;
    return (i > 0 ? i : -1);
}


/* calculate number of chars left to start of previous word */
static int edfld_prevword(ed)
struct edit_field *ed;
{
    int i, len;
    char *s;

    edfld_integrity_check(ed);

    i = ed->curr_pos;
    s = ed->buf+ed->curr_pos;
    for ( ; i > 0 && isspace(s[-1]) ; --i, --s)
	;
    for ( ; i > 0 && !isspace(s[-1]) ; --i, --s)
	;
    i -= ed->curr_pos;
    return (i < 0 ? -i : -1);
}

