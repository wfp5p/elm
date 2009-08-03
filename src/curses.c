
static char rcsid[] = "@(#)$Id: curses.c,v 1.6 1999/03/24 14:03:58 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.6 $   $State: Exp $
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
 * $Log: curses.c,v $
 * Revision 1.6  1999/03/24  14:03:58  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.5  1996/05/09  15:51:18  wfp5p
 * Alpha 10
 *
 * Revision 1.4  1996/03/14  17:27:56  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:01  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:03  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**  This library gives programs the ability to easily access the
     termcap information and write screen oriented and raw input
     programs.  The routines can be called as needed, except that
     to use the cursor / screen routines there must be a call to
     InitScreen() first.
**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "port_termios.h"
#include "s_elm.h"

#include <assert.h>
#ifdef I_STDARG
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#define S_(sel, str)	catgets(elm_msg_cat, ElmSet, (sel), (str))

#define DEFAULT_LINES 24
#define DEFAULT_COLS 80

static struct termios TC_raw_tty;	/* tty settings for raw mode	*/
static struct termios TC_original_tty;	/* tty settings for cooked mode	*/
static char TC_tcapbuf[1024];		/* storage for terminal entry	*/
static char TC_tcapstrs[1024];		/* storage for extracted strings*/
static int TC_curr_line, TC_curr_col;	/* current cursor position	*/

/* termcap capabilities */
static int TC_errors;
static char *TC_start_termcap, *TC_end_termcap;
static char *TC_clearscreen, *TC_cleartoeoln, *TC_cleartoeos;
static char *TC_moveto, *TC_up, *TC_down, *TC_right, *TC_left;
static char *TC_start_insert, *TC_end_insert, *TC_char_insert, *TC_pad_insert;
static char *TC_start_delete, *TC_end_delete, *TC_char_delete;
static char *TC_start_standout, *TC_end_standout;
static char *TC_transmit_on, *TC_transmit_off;
static int TC_lines, TC_columns, TC_tabspacing;
static int TC_automargin, TC_eatnewlineglitch;

static void tget_complain P_((const char *, const char *));


/* 
 * Multi-byte special function keys are parsed by traversing
 * a tree constructed of (struct knode) elements.
 */
#define KCHUNK 8
struct knode {
	int kcode;		/* return value for terminal nodes	*/
	char *kval;		/* list of "next byte" values		*/
	struct knode **knext;	/* nodes corresponding to those values	*/
	int keylist_len;	/* number entries in above lists	*/
	int keylist_alloc;	/* allocated size of these lists	*/
};

/* root of the multi-byte parsing tree */
static struct knode *Knode_root;

static struct knode *knode_new P_((void));
static int knode_addkey P_((const char *, char **, int));

static void SetScreenSize P_((void));
static int outchar P_((int));

extern char *tgetstr(), *tgoto();


