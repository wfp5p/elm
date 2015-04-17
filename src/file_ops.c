#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

/*
 * These routines perform many common file operations, with the
 * addition of error checking.  They include:
 *
 * file_open	fopen() with error checking
 * file_close	fclose() with error checking
 * file_seek	fseek() with error checking
 * file_copy	copy from one (FILE *) to another (FILE *).
 * file_rename	rename() with error checking
 *
 * These routines follow ALMOST the same calling and return semantics as
 * their Unix namesakes.  (The chief difference is that some require
 * some additional filename args in the event an error message must be
 * displayed.)  Whenever an error occurs, these routines will terminate
 * with a message showing on the error line (via the error() routines).
 */

FILE *file_open(const char *fname, const char *fmode)
{
    FILE *fp;
    const char *modestr;
    int err;
    int fp_handle;


    if (fmode[0] != 'r') { save_file_stats(fname); }
    /* no reason to call save_file_stats if it's read only */

    if ( (fmode[0] == 'w') && (fmode[1] != '+') ) /* it's temp file */
    {
       unlink(fname);
       if ( ((fp_handle = open(fname, O_RDWR|O_CREAT|O_EXCL,0600)) != -1) &&
	    ((fp = fdopen(fp_handle,fmode)) != NULL) ) {
	  restore_file_stats(fname);
          return fp;
       }
    }
    else if ((fp = fopen(fname, fmode)) != NULL) {
	if (fmode[0] != 'r') {restore_file_stats(fname);}
	return fp;
    }

    err = errno;
    dprint(1, (debugfile, "file_open(%s,%s) FAILED [%s]\n",
	fname, fmode, strerror(err)));
    switch (*fmode) {
    case 'r':
	modestr = (fmode[1] == '+'
	    ? catgets(elm_msg_cat, ElmSet, ElmFileOpenModeUpdate, "update")
	    : catgets(elm_msg_cat, ElmSet, ElmFileOpenModeRead, "read"));
	break;
    case 'w':
	modestr = (fmode[1] == '+'
	    ? catgets(elm_msg_cat, ElmSet, ElmFileOpenModeCreate, "create")
	    : catgets(elm_msg_cat, ElmSet, ElmFileOpenModeWrite, "write"));
	break;
    case 'a':
	modestr = catgets(elm_msg_cat, ElmSet, ElmFileOpenModeAppend, "append");
	break;
    default:
	modestr = fmode;
	break;
    }
    error3(catgets(elm_msg_cat, ElmSet, ElmFileOpenFailed,
	    "Cannot open \"%s\" for %s.  [%s]"),
	    fname, modestr, strerror(err));

    errno = err;
    return (FILE *) NULL;
}

int file_close(FILE *fp, const char *fname)
{
    int err;

    if (ferror(fp) != 0) {
	err = errno;
	dprint(1, (debugfile, "file_close(%s) ferror() FAILED [%s]\n",
	    fname, strerror(err)));
	goto done;
    }
    if (fclose(fp) != 0) {
	err = errno;
	dprint(1, (debugfile, "file_close(%s) fclose() FAILED [%s]\n",
	    fname, strerror(err)));
	goto done;
    }
    return 0;

done:
    error2(catgets(elm_msg_cat, ElmSet, ElmFileCloseFailed,
		"Error closing \"%s\".  [%s]"), fname, strerror(errno));
    errno = err;
    return -1;
}

int file_access(const char *name, int mode)
{
    int rc, err;

    if ((rc = can_access(name, mode)) < 0) {
	err = errno;
	error2(catgets(elm_msg_cat, ElmSet, ElmFileAccessFailed,
		    "Cannot access file \"%s\".  [%s]"),
		    basename(name), strerror(err));
	errno = err;
    }
    return rc;
}

int file_seek(FILE *fp, long offset, int whence, const char *fname)
{
    int err;

    if (fseek(fp, offset, whence) == 0)
	return 0;
    err = errno;
    dprint(1, (debugfile, "file_seek(%s,%lu,%d) FAILED [%s]\n",
	fname, (unsigned long) offset, whence, strerror(err)));
    switch (whence) {
    case SEEK_END:
	error3(catgets(elm_msg_cat, ElmSet, ElmFileSeekEndFailed,
		    "Error seeking %lu bytes from end in \"%s\".  [%s]"),
		    (unsigned long)offset, fname, strerror(err));
	break;
    case SEEK_CUR:
	error3(catgets(elm_msg_cat, ElmSet, ElmFileSeekCurFailed,
		    "Error seeking ahead %lu bytes in \"%s\".  [%s]"),
		    (unsigned long)offset, fname, strerror(err));
	break;
    case SEEK_SET:
    default:
	error3(catgets(elm_msg_cat, ElmSet, ElmFileSeekSetFailed,
		    "Error seeking byte %lu in \"%s\".  [%s]"),
		    (unsigned long)offset, fname, strerror(err));
	break;
    }
    errno = err;
    return -1;
}


#ifndef BUFSIZ
# define BUFSIZ 1024
#endif

int file_copy(FILE *fpsrc, FILE *fpdest, const char *srcname, const char *destname)
{
    char buf[BUFSIZ];
    int i, err;

    while ((i = fread(buf, 1, sizeof(buf), fpsrc)) > 0) {
	if (fwrite(buf, 1, i, fpdest) != i) {
	    err = errno;
	    dprint(1, (debugfile, "file_copy(%s,%s) fwrite FAILED [%s]\n",
		    (srcname ? srcname : "<src>"),
		    (destname ? destname: "<dest>"),
		    strerror(err)));
	    if (destname) {
		error2(catgets(elm_msg_cat, ElmSet,
			ElmFileCopyErrorWritingFile,
			"Error writing \"%s\".  [%s]"),
			destname, strerror(err));
	    } else {
		error1(catgets(elm_msg_cat, ElmSet,
			ElmFileCopyErrorWritingOutput,
			"Error writing output file.  [%s]"),
			strerror(err));
	    }
	    return -1;
	}
    }
    if (i < 0) {
	err = errno;
	dprint(1, (debugfile, "file_copy(%s,%s) fread FAILED [%s]\n",
		(srcname ? srcname : "<src>"),
		(destname ? destname: "<dest>"),
		strerror(err)));
	if (srcname) {
	    error2(catgets(elm_msg_cat, ElmSet, ElmFileCopyErrorReadingFile,
		    "Error reading \"%s\".  [%s]"), srcname, strerror(err));
	} else {
	    error2(catgets(elm_msg_cat, ElmSet, ElmFileCopyErrorReadingInput,
		    "Error reading input file.  [%s]"), strerror(err));
	}
	return -1;
    }

    return 0;
}

int file_rename(const char *srcname, const char *destname)
{
    int err;

    if (rename(srcname, destname) == 0)
	return 0;
    err = errno;
    dprint(1, (debugfile, "file_rename(%s,%s) FAILED [%s]\n",
	    srcname, destname, strerror(err)));
    error2(catgets(elm_msg_cat, ElmSet, ElmFileRenameFailed,
	    "Error renaming \"%s\".  [%s]"), srcname, strerror(err));
    return -1;
}

