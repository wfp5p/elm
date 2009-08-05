

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.7 $   $State: Exp $
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
 * $Log: returnadd.c,v $
 * Revision 1.7  1996/03/14  17:29:48  wfp5p
 * Alpha 9
 *
 * Revision 1.6  1995/09/29  17:42:24  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.5  1995/07/18  19:00:08  wfp5p
 * Alpha 6
 *
 * Revision 1.4  1995/06/12  20:33:36  wfp5p
 * Alpha 2 clean up
 *
 * Revision 1.3  1995/06/08  13:41:07  wfp5p
 * A few mostly cosmetic changes
 *
 * Revision 1.2  1995/05/10  13:34:52  wfp5p
 * Added mailing list stuff by Paul Close <pdc@sgi.com>
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This set of routines is used to generate real return addresses
    and also return addresses suitable for inclusion in a users
    alias files (ie optimized based on the pathalias database).

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"


void
get_existing_address(buffer, msgnum)
char *buffer;
int msgnum;
{
	/** This routine is called when the message being responded to has
	    "To:xyz" as the return address, signifying that this message is
	    an automatically saved copy of a message previously sent.  The
	    correct to address can be obtained fairly simply by reading the
	    To: header from the message itself and (blindly) copying it to
	    the given buffer.  Note that this header can be either a normal
	    "To:" line (Elm) or "Originally-To:" (previous versions e.g.Msg)
	**/

	char mybuf[LONG_STRING];
	register char ok = 1, in_to = 0;
	int  err;

	buffer[0] = '\0';

	/** first off, let's get to the beginning of the message... **/

	if(msgnum < 0 || msgnum >= curr_folder.num_mssgs) {
	  dprint(1, (debugfile,
		"Error: %d not a valid message number num_mssgs = %d (%s)",
		msgnum, curr_folder.num_mssgs, "get_existing_address"));
	  error1(catgets(elm_msg_cat, ElmSet, ElmNotAValidMessageNum,
		"%d not a valid message number!"), msgnum);
	  return;
	}
        if (fseek(curr_folder.fp, curr_folder.headers[msgnum]->offset, 0) == -1) {
	    err = errno;
	    dprint(1, (debugfile, 
		    "Error: seek %ld bytes into file hit errno %s (%s)", 
		    curr_folder.headers[msgnum]->offset, strerror(err), 
		    "get_existing_address"));
	    error2(catgets(elm_msg_cat, ElmSet, ElmCouldntSeekBytesIntoFlle,
		   "Couldn't seek %ld bytes into file (%s)."),
	           curr_folder.headers[msgnum]->offset, strerror(err));
	    return;
        }
 
        /** okay!  Now we're there!  **/

        while (ok) {
          ok = (int) (mail_gets(mybuf, LONG_STRING, curr_folder.fp) != 0);
	  no_ret(mybuf);	/* remove return character */

          if (header_cmp(mybuf, "To", (char *)NULL) ||
	      header_cmp(mybuf, "Original-To", (char *)NULL)) {
	    in_to = TRUE;
	    strcpy(buffer, index(mybuf, ':') + 1);
          }
	  else if (in_to && whitespace(mybuf[0])) {
	    strcat(buffer, " ");		/* tag a space in   */
	    strcat(buffer, (char *) mybuf + 1);	/* skip 1 whitespace */
	  }
	  else if (strlen(mybuf) < 2)
	    return;				/* we're done for!  */
	  else
	    in_to = 0;
      }
}