PUBLIC int InitScreen()
{
    int i;
    char *tp, *cp;

    assert(OPMODE_IS_INTERACTIVE(opmode));

    if ((cp = getenv("TERM")) == NULL) {
	fprintf(stderr, S_(ElmInitScreenNoTerm,
"You must set the \"TERM\" environment parameter so that I know what\n\
kind of terminal you are using.\n"));
	return -1;
    }

    i = tgetent(TC_tcapbuf, cp);
    switch (i) {
    case 1:
	/* ok! */
	break;
    case 0:
	fprintf(stderr, S_(ElmInitScreenUnknownTerm,
"I cannot find any information on terminal type \"%s\" in your\n\
termcap/terminfo database.  Please check your \"TERM\" setting.\n"), cp);
	return -1;
    case -1:
	fprintf(stderr, S_(ElmInitScreenNoTcap,
"I cannot access your termcap/terminfo database to load the\n\
information on your terminal.\n"));
	return -1;
    default:
	fprintf(stderr, S_(ElmInitScreenGetentFailed,
"Unable to retrieve termcap/terminfo database entry for your\n\
terminal (tgetent return code %d).\n"), i);
	return -1;
    }

    /* load in termcap capabilities we need */
    TC_errors = 0;
    tp = TC_tcapstrs;

    TC_start_termcap = tgetstr("ti", &tp);
    TC_end_termcap = tgetstr("te", &tp);

    TC_clearscreen = tgetstr("cl", &tp);
    if (!TC_clearscreen)
	tget_complain("cl", S_(ElmInitScreenTcapcl, "clear screen"));
    TC_cleartoeoln = tgetstr("ce", &tp);
    if (!TC_cleartoeoln)
	tget_complain("ce", S_(ElmInitScreenTcapce, "clear to end of line"));
    TC_cleartoeos = tgetstr("cd", &tp);
    if (!TC_cleartoeos)
	tget_complain("cd", S_(ElmInitScreenTcapcd, "clear to end of display"));

    TC_moveto = tgetstr("cm", &tp);
    if (!TC_moveto)
	tget_complain("cm", S_(ElmInitScreenTcapcm, "cursor motion"));

    TC_up = tgetstr("up", &tp);
    if (!TC_up)
	tget_complain("cm", S_(ElmInitScreenTcapcu, "move cursor up"));

    TC_down = tgetstr("do", &tp);
    if (TC_down && streq(TC_down, "\n"))
	TC_down = NULL;

    TC_right = tgetstr("nd", &tp);
    if (!TC_right)
	tget_complain("nd", S_(ElmInitScreenTcapnd, "move cursor right"));

    TC_left = tgetstr("le", &tp);
    if (!TC_left) {
	if (tgetflag("bs")) {
	    if ((TC_left = tgetstr("bc", &tp)) == NULL)
		TC_left = "\b";
	} else {
#ifdef notdef
	    tget_complain("le", S_(ElmInitScreenTcaple, "move cursor left"));
#else
	    /*
	     * In theory, we ought to be bailing out here.  If the system
	     * doesn't enable the "bs" flag, then it ought to define
	     * a "bc" capability.  In practice, some termcap entries are
	     * busted, and do neither when backspace should be used.
	     * So rather than complain, just presume backspace is ok.
	     */
	    TC_left = "\b";
#endif
	}
    }

    TC_start_insert = tgetstr("im", &tp);
    TC_end_insert = tgetstr("ei", &tp);
    TC_char_insert = tgetstr("ic", &tp);
    TC_pad_insert = tgetstr("ip", &tp);

    TC_start_delete = tgetstr("dm", &tp);
    TC_end_delete = tgetstr("ed", &tp);
    TC_char_delete = tgetstr("dc", &tp);

    TC_start_standout = tgetstr("so", &tp);
    TC_end_standout = tgetstr("se", &tp);
    TC_transmit_on = tgetstr("ks", &tp);
    TC_transmit_off = tgetstr("ke", &tp);

    if ((TC_lines = tgetnum("li")) < 0)
	TC_lines = DEFAULT_LINES;
    if ((TC_columns = tgetnum("co")) < 0)
	TC_columns = DEFAULT_COLS;
    if ((TC_tabspacing = tgetnum("it")) < 0)
	TC_tabspacing = 8;

    TC_automargin = tgetflag("am");
    TC_eatnewlineglitch = tgetflag("xn");

    if (TC_errors)
	return -1;

    /* see if environment overrides termcap screen size */
    if ((cp = getenv("LINES")) != NULL && sscanf(cp, "%d", &i) == 1)
	TC_lines = i;
    if ((cp = getenv("COLS")) != NULL && sscanf(cp, "%d", &i) == 1)
	TC_columns = i;

    /* get the cursor control keys... */
    Knode_root = knode_new();
    (void) knode_addkey("kd", &tp, KEY_DOWN);
    (void) knode_addkey("ku", &tp, KEY_UP);
    (void) knode_addkey("kl", &tp, KEY_LEFT);
    (void) knode_addkey("kr", &tp, KEY_RIGHT);
    (void) knode_addkey("kh", &tp, KEY_HOME);
    (void) knode_addkey("kD", &tp, KEY_DC);
    (void) knode_addkey("kI", &tp, KEY_IC);
    (void) knode_addkey("kN", &tp, KEY_NPAGE);
    (void) knode_addkey("kP", &tp, KEY_PPAGE);
    (void) knode_addkey("kB", &tp, KEY_BTAB);
    (void) knode_addkey("kH", &tp, KEY_END);	/* termcap-ish */
    (void) knode_addkey("@7", &tp, KEY_END);	/* terminfo-ish */

    /* retrieve tty line settings */
    if (tcgetattr(STDIN_FILENO, &TC_original_tty) < 0) {
	fprintf(stderr, S_(ElmInitScreenGetattrFailed,
	      "Cannot retrieve attributes for your terminal line. [%s]\n"),
	      strerror(errno));
	return -1;
    }

    /* setup configuration for raw mode */
    bcopy((char *)&TC_original_tty, (char *)&TC_raw_tty,
		sizeof(struct termios));
#if defined(TERMIO) || defined(TERMIOS)
#ifdef notdef /*FOO - I would like to run raw - but that breaks builtin editor*/
    TC_raw_tty.c_lflag &= ~(ICANON|ISIG|ECHO);
#else
    TC_raw_tty.c_lflag &= ~(ICANON|ECHO);
#endif
    TC_raw_tty.c_cc[VMIN] = 1;
    TC_raw_tty.c_cc[VTIME] = 0;
#else
    TC_raw_tty.sg_flags &= ~(ECHO);
#ifdef notdef /*FOO - I would like to run raw - but that breaks builtin editor*/
    TC_raw_tty.sg_flags |= RAW;
#else
    TC_raw_tty.sg_flags |= CBREAK;
#endif
#endif

    /* setup terminal information structure */
    Term.status = TERM_IS_INIT;
    if (TC_start_standout && TC_end_standout)
	Term.status |= TERM_CAN_SO;
    if (TC_char_delete)
	Term.status |= TERM_CAN_DC;
    if (TC_start_insert || TC_char_insert)
	Term.status |= TERM_CAN_IC;

#if defined(TERMIOS) || defined(TERMIO)
    Term.erase_char = TC_original_tty.c_cc[VERASE];
    Term.kill_char = TC_original_tty.c_cc[VKILL];
#ifdef VWERASE
    Term.werase_char = TC_original_tty.c_cc[VWERASE];
#else
    Term.werase_char = ctrl('W');
#endif
    Term.intr_char = TC_original_tty.c_cc[VINTR];
#else
    Term.erase_char = TC_original_tty.sg_erase;
    Term.kill_char = TC_original_tty.sg_kill;
    Term.werase_char = ctrl('W');
    Term.intr_char = ctrl('C');
#endif

    /* haven't a clue where the cursor is */
    InvalidateCursor();

    /* initialize window size */
    SetScreenSize();

    /* force "->" if screen cannot do standout */
    /* assuming elmrc has been processed already... */
    if (!(Term.status & TERM_CAN_SO))
	arrow_cursor = TRUE;

    return 0;
}


