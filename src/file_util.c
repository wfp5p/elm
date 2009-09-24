

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
 * $Log: file_util.c,v $
 * Revision 1.4  1996/03/14  17:29:34  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:10  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/06/30  14:56:25  wfp5p
 * Alpha 5
 *
 * Revision 1.1.1.1  1995/04/19  20:38:36  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** File oriented utility routines for ELM 

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"
#include "port_stat.h"

extern char *getlogin();


long bytes(name)
char *name;
{
	struct stat sbuf;

	if (stat(name, &sbuf) == 0)
	  return (long) sbuf.st_size;
	if (errno == ENOENT)
	  return 0L;

	ShutdownTerm();
	error2(stderr, catgets(elm_msg_cat, ElmSet, ElmErrorFstat,
	    "Cannot fstat \"%s\"! [%s]"), name, strerror(errno));
	leave(LEAVE_EMERGENCY);
}


long fsize(fd)
FILE *fd;
{
	struct stat sbuf;

	fflush (fd); /* ensure stdio buffers are flushed to disk! */
	(void) fstat(fileno(fd), &sbuf);
	return (long) sbuf.st_size;
}


int copy(fname_src, fname_dest, isspool)
const char *fname_src, *fname_dest;
int isspool;
{
    /*
     * Copy a specified file to the indicated destination.
     * Return 0 on success.  On failure, display error and return -1.
     */

    FILE *fp_src, *fp_dest;
    int rc;
#ifdef FTRUNCATE
    int do_truncate; /* are we overwriting the existing file? */
    off_t size_src; /* size of infile, used for ftruncate call */
#endif

    rc = -1;
    fp_src = NULL;
    fp_dest = NULL;

    if ((fp_src = file_open(fname_src, "r")) == NULL)
	return -1;

#ifdef FTRUNCATE
    /* If the user is over quota but the "dest" file exists and
     * "src" is not larger, then opening for update, copying, and
     * truncating to the new size may work or will at least
     * prevent the old file from being clobbered since no new
     * blocks need to be allocated.  If we instead open for write,
     * we'll free the blocks and won't be able to allocate new ones.
     * Since r+ requires the file to already exist, we also try
     * w in case the file is new.  If we open for update (r+), we
     * will need to perform a ftruncate.
     */
    size_src = fsize(fp_src);
    do_truncate = ((fp_dest = fopen(fname_dest, "r+")) != NULL);
#endif
    if (fp_dest == NULL && (fp_dest = file_open(fname_dest, "w")) == NULL)
	goto done;
    if (!isspool && groupid != mailgroupid)
	    chown(fname_dest, userid, groupid);

    if (file_copy(fp_src, fp_dest, fname_src, fname_dest) < 0)
	goto done;

#ifdef FTRUNCATE
    fflush(fp_dest);
    if (do_truncate && ftruncate(fileno(fp_dest), size_src) != 0) {
      error1(catgets(elm_msg_cat, ElmSet, ElmTruncateFailedCopy,
		"Error on ftruncate() of \"%s\"! [%s]"),
		fname_dest, strerror(errno));
      goto done;
    }
#endif
    if (file_close(fp_dest, fname_dest) < 0)
	goto done;
    fp_dest = NULL;
    if (file_close(fp_src, fname_src) < 0)
	goto done;
    fp_src = NULL;
    rc = 0;

done:
    if (fp_dest != NULL)
	(void) fclose(fp_dest);
    if (fp_src != NULL)
	(void) fclose(fp_src);
    return rc;
}


#define FORWARDSIGN	"Forward to "
int
check_mailfile_size(mfile)
char *mfile;
{
	/** Check to ensure we have mail.  Only used with the '-z'
	    starting option. So we output a diagnostic if there is
	    no mail to read (including  forwarding).
	    Return 0 if there is mail,
		   <0 if no permission to check,
		   1 if no mail,
		   2 if no mail because mail is being forwarded.
	 **/

	char firstline[SLEN];
	int retcode;
	struct stat statbuf;
	FILE *fp = NULL;

	/* see if file exists first */
	if (can_access(mfile, READ_ACCESS) != 0)
	  retcode = 1;					/* no file or no perm */

	/* read access - now see if file has a reasonable size */
	else if ((fp = fopen(mfile, "r")) == NULL)
	  retcode = -1;		/* no perm? should have detected this above! */
	else if (fstat(fileno(fp), &statbuf) == -1) 
	  retcode = -1;					/* arg error! */
	else if (statbuf.st_size < 2)		
	  retcode = 1;	/* empty or virtually empty, e.g. just a newline */

	/* file has reasonable size - see if forwarding */
	else if (mail_gets (firstline, SLEN, fp) == 0)
	  retcode = 1;		 /* empty? should have detected this above! */
	else if (strbegConst(firstline, FORWARDSIGN))
	  retcode = 2;					/* forwarding */

	/* not forwarding - so file must have some mail in it */
	else
	  retcode = 0;

	/* now display the appropriate message if there isn't mail in it */
	switch(retcode) {

	case -1:	fprintf(stderr, catgets(elm_msg_cat, ElmSet,
				ElmNoPermRead,
				"You have no permission to read %s!\n\r"),
				mfile);
			break;
	case 1:		fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmNoMail,
				"You have no mail.\n\r"));
			break;
	case 2:		no_ret(firstline) /* remove newline before using */
			fprintf(stderr, catgets(elm_msg_cat,
				ElmSet,ElmMailBeingForwarded,
				"Your mail is being forwarded to %s.\n\r"),
			  firstline + strlen(FORWARDSIGN));
			break;
	}
	if (fp)
	  fclose(fp);

	return(retcode);
}

