

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.9 $   $State: Exp $
 *
 * This file and all associated files and documentation:
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: elm.c,v $
 * Revision 1.9  1996/08/08  19:49:23  wfp5p
 * Alpha 11
 *
 * Revision 1.8  1996/05/09  15:51:18  wfp5p
 * Alpha 10
 *
 * Revision 1.7  1996/03/14  17:27:59  wfp5p
 * Alpha 9
 *
 * Revision 1.6  1996/03/13  14:37:59  wfp5p
 * Alpha 9 before Chip's big changes
 *
 * Revision 1.5  1995/09/29  17:42:05  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.4  1995/09/11  15:19:05  wfp5p
 * Alpha 7
 *
 * Revision 1.3  1995/05/10  13:34:48  wfp5p
 * Added mailing list stuff by Paul Close <pdc@sgi.com>
 *
 * Revision 1.2  1995/04/20  21:01:46  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/* Main program of the ELM mail system! 
*/

#define INTERN
#include "elm_defs.h"
#include <setjmp.h>	/* so that "GetKey_jmpbuf" gets defined */
#include "elm_globals.h"
#include "s_elm.h"

#ifdef I_TIME
#  include <time.h>
#endif
#ifdef I_SYSTIME
#  include <sys/time.h>
#endif
#ifdef BSD
#  include <sys/timeb.h>
#endif

long bytes();
char *format_long(), *parse_arguments();

main(argc, argv)
int argc;
char *argv[];
{
	int  ch;
	char address[SLEN], to_whom[SLEN], *req_mfile;
	int  i,j;      		/** Random counting variables (etc)          **/
	int  pageon, 		/** for when we receive new mail...          **/
	     last_in_folder;	/** for when we receive new mail too...      **/
	long num;		/** another variable for fun..               **/

	initialize_common();
	req_mfile = parse_arguments(argc, argv, to_whom);
	initialize(req_mfile);

	if (OPMODE_IS_SENDMODE(opmode)) {
	  if (OPMODE_IS_INTERACTIVE(opmode)) {
	    sprintf(address, catgets(elm_msg_cat, ElmSet, ElmSendOnlyMode,
		  "Send only mode [ELM %s]"), version_buff);
	    CenterLine(1, address);
	  }

	  if (to_whom && *to_whom)
	      dprint(3, (debugfile, "Mail-only: mailing to\n-> \"%s\"\n",
		      format_long(to_whom, 3)));
	  else
	      dprint(3, (debugfile, "Mail-only; no recipient specified\n"));

	  (void) send_message(to_whom, (char *)NULL, batch_subject,
		SM_ORIGINAL); 
	  leave(LEAVE_NORMAL);
	}

        headers_per_page = LINES - (mini_menu ? 13 : 8);
        if (headers_per_page < 1)
	    headers_per_page = 1;
    
        /* read in the folder */
        newmbox(req_mfile, FALSE);

	redraw = 1;

	while (1) {

	  if (redraw)
	    showscreen();
	  redraw = 0;
	  nufoot = 0;
	  nucurr = 0;

	  if (curr_folder.fp)
		  fflush (curr_folder.fp);

	  if ((num = bytes(curr_folder.filename)) != curr_folder.size) {
	    dprint(2, (debugfile, "Just received %d bytes more mail (elm)\n", 
		    num - curr_folder.size));
	    error(catgets(elm_msg_cat, ElmSet, ElmNewMailHangOn,
	      "New mail has arrived! Hang on..."));
	    last_in_folder = curr_folder.num_mssgs;
	    pageon = header_page;
	    newmbox(curr_folder.filename, TRUE);	/* last won't be touched! */
	    clear_error();
	    header_page = pageon;

	    if (selected)               /* update count of selected messages */
	      selected += curr_folder.num_mssgs - last_in_folder;

	    if (on_page(curr_folder.curr_mssg))   /* do we REALLY have to rewrite? */
	      showscreen();
	    else {
	      update_title();
	      ClearLine(LINES-1);	     /* remove reading message... */
	      if ((curr_folder.num_mssgs - last_in_folder) == 1)
	        error(catgets(elm_msg_cat, ElmSet, ElmNewMessageRecv,
		       "1 new message received."));
	      else
	        error1(catgets(elm_msg_cat, ElmSet, ElmNewMessageRecvPlural,
		       "%d new messages received."), 
		       curr_folder.num_mssgs - last_in_folder);
	    }
	  }

	  prompt(nls_Prompt);

	  CleartoEOLN();
	  ch = GetKey(timeout);
	  CleartoEOS();
	  if (clear_error())
	    MoveCursor(LINES-3, strlen(nls_Prompt));

#ifdef DEBUG
	  dprint(4, (debugfile, "\nCommand: %c [%d]\n\n", ch, ch));
#endif

	  switch (ch) {

	    case KEY_TIMEOUT:
		break;

	    case KEY_REDRAW:
		++redraw;
		break;

	    case '?' 	:  if (help(FALSE))
			     redraw++;
			   else
			     nufoot++;
			   break;

	    case '$'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
			     ElmSet, ElmResyncFolder,
			     "Resynchronize folder"));
			   redraw += resync();
			   nucurr = get_page(curr_folder.curr_mssg);
			   break;

	    case '|'    :  WriteChar('|'); 
			   if (curr_folder.num_mssgs < 1) {
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailToPipe,
			       "No mail to pipe!"));
			     FlushInput();
			   } else {
                             redraw += do_pipe();		
			   }
			   break;

