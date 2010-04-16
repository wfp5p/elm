

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.9 $   $State: Exp $
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
 * $Log: fileio.c,v $
 * Revision 1.9  1996/10/28  16:58:05  wfp5p
 * Beta 1
 *
 * Revision 1.8  1996/08/08  19:49:25  wfp5p
 * Alpha 11
 *
 * Revision 1.7  1996/03/14  17:29:34  wfp5p
 * Alpha 9
 *
 * Revision 1.6  1995/09/29  17:42:10  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.5  1995/09/11  15:19:07  wfp5p
 * Alpha 7
 *
 * Revision 1.4  1995/06/22  14:48:47  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.3  1995/06/08  13:41:04  wfp5p
 * A few mostly cosmetic changes
 *
 * Revision 1.2  1995/04/20  21:01:47  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:36  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** File I/O routines, including deletion from the folder!

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"
#include "mailfile.h"
#include "port_stat.h"

static char *makeAttString
    P_((char *, int, const char *, int, const struct header_rec *));

static void copy_write_error_exit(err)
int err;
{
	ShutdownTerm();
	error1(catgets(elm_msg_cat, ElmSet, ElmWriteCopyMessageFailed,
		"Write failed in copy_message()! [%s]"), strerror(err));
	leave(LEAVE_EMERGENCY);
}


