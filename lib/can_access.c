

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
 * $Log: can_access.c,v $
 * Revision 1.3  1999/03/24  14:03:50  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.2  1995/09/29  17:41:04  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/*
 * can_access() - Verify that a user can access a filesystem entry
 * as their normal uid/gid, and also that the entry is a regular file.
 * Returns 0 on success.  Returns -1 and sets errno on failure.
 */

#include "elm_defs.h"
#include <errno.h>
#include "port_stat.h"
#include "port_wait.h"

#ifndef VFORK
# ifndef vfork
#  define vfork fork
# endif
#endif

static int painful_access_check P_((const char *, int));


int
can_access(fname, mode)
const char *fname;
int mode;
{
    struct stat stat_buf;
    int rc;

    if (mode == F_OK || (getuid() == geteuid() && getgid() == getegid()))
	rc = access(fname, mode);
    else
	rc = painful_access_check(fname, mode);

    if ( (rc == 0) && 
	 (stat(fname, &stat_buf) == 0) && 
	 ( !S_ISREG(stat_buf.st_mode) && !S_ISCHR(stat_buf.st_mode) ) )
   {
	errno = EISDIR; /* well...at least it is not a file */
	rc = -1;
    }

    return rc;
}


#ifndef I_UNISTD
void _exit();
#endif

static int
painful_access_check(file, mode)
const char *file; 
int   mode;
{
	int pid, w, err; 
	waitstatus_t status;
	register SIGHAND_TYPE (*istat)(), (*qstat)();

	if ((pid = vfork()) < 0) {
	  errno = EAGAIN;
	  return -1;
	}

	if (pid == 0) {
	  SETGID(getgid());
	  setuid(getuid());
	  _exit(access(file, mode) == 0 ? 0 : errno);
	}

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);

	while ((w = wait(&status)) != pid) {
	  if (w < 0 && errno != EINTR)
	    break;
	}
	err = errno;

	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);

	/* wait() failed?? */
	if (w < 0) {
	  errno = err;
	  return -1;
	}

	/* subprocess stopped?? */
	if (!WIFEXITED(status)) {
	  errno = EINTR; /* ehhh...pick one...shouldn't really happen */
	  return -1;
	}

	/* exit status is errno from access check */
	if ((err = WEXITSTATUS(status)) != 0) {
	  errno = err;
	  return -1;
	}

	errno = 0;
	return 0;
}


#ifdef _TEST /*{*/

static char *mstr[] = {
	"F_OK",			/* 00 --- */
	"X_OK",			/* 01 --x */
	"W_OK",			/* 02 -w- */
	"W_OK X_OK",		/* 03 -wx */
	"R_OK",			/* 04 r-- */
	"R_OK X_OK",		/* 05 r-x */
	"R_OK W_OK",		/* 06 rw- */
	"R_OK W_OK X_OK",	/* 07 rwx */
};

main()
{
    char path[512], buf[512], *pstr;
    int mode, rc, err;

    (void) strcpy(path, "/blurfl/grimmitz");
    mode = 0;

    for (;;) {

	printf("path [%s] : ", path);
	if (gets(buf) == NULL)
	    break;
	if (buf[0] != '\0')
	    (void) strcpy(path, buf);

	printf("mode [%d] : ", mode);
	if (gets(buf) == NULL)
	    break;
	if (buf[0] != '\0')
	    mode = atoi(buf);

	rc = can_access(path, mode);
	err = errno;

	switch (rc) {
	case 0:
	    printf("can_access(\"%s\", \"%s\") succeeded\n",
			path, mstr[mode&07]);
	    break;
	case -1:
	    printf("can_access(\"%s\", \"%s\") failed [%s]\n",
			path, mstr[mode&07], strerror(err));
	    break;
	default:
	    printf("can_access(\"%s\", \"%s\") ???rc=%d??? [%s]\n",
			path, mstr[mode&07], rc, strerror(err));
	    break;
	}
	putchar('\n');

    }

    putchar('\n');
    exit(0);
}

#endif /*}_TEST*/

