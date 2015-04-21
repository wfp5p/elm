

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.6 $   $State: Exp $
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
 * $Log: lock.c,v $
 * Revision 1.6  1999/03/24  14:04:02  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.5  1996/03/14  17:29:41  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1996/03/13  14:38:00  wfp5p
 * Alpha 9 before Chip's big changes
 *
 * Revision 1.3  1995/09/29  17:42:17  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:12  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:37  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** leave current folder, updating etc. as needed...

**/


#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

#ifdef USE_FCNTL_LOCKING
# define SYSCALL_LOCKING
#else
# ifdef USE_FLOCK_LOCKING
#   define SYSCALL_LOCKING
# endif
#endif

#ifdef SYSCALL_LOCKING
#  if (defined(BSD) || !defined(apollo))
#    include <sys/file.h>
#  endif
#endif
#ifdef I_TIME
#  include <time.h>
#endif
#ifdef I_SYSTIME
#  include <sys/time.h>
#endif


#define	FLOCKING_OK	0
#define	FLOCKING_RETRY	1
#define	FLOCKING_FAIL	-1

static int  lock_state = OFF;

#ifdef	USE_DOTLOCK_LOCKING
static char *lockfile=(char*)0;
#endif  /* USE_DOTLOCK_LOCKING */

static int flock_fd,	/* file descriptor for flocking mailbox itself */
	   create_fd;	/* file descriptor for creating lock file */

#ifdef USE_FCNTL_LOCKING
static struct flock lock_info;
#endif

static int Grab_the_file(int flock_fd)
{
    errno = 0;

#ifdef   USE_FCNTL_LOCKING
    lock_info.l_type = F_WRLCK;
    lock_info.l_whence = 0;
    lock_info.l_start = 0;
    lock_info.l_len = 0;

    if (fcntl(flock_fd, F_SETLK, &lock_info)) {
	return ((errno == EACCES) || (errno == EAGAIN))
		? FLOCKING_RETRY
		: FLOCKING_FAIL ;
    }
#endif

#ifdef	USE_FLOCK_LOCKING
    if (flock (flock_fd, LOCK_NB | LOCK_EX)) {

	retcode = ((errno == EWOULDBLOCK) || (errno == EAGAIN))
		   ? FLOCKING_RETRY
		   : FLOCKING_FAIL ;

#ifdef USE_FCNTL_LOCKING
	lock_info.l_type = F_UNLCK;
	lock_info.l_whence = 0;
	lock_info.l_start = 0;
	lock_info.l_len = 0;

	/*
	 *  Just unlock it because we did not succeed with the
	 *  flock()-style locking. Never mind the return value.
	 *  It was our own lock anyway if we ever got this far.
	 */
	fcntl (flock_fd, F_SETLK, &lock_info);
#endif
	return retcode;
    }
#endif

    return FLOCKING_OK;
}

static int Release_the_file(int flock_fd)
{
    int	fcntlret = 0,
	flockret = 0,
	fcntlerr = 0,
	flockerr = 0;

#ifdef	USE_FLOCK_LOCKING
    errno = 0;
    flockret = flock (flock_fd, LOCK_UN);
    flockerr = errno;
#endif

#ifdef	USE_FCNTL_LOCKING
    lock_info.l_type = F_UNLCK;
    lock_info.l_whence = 0;
    lock_info.l_start = 0;
    lock_info.l_len = 0;

    errno = 0;
    fcntlret = fcntl (flock_fd, F_SETLK, &lock_info);
/*    fcntlerr = errno; */
#endif

    if (fcntlret) {
	errno = fcntlerr;
	return fcntlret;
    }
    else if (flockret) {
	errno = flockerr;
	return flockret;
    }
    return 0;
}

