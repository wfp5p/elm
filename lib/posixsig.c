

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
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
 * $Log: posixsig.c,v $
 * Revision 1.2  1995/09/29  17:41:27  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** Duplicate the old signal() call with POSIX sigaction

**/

#include "elm_defs.h"

#ifdef POSIX_SIGNALS

#ifndef SIG_ERR
#  ifdef BADSIG
#    define SIG_ERR BADSIG
#  else
#    define SIG_ERR -1
#  endif /* BADSIG */
#endif /* SIG_ERRR */

/*
 * This routine used to duplicate the old signal() calls
 */
SIGHAND_TYPE
#if ANSI_C && !defined(apollo)
(*posix_signal(signo, fun))(int)
	int signo;
	SIGHAND_TYPE (*fun)(int);
#else
(*posix_signal(signo, fun))()
	int signo;
	SIGHAND_TYPE (*fun)();
#endif
{
	struct sigaction act;	/* new signal action structure */
	struct sigaction oact;  /* returned signal action structure */ 

	/*   Setup a sigaction struct */

 	act.sa_handler = fun;        /* Handler is function passed */
	sigemptyset(&(act.sa_mask)); /* No signal to mask while in handler */
	act.sa_flags = 0;
#ifdef SA_INTERRUPT
	act.sa_flags |= SA_INTERRUPT;           /* SunOS */
#endif

	/* use the sigaction() system call to set new and get old action */

	sigemptyset(&oact.sa_mask);
	if(sigaction(signo, &act, &oact))
		/* If sigaction failed return -1 */
	    return(SIG_ERR);
	else
        	/* use the previous signal handler as a return value */
	    return(oact.sa_handler);
}
#endif /* POSIX_SIGNALS */
