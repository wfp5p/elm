
static char rcsid[] = "@(#)$Id: safemalloc.c,v 1.3 1996/03/14 17:27:43 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: safemalloc.c,v $
 * Revision 1.3  1996/03/14  17:27:43  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:36  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"
#include "s_error.h"

/*
 * These routines perform dynamic memory allocation with error checking.
 * The "safe_malloc_fail_handler" vector points to a routine that is invoked
 * if memory allocation fails.  The default error handler displays a message
 * and aborts the program.
 */


void dflt_safe_malloc_fail_handler(proc, len)
const char *proc;
unsigned len;
{
	fprintf(stderr, catgets(elm_msg_cat, ErrorSet,
		    ErrorSafeMallocOutOfMemory,
		    "error - out of memory [%s failed allocating %d bytes]\n"),
		    proc, len);
	exit(1);
}


void (*safe_malloc_fail_handler) P_((const char *, unsigned))
	    = dflt_safe_malloc_fail_handler;


malloc_t safe_malloc(len)
unsigned len;
{
	malloc_t p;
	if ((p = malloc(len)) == NULL)
		(*safe_malloc_fail_handler)("safe_malloc", len);
	return p;
}


malloc_t safe_realloc(p, len)
malloc_t p;
unsigned len;
{
	if ((p = (p == NULL ? malloc(len) : realloc((malloc_t)p, len))) == NULL)
		(*safe_malloc_fail_handler)("safe_realloc", len);
	return p;
}


char *safe_strdup(s)
const char *s;
{
	char *p;
	if ((p = (char *) malloc(strlen(s)+1)) == NULL)
		(*safe_malloc_fail_handler)("safe_strdup", strlen(s)+1);
	return strcpy(p, s);
}