int elm_lock(int direction)
{
      /** Create lock file to ensure that we don't get any mail
	  while altering the folder contents!
	  If it already exists sit and spin until
	     either the lock file is removed...indicating new mail
	  or
	     we have iterated MAX_ATTEMPTS times, in which case we
	     either fail or remove it and make our own (determined
	     by if REMOVE_AT_LAST is defined in header file

	  If direction == LOCK_INCOMING then DON'T remove the lock file
	  on the way out!  (It'd mess up whatever created it!).

	  But if that succeeds and if we are also locking by flock(),
	  follow a similar algorithm. Now if we can't lock by flock(),
	  we DO need to remove the lock file, since if we got this far,
	  we DID create it, not another process.
      **/

      int create_iteration = 0,
		   flock_iteration = 0;
      int kill_status;
      char pid_buffer[TLEN];
      int grabrc;

#ifdef	USE_DOTLOCK_LOCKING		/* { USE_DOTLOCK_LOCKING  */
      /* formulate lock file name */
      lockfile = mk_lockname(curr_folder.filename);

      if (mailgroupid != groupid)
        setegid(mailgroupid); /* reset id so that we can get at lock file */


      /** first, try to read the lock file, and if possible, check the pid.
	  If we can validate that the pid is no longer active, then remove
	  the lock file.
       **/
      errno = 0; /* some systems don't clear the errno flag */
      if((create_fd=open(lockfile,O_RDONLY)) != -1) {
	if (read(create_fd, pid_buffer, sizeof(pid_buffer)) > 0) {
	  create_iteration = atoi(pid_buffer);
	  if (create_iteration) {
	    kill_status = kill(create_iteration, 0);
	    if (kill_status != 0 && errno != EPERM) {
/*	      close(create_fd); */
	      if (unlink(lockfile) != 0) {
		ShutdownTerm();
		show_error(catgets(elm_msg_cat, ElmSet,
		  ElmLeaveCouldntRemoveCurLock,
		  "Couldn't remove current lock file %s! [%s]"),
		  lockfile, strerror(errno));
	        if (mailgroupid != groupid)
		  setegid(groupid);
		leave(direction == LOCK_INCOMING ? LEAVE_ERROR : LEAVE_EMERGENCY);
	      }
	    }
	  }
	}
        close(create_fd);
	create_iteration = 0;
      }


      /* try to assert create lock file MAX_ATTEMPTS times */
      do {

	errno = 0;
	if((create_fd=open(lockfile,O_WRONLY | O_CREAT | O_EXCL,0444)) != -1)
	  break;
	else {
	  if(errno != EEXIST && errno != EACCES) {
	    /* Creation of lock failed NOT because it already exists!!! */
            /* If /var/mail nfs mounted on Solaris 2.3 at least you can */
	    /* get EACCES.  Treat it like EEXIST. */
	    ShutdownTerm();
	    show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveErrorCreatingLock,
		"Couldn't create lock file %s! [%s]"),
		lockfile, strerror(errno));
	    if (mailgroupid != groupid)
	      setegid(groupid);
	    leave(LEAVE_ERROR);
	  }
	}
	dprint(2, (debugfile,"File '%s' already exists!  Waiting...(lock)\n",
	  lockfile));
	show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveWaitingToRead,
	  "Waiting to read mailbox while mail is being received: attempt #%d"),
	  create_iteration);
	sleep(2);
      } while (create_iteration++ < MAX_ATTEMPTS);
      clear_error();

      if(!(create_fd >= 0 || errno == ENOENT)) {

	/* we weren't able to create the lock file */

#ifdef REMOVE_AT_LAST

	/** time to waste the lock file!  Must be there in error! **/
	dprint(2, (debugfile,
	   "Warning: I'm giving up waiting - removing lock file(lock)\n"));
	if (direction == LOCK_INCOMING)
	  show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveTimedOutRemoving,
		"Timed out - removing current lock file..."));
	else
	  show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveThrowingAwayLock,
		"Throwing away the current lock file!"));

	if (unlink(lockfile) != 0) {
	  ShutdownTerm();
	  show_error(stderr, catgets(elm_msg_cat, ElmSet,
	        ElmLeaveCouldntRemoveCurLock,
	    "Couldn't remove current lock file %s! [%s]",
	    lockfile, strerror(errno)));
	  if (mailgroupid != groupid)
	    setegid(groupid);
	  leave(direction == LOCK_INCOMING ? LEAVE_ERROR : LEAVE_EMERGENCY);
	}

	/* we've removed the bad lock, let's try to assert lock once more */
	if((create_fd=open(lockfile,O_WRONLY | O_CREAT | O_EXCL,0444)) == -1){

	  /* still can't lock it - just give up */
	  ShutdownTerm();
	  show_error(stderr, catgets(elm_msg_cat, ElmSet,
	    ElmLeaveErrorCreatingLock,
	    "Couldn't create lock file %s! [%s]",
	    lockfile, strerror(errno)));
	  if (mailgroupid != groupid)
	    setegid(groupid);
	  leave(LEAVE_ERROR);
	}
