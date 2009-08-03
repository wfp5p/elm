
static char rcsid[] = "@(#)$Id: elm_access.c,v 1.2 1995/09/29 17:41:08 wfp5p Exp $";

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
 * $Log: elm_access.c,v $
 * Revision 1.2  1995/09/29  17:41:08  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1  1995/09/11  15:18:53  wfp5p
 * Alpha 7
 *
 *
 ******************************************************************************/

/** elm_access - does this file exist, note a bare symlink pointing to nothing
		 is considered existing for this purpose.  This differs from
		 the normal access() routine.  This is required to prevent
		 spoofing on the elm temporary files.

**/

#include "elm_defs.h"
#include "port_stat.h"

int
elm_access(file, mode)
const char *file; 
int   mode;
{
	/** returns ZERO iff access to file is permitted, or if exists
	    access is being checked if a bare symlink exists
	    or "errno" otherwise **/

	struct stat stat_buf;

	if (mode != ACCESS_EXISTS)
		return access(file, mode);

	/** if the lstat succeeds, the file exists, as it was statable **/

	return lstat(file, &stat_buf);
}