copy_message(dest_file, msgnum, cm_options)
FILE *dest_file;
int msgnum, cm_options;
{
	/** Copy selected message to destination file, with optional 'prefix'
	    as the prefix for each line.  If remove_header is true, it will
	    skip lines in the message until it finds the end of header line...
	    then it will start copying into the file... If remote is true
	    then it will append "remote from <hostname>" at the end of the
	    very first line of the file (for remailing)

	    If "update_status" is true then it will write a new Status:
	    line at the end of the headers.  It never copies an existing one.

	    If "decode" is true, prompt for key if the message is encrypted,
	    else just copy it as it is.
	**/

    /*
     *	Changed buffer[SLEN] to buffer[VERY_LONG_STRING] to make it
     *	big enough to contain a full length header line. Any header
     *	is allowed to be at least 1024 bytes in length. (r.t.f. RFC)
     *	14-Sep-1993 Jukka Ukkonen <ukkonen@csc.fi>
     */

    char *buffer;
    char *prefix;
    char attrbuf[SLEN];
    register struct header_rec *msg_header;
    register int  lines, front_line, next_front,
		  in_header = 1, first_line = TRUE, ignoring = FALSE;
    int forwarding	= !!(cm_options & CM_FORWARDING);
    int remove_header	= !!(cm_options & CM_REMOVE_HEADER);
    int remote		= !!(cm_options & CM_REMOTE);
    int update_status	= !!(cm_options & CM_UPDATE_STATUS);
    int remail		= !!(cm_options & CM_REMAIL);


    int strip_from = remail;
    int	end_header = 0;
    int sender_added = 0;
    static int start_encode_len = 0, end_encode_len = 0;
    int bytes_seen = 0;
    int buf_len, err;
    struct mailFile mailFile;
    FAST_COMP_DECLARE;

    /** set up lengths of encode/decode strings so we don't have to strlen()
     ** every time */
    if (start_encode_len == 0) {
      start_encode_len = strlen(MSSG_START_ENCODE);
      end_encode_len = strlen(MSSG_END_ENCODE);
    }

    msg_header = curr_folder.headers[msgnum-1];
    prefix = ((cm_options & CM_PREFIX) ? prefixchars : "");

      /** get to the first line of the message desired **/

    if (fseek(curr_folder.fp, msg_header->offset, 0) == -1) {
       dprint(1, (debugfile,
		"ERROR: Attempt to seek %d bytes into file failed (%s)",
		msg_header->offset, "copy_message"));
       error1(catgets(elm_msg_cat, ElmSet, ElmSeekFailed,
	     "ELM [seek] failed trying to read %d bytes into file."),
	     msg_header->offset);
       return;
    }

    /* how many lines in message? */

    lines = msg_header->lines;


    /* now while not EOF & still in message... copy it! */

    next_front = TRUE;
    mailFile_attach(&mailFile, curr_folder.fp);

    /* emit the opening attribution string */
    if (cm_options & CM_ATTRIBUTION) {
      if (forwarding) {
	if (fwdattribution[0] != '\0') {
	  prefix = "";
	  fputs(makeAttString(attrbuf, sizeof(attrbuf),
		  fwdattribution, 0, msg_header), dest_file);
	  putc('\n', dest_file);
	} else {
	  fputs(catgets(elm_msg_cat, ElmSet, ElmCopyMssgAttributionForwarded,
		  "Forwarded message:\n"), dest_file);
	}
      } else if (attribution[0]) {
	fputs(makeAttString(attrbuf, sizeof(attrbuf),
	    attribution, 0, msg_header), dest_file);
	putc('\n', dest_file);
      }
    }

    while (lines) {
      if (! (buf_len = mailFile_gets(&buffer, &mailFile)))
        break;

      bytes_seen += buf_len;
      front_line = next_front;

      if(buffer[buf_len - 1] == '\n') {
	lines--;	/* got a full line */
	next_front = TRUE;
      }
      else
	next_front = FALSE;

      if (front_line && ignoring)
	ignoring = whitespace(buffer[0]);

      if (ignoring)
	continue;

      /* preload the first char of the line for fast comparisons */
      fast_comp_load(buffer[0]);


      /* are we still in the header? */

      if (in_header && front_line) {
	if (buf_len < 2) {
	  in_header = 0;
	  bytes_seen = 0;
	  end_header = -1;
	  if (remail && !sender_added) {
	    if (fprintf(dest_file, "%sSender: %s\n", prefix, user_name) == EOF) {
	      copy_write_error_exit(errno);
	    }
	  }
	}
	else if (!isspace(*buffer)
	      && strchr(buffer, ':') == NULL
		) {
	  in_header = 0;
	  bytes_seen = 0;
	  end_header = 1;
	  if (remail && !sender_added) {
	    if (fprintf(dest_file, "%sSender: %s\n", prefix, user_name) == EOF) {
	      copy_write_error_exit(errno);
	    }
	  }
	} else if (in_header && remote && fast_header_cmp(buffer, "Sender", (char *)NULL)) {
	  if (remail)
	    if (fprintf(dest_file, "%sSender: %s\n", prefix, user_name) == EOF) {
	      copy_write_error_exit(errno);
	    }
	  sender_added = TRUE;
	  continue;
	}
	if (end_header) {
	  if (update_status) {
	      if (isoff(msg_header->status, NEW)) {
		if (ison(msg_header->status, UNREAD)) {
		  if (fprintf(dest_file, "%sStatus: O\n", prefix) == EOF) {
		    copy_write_error_exit(errno);
		  }
		} else	{ /* read */
		  int x;
#ifdef BSD
		  if (fprintf(dest_file, "%sStatus: OR", prefix) == EOF)
#else
		  if (fprintf(dest_file, "%sStatus: RO", prefix) == EOF)
#endif
		  {
		    copy_write_error_exit(errno);
		  }

		  if (ison(msg_header->status, REPLIED_TO))
		  {
                   if (putc('r', dest_file) == EOF)
		       copy_write_error_exit (errno);
		  }

		  for (x=0;msg_header->mailx_status[x] != '\0'; x++)
                  {
		     if ( strchr("ROr",msg_header->mailx_status[x]) == NULL)
		     {
		        if (putc(msg_header->mailx_status[x], dest_file) == EOF)
			{
			   copy_write_error_exit(errno);
			}
		     }
		  }

                  if (putc('\n', dest_file) == EOF)
		     copy_write_error_exit (errno);

		}

		update_status = FALSE; /* do it only once */
               }  /* else if NEW - indicate NEW with no Status: line. This is
		 * important if we resync a mailfile - we don't want
		 * NEW status lost when we copy each message out.
		 * It is the responsibility of the function that calls
		 * this function to unset NEW as appropriate to its
		 * reason for using this function to copy a message
		 */

		/*
		 * add the missing newline for RFC 822
		 */
	      if (end_header > 0) {
		/* add the missing newline for RFC 822 */
		if (putc('\n', dest_file) == EOF) {
		  copy_write_error_exit(errno);
		}
	      }
	  }
	}
      }

      if (in_header) {
	/* Process checks while in header area */

	if (remove_header) {
	  ignoring = TRUE;
	  continue;
	}

	/* add remote on to front? */
	if (first_line && remote) {
	  no_ret(buffer);
	  if (!strip_from && fprintf(dest_file, "%s%s remote from %s\n",
		  prefix, buffer, host_name) == EOF) {
		copy_write_error_exit(errno);
	  }
	  first_line = FALSE;
	  continue;
	}

	if (!forwarding) {
	  if (fast_header_cmp(buffer, "Status", (char *)NULL) ||
		    /* we will output a new Status: line later, if desired. */
		    (strip_from && fast_stribegConst(buffer, ">From"))) {
	    ignoring = TRUE;
	    continue;
	  } else {
	    err = 0;
	    if (*prefix) err = fputs(prefix, dest_file);
	    if (err != EOF) err = fwrite(buffer, 1, buf_len, dest_file);
	    if (err != buf_len) copy_write_error_exit(errno);
	    continue;
	  }
	}
	else { /* forwarding */

	  if (fast_header_cmp(buffer, "Received", (char *)NULL)
		  || fast_stribegConst(buffer, ">From")
		  || fast_header_cmp(buffer, "Status", (char *)NULL)
		  || fast_header_cmp(buffer, "Return-Path", (char *)NULL))
	      ignoring = TRUE;
	  else
	    if (remail && fast_header_cmp(buffer, "To", (char *)NULL)) {
	      if (fprintf(dest_file, "%sOrig-%s", prefix, buffer) == EOF) {
		copy_write_error_exit(errno);
	      }
	    } else {
	      err = 0;
	      if (*prefix) err = fputs(prefix, dest_file);
	      if (err != EOF) err = fwrite(buffer, 1, buf_len, dest_file);
	      if (err != buf_len) copy_write_error_exit(errno);
	    }
	}
      }
      else { /* not in header */
        /* Process checks that occur after the header area */

	err = 0;
	if (*prefix) err = fputs(prefix, dest_file);
	if (err != EOF) err = fwrite(buffer, 1, buf_len, dest_file);
	if (err != buf_len) copy_write_error_exit(errno);
      }
    }

    if (buf_len + strlen(prefix) > 1)
	if (putc('\n', dest_file) == EOF) {	/* blank line to keep mailx happy *sigh* */
	  copy_write_error_exit(errno);
	}


    /* emit the closing attribution */
    if ((cm_options & CM_ATTRIBUTION) && forwarding && fwdattribution[0]) {
      fputs(makeAttString(attrbuf, sizeof(attrbuf),
	      fwdattribution, 1, msg_header), dest_file);
      putc('\n', dest_file);
    }

    /* Since fprintf is buffered, its return value is only useful for
     * writes which exceed the blocksize.  Do a fflush to ensure that
     * the message has, in fact, been written.
     */
    if (fflush(dest_file) == EOF) {
      copy_write_error_exit(errno);
    }
    mailFile_detach(&mailFile);
}


