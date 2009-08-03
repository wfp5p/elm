
static char rcsid[] = "@(#)$Id: out_utils.c,v 1.4 1996/05/09 15:51:23 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
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
 * $Log: out_utils.c,v $
 * Revision 1.4  1996/05/09  15:51:23  wfp5p
 * Alpha 10
 *
 * Revision 1.3  1996/03/14  17:29:44  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:20  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:37  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This file contains routines used for output in the ELM program.

**/

#include "elm_defs.h"
#include "elm_globals.h"


PUBLIC void PutLine0(x, y, line)
int x,y;
const char *line;
{
    if (x >= 0 && y >= 0)
	MoveCursor(x, y);
    while (*line)
	WriteChar(*line++);
}

/*VARARGS3*/
PUBLIC void PutLine1(x,y, line, arg1)
int x,y;
const char *line;
const char *arg1;
{
    char buffer[VERY_LONG_STRING];
    sprintf(buffer, line, arg1);
    PutLine0(x, y, buffer);
}

/*VARARGS3*/
PUBLIC void PutLine2(x,y, line, arg1, arg2)
int x,y;
const char *line;
const char *arg1, *arg2;
{
    char buffer[VERY_LONG_STRING];
    MCsprintf(buffer, line, arg1, arg2);
    PutLine0(x, y, buffer);
}

/*VARARGS3*/
PUBLIC void PutLine3(x,y, line, arg1, arg2, arg3)
int x,y;
const char *line;
const char *arg1, *arg2, *arg3;
{
    char buffer[VERY_LONG_STRING];
    MCsprintf(buffer, line, arg1, arg2, arg3);
    PutLine0(x, y, buffer);
}


PUBLIC void CenterLine(line, str)
int line;
const char *str;
{
    int col;

    /* some systems require cast to ensure signed arithmetic */
    col = (COLS - (int)strlen(str)) / 2;
    if (col < 0)
	col = 0;
    PutLine0(line, col, str);
}


static char err_buffer[SLEN];		/* store last error message */

PUBLIC void show_last_error()
{
    int lines_of_msg;

    if (Term.status & TERM_IS_INIT) {
	lines_of_msg = (strlen(err_buffer) + COLS - 1) / COLS;
	ClearLine(LINES);
	if (lines_of_msg > 1)
	    PutLine0(LINES + 1 - lines_of_msg, 0, err_buffer);
	else
	    CenterLine(LINES, err_buffer);
    } else {
	fputs(err_buffer, stderr);
	putc('\r', stderr);
	putc('\n', stderr);
    }
    FlushOutput();
}

PUBLIC int clear_error()
{
    if (err_buffer[0] == '\0')
	return FALSE;
    ClearLine(LINES);
    err_buffer[0] = '\0';
    return TRUE;
}

PUBLIC void set_error(s)
const char *s;
{
    strcpy(err_buffer, s);
}

PUBLIC void error(s)
const char *s;
{
  strcpy(err_buffer, s);
  show_last_error();
}

/*VARARGS1*/
PUBLIC void error1(s, a)
const char *s, *a;
{
    char buffer[SLEN];
    sprintf(buffer, s, a);
    error(buffer);
}

/*VARARGS1*/
PUBLIC void error2(s, a1, a2)
const char *s, *a1, *a2;
{
    char buffer[SLEN];
    MCsprintf(buffer, s, a1, a2);
    error(buffer);
}

/*VARARGS1*/
PUBLIC void error3(s, a1, a2, a3)
const char *s, *a1, *a2, *a3;
{
    char buffer[SLEN];
    MCsprintf(buffer, s, a1, a2, a3);
    error(buffer);
}

PUBLIC void lower_prompt(s)
const char *s;
{
	/** prompt user for input on LINES-1 line, left justified **/

	PutLine0(LINES-1,0,s);
	CleartoEOLN();
}


PUBLIC void prompt(s)
const char *s;
{
	/** prompt user for input on LINES-3 line, left justified **/

	PutLine0(LINES-3,0,s);
	CleartoEOLN();
}


static char central_message_buffer[SLEN];

PUBLIC void set_central_message(string, arg)
const char *string, *arg;
{
	/** set up the given message to be displayed in the center of
	    the current window **/ 

	sprintf(central_message_buffer, string, arg);
}

PUBLIC void display_central_message()
{
	/** display the message if set... **/

	if (central_message_buffer[0] != '\0') {
	  ClearLine(LINES-15);
	  CenterLine(LINES-15, central_message_buffer);
	  FlushOutput();
	}
}

PUBLIC void clear_central_message()
{
	/** clear the central message buffer **/

	central_message_buffer[0] = '\0';
}

