
static char rcsid[] = "@(#)$Id: ldstate.c,v 1.3 1995/09/29 17:41:14 wfp5p Exp $";

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
 * $Log: ldstate.c,v $
 * Revision 1.3  1995/09/29  17:41:14  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/06/01  13:13:27  wfp5p
 * Readmsg was fixed to work correctly if called from within elm.  From Chip
 * Rosenthal <chip@unicom.com>
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

/*
 * Retrieve Elm folder state.
 *
 * The SY_DUMPSTATE option to "system_call()" causes Elm to dump the
 * current folder state before spawning a shell.  This allows programs
 * running as an Elm subprocess (e.g. "readmsg") to obtain information
 * on the folder.  (See the "system_call()" code for additional info
 * on the format of the state file.)
 *
 * This procedure returns -1 on the event of an error (corrupt state
 * file or malloc failed).
 *
 * A zero return does NOT necessarily mean that folder state information
 * was retrieved.  On a zero return, inspect the "folder_name" element.
 * If it was NULL then there was no state file found.  If it is non-NULL
 * then valid folder state information was found and loaded into
 * the (struct folder_state) record.
 */

#if !defined(ANSI_C) && !defined(atol)  /* avoid problems with systems that declare atol as a macro */
extern long atol();
#endif

static char *elm_fgetline(buf, buflen, fp)
char *buf;
unsigned buflen;
FILE *fp;
{
    if (fgets(buf, buflen, fp) == NULL)
	return (char *) NULL;
    buf[strlen(buf)-1] = '\0';
    return buf;
}


int load_folder_state_file(fst)
struct folder_state *fst;
{
    char buf[SLEN], *state_fname;
    int status, i;
    FILE *fp;

    /* clear out the folder status record */
    fst->folder_name = NULL;
    fst->num_mssgs = -1;
    fst->idx_list = NULL;
    fst->clen_list = NULL;
    fst->num_sel = -1;
    fst->sel_list = NULL;

    /* see if we can find a state file */
    if ((state_fname = getenv(FOLDER_STATE_ENV)) == NULL)
	return 0;
    if ((fp = fopen(state_fname, "r")) == NULL)
	return 0;

    /* initialize status to failure */
    status = -1;

    /* retrieve pathname of the folder */
    if (elm_fgetline(buf, sizeof(buf), fp) == NULL || buf[0] != 'F')
	goto done;
    if ((fst->folder_name = malloc(strlen(buf+1) + 1)) == NULL)
	goto done;
    (void) strcpy(fst->folder_name, buf+1);

    /* retrieve number of messages in the folder */
    if (elm_fgetline(buf, sizeof(buf), fp) == NULL || buf[0] != 'N')
	goto done;
    fst->num_mssgs = atoi(buf+1);

    /* allocate space to hold the indices */
    fst->idx_list = (long *) malloc(fst->num_mssgs * sizeof(long));
    if (fst->idx_list == NULL)
	goto done;
    fst->clen_list = (long *) malloc(fst->num_mssgs * sizeof(long));
    if (fst->clen_list == NULL)
	goto done;


    /* load in the indices of the messages */
    for (i = 0 ; i < fst->num_mssgs ; ++i) {
	if (elm_fgetline(buf, sizeof(buf), fp) == NULL || buf[0] != 'I')
	    goto done;
	if (sscanf(buf, "I%ld %ld", &fst->idx_list[i], &fst->clen_list[i]) != 2)
	    goto done;
    }

    /* load in the number of messages selected */
    if (elm_fgetline(buf, sizeof(buf), fp) == NULL || buf[0] != 'C')
	goto done;
    fst->num_sel = atoi(buf+1);

    /* it is possible that there are no selections */
    if (fst->num_sel > 0) {

	/* allocate space to hold the list of selected messages */
	fst->sel_list = (int *) malloc(fst->num_sel * sizeof(int));
	if (fst->sel_list == NULL)
	    goto done;

	/* load in the list of selected messages */
	for (i = 0 ; i < fst->num_sel ; ++i) {
	    if (elm_fgetline(buf, sizeof(buf), fp) == NULL || buf[0] != 'S')
		goto done;
	    fst->sel_list[i] = atoi(buf+1);
	}

    }

    /* that should be the end of the file */
    if (elm_fgetline(buf, sizeof(buf), fp) != NULL)
	goto done;

    /* success */
    status = 0;

done:
    /* FOO - should we be freeing the malloced space on failure??? */
    (void) fclose(fp);
    return status;
}