#ifdef ALLOW_SUBSHELL
	    case '!'    :  WriteChar('!'); 
                           redraw += subshell();		
			   break;
#endif

	     case '&'   : TreatAsSpooled = !TreatAsSpooled;
			  if (TreatAsSpooled) {
			     error(catgets(elm_msg_cat, ElmSet, ElmMagicOn,
						   "[Magic On]"));
			   } else {
			     error(catgets(elm_msg_cat, ElmSet, ElmMagicOff,
						   "[Magic Off]"));
			   } 
	                  break;


	    case '%'    :  if (curr_folder.curr_mssg > 0) {
			     get_return(address, curr_folder.curr_mssg-1);
			     clear_error();
			     PutLine1(LINES,(COLS-strlen(address))/2,
				      "%.78s", address);	
			   } else {
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailReturnAddress,
			       "No mail to get return address of!")); 
			   }
			   break;

	    case '<'    :  /* scan current message for calendar information */
			   break;

	    case 'a'    :  alias();
			   redraw++;
			   break;
			
	    case 'b'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
					ElmSet, ElmBounceMessage,
					"Bounce message"));
			   if (curr_folder.num_mssgs < 1) {
			     error(catgets(elm_msg_cat,
			       ElmSet, ElmNoMailToBounce,
			       "No mail to bounce!"));
			     FlushInput();
			   }
			   else 
			     nufoot = remail();
			   break;

	    case 'c'    :  define_softkeys(SOFTKEYS_CHANGE);
			   redraw += change_file(catgets(elm_msg_cat, ElmSet,
					ElmChangeFolder,
					"Change folder"));
			   define_softkeys(SOFTKEYS_MAIN);
			   break;

#ifdef ALLOW_MAILBOX_EDITING
	    case 'e'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmEditFolder,
				"Edit folder"));
			   if (curr_folder.curr_mssg > 0) {
			     edit_mailbox();
			   }
			   else {
			     error(catgets(elm_msg_cat, ElmSet, ElmFolderIsEmpty,
			       "Folder is empty!"));
			   }
			   break;
#else
	    case 'e'    : error(catgets(elm_msg_cat, ElmSet, ElmNoFolderEdit,
		    "Folder editing isn't configured in this version of ELM."));
			  break;