static void tget_complain(capname, descrip)
const char *capname;
const char *descrip;
{
    fprintf(stderr, S_(ElmInitScreenTcapMissing,
		"Your terminal does not support the \"%s\" function (%s).\n"),
		descrip, capname);
    ++TC_errors;
}


PUBLIC void ShutdownTerm()
{
    int err;

    /*
     * This routine commonly is called on the way out due to an error.
     * Preserve the errno status across the shutdown so that diagnostics
     * can be produced.
     */
    err = errno;

    if (Term.status & TERM_IS_INIT) {
	softkeys_off();
	EnableFkeys(OFF);
	MoveCursor(LINES, 0);
	NewLine();
	Raw(OFF);
    }
    Term.status &= ~TERM_IS_INIT;

    errno = err;
}


static void SetScreenSize()
{
    LINES = TC_lines - 1;  /* I'm sure I don't understand this lines-1 jazz */
    COLS = TC_columns;
#ifdef TIOCGWINSZ
    {
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
	    if (w.ws_row > 0)
		LINES = w.ws_row - 1;
	    if (w.ws_col > 0)
		COLS = w.ws_col;
	}
    }
#endif
}


/*
 * Allocate an empty (struct knode).
 */
static struct knode *knode_new()
{
    struct knode *knode;

    knode = (struct knode *) safe_malloc(sizeof(struct knode));
    knode->kcode = 0;
    knode->kval = (char *) safe_malloc(KCHUNK);
    knode->knext = (struct knode **) safe_malloc(KCHUNK*sizeof(struct knode *));
    knode->keylist_len = 0;
    knode->keylist_alloc = KCHUNK;
   return(knode);
}


/*
 * Place key rooted at "knode".  The "kstr" is the string that
 * corresponds to the value when the key is struck, and the "kcode"
 * is the code that should be assigned to this key.
 */