int
get_return(buffer, msgnum)
char *buffer;
int msgnum;
{
	/** reads msgnum message again, building up the full return 
	    address including all machines that might have forwarded 
	    the message.  Returns whether it is using the To line **/

	char buf[SLEN], name1[SLEN], name2[SLEN], lastname[SLEN];
	char hold_return[SLEN], alt_name2[SLEN], buf2[SLEN];
	int lines, len_buf, len_buf2, colon_offset, decnet_found;
	int using_to = FALSE, err;

	/* now initialize all the char buffers [thanks Keith!] */

	buf[0] = name1[0] = name2[0] = lastname[0] = '\0';
	hold_return[0] = alt_name2[0] = buf2[0] = '\0';

	/** get to the first line of the message desired **/

	if(msgnum < 0 || msgnum >= curr_folder.num_mssgs) {
	  dprint(1, (debugfile,
		"Error: %d not a valid message number num_mssgs = %d (%s)",
		msgnum, curr_folder.num_mssgs, "get_return"));
	  error1(catgets(elm_msg_cat, ElmSet, ElmNotAValidMessageNum,
		"%d not a valid message number!"), msgnum);
	  return(using_to);
	}

	if (fseek(curr_folder.fp, curr_folder.headers[msgnum]->offset, 0) == -1) {
	  err = errno;
	  dprint(1, (debugfile,
		"Error: seek %ld bytes into file hit errno %s (%s)", 
		curr_folder.headers[msgnum]->offset, strerror(err), 
	        "get_return"));
	  error2(catgets(elm_msg_cat, ElmSet, ElmCouldntSeekBytesIntoFlle,
		"Couldn't seek %ld bytes into file (%s)."),
	       curr_folder.headers[msgnum]->offset, strerror(err));
	  return(using_to);
	}
 
	/** okay!  Now we're there!  **/

	lines = curr_folder.headers[msgnum]->lines;

	buffer[0] = '\0';

	if (len_buf2 = mail_gets(buf2, SLEN, curr_folder.fp)) {
	  if(buf2[len_buf2-1] == '\n')
	    lines--; /* got a full line */
	}

	while (len_buf2 && lines) {
	  buf[0] = '\0';
	  strncat(buf, buf2, SLEN);
	  len_buf = strlen(buf);
	  if (len_buf2 = mail_gets(buf2, SLEN, curr_folder.fp)) {
	    if(buf2[len_buf2-1] == '\n')
	      lines--; /* got a full line */
	  }
	  while (len_buf2 && lines && whitespace(buf2[0]) && len_buf >= 2) {
	    if (buf[len_buf-1] == '\n') {
	      len_buf--;
	      buf[len_buf] = '\0';
	    }
	    strncat(buf, buf2, (SLEN-len_buf-1));
	    len_buf = strlen(buf);
	    if (len_buf2 = mail_gets(buf2, SLEN, curr_folder.fp)) {
	      if(buf2[len_buf2-1] == '\n')
		lines--; /* got a full line */
	    }
	  }

/* At this point, "buf" contains the unfolded header line, while "buf2" contains
   the next single line of text from the mail file */

	  if (strbegConst(buf, "From ")) 
	    sscanf(buf, "%*s %s", hold_return);
	  else if (stribegConst(buf, ">From")) {
	    sscanf(buf,"%*s %s %*s %*s %*s %*s %*s %*s %*s %s %s", 
	           name1, name2, alt_name2);
	    if (strcmp(name2, "from") == 0)		/* remote from xyz  */
	      strcpy(name2, alt_name2);
	    else if (strcmp(name2, "by") == 0)	/* forwarded by xyz */
	      strcpy(name2, alt_name2);
	    add_site(buffer, name2, lastname);
	  }

#ifdef USE_EMBEDDED_ADDRESSES

	  else if (header_cmp(buf, "From", (char *)NULL)) {
	    (void) parse_arpa_mailbox(buf+5, hold_return, sizeof(hold_return),
			(char *)NULL, 0, (char **)NULL);
	    buffer[0] = '\0';
          }
          else if (header_cmp(buf, "Reply-To", (char *)NULL)) {
	    if (parse_arpa_mailbox(buf+9, buffer, SLEN,
			(char *)NULL, 0, (char **)NULL) == 0)
	      return(using_to);
          }

#endif

	  else if (len_buf < 2)	/* done with header */
            lines = 0; /* let's get outta here!  We're done!!! */
	}

	if (buffer[0] == '\0')
	  strcpy(buffer, hold_return); /* default address! */
	else
	  add_site(buffer, name1, lastname);	/* get the user name too! */

	if (header_cmp(buffer, "To", (char *)NULL)) {	/* backward compatibility ho */
	  get_existing_address(buffer, msgnum);
	  using_to = TRUE;
	}
	else {
	  /*
	   * KLUDGE ALERT - DANGER WILL ROBINSON
	   * We can't just leave a bare login name as the return address,
	   * or it will be alias-expanded.
	   * So we qualify it with the current host name (and, maybe, domain).
	   * Sigh.
	   */

	  if ((colon_offset = qchloc(buffer, ':')) > 0)
		decnet_found = buffer[colon_offset + 1] == ':';
	  else
		decnet_found = NO;
		
	  if (qchloc(buffer, '@') < 0
	   && qchloc(buffer, '%') < 0
	   && qchloc(buffer, '!') < 0
	   && decnet_found == NO)
	  {
#ifdef INTERNET
	    sprintf(buffer + strlen(buffer), "@%s", host_fullname);
#else
	    strcpy(buf, buffer);
	    sprintf(buffer, "%s!%s", host_name, buf);
#endif
	  }

	  /*
	   * If we have a space character,
	   * or we DON'T have '!' or '@' chars,
	   * append the user-readable name.
	   */
	  if (qchloc(curr_folder.headers[msgnum]->from, ' ') >= 0 ||
	      (qchloc(curr_folder.headers[msgnum]->from, '!') < 0 &&
	       qchloc(curr_folder.headers[msgnum]->from, '@') < 0)) {
	       sprintf(buffer + strlen(buffer),
		       " (%s)", curr_folder.headers[msgnum]->from);
          }
	}

	return(using_to);
}
