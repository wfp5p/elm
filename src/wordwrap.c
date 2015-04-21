

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
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
 * $Log: wordwrap.c,v $
 * Revision 1.3  1996/03/14  17:30:00  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:38  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/***  Routines to wrap lines when using the "builtin" editor

***/

#include "elm_defs.h"
#include "elm_globals.h"

unsigned alarm();

#define isstopchar(c)		(c == ' ' || c == '\t')
#define isslash(c)		(c == '/')
#define erase_a_char()		{ WriteChar(BACKSPACE); WriteChar(' '); \
			          WriteChar(BACKSPACE); }

	/* WARNING: this macro destroys nr */
#define erase_tab(nr)		do WriteChar(BACKSPACE); while (--(nr) > 0)

static int line_wrap(char *string, char *tail, int *count, int *tabs);

int wrapped_enter(char *string, char *tail, int x, int y, FILE *edit_fd,
		  int *append_current)
{
	/** This will display the string on the screen and allow the user to
	    either accept it (by pressing RETURN) or alter it according to
	    what the user types.   The various flags are:
	         string    is the buffer to use (with optional initial value)
		 tail	   contains the portion of input to be wrapped to the
			   next line
	 	 x,y	   is the location we're at on the screen (-1,-1 means
			   that we can't use this info and need to find out
			   the current location)
		 append_current  means that we have an initial string and that
			   the cursor should be placed at the END of the line,
			   not the beginning (the default).

	    If we hit an interrupt or EOF we'll return non-zero.
	**/

	int ch, wrapcolumn = 70, iindex = 0;
	int addon = 0;	/* Space added by tabs. iindex+addon == column */
	int tindex = 0;	/* Index to the tabs array. */
	int tabs[10];	/* Spaces each tab adds. size <= wrapcolumn/8+1 */
	register int ch_count = 0, escaped = FALSE;
	long newpos, pos;
	char line[SLEN];

	if(!(x >=0 && y >= 0))
	  GetCursorPos(&x, &y);
	PutLine(x, y, "%s", string);

	CleartoEOLN();

	if (! *append_current) {
	  MoveCursor(x,y);
	}
	else
	  iindex = strlen(string);

	/** now we have the screen as we want it and the cursor in the
	    right place, we can loop around on the input and return the
	    string as soon as the user presses <RETURN> or the line wraps.
	**/

	do {
	  ch = ReadCh();

	  if (ch == ctrl('D')) {		/* we've hit EOF */
	    *append_current = 0;
	    return(1);
	  }

	  if (ch_count++ == 0) {
	    if (ch == '\n' || ch == '\r') {
	      *append_current = 0;
	      return(0);
	    }
	    else if (! *append_current) {
	      CleartoEOLN();
	      iindex = (*append_current? strlen(string) : 0);
	    }
	  }

	  if (!escaped) {
	    if (ch == Term.erase_char)
	      ch = ctrl('H');
	    if (ch == Term.kill_char)
	      ch = ctrl('U');
	  }

	  switch (ch) {

	  case '\n':
	  case '\r':
	    string[iindex] = '\0';
	    *append_current = 0;
	    return(0);

	  case '\0':
	    FlushInput(); 	/* remove extraneous chars, if any */
	    string[0] = '\0'; /* clean up string, and... */
	    *append_current = 0;
	    return(-1);

	  case ctrl('H'):
	    if (iindex > 0) {
  	      iindex--;
	      if (string[iindex] == '\t') {
		addon -= tabs[--tindex] - 1;
		erase_tab(tabs[tindex]);
	      } else erase_a_char();
	    } else { /** backspace to end of previous line **/

	      fflush(edit_fd);
	      if ((pos = ftell(edit_fd)) <= 0L) { /** no previous line **/
		Beep();

	      } else {

		/** get the last 256 bytes written **/
		if ((newpos = pos - 256L) <= 0L) newpos = 0;
		(void) fseek(edit_fd, newpos, 0L);
		(void) fread(line, sizeof(*line), (int) (pos-newpos),
		             edit_fd);
		pos--;

		/** the last char in line should be '\n'
			change it to null **/
		if (line[(int) (pos-newpos)] == '\n')
		  line[(int) (pos-newpos)] = '\0';

		/** find the end of the previous line ('\n') **/
		for (pos--; pos > newpos && line[(int) (pos-newpos)] != '\n';
			pos--);
		/** check to see if this was the first line in the file **/
		if (line[(int) (pos-newpos)] == '\n') /** no - it wasn't **/
		  pos++;
		(void) strcpy(string, &line[(int) (pos-newpos)]);
		line[(int) (pos-newpos)] = '\0';

		/** truncate the file to the current position
			THIS WILL NOT WORK ON SYS-V **/
		(void) fseek(edit_fd, newpos, 0L);
		(void) fputs(line, edit_fd);
		fflush(edit_fd);
		(void) ftruncate(fileno(edit_fd), (int) ftell(edit_fd));
		(void) fseek(edit_fd, ftell(edit_fd), 0L);

		/** rewrite line on screen and continue working **/
		GetCursorPos(&x, &y);
		if (x > 0) x--;
		PutLine(x, y, "%s", string);
		CleartoEOLN();
		iindex = strlen(string);

		/* Reload tab positions */
		addon = tindex = 0;
		for (pos = 0; pos < iindex; pos++)
		  if (string[pos] == '\t')
		    addon += (tabs[tindex++] = 8 - ((pos+addon) & 07)) - 1;
	      }
	    }
	    break;

	  case ctrl('W'):
	    if (iindex == 0)
	      break;		/* no point staying here.. */
	    iindex--;
	    if (isslash(string[iindex])) {
	      erase_a_char();
	    }
	    else {
	      while (iindex >= 0 && isspace(string[iindex])) {
		if (string[iindex] == '\t') {
		  addon -= tabs[--tindex] - 1;
		  erase_tab(tabs[tindex]);
		} else erase_a_char();
	        iindex--;
	      }

	      while (iindex >= 0 && ! isstopchar(string[iindex])) {
	        iindex--;
	        erase_a_char();
	      }
	      iindex++;	/* and make sure we point at the first AVAILABLE slot */
	    }
	    break;

	  case ctrl('U'):
	    MoveCursor(x,y);
	    CleartoEOLN();
	    iindex = 0;
	    break;

	  case ctrl('R'):
	    string[iindex] = '\0';
	    PutLine(x,y, "%s", string);
	    CleartoEOLN();
	    break;

	  default:
	    if (escaped) {
	      WriteChar(BACKSPACE);
	      iindex--;
	    } else if (ch == '\t') {
		addon += (tabs[tindex++] = 8 - ((addon+iindex) & 07)) - 1;
	    } else if (ch > 0xFF || !isprint(ch)) {
	      Beep();
	      break;
	    }
	    string[iindex++] = ch;
	    WriteChar(ch);
	    break;

	  }

	  escaped = (!escaped && ch == '\\');

	} while (iindex+addon < wrapcolumn);

	string[iindex] = '\0';
	*append_current = line_wrap(string,tail,&iindex,&tabs[tindex-1]);

	return(0);
}

static int line_wrap(char *string, char *tail, int *count, int *tabs)
{
	/** This will check for line wrap.  If the line was wrapped,
	    it will back up to white space (if possible), write the
	    shortened line, and put the remainder at the beginning
	    of the string.  Returns 1 if wrapped, 0 if not.
	**/

	int n = *count;
	int i, j;

	/* Look for a space */
	while (n && !isstopchar(string[n]))
	  --n;

	/* If break found */
	if (n) {

	  /* Copy part to be wrapped */
	  for (i=0,j=n+1;j<=*count;tail[i++]=string[j++]);

	  /* Skip the break character and any whitespace */
	  while (n && isstopchar(string[n]))
	    --n;

	  if (n) n++; /* Move back into the whitespace */
	}

	/* If no break found */
	if (!n) {
	  (*count)--;
	  strcpy(tail, &string[*count]);
	  erase_a_char();
	} else /* Erase the stuff that will wrap */
	  while (*count > n) {
	    --(*count);
	  if (string[*count] == '\t') erase_tab(*tabs--);
	  else erase_a_char();
	  }

	string[*count] = '\0';
	return(1);
}