#else
	/* Okay...we die and leave, not updating the mailfile mbox or
	   any of those! */

	MoveCursor(LINES, 0);
	Raw(OFF);
	if (direction == LOCK_INCOMING) {
	  ShutdownTerm();
	  show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveGivingUp,
"Cannot lock folder - giving up.  Please try again in a few minutes."));
	  if (mailgroupid != groupid)
	    setegid(groupid);
	  leave(LEAVE_ERROR|LEAVE_KEEP_LOCK);
	} else {
	  ShutdownTerm();
	  show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveErrorTimedOutLock,
		"Timed out on locking mailbox.  Leaving program."));
	  if (mailgroupid != groupid)
	    setegid(groupid);
	  leave(LEAVE_ERROR);
	}
#endif
      }

      /* If we're here we successfully created the lock file */
      dprint(5,
	(debugfile, "Lock %s %s for file %s on.\n", lockfile,
	(direction == LOCK_INCOMING ? "incoming" : "outgoing"), curr_folder.filename));

      /* Place the pid of Elm into the lock file for SVR3.2 and its ilk */
      sprintf(pid_buffer, "%d", getpid());
      write(create_fd, pid_buffer, strlen(pid_buffer)+1);

      (void)close(create_fd);
     if (mailgroupid != groupid)
       setegid(groupid);
#endif	/* } USE_DOTLOCK_LOCKING */

#ifdef SYSCALL_LOCKING
      /* Now we also need to lock the file with flock(2) */

      /* Open mail file separately for locking */
      if((flock_fd = open(curr_folder.filename, O_RDWR)) < 0) {
	dprint(1, (debugfile,
	    "Error encountered attempting to reopen %s for lock\n", curr_folder.filename));
	dprint(1, (debugfile, "** %s **\n", strerror(errno)));
	MoveCursor(LINES, 0);
	Raw(OFF);
	printf(catgets(elm_msg_cat, ElmSet, ElmLeaveErrorReopenMailbox,
 "\nError encountered while attempting to reopen mailbox %s for lock;\n"),
	      curr_folder.filename);
	printf("** %s. **\n\n", strerror(errno));

#ifdef	USE_DOTLOCK_LOCKING
	(void)unlink(lockfile);
#endif /* USE_DOTLOCK_LOCKING */
	leave(LEAVE_ERROR);
      }

      /* try to assert lock MAX_ATTEMPTS times */
      do {

	switch ((grabrc = Grab_the_file (flock_fd))) {

	case	FLOCKING_OK:
	    goto    EXIT_RETRY_LOOP;

	case	FLOCKING_FAIL:
	    /*
	     *	Creation of lock failed
	     *	NOT because it already exists!!!
	     */

	    ShutdownTerm();
	    show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveErrorFlockMailbox,
		 "Cannot flock folder \"%s\"! [%s]"),
		  curr_folder.filename, strerror(errno));
#ifdef	USE_DOTLOCK_LOCKING
	    (void)unlink(lockfile);
