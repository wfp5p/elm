#ifndef _parseaddrs_h_
#define _parseaddrs_h_

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $
 *
 * This file and all associated files and documentation:
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: parseaddrs.h,v $
 * Revision 1.4  1995/09/29  17:40:53  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/09/11  15:18:48  wfp5p
 * Alpha 7
 *
 * Revision 1.2  1995/06/14  19:58:09  wfp5p
 * Changes for alpha 3
 *
 *
 ******************************************************************************/

/* various defines for "mailing list" feature */

#define TO_ME_TOKEN	"[to-me]"
#define TO_ME_DEFAULT	"------------"
#define TO_MANY_TOKEN	"[to-many]"
#define TO_MANY_DEFAULT	"============"
#define CC_ME_TOKEN	"[cc-me]"
#define CC_ME_DEFAULT	"============"

struct addrs {
    char **str;
    int len;
    int max;
};

extern void parseaddrs P_((char *, struct addrs *,int));
extern void freeaddrs P_((struct addrs *));
extern void mlist_push P_((struct addrs *arr, char *str));
extern void mlist_init P_((void));
extern int  addrmatch P_((struct addrs *, struct addrs *)); 

#endif /*_parseaddrs_h_*/