static int knode_addkey(capname, tp, kcode)
const char *capname;
char **tp;
int kcode;
{
    int i;
    char *kstr;
    struct knode *knode;

    if ((kstr = tgetstr(capname, tp)) == NULL || strlen(kstr) < 2)
	return -1;

    knode = Knode_root;
    while (*kstr != '\0') {

	/* see if this character already is in the tree */
	for (i = 0 ; i < knode->keylist_len ; ++i) {
	    if (knode->kval[i] == *kstr)
		    break;
	}

	/* nope ... need to add it */
	if (i >= knode->keylist_len) {
	    if (knode->keylist_len >= knode->keylist_alloc) {
		knode->keylist_alloc += KCHUNK;
		knode->kval = (char *) safe_realloc((malloc_t)knode->kval,
			    knode->keylist_alloc);
		knode->knext = (struct knode **) safe_realloc(
			    (malloc_t)knode->knext,
			    knode->keylist_alloc * sizeof(struct knode *));
	    }
	    assert(i == knode->keylist_len);
	    knode->kval[i] = *kstr;
	    knode->knext[i] = knode_new();
	    ++knode->keylist_len;
	}

	/* descend into the tree */
	knode = knode->knext[i];
	++kstr;

    }

    /* place this key code at the terminal node */
    assert(knode->keylist_len == 0);
    knode->kcode = kcode;

    return 0;
}


/*
 * Iteratively parse a multi-byte special function key.
 *
 * This routine should be called once with a "kval" of zero.
 * Then it is called iteratively for each byte in the key sequence.
 * Returns a positive value (the key code) once the key sequence is resolved.
 * Returns 0 if more bytes are needed to resolve this key.
 * Returns -1 if the sequence cannot be mapped to a key.
 */
PUBLIC int knode_parse(kval)
int kval;
{
    static struct knode *knode = NULL;
    int code, i;

    /* initialize for new key sequence */
    if (kval == 0) {
	knode = Knode_root;
	return 0;
    }

    assert(knode != NULL);

    /* locate this byte in the tree */
    code = -1;
    for (i = 0 ; i < knode->keylist_len ; ++i) {
	if (knode->kval[i] == (char)kval) {
	    knode = knode->knext[i];
	    code = knode->kcode;
	    break;
	}
    }

    if (code != 0) {
	/* either key is resolved, or it is something we don't understand */
	knode = NULL;
    }
    return code;
}


PUBLIC void EnableFkeys(newstate)
int newstate;
{
    /*
     * Emit the sequence that some terminals require to
     * enable/disable the special function keys.
     */
    if (!TC_transmit_on || !TC_transmit_off)
	return;
    if (!newstate == !(Term.status & TERM_IS_FKEY))
	return;
    if (newstate) {
	tputs(TC_transmit_on, 1, outchar);
	Term.status |= TERM_IS_FKEY;
    } else {
	tputs(TC_transmit_off, 1, outchar);
	Term.status &= ~TERM_IS_FKEY;
    }
/*    fflush(stdout);*/
    FlushOutput();
}


#if defined(SIGWINCH) || defined(SIGCONT)
/* if screen size changed, update and return TRUE */
PUBLIC void ResizeScreen()
{
    /* reset the flag that indicates a sig was caught */
    caught_screen_change_sig = 0;
#ifdef SIGWINCH
    /* reload the LINES/COLS values */
    SetScreenSize();
#endif
}
#endif /* defined(SIGWINCH) || defined(SIGCONT) */


PUBLIC void GetCursorPos(line_p, col_p)
int *line_p, *col_p;
{
    assert(TC_curr_col >= 0 && TC_curr_col < COLS);
    assert(TC_curr_line >= 0 && TC_curr_line <= LINES);
    *line_p = TC_curr_line;
    *col_p = TC_curr_col;
}


PUBLIC void InvalidateCursor()
{
    /* indicate the cursor might not be where we think it is */
    TC_curr_line = -1;
    TC_curr_col = -1;
}


