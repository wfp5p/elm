

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
 * $Log: can_open.c,v $
 * Revision 1.3  1999/03/24  14:03:51  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.2  1995/09/29  17:41:06  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** can_open - can this user open this file using their normal uid/gid

**/

#include "elm_defs.h"
#include <errno.h>
#include "port_stat.h"
#include "port_wait.h"

#ifndef I_UNISTD
void _exit();
#endif

int can_open(const char *file, const char *mode)
{
	/** Returns 0 iff user can open the file.  This is not
	    the same as can_access - it's used for when the file might
	    not exist... **/

	FILE *fd;
	int pid, err, w, preexisted;
	waitstatus_t status;
	register SIGHAND_TYPE (*istat)(), (*qstat)();

#ifdef VFORK
	if ((pid = vfork()) == 0)
#else
	if ((pid = fork()) == 0)
#endif
	{
	  setegid(getgid());
	  setuid(getuid());		/** back to normal userid **/
	  preexisted = (access(file, ACCESS_EXISTS) == 0);
	  if ((fd = fopen(file, mode)) == NULL)
	    _exit(errno);
	  fclose(fd);		/* don't just leave it open! */
	  if(!preexisted)	/* don't leave it if this test created it! */
	    unlink(file);
	  _exit(0);
	}

	if (pid < 0)
	  return EAGAIN;

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);

	while ((w = wait(&status)) != pid) {
	  if (w < 0 && errno != EINTR)
	    break;
	}
	err = errno;

	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);

	if (w < 0)
	  return err;
	if (!WIFEXITED(status))
	  return EINTR; /* ehhh...pick one...shouldn't really happen */
	return WEXITSTATUS(status);
}
