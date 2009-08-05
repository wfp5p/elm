

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: mail_gets.c,v $
 * Revision 1.2  1995/09/29  17:41:15  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** get a line from the mail file, but be tolerant of nulls

  The length of the line is returned

**/

#include "elm_defs.h"

int
mail_gets(buffer, size, mailfile)
char *buffer;
int size;
FILE *mailfile;
{
	register int line_bytes = 0, ch;
	register char *c = buffer;

	size--; /* allow room for zero terminator on end, just in case */

	while (!feof(mailfile) && !ferror(mailfile) && line_bytes < size) {
	  ch = getc(mailfile); /* Macro, faster than  fgetc() ! */

	  if (ch == EOF)
	  {
	    if (line_bytes > 0 && *c != '\n')
	    {
	        ++line_bytes;
	    	*c++ = '\n';
	    }
	    break;
	  }

	  *c++ = ch;
	  ++line_bytes;

	  if (ch == '\n')
	    break;
	}
	*c = 0;	/* Actually this should NOT be needed.. */
	return line_bytes;
}
