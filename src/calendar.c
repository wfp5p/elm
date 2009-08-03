
static char rcsid[] = "@(#)$Id: calendar.c,v 1.3 1996/03/14 17:27:54 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
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
 * $Log: calendar.c,v $
 * Revision 1.3  1996/03/14  17:27:54  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:01  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This routine implements a rather snazzy idea suggested by Warren
    Carithers of the Rochester Institute of Technology that allows
    mail to contain entries formatted in a manner that will allow direct
    copying into a users calendar program.

    All lines in the current message beginning with "->", e.g.

	-> Mon 04/21 1:00p meet with chairman candidate

    get copied into the user's calendar file.

**/

#include "elm_defs.h"
#include "elm_globals.h"

#ifdef ENABLE_CALENDAR		/* if not defined, this will be an empty file */

#include "s_error.h"

scan_calendar()
{
	FILE *calendar;
	int  count;
	int  err;

	/* First step is to open the calendar file for appending... **/

	if (can_open(calendar_file, "a") != 0) {
	  err = errno;
	  dprint(2, (debugfile,
		  "Error: wrong permissions to append to calendar %s\n",
		  calendar_file));
	  dprint(2, (debugfile, "** - %s **\n", strerror(err)));
	  error1(catgets(elm_msg_cat, ErrorSet, ErrorCalendarCanOpen,
		  "Not able to append to file %s!"), calendar_file);
	  return; 
	}

	save_file_stats(calendar_file);

	if ((calendar = fopen(calendar_file,"a")) == NULL) {
	  err = errno;
	  dprint(2, (debugfile, 
		"Error: couldn't append to calendar file %s (scan)\n", 
		calendar_file));
	  dprint(2, (debugfile, "** - %s **\n", strerror(err)));
	  error1(catgets(elm_msg_cat, ErrorSet, ErrorCalendarAppend,
		  "Couldn't append to file %s!"), calendar_file);
	  return; 
	}
	
	count = extract_info(calendar);

	fclose(calendar);

	restore_file_stats(calendar_file);

	if (count > 0) {
	  if (count > 1)
	    error1(catgets(elm_msg_cat, ErrorSet, ErrorCalendarSavedPlural,
		    "%d entries saved in calendar file."), count);
	  else
	    error(catgets(elm_msg_cat, ErrorSet, ErrorCalendarSaved,
		    "1 entry saved in calendar file."));
	} else 
	  error(catgets(elm_msg_cat, ErrorSet, ErrorCalendarNoneSaved,
		  "No calendar entries found in that message."));

	return;
}

int
extract_info(save_to_fd)
FILE *save_to_fd;
{
	/** Save the relevant parts of the current message to the given
	    calendar file.  The only parameter is an opened file
	    descriptor, positioned at the end of the existing file **/
	    
	register int entries = 0, lines;
	int line_length;
	char buffer[SLEN], *cp, *is_cal_entry();
	struct header_rec *hdr;

	hdr = curr_folder.headers[curr_folder.curr_mssg-1];

    	/** get to the first line of the message desired **/

    	if (fseek(curr_folder.fp, hdr->offset, 0) == -1) {
       	  dprint(1,(debugfile, 
		"ERROR: Attempt to seek %d bytes into file failed (%s)",
		hdr->offset, "extract_info"));
       	  error1(catgets(elm_msg_cat, ErrorSet, ErrorCalendarSeek,
		  "ELM [seek] failed trying to read %d bytes into file."),
	     	hdr->offset);
       	  return(0);
    	}

        /* how many lines in message? */

        lines = hdr->lines;

        /* now while not EOF & still in message... scan it! */

	while (lines) {

          if((line_length = mail_gets(buffer, SLEN, curr_folder.fp)) == 0)
	    break;

	  if(buffer[line_length - 1] == '\n')
	    lines--;					/* got a full line */

	  if((cp = is_cal_entry(buffer)) != NULL) {
	    entries++;
	    fprintf(save_to_fd,"%s", cp);
	  }

	}
	dprint(4,(debugfile,
		"Got %d calender entr%s.\n", entries, entries > 1? "ies":"y"));

	return(entries);
}

char *
is_cal_entry(string)
register char *string;
{
	/* If string is of the form
         * {optional white space} ->{optional white space} {stuff}
	 * return a pointer to stuff, otherwise return NULL.
	 */
	while( whitespace(*string) )
	  string++;      /* strip leading W/S */
	
	if(strncmp(string, "->", 2) == 0) {
	  for(string +=2 ; whitespace(*string); string++)
		  ;
	  return(string);
	}
	return(NULL);
}

#endif
