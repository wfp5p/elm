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
 * $Log: mailfile.h,v $
 * Revision 1.3  1995/09/29  17:40:50  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:18:48  wfp5p
 * Alpha 7
 *
 * Revision 1.1  1995/06/22  14:48:21  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include <stdio.h>

# define MAILFILE_BSIZE 8192

struct mailFile {
	FILE *filedes;	/* stdio file descriptor input */
	FILE *copy;	/* output copy of input, if any */
	char *buffer;	/* current buffer */
	char *offset;	/* current position within buffer (relative) */
	size_t remain;	/* number of bytes remaining in buffer */
	int savechar;	/* character in the buffer null-ed to term. string */
        int charsaved;  /* "savechar" is saved */
	void (*error)();/* function to call on copy write error */
};

extern void mailFile_attach P_((struct mailFile *mailfile, FILE *filedes));
extern void mailFile_copy P_((struct mailFile *mailfile, FILE *filedes, void(*error)()));
extern int  mailFile_gets P_((char **buffer, struct mailFile *mailfile));
extern long mailFile_tell P_((struct mailFile *mailfile));
extern int  mailFile_seek P_((struct mailFile *mailfile, long offset));
extern void mailFile_detach P_((struct mailFile *mailfile));
