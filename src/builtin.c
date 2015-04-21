

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
 * $Log: builtin.c,v $
 * Revision 1.4  1996/03/14  17:27:54  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:00  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/07/18  18:59:54  wfp5p
 * Alpha 6
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This is the built-in pager for displaying messages while in the Elm
    program.  It's a bare-bones pager with precious few options. The idea
    is that those systems that are sufficiently slow that using an external
    pager such as 'more' is too slow, then they can use this!

    Also added support for the "builtin+" pager (clears the screen for
    each new page) including a two-line overlap for context...

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

static	int unfilled_lines, form_title;

static int lines_displayed;	    /* total number of lines displayed      */
static int total_lines_to_display;  /* total number of lines in message     */
static int pages_displayed; 	    /* for the nth page titles and all      */


static int title_for_page(int page);

int get_lines_displayed(void)
{
	return lines_displayed;
}

void set_lines_displayed(int x)
{
	lines_displayed = x;
}

int start_builtin(int lines_in_message)
{
	/** clears the screen and resets the internal counters... **/

	dprint(8,(debugfile,
		"displaying %d lines from message using internal pager\n",
		lines_in_message));

	unfilled_lines = LINES;
	form_title = 1;
	lines_displayed = 0;
        pages_displayed = 1;

	total_lines_to_display = lines_in_message;
}

static int next_line(char **inputptr, int *inputlenptr, char *output,
		     int *outputlenptr, unsigned width)
{
	/* Copy characters from input to output and copy
	 * remainder of output to output. In copying use ^X notation for
	 * control characters, '?' non-ascii characters, expand tabs
	 * to correct number of spaces till next tab stop.
	 * Column zero of the next line is considered to be the tab stop
	 * that follows the last one that fits on a line.
	 * Copy until newline/return encountered, null char encountered,
	 * width characters producted in output buffer.
	 * Formfeed is handled exceptionally. If encountered it
	 * is removed from input and 1 is returned. Otherwise 0 is returned.
	 */

	char *optr, *iptr;
	unsigned chars_output, nt;
	int ret_val;
	int ilen = *inputlenptr;

	optr = output;
	iptr = *inputptr;
	chars_output = 0;

	ret_val = 0;	/* presume no formfeed */
	while(ilen > 0) {	/* while there is something */

	  if(chars_output >= width  ) {	/* no more room on line */
	    *optr++ = '\r';
	    *optr++ = '\n';
	    /* if next input character is newline or return,
	     * we can skip over it since we are outputing a newline anyway */
	    if((*iptr == '\n') || (*iptr == '\r'))
	      { iptr++; --ilen; }
	    break;
	  } else if (*iptr == '\n' || *iptr == '\r') {	/*newline or return */
	    *optr++ = '\r';
	    *optr++ = '\n';
	    iptr++; --ilen;
	    break;			/* end of line */
	  } else if(*iptr == '\f') {		/* formfeed */
	    /* if next input character is newline or return,
	     * we can skip over it since we are outputing a formfeed anyway */
	    if((*++iptr == '\n') || (*iptr == '\r'))
	      { iptr++; --ilen; }
	    --ilen;
	    ret_val = 1;
	    break;			/* leave rest of screen clear */
	  } else if(*iptr == '\t') {	/* tab stop */
	    if((nt=tabstop(chars_output)) >= width) {
	      *optr++ = '\n';		/* won't fit on this line - autowrap */
	      *optr++ = '\r';		/* tab by tabbing so-to-speak to 1st */
	      iptr++;			/* column of next line */
	      --ilen;
	      break;
	    } else {		/* will fit - output proper num of spaces */
	      while(chars_output < nt) {
		chars_output++;
		*optr++ = ' ';
	      }
	      iptr++;
	      --ilen;
	    }
	  } else if(iscntrl(*iptr & 0xff)) {	/* Non-white ctrl char */
	    if (chars_output + 2 <= width) {
		*optr++ = '^';
		*optr++ = ctrl(*iptr);
		iptr++; --ilen;
		chars_output += 2;
	    } else {
		break;
	    }
	  } else {		/* Assume a printing char in case isprint */
	    *optr++ = *iptr++;	/* macro fails on true 8-bit characters.  */
	    chars_output++;
	    --ilen;
	  }
	}
	*optr = '\0';
	*inputptr = iptr;
	*inputlenptr = ilen;
	*outputlenptr = chars_output;
	return(ret_val);
}