PUBLIC void MoveCursor(line, col)
{
    int d_col, d_line, d_total;
    assert(col >= 0 && col < COLS && line >= 0 && line <= LINES);

    /* cursor is correct */
    if (TC_curr_col == col && TC_curr_line == line)
	return;

    /* if we don't know where the cursor is right now, perform absolute move */
    if (TC_curr_col < 0 || TC_curr_line < 0) {
do_absolute_motion:
	tputs(tgoto(TC_moveto, col, line), 1, outchar);
	TC_curr_col = col;
	TC_curr_line = line;
	return;
    }

    /* move to beginning of line */
    if (col == 0) {
	if (TC_curr_col > 0) {
	    putchar('\r');
	    TC_curr_col = 0;
	}
	if (TC_curr_line == line) {
	    /* beginning of same line */
	    return;
	}
	if (TC_curr_line == line-1) {
	    /* beginning of next line */
	    putchar('\n');
	    ++TC_curr_line;
	    return;
	}
    }

    /* see if we are close enough so that relative motion makes sense */
    d_col = col - TC_curr_col;
    d_line = line - TC_curr_line;
    d_total = (d_col < 0 ? -d_col : d_col) + (d_line < 0 ? -d_line : d_line);
    if (d_total > 5)
	goto do_absolute_motion;

    /* relative horizontal motion */
    if (d_col != 0) {
	assert(TC_left != NULL && TC_right != NULL);
	while (d_col < 0) {
	    /* negative value => physical > logical => must move left */
	    tputs(TC_left, 1, outchar);
	    ++d_col;
	}
	while (d_col > 0) {
	    /* positive value => logical > physiacl => must move right */
	    tputs(TC_right, 1, outchar);
	    --d_col;
	}
	TC_curr_col = col;
    }

    /* relative vertical motion */
    if (d_line != 0) {
	if (TC_up == NULL || TC_down == NULL)
	    goto do_absolute_motion;
	while (d_line < 0) {
	    /* negative value => physical > logical => must move up */
	    tputs(TC_up, 1, outchar);
	    ++d_line;
	}
	while (d_line > 0) {
	    /* positive value => logical > physical => must move down */
	    tputs(TC_down, 1, outchar);
	    --d_line;
	}
	TC_curr_line = line;
    }

    assert(TC_curr_col == col);
    assert(TC_curr_line == line);
}


PUBLIC void ClearScreen()
{
    assert(TC_clearscreen != NULL);
    tputs(TC_clearscreen, 1, outchar);
    TC_curr_line = TC_curr_col = 0;
    FlushOutput();
}


PUBLIC void CleartoEOLN()
{
    assert(TC_cleartoeoln != NULL);
    tputs(TC_cleartoeoln, 1, outchar);
    FlushOutput();
}


PUBLIC void CleartoEOS()
{
    assert(TC_cleartoeos != NULL);
    tputs(TC_cleartoeos, 1, outchar);
    FlushOutput();
}


PUBLIC void StartStandout()
{
    assert(TC_start_standout != NULL);
    tputs(TC_start_standout, 1, outchar);
    FlushOutput();
}


PUBLIC void EndStandout()
{
    assert(TC_end_standout != NULL);
    tputs(TC_end_standout, 1, outchar);
    FlushOutput();
}


PUBLIC void WriteChar(ch)
int ch;
{
	/** write a character to the current screen location. **/

	static int wrappedlastchar = 0;
	int justwrapped, nt;

	ch &= 0xFF;
	justwrapped = 0;

	/* if return, just go to left column. */
	if(ch == '\r') {
	  if (wrappedlastchar)
	    justwrapped = 1;                /* preserve wrap flag */
	  else {
	    putchar('\r');
	    TC_curr_col = 0;
	  }
	}

	/* if newline and terminal just did a newline without our asking,
	 * do nothing, else output a newline and increment the line count */
	else if (ch == '\n') {
	  if (!wrappedlastchar) {
	    putchar('\n');
	    if (TC_curr_line < LINES)
	      ++TC_curr_line;
	  }
	}

	/* if backspace, move back  one space  if not already in column 0 */
	else if (ch == BACKSPACE) {
	    if (TC_curr_col != 0) {
		assert(TC_left != NULL);
		tputs(TC_left, 1, outchar);
		TC_curr_col--;
	    }
	    else if (TC_curr_line > 0) {
		TC_curr_col = COLS - 1;
		TC_curr_line--;
		tputs(tgoto(TC_moveto, TC_curr_col, TC_curr_line), 1, outchar);
	    }
	    /* else BACKSPACE does nothing */
	}

	/* if bell, ring the bell but don't advance the column */
	else if (ch == '\007') {
	  putchar(ch);
	}

	/* if a tab, output it */
	else if (ch == '\t') {
	  if ((nt = tabstop(TC_curr_col)) >= COLS) {
	    putchar('\r');
	    putchar('\n');
	    TC_curr_col = 0;
	    ++TC_curr_line;
	  } else {
	    if (TC_tabspacing == 8) {
	      putchar(ch);
	      TC_curr_col = nt;
	    } else {
	      while (TC_curr_col < nt) {
		putchar(' ');
		++TC_curr_col;
	      }
	    }
	  }
	}

	else {
	  /* if some kind of non-printable character change to a '?' */
#ifdef ASCII_CTYPE
	  if(!isascii(ch) || !isprint(ch))
#else
	  if ( (!isprint(ch) && !(ch & ~0x7f)) ||  ch == 0226)
/* 0226 can mess up xterm */ 	       
#endif
	    ch = '?';

	  /* if we only have one column left, simulate automargins if
	   * the terminal doesn't have them */
	  if (TC_curr_col == COLS - 1) {
	    putchar(ch);
	    if (!TC_automargin || TC_eatnewlineglitch) {
	      putchar('\r');
	      putchar('\n');
	    }
	    if (TC_curr_line < LINES)
	      ++TC_curr_line;
	    TC_curr_col = 0;
	    justwrapped = 1;
	  }

	  /* if we are here this means we have no interference from the
	   * right margin - just output the character and increment the
	   * column position. */
	  else {
	    putchar(ch);
	    TC_curr_col++;
	  }
	}

	wrappedlastchar = justwrapped;
}


