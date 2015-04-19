

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.6 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: savecopy.c,v $
 * Revision 1.6  1999/03/24  14:04:04  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.5  1996/05/09  15:51:26  wfp5p
 * Alpha 10
 *
 * Revision 1.4  1996/03/14  17:29:50  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:26  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:29  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** Save a copy of the specified message in a folder.

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "sndhdrs.h"
#include "s_elm.h"
#ifdef I_TIME
# include <time.h>
#endif
#ifdef I_SYSTIME
# include <sys/time.h>
#endif


static const char *cf_english P_((const char *));

extern char *address_to_alias(const char *address);


/*
 * save_copy() - Append a copy of the message contained in "filename" to
 * the file specified by "copy_file".  This routine simply gets all of
 * the filenames right, and then invokes "save_mssg()" to do
 * the dirty work.
 */
int save_copy(const char *fname_dest, const char *fname_mssg,
	      const SEND_HEADER *shdr, int form)
{
	char  buffer[SLEN],	/* read buffer 		       */
	      savename[SLEN],	/* name of file saving into    */
	      msg_buffer[SLEN];
	char *return_alias;
	int is_ordinary_file;
	int  err;

	/* presume fname_dest is okay as is for now */
	strcpy(savename, fname_dest);

	/* if save_by_name or save_by_alias wanted */
	if((strcmp(fname_dest, "=") == 0)  || (strcmp(fname_dest, "=?") == 0)) {
	    if ((save_by_alias &&
		   (return_alias = address_to_alias(shdr->expanded_to)) != NULL))
		strcpy(buffer, return_alias);
	    else
	        if (save_by_name)
	          get_return_name(shdr->expanded_to, buffer, TRUE);
	        else
	      	  get_return_name(shdr->to, buffer, TRUE);

	  if (strlen(buffer) == 0) {

	    /* can't get file name from 'to' -- use sent_mail instead */
	    dprint(3, (debugfile,
		"Warning: get_return_name couldn't break down %s\n", shdr->to));
	    error1(catgets(elm_msg_cat, ElmSet, ElmCannotDetermineToName,
"Cannot determine `to' name to save by! Saving to \"sent\" folder %s instead."),
	      sent_mail);
	    strcpy(savename, "<");
	    if (sleepmsg > 0)
		sleep(sleepmsg);
	  } else
	    sprintf(savename, "=%s", buffer);		/* good! */
	}

	expand_filename(savename);

	/*
	 *  If saving conditionally by logname but folder doesn't
	 *  exist save to sent folder instead.
	 */
	if((strcmp(fname_dest, "=?") == 0)
	      && (access(savename, WRITE_ACCESS) != 0)) {
	  dprint(5, (debugfile,
	    "Conditional save by name: file %s doesn't exist - using \"<\".\n",
	    savename));
	  strcpy(savename, "<");
	  expand_filename(savename);
	}

	/*
	 *  Allow options
	 *  confirm_files, confirm_folders,
	 *  confirm_append and confirm_create
	 *  to control where the actual copy
	 *  should be saved.
	 */
	is_ordinary_file = strncmp (savename, folders, strlen(folders));

        if (elm_access(savename, ACCESS_EXISTS)== 0) {	/* already there!! */
	    if (confirm_append || (confirm_files && is_ordinary_file)) {
		/*
		 *  OK in batch mode it may be impossible
		 *  to ask the user to confirm. So we have
		 *  to use sent_mail anyway.
		 */
		if (!OPMODE_IS_INTERACTIVE(opmode)) {
		    strcpy(savename, sent_mail);
		}
		else {
		    if (is_ordinary_file)
		      sprintf(msg_buffer, catgets(elm_msg_cat, ElmSet,
			  ElmConfirmFilesAppend,
			  "Append to an existing file `%s'?"), savename);
		    else
		      sprintf(msg_buffer, catgets(elm_msg_cat, ElmSet,
			  ElmConfirmFolderAppend,
			  "Append to mail folder `%s'?"), savename);

		    /* FOO - does this really need to be "clear-and-center" ? */
		    if (!enter_yn(msg_buffer, FALSE, LINES-2, TRUE)) {
			strcpy(savename, sent_mail);
			PutLine1 (LINES-2, 0, catgets(elm_msg_cat, ElmSet,
				  ElmSavingToInstead,
				  "Alright - saving to `%s' instead"),
				  savename);
			FlushOutput();
			if (sleepmsg > 0)
				sleep(sleepmsg);
			ClearLine (LINES-2);
		    }
		}
	    }
	}
        else {
            if (confirm_create || (confirm_folders && !is_ordinary_file)) {
		/*
		 *  OK in batch mode it may be impossible
		 *  to ask the user to confirm. So we have
		 *  to use sent_mail anyway.
		 */
		if (!OPMODE_IS_INTERACTIVE(opmode)) {
		    strcpy(savename, sent_mail);
		}
		else {
		    if (is_ordinary_file)
		      sprintf(msg_buffer, catgets(elm_msg_cat, ElmSet,
			  ElmConfirmFilesCreate,
			  "Create a new file `%s'?"), savename);
		    else
		      sprintf(msg_buffer, catgets(elm_msg_cat, ElmSet,
			  ElmConfirmFolderCreate,
			  "Create a new mail folder `%s'?"), savename);

		    /* FOO - does this really need to be "clear-and-center" ? */
		    if (!enter_yn(msg_buffer, FALSE, LINES-2, TRUE)) {
			strcpy(savename, sent_mail);
			PutLine1 (LINES-2, 0, catgets(elm_msg_cat, ElmSet,
				  ElmSavingToInstead,
				  "Alright - saving to `%s' instead"),
				  savename);
			FlushOutput();
			if (sleepmsg > 0)
				sleep(sleepmsg);
			ClearLine (LINES-2);
		    }
		}
	    }
	}

	if ((err = can_open(savename, "a"))) {
	  dprint(2, (debugfile,
	  "Error: attempt to autosave to a file that can't be appended to!\n"));
	  dprint(2, (debugfile, "\tfilename = \"%s\"\n", savename));
	  dprint(2, (debugfile, "** %s **\n", strerror(err)));

	  /* Lets try sent_mail before giving up */
	  if(strcmp(sent_mail, savename) == 0) {
	    /* we are ALREADY using sent_mail! */
	    error1(catgets(elm_msg_cat, ElmSet, ElmCannotSaveTo,
			"Cannot save to %s!"), savename);
	    return(FALSE);
	  }

	  if ((err = can_open(sent_mail, "a"))) {
	    dprint(2, (debugfile,
	  "Error: attempt to autosave to a file that can't be appended to!\n"));
	    dprint(2, (debugfile, "\tfilename = \"%s\"\n", sent_mail));
	    dprint(2, (debugfile, "** %s **\n", strerror(err)));
	    error2(catgets(elm_msg_cat, ElmSet, ElmCannotSaveToNorSent,
		    "Cannot save to %s nor to \"sent\" folder %s!"),
		    savename, sent_mail);
	    return(FALSE);
	  }
	  error2(catgets(elm_msg_cat, ElmSet, ElmCannotSaveToSavingInstead,
		"Cannot save to %s! Saving to \"sent\" folder %s instead."),
	        savename, sent_mail);
	  if (sleepmsg > 0)
		sleep(sleepmsg);
	  strcpy(savename, sent_mail);
	}

	return save_mssg(savename, fname_mssg, shdr, form);
}