#endif
		
	    case 'f'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmForward,
				"Forward"));
			   if (curr_folder.curr_mssg > 0) {
			     if(forward())
			       redraw++;
			     else
			       nufoot++;
			   } else {
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailToForward,
			       "No mail to forward!"));
			     FlushInput();
			   }
			   break;

	    case 'g'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmGroupReply,
				"Group reply"));
			   if (curr_folder.curr_mssg > 0) {
			     if (curr_folder.headers[curr_folder.curr_mssg-1]->status & FORM_LETTER) {
			       error(catgets(elm_msg_cat, ElmSet, ElmCantGroupReplyForm,
				 "Can't group reply to a Form!!"));
			       FlushInput();
			     }
			     else {
			       redraw += reply_to_everyone();	
			     }
			   }
			   else {
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailToReply,
			       "No mail to reply to!")); 
			     FlushInput();
			   }
			   break;

	    case 'h'    :  if (filter)
			     PutLine0(-1, -1, catgets(elm_msg_cat,
			       ElmSet, ElmMessageWithHeaders,
			       "Message with headers..."));
			   else
			     PutLine0(-1, -1, catgets(elm_msg_cat,
			       ElmSet, ElmDisplayMessage,
			       "Display message"));
			   if(curr_folder.curr_mssg > 0) {
			     j = filter;
			     filter = FALSE;
			     i = show_msg(curr_folder.curr_mssg);
			     ResizeScreen();
			     while (i)
				i = process_showmsg_cmd(i);
			     filter = j;
			     redraw++;
			     (void)get_page(curr_folder.curr_mssg);
			   } else
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailToRead,
			       "No mail to read!"));
			   break;

	    case 'M'	:  if (show_mlists) 
	                   {
			     char buffer[SLEN];
			      
			     strcpy(buffer, catgets(elm_msg_cat, ElmSet,
						    ElmMlistOff,
						    "[Mlists Off]"));
			      
			     PutLine0(LINES,(COLS-10)/2,buffer);
			     show_mlists = 0;
			   } 
	                   else 
	                   {
			     char buffer[SLEN];
			      
			     strcpy(buffer, catgets(elm_msg_cat, ElmSet,
						    ElmMlistOn,
						    "[Mlists On]"));
			      
			     PutLine0(LINES,(COLS-10)/2,buffer);

			     show_mlists = 1;
			   }
			   last_header_page = -1;	/* force a redraw */
			   show_headers();
			   break;

	    case 'm'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmMail,
				"Mail"));
			   redraw += send_message((char *)NULL,
			       (char *)NULL, (char *)NULL, SM_ORIGINAL); 
			   break;

	    case ' '    : 
	    case ctrl('J'):
	    case ctrl('M'): PutLine0(-1, -1, catgets(elm_msg_cat,
			      ElmSet, ElmDisplayMessage,
			      "Display message"));	
			   if(curr_folder.curr_mssg > 0 ) {
			     define_softkeys(SOFTKEYS_READ);
			     i = show_msg(curr_folder.curr_mssg);
    		             ResizeScreen();
			     while (i)
				i = process_showmsg_cmd(i);
			     define_softkeys(SOFTKEYS_MAIN);
			     redraw++;
			     (void)get_page(curr_folder.curr_mssg);
			   } else
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailToRead,
			       "No mail to read!"));
			   break;

	    case 'n'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmNextMessage,
				"Next Message"));
			   if(curr_folder.curr_mssg > 0 ) {
			     define_softkeys(SOFTKEYS_READ);
			     i = show_msg(curr_folder.curr_mssg);
			     ResizeScreen();
			     while (i)
			       i = process_showmsg_cmd(i);
			     define_softkeys(SOFTKEYS_MAIN);
			     redraw++;
			     if (++curr_folder.curr_mssg > curr_folder.num_mssgs)
			       curr_folder.curr_mssg = curr_folder.num_mssgs;
			     (void)get_page(curr_folder.curr_mssg);
			   } else
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailToRead,
			       "No mail to read!"));
			   break;

	    case 'o'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmOptions,
				"Options"));
			   if((i=options()) > 0)
			     get_page(curr_folder.curr_mssg);
			   redraw++;	/* always fix da screen... */
			   break;

	    case 'p'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmPrintMail,
				"Print mail"));
			   if (curr_folder.num_mssgs < 1) {
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailToPrint,
			       "No mail to print!"));
			   } else if (print_msg(TRUE) != 0)
			     redraw++;
			   break;

	    case 'q'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmQuit,
				"Quit"));
			   quit(TRUE);		
			   break;

	    case 'Q'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmQuickQuit,
				"Quick quit"));
			   quit(FALSE);		
			   break;

	    case 'r'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
			     ElmSet, ElmReplyToMessage,
			     "Reply to message"));
			   if (curr_folder.curr_mssg > 0) 
			     redraw += reply();	
			   else {
			     error(catgets(elm_msg_cat, ElmSet, ElmNoMailToReplyTo,
			       "No mail to reply to!")); 
			     FlushInput();
			   }
			   break;

	    case '>'    : /** backwards compatibility **/

	    case 'C'	:
	    case 's'    :  if  (curr_folder.num_mssgs < 1) {
			     if (ch != 'C')
			       error(catgets(elm_msg_cat, ElmSet, ElmNoMailToSave,
				 "No mail to save!"));
			     else
			       error(catgets(elm_msg_cat, ElmSet, ElmNoMailToCopy,
				 "No mail to copy!"));
			     FlushInput();
			   }
			   else {
			     if (save(&redraw, FALSE, (ch != 'C'))
				 && resolve_mode && ch != 'C') {
			       if((i=next_message(curr_folder.curr_mssg-1, TRUE)) != -1) {
				 curr_folder.curr_mssg = i+1;
				 nucurr = get_page(curr_folder.curr_mssg);
			       }
			     }
			   }
			   ClearLine(LINES-2);		
			   break;