/*
 * Insert a character at the current screen location, shifting the rest
 * of the line right one column (char in last col is lost).  Returns 0
 * if insert successful, -1 if insert capability not available.  Can be
 * invoked with a negative value to check whether insert can be done
 * on this terminal.
 *
 * This routine is guaranteed for only a *limited* set of circumstances.
 * The character must be a plain printing character, and the insertion
 * cannot occur in the last column of the line.
 */
PUBLIC void InsertChar(ch)
int ch;
{
    assert(TC_start_insert || TC_char_insert);
    assert(isprint(ch) && TC_curr_col < COLS-1);
    if (TC_start_insert && *TC_start_insert)
	tputs(TC_start_insert, 1, outchar);
    if (TC_char_insert && *TC_char_insert)
	tputs(TC_char_insert, 1, outchar);
    outchar(ch);
    ++TC_curr_col;
    if (TC_pad_insert && *TC_pad_insert)
	tputs(TC_pad_insert, 1, outchar);
    if (TC_end_insert && *TC_end_insert)
	tputs(TC_end_insert, 1, outchar);
}


/*
 * Delete a number characters from the display starting at the current
 * location, shifting the rest of the line left to fill in the deleted
 * spaces.  Returns 0 if deletion successful, -1 if delete capability
 * not available.  Can be invoked with a negative value to check whether
 * delete can be done on this terminal.
 */
PUBLIC void DeleteChar(n)
int n;
{
    assert (TC_char_delete && n > 0);
    if (TC_start_delete && *TC_start_delete)
	tputs(TC_start_delete, 1, outchar);
    while (--n >= 0)
	tputs(TC_char_delete, 1, outchar);
    if (TC_end_delete && *TC_end_delete)
	tputs(TC_end_delete, 1, outchar);
}


PUBLIC void Raw(state)
int state;
{
    int do_tite;

    do_tite = !(state & NO_TITE);
    state &= ~NO_TITE;

    /* don't bother if already in desired state */
    if (!state == !(Term.status & TERM_IS_RAW))
	return;

    FlushOutput();
    if (state) {
	(void) tcsetattr(STDIN_FILENO, TCSADRAIN, &TC_raw_tty);
	if (do_tite && use_tite && TC_start_termcap)
	    tputs(TC_start_termcap, 1, outchar);
	Term.status |= TERM_IS_RAW;
    } else {
	if (do_tite && use_tite && TC_end_termcap) {
	    tputs(TC_end_termcap, 1, outchar);
	    fflush(stdout);
	}
	(void) tcsetattr(STDIN_FILENO, TCSADRAIN, &TC_original_tty);
	Term.status &= ~TERM_IS_RAW;
    }
}


PUBLIC int ReadCh()
{
    /*
     *	read a character with Raw mode set!
     *
     *	EAGAIN & EWOULDBLOCK are recognized just in case
     *	O_NONBLOCK happens to be in effect.
     */

    int ch;

    FlushOutput();
    while ((ch = getchar()) <= 0) {
	if (errno == EINTR)
	    continue;
#ifdef	EAGAIN
	if (errno == EAGAIN)
	    continue;
#endif
#ifdef	EWOULDBLOCK
	if (errno == EWOULDBLOCK)
	    continue;
#endif
	ShutdownTerm();
	if (feof(stdin))
	    error("Unexepcted EOF from terminal!");
	else
	    error1("Error reading terminal! [%s]", strerror(errno));
	leave(LEAVE_EMERGENCY);
    }
    return ch;
}