#endif /* USE_DOTLOCK_LOCKING */
	    leave(LEAVE_ERROR);

	    break;

	case	FLOCKING_RETRY:
	default:
	    dprint (2, (debugfile,
	      "Mailbox '%s' already locked!  Waiting...(lock)\n", curr_folder.filename));
	    show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveWaitingToRead,
			   "Waiting to read mailbox while mail is being received: attempt #%d"),
		    flock_iteration);
	    sleep(2);
	}
      } while (flock_iteration++ < MAX_ATTEMPTS);

EXIT_RETRY_LOOP:
      clear_error();

      if(grabrc != FLOCKING_OK) {
	/* We couldn't lock the file. We die and leave not updating
	 * the mailfile mbox or any of those! */
	ShutdownTerm();
	show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveGivingUp,
"Cannot lock folder - giving up.  Please try again in a few minutes."));
#ifdef	USE_DOTLOCK_LOCKING
	(void)unlink(lockfile);
#endif
	leave(LEAVE_ERROR);
      }

      /* We locked the file */
      dprint(5,
	(debugfile, "Lock %s on file %s on.\n",
	(direction == LOCK_INCOMING ? "incoming" : "outgoing"), curr_folder.filename));
#endif /* SYSCALL_LOCKING */

      dprint(5,
	(debugfile, "Lock %s for file %s on successfully.\n",
	(direction == LOCK_INCOMING ? "incoming" : "outgoing"), curr_folder.filename));
      lock_state = ON;
      return(0);
}

int elm_unlock(void)
{
	/** Remove the lock file!    This must be part of the interrupt
	    processing routine to ensure that the lock file is NEVER
	    left sitting in the mailhome directory!

	    If also using flock(), remove the file lock as well.
	 **/

	int retcode = 0;

#ifndef USE_DOTLOCK_LOCKING
	dprint(5,
	  (debugfile, "Lock (no .lock) for file %s %s off.\n",
	    curr_folder.filename, (lock_state == ON ? "going" : "already")));
#else   /* USE_DOTLOCK_LOCKING */
	dprint(5,
	  (debugfile, "Lock %s for file %s %s off.\n",
 	    (lockfile ? lockfile : "none"),
	    curr_folder.filename,
	    (lock_state == ON ? "going" : "already")));
#endif  /* USE_DOTLOCK_LOCKING */

	if(lock_state == ON) {

#ifdef SYSCALL_LOCKING
		retcode = Release_the_file (flock_fd);
		if (retcode) {

		dprint(1, (debugfile,
			   "Error %s\n\ttrying to unlock file %s (%s)\n",
			   strerror(errno), curr_folder.filename, "unlock"));

		/* try to force unlock by closing file */
		if (close (flock_fd) == -1) {
		    dprint (1, (debugfile,
	      "Error %s\n\ttrying to force unlock file %s via close() (%s)\n",
				strerror(errno),
				curr_folder.filename, "unlock"));
		    show_error(catgets (elm_msg_cat, ElmSet,
				     ElmLeaveCouldntUnlockOwnMailbox,
				     "Couldn't unlock my own mailbox %s!"),
				     curr_folder.filename);
		    return(retcode);
		}
	    }
	    (void)close(flock_fd);
#endif
#ifndef	USE_DOTLOCK_LOCKING	/* { USE_DOTLOCK_LOCKING */
	   lock_state = OFF;		/* indicate we don't have a lock on */
#else
	  if (mailgroupid != groupid)
	    setegid(mailgroupid);
	  if((retcode = unlink(lockfile)) == 0) {	/* remove lock file */
	    *lockfile = '\0';		/* null lock file name */
	    lock_state = OFF;		/* indicate we don't have a lock on */
	  } else {
	    dprint(1, (debugfile,
	      "Error %s\n\ttrying to unlink file %s (%s)\n",
	      strerror(errno), lockfile,"unlock"));
	      show_error(catgets(elm_msg_cat, ElmSet, ElmLeaveCouldntRemoveOwnLock,
		"Couldn't remove my own lock file %s!"), lockfile);
	  }
	  if (mailgroupid != groupid)
	    setegid(groupid);
#endif	/* } !USE_DOTLOCK_LOCKING */
	}
	return(retcode);
}