/*
 * save_mssg() - Append a mail message to fname_dest.
 */
int save_mssg(const char *fname_dest, const char *fname_mssg,
	      const SEND_HEADER *shdr, int form)
{
    int rc, err, len;
    char *fmode;
    long clen_pos, clen_begdata, clen_enddata;
    char buf[SLEN];
    time_t now;
    FILE *fp_copy, *fp_mssg;

    rc = FALSE;
    fp_mssg = fp_copy = NULL;

    if ((fp_mssg = file_open(fname_mssg, "r")) == NULL)
	goto done;
    fmode = (access(fname_dest, ACCESS_EXISTS) == 0 ? "r+" : "w");
    if ((fp_copy = file_open(fname_dest, fmode)) == NULL)
	goto done;
    if (file_seek(fp_copy, 0L, SEEK_END, fname_dest) < 0)
	goto done;

    putc('\n', fp_copy);
    time(&now);
    fprintf(fp_copy, "From %s %s", user_name, ctime(&now));

    /* dump the header to the end of the copy file */
    (void) sndhdr_output(fp_copy, shdr, (form == YES), TRUE);

    /* add Content-Length: for UAs that care about it */
    fputs("Content-Length: ", fp_copy);
    clen_pos = ftell(fp_copy);
    fputs("          \n", fp_copy); /* length fixup to go here */

    /* terminate the header */
    putc('\n', fp_copy);

    /* write out the message body */
    clen_begdata = ftell(fp_copy);
    while ((len = mail_gets(buf, sizeof(buf), fp_mssg)) > 0) {

	if (fwrite(buf, 1, len, fp_copy) != len) {
	    ShutdownTerm();
	    error1(catgets(elm_msg_cat, ElmSet, ElmWriteFailedSaveMssg,
			"Write failed in save_mssg! [%s]"), strerror(errno));
	    leave(LEAVE_EMERGENCY);
	}

	if (buf[0] == '[') {
	    if (strbegConst(buf, MSSG_DONT_SAVE))
		break;
	    if (strbegConst(buf, MSSG_DONT_SAVE2))
		break;
	}

    }
    clen_enddata = ftell(fp_copy);

    /* ensure message ends with a newline */
    putc('\n', fp_copy);

    /* go fixup the content length header */
    if (file_seek(fp_copy, clen_pos, SEEK_SET, fname_dest) < 0)
	goto done;
    fprintf(fp_copy, "%d", clen_enddata - clen_begdata);

    /* copy complete */
    if (file_close(fp_copy, fname_dest) < 0)
	goto done;
    fp_copy = NULL;
    if (file_close(fp_mssg, fname_mssg) < 0)
	goto done;
    fp_mssg = NULL;

    rc = TRUE;

done:
    err = errno;
    if (fp_mssg != NULL)
	(void) fclose(fp_mssg);
    if (fp_copy != NULL)
	(void) fclose(fp_copy);
    errno = err;
    return rc;
}