#ifdef ALLOW_STATUS_CHANGING
	    case 'S'    :  PutLine0(-1, -1, "Status");
			   /*  catgets(elm_msg_cat, ElmSet, ElmOptions, */
			   if((i=ch_status()) > 0)
			     get_page(curr_folder.curr_mssg);
			   redraw++;	/* always fix da screen... */
			   break;
#else
	    case 'S'    : error(catgets(elm_msg_cat, ElmSet, ElmNoStatusChange,
		"Status changing isn't configured in this version of ELM."));
			  break;
#endif

	    case 'X'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmQuickExit,
				"Quick Exit"));
			   quit_abandon(FALSE);

	    case ctrl('Q') :
	    case 'x'    :  PutLine0(-1, -1, catgets(elm_msg_cat,
				ElmSet, ElmExit,
				"Exit"));  
			   quit_abandon(TRUE);
			   break;
	    
	    case '@':
	    case '#':
			for (;;) {
			    error(
"Debug:  display p)age, current m)essage, t)erminal, or q)uit to return.");
			    if ((i = ReadCh()) == 'q')
				break;
			    switch (i) {
				case 'p': debug_page();		break;
				case 'm': debug_message();	break;
				case 't': debug_terminal();	break;
				default:  Beep();
			    }
			}
			clear_error();
			redraw++;
			break;

	    /* None of the menu specific commands were chosen, therefore
	     * it must be a "motion" command (or an error).               */
	    default	: motion(ch);

	  }

	  if (redraw) {
	    showscreen();
	    redraw = 0;
	    nucurr = 0;
	    nufoot = 0;
	  }

          check_range();
	    
	  if (nucurr == NEW_PAGE) 
	    show_headers();
	  else if (nucurr == SAME_PAGE)
	    show_current();
	  else if (nufoot) {
	    if (mini_menu) {
	      MoveCursor(LINES-7, 0);  
              CleartoEOS();
	      show_menu();
	    }
	    else {
	      MoveCursor(LINES-4, 0);
	      CleartoEOS();
	    }
	    show_last_error();	/* for those operations that have to
				 * clear the footer except for a message.
				 */
	  }

	} /* the BIG while loop! */
}

