

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

void PutLine0(int x, int y, const char *line)
{
    if (x >= 0 && y >= 0)
	MoveCursor(x, y);
    while (*line)
	WriteChar(*line++);
}

void PutLine1(int x, int y, const char *line, ...)
{
	va_list args;

	char buffer[VERY_LONG_STRING];
	va_start(args, line);
	sprintf(buffer, line, args);
	va_end(args);
	PutLine0(x, y, buffer);
}

void PutLine2(int x, int y, const char *line, const char *arg1,
	      const char *arg2)
{
    char buffer[VERY_LONG_STRING];
    MCsprintf(buffer, line, arg1, arg2);
    PutLine0(x, y, buffer);
}

/*VARARGS3*/
void PutLine3(int x, int y, const char *line, const char *arg1,
	      const char *arg2, const char *arg3)
{
    char buffer[VERY_LONG_STRING];
    MCsprintf(buffer, line, arg1, arg2, arg3);
    PutLine0(x, y, buffer);
}

void CenterLine(int line, const char *str)
{
    int col;

    /* some systems require cast to ensure signed arithmetic */
    col = (COLS - (int)strlen(str)) / 2;
    if (col < 0)
	col = 0;
    PutLine0(line, col, str);
}


static char err_buffer[SLEN];		/* store last error message */

void show_last_error(void)
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

void error(const char *s)
{
  strcpy(err_buffer, s);
  show_last_error();
}

/*VARARGS1*/
void error1(const char *s, const char *a)
{
    char buffer[SLEN];
    sprintf(buffer, s, a);
    error(buffer);
}

/*VARARGS1*/
void error2(const char *s, const char *a1, const char *a2)
{
    char buffer[SLEN];
    MCsprintf(buffer, s, a1, a2);
    error(buffer);
}

/*VARARGS1*/
void error3(const char *s, const char *a1, const char *a2, const char *a3)
{
    char buffer[SLEN];
    MCsprintf(buffer, s, a1, a2, a3);
    error(buffer);
}

void lower_prompt(const char *s)
{
	/** prompt user for input on LINES-1 line, left justified **/

	PutLine0(LINES-1,0,s);
	CleartoEOLN();
}

void prompt(const char *s)
{
	/** prompt user for input on LINES-3 line, left justified **/

	PutLine0(LINES-3,0,s);
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

