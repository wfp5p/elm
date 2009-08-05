

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
 * $Log: mk_lockname.c,v $
 * Revision 1.2  1995/09/29  17:41:19  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

static char lock_name[SLEN];

char * mk_lockname(file_to_lock)
const char *file_to_lock;
{
      /** Create the proper name of the lock file for file_to_lock,
            which is presumed to be a spool file full path (see
            get_folder_type()), and put it in the static area lock_name.
            Return lock_name for informational purposes.
       **/

#ifdef XENIX
        /* lock is /tmp/[basename of file_to_lock].mlk */
        sprintf(lock_name, "/tmp/%.10s.mlk", basename(file_to_lock));
#else
        /* lock is [file_to_lock].lock */
        sprintf(lock_name, "%s.lock", file_to_lock);
#endif
        return(lock_name);
}