debug_page()
{
    int i, first, last, line;
    char buffer[SLEN];

    first = header_page * headers_per_page;	/* starting header */
    if ((last = first + (headers_per_page-1)) >= curr_folder.num_mssgs) 
	last = curr_folder.num_mssgs-1;

    ClearScreen();
    PutLine1(0, 0, "Current mail file = %s",
		curr_folder.filename);
    PutLine2(1, 0, "Current message number = %-8d  %d message(s) total",
		curr_folder.curr_mssg, curr_folder.num_mssgs);
    PutLine2(2, 0, "Header_page = %-8d             %d page(s) total",
		header_page+1, (int) (curr_folder.num_mssgs / headers_per_page) + 1);
    sprintf(buffer, "%3s %-16s %-34s %5s %8s %8s",
		"Num", "From", "Subject", "Lines", "Offset", "ContLen");
    PutLine0(4, 0, buffer);

    line = 5;
    for (i = first ; i <= last ; ++i) {
	sprintf(buffer, 
		    "%3d %-16.16s %-35.35s %4d %8d %8d",
		    i+1,
		    curr_folder.headers[i]->from, 
		    curr_folder.headers[i]->subject,
		    curr_folder.headers[i]->lines,
		    curr_folder.headers[i]->offset,
		    curr_folder.headers[i]->content_length);
	PutLine0(++line, 0, buffer);
    }

}


debug_message()
{
	/**** Spit out the current message record.  Include EVERYTHING
	      in the record structure. **/
	
	struct header_rec *hdr;
	char buffer[SLEN];
	time_t tval;
	int row, i;

#define ShowStr(row, name, str) PutLine2((row), 0, \
	    (strlen(str) > 62 ? "%-16s|%.60s..." : "%-16s|%s|"), (name), (str))
#define ShowNum(row, name, num) PutLine2((row), 0, \
	    "%-16s%ld", (name), (long)(num))

	ClearScreen();
	PutLine1(0, 24,
		    "--- Debug Display - Message %d Header_Rec ---",
		    curr_folder.curr_mssg);

	PutLine0(2, 24, "status:");
	PutLine0(2, 32, "A  C  D  E  F  M  D  N  N  O  P  T  U  V");
	PutLine0(3, 32, "c  o  e  x  o  i  e  p  e  l  r  a  r  i");
	PutLine0(4, 32, "t  n  l  p  r  m  c  l  w  d  i  g  g  s");
	PutLine0(5, 32, "n  f  d  d  m  e  o  a        v  d  n  i");

	if (curr_folder.num_mssgs == 0) {
	    CenterLine(8, catgets(elm_msg_cat, ElmSet, ElmNoMailToCheck,
			      "No mail to check."));
	    return;
	}
	hdr = curr_folder.headers[curr_folder.curr_mssg-1];

	ShowNum(2, "index_number:", hdr->index_number);
	ShowNum(3, "offset:", hdr->offset);
	ShowNum(4, "lines:", hdr->lines);
	ShowNum(5, "content_length:", hdr->content_length);
	ShowNum(6, "cc_index:", hdr->cc_index);
	ShowStr(7, "mailx_status:", hdr->mailx_status);

#define SHOW_STATUS(hdr, FLAG)	(!!((hdr)->status & (FLAG)))
	sprintf(buffer, "%d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d",
		    SHOW_STATUS(hdr, ACTION),
		    SHOW_STATUS(hdr, CONFIDENTIAL),
		    SHOW_STATUS(hdr, DELETED),
		    SHOW_STATUS(hdr, EXPIRED),
		    SHOW_STATUS(hdr, FORM_LETTER),
		    SHOW_STATUS(hdr, MIME_MESSAGE),
		    SHOW_STATUS(hdr, MIME_NEEDDECOD),
		    SHOW_STATUS(hdr, MIME_NOTPLAIN),
		    SHOW_STATUS(hdr, NEW),
		    SHOW_STATUS(hdr, UNREAD),
		    SHOW_STATUS(hdr, PRIVATE),
		    SHOW_STATUS(hdr, TAGGED),
		    SHOW_STATUS(hdr, URGENT),
		    SHOW_STATUS(hdr, VISIBLE));
	PutLine0(7, 32, buffer);
	row = 8;

	trim_trailing_spaces(strcpy(buffer, ctime(&hdr->received_time)));
	ShowStr(++row, "received_time:", buffer);
	tval = hdr->time_sent + hdr->tz_offset;
	trim_trailing_spaces(strcpy(buffer, ctime(&tval)));
	ShowStr(++row, "time_sent:", buffer);
	ShowStr(++row, "time_zone:", hdr->time_zone);
	ShowNum(++row, "tz_offset:", hdr->tz_offset);
	ShowStr(++row, "from:", hdr->from);
	ShowStr(++row, "to:", hdr->to);
	ShowStr(++row, "subject:", hdr->subject);
	ShowStr(++row, "messageid:", hdr->messageid);
	ShowStr(++row, "allfrom:", hdr->allfrom);
	ShowStr(++row, "allto:", hdr->allto);

	ShowNum(++row, "ml_cc_index:", hdr->ml_cc_index);
	ShowStr(++row, "ml_to:", "");
	WriteChar('\b');	/* erase closing "|" added above */
	for (i = 0; i < hdr->ml_to.len; i++) {
	    if (i > 0)
		PutLine0(-1, -1, ", ");
	    PutLine0(-1, -1, hdr->ml_to.str[i]);
	}
	WriteChar('|');
}

