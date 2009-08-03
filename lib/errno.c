
static char rcsid[] = "@(#)$Id: errno.c,v 1.2 1995/09/29 17:41:08 wfp5p Exp $";

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
 * $Log: errno.c,v $
 * Revision 1.2  1995/09/29  17:41:08  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This routine maps error numbers to error names and error messages.
    These are all directly ripped out of the include file errno.h, and
    are HOPEFULLY standardized across the different breeds of Unix!!

    If (alas) yours are different, you should be able to use awk to
    mangle your errno.h file quite simply...

**/

#include "elm_defs.h"

#ifndef STRERROR

#ifdef ERRLST
extern char *sys_errlist[];
extern int sys_nerr;
#else
static char *sys_errlist[] = { 
/*  0 - NOERROR	*/ "No error status currently",
/*  1 - EPERM	*/ "Not super-user",
/*  2 - ENOENT	*/ "No such file or directory",
/*  3 - ESRCH	*/ "No such process",
/*  4 - EINTR	*/ "Interrupted system call",
/*  5 - EIO	*/ "I/O error",
/*  6 - ENXIO	*/ "No such device or address",
/*  7 - E2BIG	*/ "Arg list too long",
/*  8 - ENOEXEC	*/ "Exec format error",
/*  9 - EBADF	*/ "Bad file number",
/* 10 - ECHILD	*/ "No children",
/* 11 - EAGAIN	*/ "No more processes",
/* 12 - ENOMEM	*/ "Not enough core",
/* 13 - EACCES	*/ "Permission denied",
/* 14 - EFAULT	*/ "Bad address",
/* 15 - ENOTBLK	*/ "Block device required",
/* 16 - EBUSY	*/ "Mount device busy",
/* 17 - EEXIST	*/ "File exists",
/* 18 - EXDEV	*/ "Cross-device link",
/* 19 - ENODEV	*/ "No such device",
/* 20 - ENOTDIR	*/ "Not a directory",
/* 21 - EISDIR	*/ "Is a directory",
/* 22 - EINVAL	*/ "Invalid argument",
/* 23 - ENFILE	*/ "File table overflow",
/* 24 - EMFILE	*/ "Too many open files",
/* 25 - ENOTTY	*/ "Not a typewriter",
/* 26 - ETXTBSY	*/ "Text file busy",
/* 27 - EFBIG	*/ "File too large",
/* 28 - ENOSPC	*/ "No space left on device",
/* 29 - ESPIPE	*/ "Illegal seek",
/* 30 - EROFS	*/ "Read only file system",
/* 31 - EMLINK	*/ "Too many links",
/* 32 - EPIPE	*/ "Broken pipe",
/* 33 - EDOM	*/ "Math arg out of domain of func",
/* 34 - ERANGE	*/ "Math result not representable",
/* 35 - ENOMSG	*/ "No message of desired type",
/* 36 - EIDRM	*/ "Identifier removed"
	};
static int sys_nerr = 37;
#endif /* ERRLST */

#ifdef strerror
# undef strerror
#endif

char *strerror(errnumber)
int errnumber;
{
	static char buffer[50];
	if (errnumber < 0 || errnumber >= sys_nerr)  {
	  sprintf(buffer,"ERR-UNKNOWN (%d)", errnumber);
	  return(buffer);
	}
	return( sys_errlist[errnumber] );
}

#endif /* !STRERROR */
