

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.5 $   $State: Exp $
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
 * $Log: utils.c,v $
 * Revision 1.5  1996/03/14  17:30:00  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1995/09/29  17:42:36  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/09/11  15:19:33  wfp5p
 * Alpha 7
 *
 * Revision 1.2  1995/06/30  14:56:28  wfp5p
 * Alpha 5
 *
 * Revision 1.1.1.1  1995/04/19  20:38:39  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** Utility routines for ELM

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

void leave P_((int));


/*
 * The initialize() procedure sets the "xalloc_fail_handler" vector to
 * point here in the event that xmalloc() or friends fail.
 */
void malloc_failed_exit(const char *proc, unsigned int len)
{
    ShutdownTerm();
    show_error(catgets(elm_msg_cat, ElmSet, ElmCouldntMallocBytes,
	"Out of memory!  [%s couldn't allocate %d bytes]"), proc, len);
    leave(LEAVE_EMERGENCY);
}


static int ignore_sigs[] = {
    SIGHUP,
    SIGINT,
    SIGQUIT,
#ifdef SIGTSTP
    SIGTSTP,
#endif
#ifdef SIGSTOP
    SIGSTOP,
#endif
#ifdef SIGCONT
    SIGCONT,
#endif
#ifdef SIGWINCH
    SIGWINCH,
#endif
    0
};


void leave(int mode)
{
    char buf[SLEN], *s;
    int i;

    /*
     * Ignore signals until cleanup is complete.  This is particularly
     * important for some OSes (e.g. Ultrix) that like to generate
     * additional SIGCONTs.
     */
    for (i = 0 ; ignore_sigs[i] > 0 ; ++i)
	signal(ignore_sigs[i], SIG_IGN);

    dprint(2, (debugfile, "\nLeaving mailer - leave() mode %o\n", mode));

    ShutdownTerm();

    if (mode == LEAVE_EMERGENCY) {
	show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveEmergencyExitTaken,
		"Emergency exit taken!"));
    }

    /* delete composition file */
    sprintf(buf, "%s%s%d", temp_dir, temp_file, getpid());
    if (access(buf, ACCESS_EXISTS) == 0) {
	if (mode & LEAVE_KEEP_EDITTMP)
	    show_error(catgets(elm_msg_cat, ElmSet, ElmLeavePreservingEditor,
			"Preserving editor composition file \"%s\" ..."), buf);
	else
	    (void) unlink(buf);
    }

    /* delete temporary copy of spool folder */
    if (curr_folder.tempname[0] && access(curr_folder.tempname, ACCESS_EXISTS) == 0) {
	if (mode & LEAVE_KEEP_TEMPFOLDER) {
	    show_error(catgets(elm_msg_cat, ElmSet, ElmLeavePreservingTemp,
			"Preserving temporary folder \"%s\" ..."),
			curr_folder.tempname);
	} else {
	  show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveDiscardingChanges,
		      "Discarding any changes made to mail folder ..."));
	  (void) unlink(curr_folder.tempname);
	}
    }

    /* unlock the mailbox */
    if (curr_folder.flags & FOLDER_IS_SPOOL) {
	if (mode & LEAVE_KEEP_LOCK) {
	    s = mk_lockname(curr_folder.filename);
	    if (access(s, ACCESS_EXISTS) == 0) {
		show_error(catgets(elm_msg_cat, ElmSet, ElmLeavePreservingLock,
			    "Preserving folder lock file \"%s\" ..."), s);
	    }
	} else {
	    if (curr_folder.fp)
		fflush (curr_folder.fp);
	    elm_unlock();
	}
    }

    exit(LEAVE_EXIT_STATUS(mode));
}


int get_page(int msg_pointer)
{
	/** Ensure that 'current' is on the displayed page,
	    returning NEW_PAGE iff the page changed! **/

	register int first_on_page, last_on_page;

	if (headers_per_page == 0)
	  return(SAME_PAGE); /* What else can I do ? */

	first_on_page = (header_page * headers_per_page) + 1;

	last_on_page = first_on_page + headers_per_page - 1;

	if (selected)	/* but what is it on the SCREEN??? */
	  msg_pointer = compute_visible(msg_pointer);

	if (selected && msg_pointer > selected)
	  return(SAME_PAGE);	/* too far - page can't change! */

	if (msg_pointer > last_on_page) {
	  header_page = (int) (msg_pointer-1)/ headers_per_page;
	  return(NEW_PAGE);
	}
	else if (msg_pointer < first_on_page) {
	  header_page = (int) (msg_pointer-1) / headers_per_page;
	  return(NEW_PAGE);
	}
	else
	  return(SAME_PAGE);
}

char *nameof(char *filename)
{
	/** checks to see if 'filename' has any common prefixes, if
	    so it returns a string that is the same filename, but
	    with '=' as the folder directory, or '~' as the home
	    directory..
	**/

	static char buffer[STRING];
	register int i = 0, iindex = 0, len;

	len = strlen(folders);
	if (strncmp(filename, folders, len) == 0 &&
	    len > 0 && (filename[len] == '/' || filename[len] == '\0')) {
	  buffer[i++] = '=';
	  iindex = len;
	  if(filename[iindex] == '/')
	    iindex++;
	}
	else
	{
	  len = strlen(user_home);
	  if (strncmp(filename, user_home, len) == 0 &&
	      len > 1 && (filename[len] == '/' || filename[len] == '\0')) {
	    buffer[i++] = '~';
	    iindex = len;
	  }
	  else iindex = 0;
	}

	while (filename[iindex] != '\0')
	  buffer[i++] = filename[iindex++];
	buffer[i] = '\0';

	return( (char *) buffer);
}

