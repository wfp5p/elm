
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
 * $Log: gcos_name.c,v $
 * Revision 1.2  1995/09/29  17:41:10  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"

char *
gcos_name(gcos_field, logname)
char *gcos_field;
const char *logname;
{
    /** Return the full name found in a passwd file gcos field **/

#ifdef BERKNAMES

    static char fullname[SLEN];
    const char *gcoscp, *lncp;
    char *fncp, *end;


    /* full name is all chars up to first ',' (or whole gcos, if no ',') */
    /* replace any & with logname in upper case */

    for(fncp = fullname, gcoscp= gcos_field, end = fullname + SLEN - 1;
        (*gcoscp != ',' && *gcoscp != '\0' && fncp != end);
	gcoscp++) {

	if(*gcoscp == '&') {
	    for(lncp = logname; *lncp; fncp++, lncp++)
		*fncp = toupper(*lncp);
	} else {
	    *fncp++ = *gcoscp;
	}
    }
    
    *fncp = '\0';
    return(fullname);
#else
#ifdef USGNAMES

    char *firstcp, *lastcp;

    /* The last character of the full name is the one preceding the first
     * '('. If there is no '(', then the full name ends at the end of the
     * gcos field.
     */
    if(lastcp = strchr(gcos_field, '('))
	*lastcp = '\0';

    /* The first character of the full name is the one following the 
     * last '-' before that ending character. NOTE: that's why we
     * establish the ending character first!
     * If there is no '-' before the ending character, then the fullname
     * begins at the beginning of the gcos field.
     */
    if(firstcp = strrchr(gcos_field, '-'))
	firstcp++;
    else
	firstcp = gcos_field;

    return(firstcp);

#else
    /* use full gcos field */
    return(gcos_field);
#endif
#endif
}