/* FOO - should this routine use the file browser? */
int name_copy_file(char *fn)
{
    /*
     * Prompt user for name of file for saving copy of outbound msg to.
     * Update "fn" with user's response.
     * Return TRUE if we need a redraw (i.e. help displayed).
     */

    int redraw, i;
    char buffer[SLEN], origbuffer[SLEN], *ncf_prompt;

    redraw = FALSE;
    ncf_prompt = catgets(elm_msg_cat, ElmSet, ElmSaveCopyInPrompt,
		"Save copy in (use '?' for help/to list folders): ");

    /* convert the save cookie to readable explanation */
    (void) strcpy(origbuffer, cf_english(fn));

    for (;;) {

	/* load in the default value */
	strcpy(buffer, origbuffer);

	/* prompt for save file name */
	MoveCursor(LINES-2, 0);
	CleartoEOS();
	PutLine0(LINES-2, 0, ncf_prompt);
	if (enter_string(buffer, sizeof(buffer), -1, -1, ESTR_REPLACE) < 0) {
	    /* aborted - restore value */
	    (void) strcpy(buffer, origbuffer);
	    break;
	}

	/* break out if if we got an answer */
	if (strcmp(buffer, "?") != 0)
	    break;

	/* user asked for help */
	redraw = TRUE;
	Raw(OFF | NO_TITE);
	ClearScreen();
	printf(catgets(elm_msg_cat, ElmSet, ElmListFoldersHelp,
"Enter: <nothing> to not save a copy of the message,\n\
\r       '<'       to save in your \"sent\" folder (%s),\n\
\r       '='       to save by name (the folder name depends on whom the\n\
\r                     message is to, in the end),\n\
\r       '=?'      to save by name if the folder already exists,\n\
\r                     and if not, to your \"sent\" folder,\n\
\r       or a filename (a leading '=' denotes your folder directory).\n\
\r\n\
\r"), sent_mail);
	printf(catgets(elm_msg_cat, ElmSet, ElmContentsOfYourFolderDir,
		    "\n\rContents of your folder directory:\n\r\n\r"));
	sprintf(buffer, "cd %s;ls -C", folders);
	(void) system_call(buffer, 0); 
	for (i = 0 ; i < 4 ; ++i)
	    printf("\n\r");
	Raw(ON | NO_TITE);

    }

    /* snarf any entry made by the user */
    if (!streq(origbuffer, buffer))
	strcpy(fn, buffer);

    /* display English expansion of new user input a while */
    PutLine0(LINES-2, strlen(ncf_prompt), cf_english(fn));
    MoveCursor(LINES-1, 0);
    FlushOutput();
    if (sleepmsg > 0)
	sleep((sleepmsg + 1) / 2);
    MoveCursor(LINES-2, 0);
    CleartoEOS();

    return redraw;
}

static const char *cf_english(const char *fn)
{
    /** Return "English" expansion for special copy file name abbreviations
	or just the file name  **/

    if(!*fn)
      return(catgets(elm_msg_cat, ElmSet, ElmNoSave, "<no save>"));
    else if(!fn[1]) {
      if(*fn == '=')
	return(catgets(elm_msg_cat, ElmSet, ElmUncondSaveByName, "<unconditionally save by name>"));
      else if(*fn == '<')
	return(catgets(elm_msg_cat, ElmSet, ElmSentFolder, "<\"sent\" folder>"));
    } else if ((fn[0] == '=') && (fn[1] == '?'))
      return(catgets(elm_msg_cat, ElmSet, ElmCondSaveByName, "<conditionally save by name>"));

    return fn;
}