static char *
makeAttString(retbuf, retbuflen, attribution, sel_field, messageHeader)
char *retbuf;			/* storage space for result		*/
int retbuflen;			/* size of result buffer		*/
const char *attribution;	/* attribution string to expand		*/
int sel_field;			/* field to select in "%[...]" list	*/
const struct header_rec *messageHeader; /* current message header info	*/
{
    const char *aptr = attribution; /* cursor into the attribution spec	*/
    char *rptr = retbuf;	/* cursor into the result buffer	*/
    const char *expval;		/* next item to add to result buffer	*/
    int explen;			/* length of expansion value		*/
    int in_selection;		/* currently doing %[...] list?		*/
    int curr_field;		/* field number of current %[...] list	*/
    char fromfield[STRING];	/* room for parsed from address         */

    --retbuflen;		/* reserve space for '\0' */
    in_selection = FALSE;

    /*
     * Process the attribution string.
     */
    for ( ; retbuflen > 1 && *aptr != '\0' ; ++aptr) {

	/*
	 * Handle the character if it is not a %-expansion.
	 */
	switch (*aptr) {

	case '|':		/* next choice of "%[sel0|sel1|...]" list */
	    if (in_selection) {
		++curr_field;
		expval = NULL;
	    } else {
		expval = aptr;
		explen = 1;
	    }
	    break;

	case ']':		/* end of "%[sel0|sel1|...]" list */
	    if (in_selection) {
		in_selection = FALSE;
		expval = NULL;
	    } else {
		expval = aptr;
		explen = 1;
	    }
	    break;

	case '\\':		/* backslash-quoting */
	    switch (*++aptr) {
	    case 't':
		expval = "\t";
		explen = 1;
		break;
	    case 'n':
		expval = "\n";
		explen = 1;
		break;
	    case '\0':
		--aptr;
		/*FALLTHRU*/
	    default:
		expval = aptr;
		explen = 1;
		break;
	    }
	    break;

	case '%':		/* special %-expansion */

	    switch (*++aptr) {

	    case 's':  /* backward compatibility with 2.4 */
	    case 'F':  /* expand from */
		expval = messageHeader->from;
		explen = strlen(expval);
		break;

	    case 'D': /* expand date */
		expval = ctime(&messageHeader->time_sent);
		explen = strlen(expval)-1; /* exclude newline */
		break;

	    case 'I': /* expand message ID */
		expval = messageHeader->messageid;
		explen = strlen(expval);
		break;

	    case 'S': /* expand subject */
		expval = messageHeader->subject;
		explen = strlen(expval);
		break;

	    case '[': /* %[sel0|sel1|...] */
		in_selection = TRUE;
		curr_field = 0;
		expval = NULL;
		break;

	    case ')': /* special case for %)F - from name */
		/*FALLTHROUGH*/
	    case '>': /* special case for %>F - from address */
		if (*++aptr != 'F') {
		    expval = aptr-2;
		    explen = 3;
		}
		else {
		    int ret;
		    if (*(aptr-1) == ')')		/* from name */
			ret = parse_arpa_mailbox(messageHeader->allfrom, NULL, 0,
			    fromfield, sizeof(fromfield), NULL);
		    else
		    if (*(aptr-1) == '>')		/* from addr */
			ret = parse_arpa_mailbox(messageHeader->allfrom,
			     fromfield, sizeof(fromfield), NULL, 0, NULL);
		    else {
			sprintf(fromfield, "error: %c", *(aptr-1));
		    }
		    if (ret < 0 || *fromfield == '\0') {
			explen = 0;
			while (*(aptr+1) && whitespace(*(aptr+1)))
			    aptr++;
		    }
		    else {
			expval = fromfield;
			explen = strlen(expval);
		    }
		}
		break;

	    case '%': /* add a % and skip on past... */
		expval = aptr;
		explen = 1;
		break;

	    case '\0':
		expval = --aptr;
		explen = 1;
		break;

	    default:
		expval = aptr-1;
		explen = 2;
		break;

	    }

	    break;

	default:			/* just a regular char */
	    expval = aptr;
	    explen = 1;

	}

	/*
	 * Add the expansion value to the result.
	 */
	if (expval != NULL && (!in_selection || curr_field == sel_field)) {
	    if (explen < retbuflen) {
		(void) strncpy(rptr, expval, explen);
		rptr += explen;
	    }
	    retbuflen -= explen;
	}

    }

    *rptr = '\0';
    return(retbuf);
}


