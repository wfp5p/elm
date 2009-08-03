
static char rcsid[] = "@(#)$Id: mailfile.c,v 1.4 1998/02/11 22:02:14 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: mailfile.c,v $
 * Revision 1.4  1998/02/11  22:02:14  wfp5p
 * Beta 2
 *
 * Revision 1.3  1995/09/29  17:41:16  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:18:55  wfp5p
 * Alpha 7
 *
 * Revision 1.1  1995/06/22  14:48:36  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

  A few stdio-type functions reimplemented for performance.  This is pretty
  well tailored to read_headers() in newmbox.c, but it is used a few other
  places as well.

  mailFile_attach
	Takes a stdio file descriptor and attaches it to a mailFile.
  
  mailFile_copy
	Indicates that a copy is to be made of the input stream into the
	specified stdio file descriptor.  This uses fwrite on large blocks,
	so it is much more efficient than using fputs on each input line.
	This is only usable if you want an exact copy, all the way to end
	of file.

  mailFile_gets
	Reads a \n terminated string from mailFile, returning the length.
	Unlike fgets, besides returning the length, this routine returns
	a pointer into its internal buffer (when possible), avoiding a copy.

  mailFile_tell
	Returns the current file position.

  mailFile_seek
	Does an absolute seek to the specified offset.  If 'copy' is
	non-NULL, fseeks there as well.  If offset is in the current
	mailFile buffer, only internal pointers are updated.

  mailFile_detach
	flushes and frees all buffers, returns control of the file
	descriptors to stdio.  Stdio file position should be treated as
	undefined after this call.

  internal:

  refill
	Refills the mailFile buffer, and writes the buffer to 'copy',
	if non-NULL.

**/

#include "elm_defs.h"
#include "mailfile.h"

static int refill();

void
mailFile_attach(mailfile, filedes)
struct mailFile *mailfile;
FILE *filedes;
{
	mailfile->filedes = filedes;
	mailfile->buffer = malloc(MAILFILE_BSIZE+1);
	mailfile->offset = NULL;
	mailfile->remain = 0;
	mailfile->savechar = -1;
	mailfile->charsaved = 0;
	mailfile->copy = NULL;
	mailfile->error = NULL;
}

void
mailFile_copy(mailfile, filedes, error)
struct mailFile *mailfile;
FILE *filedes;
void (*error)();
{
	mailfile->copy = filedes;
	mailfile->error = error;
}

int
mailFile_gets(buffer, mailfile)
char **buffer;
struct mailFile *mailfile;
{
	register char *c;
	register int n, loops, m;
	char *p;
	int size;
/*	static char vlongstring[VERY_LONG_STRING];*/
   	static char vlongstring[MAILFILE_BSIZE];
   

	if (mailfile->charsaved) {
	  *(mailfile->offset) = mailfile->savechar;
	  mailfile->charsaved = 0;
	}
	if (mailfile->remain <= 0)
	  if (refill(mailfile) == 0)
	    return 0;

	/* search for '\n', but allow '\0' (so can't use index) */
	p = mailfile->offset;
	n = mailfile->remain-1;
	loops = n / 4;
	if (loops > 0) {
	  do {
	    if (p[0] == '\n') break;
	    if (p[1] == '\n') {p++; break;}
	    if (p[2] == '\n') {p+=2; break;}
	    if (p[3] == '\n') {p+=3; break;}
	    p += 4;
	  } while (--loops != 0);
	}
	if (*p != '\n' && (m=n%4)) {
	  do {
	    if (*p == '\n') break;
	    p++;
	  } while (--m != 0);
	}

	if (*p == '\n') {
	  p++;
	  mailfile->savechar = *p;
	  mailfile->charsaved = 1;
	  *p = '\0';
	  n = p - mailfile->offset;
	  (*buffer) = mailfile->offset;
	  mailfile->offset += n;
	  mailfile->remain -= n;
	  return n;
	}

	/* else string spans buffer; must copy */

   	size = VERY_LONG_STRING - mailfile->remain;
	(*buffer) = vlongstring;
	memcpy(vlongstring, mailfile->offset, mailfile->remain);
	size = VERY_LONG_STRING - mailfile->remain;
	c = vlongstring + mailfile->remain;
	mailfile->offset += mailfile->remain;
	mailfile->remain = 0;

	size--; /* allow room for zero terminator on end, just in case */

	while (size > 0) {
	  if (mailfile->remain <= 0) {	/* empty buffer */
	    if (refill(mailfile) == 0) {	/* EOF or error */
	      if (c > (*buffer) && *c != '\n')
		*++c = '\n';
	      c++;
	      break;
	    }
	  }
	  n = mailfile->remain;
	  if (size < n)
	    n = size;
	  /* search for '\n' while copying to c; no more than n bytes */
	  p = memccpy(c, mailfile->offset, '\n', n);
	  if (p != NULL)
	    n = p - c;
	  size -= n;
	  c += n;
	  mailfile->remain -= n;
	  mailfile->offset += n;
	  if (p != 0)		/* '\n' found */
	    break;
	}
	*c = '\0';
	return (c - (*buffer));
}

static int
refill(mailfile)
struct mailFile *mailfile;
{
	mailfile->remain = 
	  fread(mailfile->buffer, 1, MAILFILE_BSIZE, mailfile->filedes);
	if (mailfile->copy && mailfile->remain > 0) {
	  if (fwrite(mailfile->buffer, 1, mailfile->remain, mailfile->copy)
	      != mailfile->remain) {
	    mailfile->error();
	  }
	}
	mailfile->offset = mailfile->buffer;
	mailfile->buffer[mailfile->remain] = '\0';
	return mailfile->remain;
}

long
mailFile_tell(mailfile)
struct mailFile *mailfile;
{
	return ftell(mailfile->filedes) - mailfile->remain;
}

int
mailFile_seek(mailfile, offset)
struct mailFile *mailfile;
long offset;
{
	int ret;
	long top, bot, curpos;

	if (mailfile->charsaved) {
	  *(mailfile->offset) = mailfile->savechar;
	  mailfile->charsaved = 0;
	}

	curpos = ftell(mailfile->filedes) - mailfile->remain;
	top = curpos - (mailfile->offset - mailfile->buffer);
	bot = curpos + mailfile->remain - 1;
	if (offset >= top && offset <= bot) {
	  /* just reposition */
	  mailfile->offset += (offset - curpos);
	  mailfile->remain -= (offset - curpos);
	  return 0;
	}
	/* else punt */
	ret = fseek(mailfile->filedes, offset, 0);
	if (mailfile->copy != NULL && ret == 0)
	  ret = fseek(mailfile->copy, offset, 0);
	refill(mailfile);
	return ret;
}

void
mailFile_detach(mailfile)
struct mailFile *mailfile;
{
	if (mailfile->filedes);
	  fflush(mailfile->filedes);
	if (mailfile->copy)
	  fflush(mailfile->copy);
	if (mailfile->buffer)
	  free(mailfile->buffer);
	mailfile->filedes = NULL;
	mailfile->copy = NULL;
	mailfile->buffer = NULL;
	mailfile->offset = NULL;
	mailfile->remain = 0;
	mailfile->savechar = -1;
	mailfile->charsaved = 0;
}
