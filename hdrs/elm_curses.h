
/* $Id: elm_curses.h,v 1.4 1999/03/24 14:03:41 wfp5p Exp $ */

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
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
 * $Log: elm_curses.h,v $
 * Revision 1.4  1999/03/24  14:03:41  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.3  1997/10/20  20:24:22  wfp5p
 * Incomingfolders no longer set Magic mode on for all remaining folders.
 *
 * Revision 1.2  1996/03/14  17:27:20  wfp5p
 * Alpha 9
 *
 * Revision 1.1  1995/09/29  17:40:46  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:30  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#define OFF	0
#define ON 	1

#define CarriageReturn()	WriteChar('\r')
#define NewLine()		(WriteChar('\r'), WriteChar('\n'))
#define ClearLine(n)		((void) (MoveCursor((n), 0), CleartoEOLN()))
#define Beep()			WriteChar(07)

/* special codes returned by GetKey() procedure */
#define KEY_DOWN	0402		/* down arrow			*/
#define KEY_UP		0403		/* up arrow			*/
#define KEY_LEFT	0404		/* left arrow			*/
#define KEY_RIGHT	0405		/* right arrow			*/
#define KEY_HOME	0406		/* home				*/
#define KEY_DC		0512		/* delete char			*/
#define KEY_IC		0513		/* insert char			*/
#define KEY_NPAGE	0522		/* next-page (or page down)	*/
#define KEY_PPAGE	0523		/* previous-page (or page up)	*/
#define KEY_BTAB	0541		/* back-tab			*/
#define KEY_END		0550		/* end				*/
#define KEY_REDRAW	0775		/* redraw signal condition	*/
#define KEY_TIMEOUT	0776		/* timeout signal condition	*/
#define KEY_UNKNOWN	0777		/* a key we don't recognize	*/

/* options to the enter_string() routine */
#define ESTR_ENTER	0	/* enter a new value			*/
#define ESTR_REPLACE	1	/* replace current value		*/
#define ESTR_UPDATE	2	/* update current value			*/
#define ESTR_NUMBER	3	/* like ENTER, but accept digits only	*/
#define ESTR_PASSWORD	4	/* like ENTER, but no output displayed	*/

/* selections for define_softkeys() routine */
#define SOFTKEYS_MAIN	0
#define SOFTKEYS_ALIAS	1
#define SOFTKEYS_YESNO	2
#define SOFTKEYS_CHANGE	3
#define SOFTKEYS_READ	4

/* term "status" settings */
#define TERM_IS_INIT	(1<<0)	/* InitScreen() has been performed	*/
#define TERM_IS_RAW	(1<<1)	/* keyboard is in raw state		*/
#define TERM_IS_FKEY	(1<<2)	/* function keys in transmit mode	*/
#define TERM_CAN_SO	(1<<3)	/* supports "standout" function		*/
#define TERM_CAN_DC	(1<<4)	/* supports "delete char" function	*/
#define TERM_CAN_IC	(1<<5)	/* supports "insert char" function	*/

/* public information about terminal status */
struct term_info {
    int lines;			/* screen height			*/
    int cols;			/* screen width				*/
    int status;			/* terminal status flags		*/
    int erase_char;		/* commonly CTRL/H			*/
    int kill_char;		/* commonly CTRL/U			*/
    int werase_char;		/* commonly CTRL/W			*/
    int intr_char;		/* commonly CTRL/C			*/
};

#define LINES	(Term.lines)
#define COLS	(Term.cols)

/*
 * global data
 */
EXTERN struct term_info Term;		/* current terminal settings	*/
#if defined(_JBLEN) || defined(_SETJMP_H) || defined(_SETJMP_H_) || defined(_SETJMP_H_INCLUDED)
EXTERN int GetKey_active;		/* set if in GetKey() in read() */
EXTERN JMP_BUF GetKey_jmpbuf;		/* setjmp buffer		*/
#endif
#if defined(SIGWINCH) || defined(SIGCONT)
EXTERN int caught_screen_change_sig;	/* SIGWINCH or SIGCONT occurred? */
#endif

/* curses.c */

int InitScreen P_((void));
void ShutdownTerm P_((void));
int knode_parse P_((int));
#if defined(SIGWINCH) || defined(SIGCONT)
void ResizeScreen P_((void));
#endif
void GetCursorPos P_((int *, int *));
void InvalidateCursor P_((void));
void MoveCursor P_((int, int));
void ClearScreen P_((void));
void CleartoEOLN P_((void));
void CleartoEOS P_((void));
void StartStandout P_((void));
void EndStandout P_((void));
void WriteChar P_((int));
void InsertChar P_((int));
void DeleteChar P_((int));
void Raw P_((int));
void EnableFkeys P_((int));
int ReadCh P_((void));
void UnreadCh P_((int));
void FlushOutput P_((void));
void FlushInput P_((void));
void debug_terminal P_((void));


/* in_utils.c */

int enter_yn P_((char *, int, int, int));
int enter_number P_((int, int, char *));
int enter_string P_((char *, int, int, int, int));
int GetKey P_((int));


/* out_utils.c */

void PutLine0 P_((int, int, const char *));
void PutLine1();
void PutLine2();
void PutLine3();
void CenterLine P_((int, const char *));
void show_last_error P_((void));
int clear_error P_((void));
void set_error P_((const char *));
void error P_((const char *));
void error1();
void error2();
void error3();
void lower_prompt P_((const char *));
void prompt P_((const char *));
void set_central_message P_((const char *, const char *));
void display_central_message P_((void));
void clear_central_message P_((void));


/* softkeys.c */

int define_softkeys P_((int));
void softkeys_on P_((void));
void softkeys_off P_((void));