static struct stat saved_buf;
static char saved_fname[SLEN];

int
save_file_stats(fname)
char *fname;
{
	/* if fname exists, save the owner, group, mode and filename.
	 * otherwise flag nothing saved. Return 0 if saved, else -1.
	 */

	if (stat(fname, &saved_buf) == 0) {
	  (void)strfcpy(saved_fname, fname, sizeof(saved_fname));
	  dprint(2, (debugfile,
	    "save_file_stats(%s) successful - owner=%d group=%d mode=%o\n",
	    fname, saved_buf.st_uid, saved_buf.st_gid, saved_buf.st_mode));
	  return(0);
	}
        saved_fname[0] = '\0';
	dprint(2, (debugfile, "save_file_stats(%s) FAILED [%s]\n",
	  fname, strerror(errno)));
	return(-1);

}

restore_file_stats(fname)
char *fname;
{
	/* if fname matches the saved file name, set the owner and group
	 * of fname to the saved owner, group and mode,
	 * else to the userid and groupid of the user and to 700.
	 * Return	-1 if the  either mode or owner/group not set
	 *		0 if the default values were used
	 *		1 if the saved values were used
	 */

	int old_umask, new_mode, new_owner, new_group, ret_code;

	new_mode = 0600;
	new_owner = userid;
	new_group = groupid;
	ret_code = 0;

	if(strcmp(fname, saved_fname) == 0) {
	  new_mode = saved_buf.st_mode;
	  new_owner = saved_buf.st_uid;
	  new_group = saved_buf.st_gid;
	  ret_code = 1;
	}
	dprint(2, (debugfile, "restore_file_stats(%s) - %s file stats\n",
	  fname, (ret_code ? "restoring previous" : "setting new")));

	old_umask = umask(0);
	if (chmod(fname, new_mode & 0777) < 0) {
	  ret_code = -1;
	  dprint(4, (debugfile, "  chmod(%s, %.3o) FAILED [%s]\n",
	      fname, (new_mode & 0777), strerror(errno)));
	} else {
	  dprint(4, (debugfile, "  chmod(%s, %.3o) succeeded\n",
	      fname, (new_mode & 0777)));
	}
	(void) umask(old_umask);

#ifdef	BSD
# define CHOWN_IS_RESTRICTED	TRUE	/* BSD restricts chown() to root */
#else
# ifdef _PC_CHOWN_RESTRICTED
#  define CHOWN_IS_RESTRICTED	pathconf(fname, _PC_CHOWN_RESTRICTED)
# else
#  define CHOWN_IS_RESTRICTED	FALSE	/* or so we would like to think... */
# endif
#endif

	if (chown(fname, new_owner, new_group) < 0) {
	  if (!CHOWN_IS_RESTRICTED)
	    ret_code = -1;
	  dprint(4, (debugfile, "  chown(%s, %d, %d) FAILED [%s]\n",
	      fname, new_owner, new_group, strerror(errno)));
	} else {
	  dprint(4, (debugfile, "  chown(%s, %d, %d) succeeded\n",
	      fname, new_owner, new_group));
	}

	return(ret_code);
}