PUBLIC void UnreadCh(ch)
int ch;
{
    ungetc(ch, stdin);
}


PUBLIC void FlushOutput()
{
    (void) fflush(stdout);
    (void) fflush(stderr);
}


PUBLIC void FlushInput()
{
    (void) fflush(stdin);
    (void) tcflush(STDIN_FILENO, TCIFLUSH);
}


static int outchar(c)
int c;
{
    /** output the given character.  From tputs... **/
    /** Note: this CANNOT be a macro!              **/

    return putc(c, stdout);
}



static void debug_termtitle P_((int, int, const char *));
static void debug_termstr P_((int, int, const char *, const char *));
static void debug_termnum P_((int, int, const char *, int));
static void debug_termflag P_((int, int, const char *, int));

PUBLIC void debug_terminal()
{
    int line;

    ClearScreen();
    CenterLine(0, "--- Debug Display - Terminal Settings ---");

    line = 2;
    debug_termstr(line, 0, "start_termcap", TC_start_termcap);
    debug_termstr(line, 1, "end_termcap", TC_end_termcap);
    ++line;
    debug_termstr(line, 0, "start_standout", TC_start_standout);
    debug_termstr(line, 1, "end_standout", TC_end_standout);
    ++line;
    debug_termstr(line, 0, "transmit_on", TC_transmit_on);
    debug_termstr(line, 1, "transmit_off", TC_transmit_off);

    line += 2;
    debug_termflag(line, 0, "ic supported?", !!(Term.status & TERM_CAN_IC));
    ++line;
    debug_termstr(line, 0, "start_insert", TC_start_insert);
    debug_termstr(line, 1, "end_insert", TC_end_insert);
    ++line;
    debug_termstr(line, 0, "char_insert", TC_char_insert);
    debug_termstr(line, 1, "pad_insert", TC_pad_insert);

    line += 2;
    debug_termflag(line, 0, "dc supported?", !!(Term.status & TERM_CAN_DC));
    ++line;
    debug_termstr(line, 0, "start_delete", TC_start_delete);
    debug_termstr(line, 1, "end_delete", TC_end_delete);
    ++line;
    debug_termstr(line, 0, "char_delete", TC_char_delete);

    line += 2;
    debug_termstr(line, 0, "moveto", TC_moveto);
    ++line;
    debug_termstr(line, 0, "up", TC_up);
    debug_termstr(line, 1, "down", TC_down);
    ++line;
    debug_termstr(line, 0, "left", TC_left);
    debug_termstr(line, 1, "right", TC_right);

    line += 2;
    debug_termnum(line, 0, "lines", TC_lines);
    debug_termnum(line, 1, "columns", TC_columns);
    ++line;
    debug_termnum(line, 0, "tabspacing", TC_tabspacing);
}


static void debug_termtitle(line, col, title)
int line, col;
const char *title;
{
    PutLine0(line, col*40, title);
    MoveCursor(line, col*40+16);
    return;
}


static void debug_termstr(line, col, title, value)
int line, col;
const char *title, *value;
{
    debug_termtitle(line, col, title);
    if (value == NULL) {
	PutLine0(-1, -1, "(null)");
	return;
    }
    if (*value == '\0') {
	PutLine0(-1, -1, "(empty)");
	return;
    }
    for ( ; *value ; ++value) {
	if (isprint(*value) || isspace(*value)) {
	    WriteChar(*value);
	} else if (*value < 040) {
	    WriteChar('^');
	    WriteChar(*value | 0100);
	} else {
	    PutLine1(-1, -1, "\\%03o", *value);
	}
    }
}


static void debug_termnum(line, col, title, value)
int line, col;
const char *title;
int value;
{
    debug_termtitle(line, col, title);
    PutLine1(-1, -1, "%d", value);
}


static void debug_termflag(line, col, title, value)
int line, col;
const char *title;
int value;
{
    debug_termtitle(line, col, title);
    PutLine0(-1, -1, (value ? "Yes" : "No"));
}

