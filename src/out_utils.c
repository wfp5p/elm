

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
#include <stdarg.h>


static void do_PutLine(int x, int y, const char *line)
{
    if (x >= 0 && y >= 0)
	MoveCursor(x, y);
    while (*line)
	WriteChar(*line++);
}

void PutLine(int x, int y, const char *line, ...)
{
    char buffer[VERY_LONG_STRING];
    va_list args;

    va_start(args, line);
    vsprintf(buffer, line, args);
    va_end(args);

    do_PutLine(x, y, buffer);
}

void CenterLine(int line, const char *str)
{
    int col;

    /* some systems require cast to ensure signed arithmetic */
    col = (COLS - (int)strlen(str)) / 2;
    if (col < 0)
	col = 0;
    PutLine(line, col, str);
}


static char err_buffer[SLEN];		/* store last error message */

void show_last_error(void)
{
    int lines_of_msg;

    if (Term.status & TERM_IS_INIT) {
	lines_of_msg = (strlen(err_buffer) + COLS - 1) / COLS;
	ClearLine(LINES);
	if (lines_of_msg > 1)
	    PutLine(LINES + 1 - lines_of_msg, 0, err_buffer);
	else
	    CenterLine(LINES, err_buffer);
    } else {
	fputs(err_buffer, stderr);
	putc('\r', stderr);
	putc('\n', stderr);
    }
    FlushOutput();
}

int clear_error(void)
{
    if (err_buffer[0] == '\0')
	return FALSE;
    ClearLine(LINES);
    err_buffer[0] = '\0';
    return TRUE;
}

void set_error(const char *s)
{
    strcpy(err_buffer, s);
}

static void do_showerror(const char *s)
{
  strcpy(err_buffer, s);
  show_last_error();
}

void error(const char *s, ...)
{
	va_list args;
	char buffer[SLEN];

	va_start(args, s);
	vsprintf(buffer, s, args);
	va_end(args);

	do_showerror(buffer);
}

void lower_prompt(const char *s)
{
	/** prompt user for input on LINES-1 line, left justified **/

	PutLine(LINES-1,0,s);
	CleartoEOLN();
}

void prompt(const char *s)
{
	/** prompt user for input on LINES-3 line, left justified **/

	PutLine(LINES-3,0,s);
	CleartoEOLN();
}


static char central_message_buffer[SLEN];

void set_central_message(const char *string, const char *arg)
{
	/** set up the given message to be displayed in the center of
	    the current window **/ 

	sprintf(central_message_buffer, string, arg);
}

void display_central_message(void)
{
	/** display the message if set... **/

	if (central_message_buffer[0] != '\0') {
	  ClearLine(LINES-15);
	  CenterLine(LINES-15, central_message_buffer);
	  FlushOutput();
	}
}

void clear_central_message(void)
{
	/** clear the central message buffer **/

	central_message_buffer[0] = '\0';
}