check_range()
{
	int count, curr, i;

	if (inalias) {
	    count = num_aliases;
	    curr = curr_alias;
	} else {
	    count = curr_folder.num_mssgs;
	    curr = curr_folder.curr_mssg;
	}

	i = compute_visible(curr);

	if ((curr < 1) || (selected && i < 1)) {
	    if (count > 0) {
	      /* We are out of range! Get to first message! */
	      if (selected)
		curr = compute_visible(1);
	      else
		curr = 1;
	    }
	    else
	      curr = 0;
	}
	else if ((curr > count) || (selected && i > selected)) {
	    if (count > 0) {
	      /* We are out of range! Get to last (visible) message! */
	      if (selected)
		curr = visible_to_index(selected)+1;
	      else
		curr = count;
	    }
	    else
	      curr = 0;
	}


	if (inalias)
	    curr_alias = curr;
	else
	    curr_folder.curr_mssg = curr;

}

static char *no_mail = NULL;
static char *no_aliases = NULL;

#define ifmain(a,b)     (inalias ? (b) : (a))

motion(ch)
int ch;
{
	/* Consolidated the standard menu navigation and delete/tag
	 * commands to a function.                                   */

	int  i;
	int count, curr;

	if (inalias) {
	    count = num_aliases;
	    curr = curr_alias;
	} else {
	    count = curr_folder.num_mssgs;
	    curr = curr_folder.curr_mssg;
	}

	if (no_mail == NULL) {
		no_mail = catgets(elm_msg_cat, ElmSet, ElmNoMailInFolder,
		  "No mail in folder!");
		no_aliases = catgets(elm_msg_cat, ElmSet, ElmNoAliases,
		  "No aliases!");
	}

	switch (ch) {

	    case '/'    :  /* scan mbox or aliases for string */
			   if  (count < 1) {
			     error1(catgets(elm_msg_cat, ElmSet,
				ElmNoItemToScan, "No %s to scan!"), nls_items);
			     FlushInput();
			   }
			   else if (pattern_match()) {
				if (inalias)
				    curr = curr_alias;
				else
				    curr = curr_folder.curr_mssg;
			        nucurr = get_page(curr);
			   }
			   break;

	   case KEY_NPAGE:
	   case KEY_RIGHT:
	   case ctrl('V'):
	    case '+'	:  /* move to next page if we're not on the last */
			   if((selected &&
			     ((header_page+1)*headers_per_page < selected))
			   ||(!selected &&
			     ((header_page+1)*headers_per_page<count))){

			     header_page++;
			     nucurr = NEW_PAGE;

			     if(move_when_paged) {
			       /* move to first message of new page */
			       if(selected)
				 curr = visible_to_index(
				   header_page * headers_per_page + 1) + 1;
			       else
				 curr = header_page * headers_per_page + 1;
			     }
			   } else
			     error(catgets(elm_msg_cat, ElmSet, ElmAlreadyOnLastPage,
			       "Already on last page."));
			   break;

	    case KEY_PPAGE:
	    case KEY_LEFT:
	    case '-'	:  /* move to prev page if we're not on the first */
			   if(header_page > 0) {
			     header_page--;
			     nucurr = NEW_PAGE;

			     if(move_when_paged) {
			       /* move to first message of new page */
			       if(selected)
				 curr = visible_to_index(
				   header_page * headers_per_page + 1) + 1;
			       else
				 curr = header_page * headers_per_page + 1;
			     }
			   } else
			     error(catgets(elm_msg_cat, ElmSet, ElmAlreadyOnFirstPage,
			       "Already on first page."));
			   break;

 	   case KEY_HOME:
	    case '='    :  if (selected)
			     curr = visible_to_index(1)+1;
			   else
			     curr = 1;
			   nucurr = get_page(curr);
			   break;

	    case KEY_END:
	    case '*'    :  if (selected) 
			     curr = (visible_to_index(selected)+1);
			   else
			     curr = count;	
			   nucurr = get_page(curr);
			   break;

/*	    case EOF    : leave(0);
                          break; */

	    case ctrl('D') :
	    case '^'    :
	    case 'd'    :  if (count < 1) {
			     error1(catgets(elm_msg_cat, ElmSet, ElmNoItemToDelete,
			       "No %s to delete!"), nls_item);
/*			     fflush(stdin);*/
			   }
			   else {
			     if(ch == ctrl('D')) {

			       /* if current item did not become deleted,
				* don't to move to the next undeleted item */
			       if(!meta_match(MATCH_DELETE)) 
				  break;

			     } else 
 			       delete_msg((ch == 'd'), TRUE);

			     if (resolve_mode) 	/* move after mail resolved */
			       if((i=next_message(curr-1, TRUE)) != -1) {
				 curr = i+1;
				 nucurr = get_page(curr);
			       }
			   }
			   break;

	    case 'H'	: { /* move to first line of page */
			    int first_on_page;

			    if (selected)
			      first_on_page = visible_to_index(
			        header_page * headers_per_page + 1) + 1;
			    else
			      first_on_page = header_page*headers_per_page + 1;

			    /* don't bother to redraw if you don't move */
			    if (curr != first_on_page) {
			      curr = first_on_page;
			      nucurr = SAME_PAGE;
			    }

			    break;
			  }

	   case ctrl('N'):
	    case 'J'    :  if(curr > 0) {
			     if((i=next_message(curr-1, FALSE)) != -1) {
			       curr = i+1;
			       nucurr = get_page(curr);
			     } else
			       error1(catgets(elm_msg_cat, ElmSet, ElmNoMoreItemBelow,
				 "No more %s below."), nls_items);
			   } else error(ifmain(no_mail, no_aliases));
			   break;

next_undel_msg:
	    case KEY_DOWN:
	    case 'j'    :  if(curr > 0) {
			     if((i=next_message(curr-1, TRUE)) != -1) {
			       curr = i+1;
			       nucurr = get_page(curr);
			     } else
			       error1(catgets(elm_msg_cat, ElmSet, ElmNoItemUndeletedBelow,
				 "No more undeleted %s below."), nls_items);
			   } else error(ifmain(no_mail, no_aliases));
			   break;

	   case ctrl('P'):
	    case 'K'    :  if(curr > 0) {
			     if((i=prev_message(curr-1, FALSE)) != -1) {
			       curr = i+1;
			       nucurr = get_page(curr);
			     } else
			       error1(catgets(elm_msg_cat, ElmSet, ElmNoMoreItemAbove,
				 "No more %s above."), nls_items);
			   } else error(ifmain(no_mail, no_aliases));
			   break;

	    case KEY_UP:
	    case 'k'    :  if(curr > 0) {
			     if((i=prev_message(curr-1, TRUE)) != -1) {
			       curr = i+1;
			       nucurr = get_page(curr);
			     } else
			       error1(catgets(elm_msg_cat, ElmSet, ElmNoMoreUndeletedAbove,
				 "No more undeleted %s above."), nls_items);
			   } else error(ifmain(no_mail, no_aliases));
			   break;

	    case 'L'	: { /* move to last line of page */
			    int last_on_page, last_of_all, last_line;

			    if (selected) {
			      last_on_page = visible_to_index(
			        (header_page + 1) * headers_per_page) + 1;
			      last_of_all = (visible_to_index(selected) + 1);
			    } else {
			      last_on_page = (header_page+1) * headers_per_page;
			      last_of_all = count;
			    }

			    if (last_on_page < last_of_all)
			      last_line = last_on_page;
			    else
			      last_line = last_of_all;

			    /* don't bother to redraw if you don't move */
			    if (curr != last_line) {
			      curr = last_line;
			      nucurr = SAME_PAGE;
			    }

			    break;
			  }

	    case 'l'    :  PutLine1(LINES-3, strlen(nls_Prompt),
				   catgets(elm_msg_cat, ElmSet, ElmLimitDisplayBy,
				   "Limit displayed %s by..."), nls_items);
			   clear_error();
			   if (limit() != 0) {
			      curr = curr_folder.curr_mssg;
			      nucurr = get_page(curr);
			     redraw++;
			   } else {
			     nufoot++;
			   }
			   break;

            case ctrl('T') :
	    case 'T'	   :
	    case 't'       :  if (count < 1) {
				error1(catgets(elm_msg_cat, ElmSet, ElmNoItemToTag,
				  "No %s to tag!"), nls_items);
			      }
			      else if (ch == 't')
				tag_message(TRUE); 
			      else if (ch == 'T') {
				tag_message(TRUE); 
				goto next_undel_msg;
			      }
			      else
				(void) meta_match(MATCH_TAG);
			      break;

	    case 'u'    :  if (count < 1) {
			     error1(catgets(elm_msg_cat, ElmSet, ElmNoItemToMarkUndeleted,
			       "No %s to mark as undeleted!"), nls_items);
			   }
			   else 
	                   {
			      
			      /* Overload u to have it mark unread a read
			       * message which is not deleted.
			       */
			      
			      if ( (!inalias) && (isoff(curr_folder.headers[curr_folder.curr_mssg-1]->status, DELETED)))
			      {
				 setit(curr_folder.headers[curr_folder.curr_mssg-1]->status, UNREAD);
				 curr_folder.headers[curr_folder.curr_mssg-1]->status_chgd = TRUE;
				 show_msg_status(curr_folder.curr_mssg-1);
			      }
			      else
			        undelete_msg(TRUE);
			      if (resolve_mode) 	/* move after mail resolved */
			        if((i=next_message(curr-1, FALSE)) != -1) {
				 curr = i+1;
				 nucurr = get_page(curr);
			      }
/*************************************************************************
 **  What we've done here is to special case the "U)ndelete" command to
 **  ignore whether the next message is marked for deletion or not.  The
 **  reason is obvious upon usage - it's a real pain to undelete a series
 **  of messages without this quirk.  Thanks to Jim Davis @ HPLabs for
 **  suggesting this more intuitive behaviour.
 **
 **  The old way, for those people that might want to see what the previous
 **  behaviour was to call next_message with TRUE, not FALSE.
**************************************************************************/
			   }
			   break;

	    case ctrl('U') : if (count < 1) {
			       error1(catgets(elm_msg_cat, ElmSet, ElmNoItemToUndelete,
				 "No %s to undelete!"), nls_items);
			     }
			     else 
			       (void) meta_match(MATCH_UNDELETE);
			     break;

	    case ctrl('L') : redraw++;	break;

	    default	: if (ch > '0' && ch <= '9') {
			    PutLine1(LINES-3, strlen(nls_Prompt), 
				    catgets(elm_msg_cat, ElmSet, ElmNewCurrentItem,
				    "New Current %s"), nls_Item);
			    UnreadCh(ch);
			    i = enter_number(LINES-3, curr, nls_item);

			    if( i > count)
			      error1(catgets(elm_msg_cat, ElmSet, ElmNotThatMany,
				"Not that many %s."), nls_items);
			    else if(selected
				&& isoff(ifmain(curr_folder.headers[i-1]->status,
			                        aliases[i-1]->status), VISIBLE))
			      error1(catgets(elm_msg_cat, ElmSet, ElmNotInLimitedDisplay,
				"%s not in limited display."), nls_Item);
			    else {
			      curr = i;
			      nucurr = get_page(curr);
			    }
			  }
			  else {
			    error(catgets(elm_msg_cat, ElmSet, ElmUnknownCommand,
			      "Unknown command. Use '?' for help."));
			    FlushInput();
			  }
	}

	if (inalias)
	    curr_alias = curr;
	else
	    curr_folder.curr_mssg = curr;
}