int display_line(char *input_line, int input_size)
{
	int	percent_done;
	/** Display the given line on the screen, taking into account such
	    dumbness as wraparound and such.  If displaying this would put
	    us at the end of the screen, put out the "MORE" prompt and wait
	    for some input.   Return non-zero if the user terminates the
	    paging (e.g. 'i') or zero if we should continue. Also,
            this will pass back the value of any character the user types in
	    at the prompt instead, if needed... (e.g. if it can't deal with
	    it at this point)
	**/

	char *pending, footer[SLEN], display_buffer[SLEN];
	int ch, formfeed, lines_more;
	int pending_len = input_size, display_len = 0;

	pending = input_line;
	CarriageReturn();

	do {

	  /* while there is more space on the screen - leave prompt line free */
	  while(unfilled_lines > 0) {

	    /* display a screen's lineful of the input line
	     * and reset pending to point to remainder of input line */
	    formfeed = next_line(&pending, &pending_len, display_buffer, &display_len, COLS);

	    if(*display_buffer == '\0') {	/* no line to display */
	      if(!formfeed)	/* no "formfeed" to display
	     			 * need more lines for screen */
		return(FALSE);
	    } else
	      PutLine(-1, -1, display_buffer);

	    /* if formfeed, clear remainder of screen */
	    if(formfeed) {
	      CleartoEOS();
	      unfilled_lines=0;
	    }
	    else
	      unfilled_lines--;

	    /* if screen is not full (leave room for prompt)
	     * but we've used up input line, return */

	    if(unfilled_lines > 0 && pending_len == 0)
	      return(FALSE);	/* we need more lines to fill screen */

	    /* otherwise continue to display next part of input line */
	  }

	  /* screen is now full - prompt for user input */
	  lines_more = total_lines_to_display - lines_displayed;
	  percent_done = (int)((100L * lines_displayed) / total_lines_to_display);
	  if (user_level == 0) {
	    if (lines_more == 1)
	      MCsprintf(footer, catgets(elm_msg_cat, ElmSet, ElmMoreUser0,
" There is 1 line left (%d%%). Press <space> for more, or 'i' to return. "),
		 percent_done);
	    else
	      MCsprintf(footer, catgets(elm_msg_cat, ElmSet, ElmMoreUser0Plural,
" There are %d lines left (%d%%). Press <space> for more, or 'i' to return. "),
		 lines_more, percent_done);
	  } else if (user_level == 1) {
	    if (lines_more == 1)
	      MCsprintf(footer, catgets(elm_msg_cat, ElmSet, ElmMoreUser1,
" 1 line more (%d%%). Press <space> for more, 'i' to return. "),
		 percent_done);
	    else
	      MCsprintf(footer, catgets(elm_msg_cat, ElmSet, ElmMoreUser1Plural,
" %d lines more (%d%%). Press <space> for more, 'i' to return. "),
		 lines_more, percent_done);
	  } else {
	    if (lines_more == 1)
	      MCsprintf(footer, catgets(elm_msg_cat, ElmSet, ElmMoreUser2,
" 1 line more (you've seen %d%%) "),
		 percent_done);
	    else
	      MCsprintf(footer, catgets(elm_msg_cat, ElmSet, ElmMoreUser2Plural,
" %d lines more (you've seen %d%%) "),
		 lines_more, percent_done);
	  }

	  MoveCursor(LINES, 0);
	  if (Term.status & TERM_CAN_SO)
	      StartStandout();
	  PutLine(-1, -1, footer);
	  if (Term.status & TERM_CAN_SO)
	      EndStandout();

	  switch(ch = GetKey(0)) {

	    case '\n':
	    case '\r':	/* scroll down a line */
			unfilled_lines = 1;
			ClearLine(LINES);
			break;

	    case ' ':	/* scroll a screenful */
			unfilled_lines = LINES;
			if(clear_pages) {
			  ClearScreen();
			  MoveCursor(0,0);
			  CarriageReturn();

			  /* output title */
			  if(title_messages && filter) {
			    title_for_page(++pages_displayed);
			    unfilled_lines -= 2;
			  }
			} else ClearLine(LINES);

			/* and keep last line to be first line of next
			 * screenful unless we had a formfeed */
			if(!formfeed) {
			  if(clear_pages)
			    PutLine(-1, -1, display_buffer);
			  unfilled_lines--;
			}
			break;

	    default:	return(ch);
	  }
	  CarriageReturn();
	} while(pending_len > 0);
	return(FALSE);
}

static int title_for_page(int page)
{
	/** Output a nice title for the second thru last pages of the message
	    we're currently reading. Note - this code is very similar to
	    that which produces the title for the first page, except that
	    page number replaces the date and the method by which it
	    gets to the screen **/

	static char title1[SLEN], title2[SLEN];
	char titlebuf[SLEN], title3[SLEN], who[SLEN];
	static int t1_len, t2_len;
	int padding, showing_to;
	struct header_rec *hdr;

	hdr = curr_folder.headers[curr_folder.curr_mssg-1];

	/* format those parts of the title that are constant for a message */
	if(form_title) {

	  showing_to = tail_of(hdr->from, who, hdr->to);

	  sprintf(title1, "%s %d/%d  ",
	      hdr->status & DELETED ? nls_deleted :
	      hdr->status & FORM_LETTER ? nls_form: nls_message,
	      curr_folder.curr_mssg, curr_folder.num_mssgs);
	  t1_len = strlen(title1);
	  sprintf(title2, "%s %s", showing_to? nls_to : nls_from, who);
	  t2_len = strlen(title2);
	}
	/* format those parts of the title that vary between pages of a mesg */
	sprintf(title3, nls_page, page);

	/* truncate or pad title2 portion on the right
	 * so that line fits exactly to the rightmost column */
	padding = COLS - (t1_len + t2_len + strlen(title3));

	sprintf(titlebuf, "%s%-*.*s%s\n\r\n\r", title1, t2_len+padding,
	    t2_len+padding, title2, title3);
	    /* extra newline is to give a blank line after title */

	PutLine(-1, -1, titlebuf);
	form_title = 0;
}
