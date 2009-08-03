
static char rcsid[] = "@(#)$Id: save.c,v 1.3 1996/05/09 15:51:25 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
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
 * $Log: save.c,v $
 * Revision 1.3  1996/05/09  15:51:25  wfp5p
 * Alpha 10
 *
 * Revision 1.2  1996/03/14  17:29:48  wfp5p
 * Alpha 9
 *
 * Revision 1.1  1996/03/13  14:42:56  wfp5p
 * Renamed file.c to save.c
 *
 * Revision 1.6  1995/09/29  17:42:08  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.5  1995/09/11  15:19:06  wfp5p
 * Alpha 7
 *
 * Revision 1.4  1995/06/30  14:56:24  wfp5p
 * Alpha 5
 *
 * Revision 1.3  1995/06/21  15:27:08  wfp5p
 * editflush and confirmtagsave are new in the elmrc (Keith Neufeld)
 * The mlist code has a little bug fix.
 *
 * Revision 1.2  1995/04/21  21:20:17  wfp5p
 * Added Wayne Davison's tag modification
 *
 * Revision 1.1.1.1  1995/04/19  20:38:36  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

#ifdef PWDINSYS
#  include <sys/pwd.h>
#else
#  include <pwd.h>
#endif

#define S_(sel, str)    catgets(elm_msg_cat, ElmSet, (sel), (str))

static char prev_fold[SLEN];	/* saved target of most recent save/copy */

int select_folder P_((char *, int, int, int, const char *, const char *, int *));
int verify_create P_((const char *));
int expand_filename P_((char *));
char *address_to_alias P_((char *address));

extern char *nameof();

/*
 * save() - Save or copy messages to a folder.
 *
 * Prompts for a target folder, and then saves all the tagged messages
 * (or the current message if none tagged) to the folder.
 *
 * Returns TRUE if the current message is saved (for the "resolve" function).
 *
 * The "redraw" flag will be set TRUE if we munge the display.
 *
 * If "silently" is TRUE then the index display updates during the
 * save are inhibited.
 *
 * If "delete" is TRUE then messages will be marked for deletion after
 * saving (the "s)ave" command).  If FALSE, they will be preserved (the
 * "C)opy" command.
 */

int save(redraw_p, silently, delete)
int *redraw_p, silently, delete;
{
    int num_tagged;		/* or zero if saving current message	*/
    int num_saved;		/* number mssgs successfully saved	*/
    int sel_mesg_num;		/* sel for building save-by filename	*/
    int current_is_tagged;	/* is current message tagged?		*/
    int target_in_folder_dir;	/* is target file in "=" directory?	*/
    int i;
    char filename[SLEN];	/* pathname to target folder		*/
    char *canon_filename;	/* result of nameof(filename)		*/
    char *word_save, *word_Save, *word_saved;
    char prompt1[SLEN], prompt2[SLEN];
    char lbuf[LONG_STRING];
    char *s;
    FILE *fp;
    char *return_alias;		/* Alias of return address		*/

    *redraw_p = FALSE;

    num_tagged = 0;
    sel_mesg_num = curr_folder.curr_mssg-1;
    current_is_tagged = ison(curr_folder.headers[curr_folder.curr_mssg-1]->status, TAGGED);

    if (delete) {
	word_save = S_(ElmSave, "save");
	word_Save = S_(ElmCapSave, "Save");
	word_saved = S_(ElmSaved, "saved");
    } else {
	word_save = S_(ElmCopy, "copy");
	word_Save = S_(ElmCapCopy, "Copy");
	word_saved = S_(ElmCopied, "copied");
    }

    /* count up tagged messages */
    for (i = 0 ; i < curr_folder.num_mssgs ; ++i) {
	if (ison(curr_folder.headers[i]->status, TAGGED)) {
	    if (num_tagged == 0)
		sel_mesg_num = i;
	    ++num_tagged;
	}
    }

    /* confirm save of tagged message if selection is elsewhere */
    if (num_tagged > 0 && !current_is_tagged && confirm_tag_save) {
	PutLine2(LINES-3, 0, S_(ElmSavecmdPrompt0, "%s%s message"),
		    nls_Prompt, word_Save);
	CleartoEOLN();
	if (!enter_yn(S_(ElmSavecmdSaveMarked, "Save marked messages?"),
			TRUE, LINES-3, FALSE)) {
	    num_tagged = 0;
	    sel_mesg_num = curr_folder.curr_mssg-1;
	}
    }

    switch (num_tagged) {
    case 0:
	sprintf(prompt1, S_(ElmSavecmdPrompt1CurrentMessage,
		    "%s%s current message"), nls_Prompt, word_Save);
	break;
    case 1:
	sprintf(prompt1, S_(ElmSavecmdPrompt1TaggedMessage,
		    "%s%s tagged message"), nls_Prompt, word_Save);
	break;
    default:
	sprintf(prompt1, S_(ElmSavecmdPrompt1TaggedMessages,
		    "%s%s tagged messages"), nls_Prompt, word_Save);
	break;
    }
    sprintf(prompt2, S_(ElmSavecmdPrompt2, "Select Folder for %s"), word_Save);

    /* build default filename to save to */
    if (save_by_name || save_by_alias) {
	get_return(lbuf, sel_mesg_num);
	filename[0] = '=';
	if ((save_by_alias && (return_alias = address_to_alias(lbuf)) != NULL))
	    strcpy(filename + 1, return_alias);
	else
	    get_return_name(lbuf, filename+1, TRUE);
    } else {
	filename[0] = '\0';
    }

    /* obtain selection from user */
    for (;;) {
	if (select_folder(filename, sizeof(filename), WRITE_ACCESS, TRUE,
		    prompt1, prompt2, redraw_p) < 0)
	    return FALSE;
	if (verify_create(filename))
	    break;
	(void) strcpy(filename, nameof(filename));
    }

    /* if we've trashed the index screen, inhibit status update displays */
    if (*redraw_p)
	silently = TRUE;

    /* open up the target */
    canon_filename = nameof(filename);
    save_file_stats(filename);
    if ((fp = file_open(filename, "a")) == NULL)
	return FALSE;

    /* save out all the messages */
    num_saved = 0;
    for (i = 0 ; i < curr_folder.num_mssgs ; ++i) {

	if (num_tagged == 0 ? (i!=sel_mesg_num) : !(curr_folder.headers[i]->status&TAGGED))
	    continue;

	if (curr_folder.headers[i]->status & NEW) {
	    /*
	     * change status from NEW before copy and reset to what it was
	     * so that copy doesn't look new, but we can preserve new status
	     * of message in this mailfile. This is important because if
	     * user does a resync, we don't want NEW status to be lost.
	     * I.e. NEW becomes UNREAD when we "really" leave a mailfile.
	     */
	    curr_folder.headers[i]->status &= ~NEW;
	    copy_message(fp, i+1, CM_UPDATE_STATUS);
	    curr_folder.headers[i]->status |= NEW;
	} else {
	    copy_message(fp, i+1, CM_UPDATE_STATUS);
	}

	if (delete)
	    curr_folder.headers[i]->status |= DELETED;
	curr_folder.headers[i]->status &= ~TAGGED;

	if (!silently)
	    show_new_status(i);
	error3(S_(ElmSavecmdMessageSavedTo,
		    "Message %d %s to \"%s\"."),
		    i+1, word_saved, canon_filename);
#ifdef notdef /*FOO*/
		/* This pause is extremely annoying.
		 * In 2.4 Elm would pause after the first save, and then
		 * ratchet right through the rest.  I'm not quite sure
		 * why there needs to be a pause here at all.
		 */
	if (sleepmsg)
	    sleep((sleepmsg+1)/2);
#endif
	++num_saved;

    }

    /* close out the target */
    i = file_close(fp, filename);
    restore_file_stats(filename);

    /* show result - unless message already displayed is meaningful */
    if (num_saved > 1 && i == 0) {
	MCsprintf(lbuf, S_(ElmSavecmdMessagesSaved,
		    "%d messages %s to \"%s\"."),
		    num_saved, word_saved, canon_filename);
	if (*redraw_p)
	    set_error(lbuf);
	else
	    error(lbuf);
    }

    /* remember this folder */
    strfcpy(prev_fold, filename, sizeof(prev_fold));

    /* return TRUE if current message has been saved */
    return (num_tagged == 0 || current_is_tagged);
}


/*
 * select_folder() - Prompt the user to enter a folder name.
 *
 * Returns 0 if selection is made, -1 if selection is aborted.
 *
 * filename - Storage for return result.  Should be initialized with
 *		a default value, or set empty if there is no default.
 *
 * filesiz - Size of "filename" buffer.
 *
 * acc_mode - Either READ_ACCESS or WRITE_ACCESS, and the selection will
 *		be guaranteed to be accessible in this fashion.  READ_ACCESS
 *		means the file exists and is readable (or is the main folder,
 *		which can be read without existing).  WRITE_ACCESS means
 *		that if the file exists, it is writable.  (Note, however,
 *		that a file that does not exist and cannot be created will
 *		be accepted.)
 *
 * 
 * allowSameFolder  - If TRUE the same folder can be selected
 * 
 * prompt1 - Prompt message that indicates command that initiated selection.
 *
 * prompt2 - Prompt for folder.  Also displayed as page title if file
 *		browser is started.
 *
 * screen_changed_p - Set TRUE if the upper portion of the display is written.
 */

int select_folder(filename, filesiz, acc_mode, allowSameFolder, prompt1, prompt2, screen_changed_p)
char *filename;
int filesiz, acc_mode, allowSameFolder;
const char *prompt1, *prompt2;
int *screen_changed_p;
{
    int prompt_line, prompt_col, help_col, fb_mode, ch;
    char fb_dir[SLEN], fb_pat[SLEN], errbuf[SLEN], *s;

    fb_mode = (FB_MBOX | (acc_mode == WRITE_ACCESS ? FB_WRITE : FB_READ));

    for (;;) {

	/* display prompt */
	MoveCursor(LINES-3, 0);
	CleartoEOS();
	PutLine1(LINES-3, 0, prompt1);
	s = S_(ElmSelfolderHelp, "(Use \"?\" for help)");
	help_col = COLS - (strlen(s)+1);
	PutLine0(LINES-3, help_col, s);
	PutLine0(LINES-2, 0, prompt2);
	PutLine0(-1, -1, ": ");
	GetCursorPos(&prompt_line, &prompt_col);
	PutLine0(-1, -1, filename);
	show_last_error();

	/* get first keystroke of input and look for help request */
	MoveCursor(prompt_line, prompt_col);
	if ((ch = ReadCh()) == '?') {
	    display_helpfile("selfolder");
	    *screen_changed_p = TRUE;
	    continue;
	}
	MoveCursor(LINES-3, help_col);
	CleartoEOLN();

	/* pushback character and retrieve string */
	UnreadCh(ch);
	if (enter_string(filename, filesiz, prompt_line, prompt_col,
		    ESTR_REPLACE) < 0 || filename[0] == '\0') {
	    MoveCursor(LINES-3, 30);
	    CleartoEOS();
	    return -1;
	}
	clear_error();

	/* user entered a file name - expand it */
	if (!expand_filename(filename)) {
	    filename[0] = '\0';
	    continue;
	}

	/* see if this is something the file browser should handle */
	if (fbrowser_analyze_spec(filename, fb_dir, fb_pat)) {
	    *screen_changed_p = TRUE;
	    if (!fbrowser(filename, filesiz, fb_dir, fb_pat,
			fb_mode, prompt2)) {
		return -1;
	    }
	}

	/* don't accept the folder already in use if allowSameFolder is FALSE */
	if ( (!allowSameFolder) && (streq(filename, curr_folder.filename)) ){
	    set_error(S_(ElmSelfolderAlreadyReading,
			"Already reading that folder!"));
	    filename[0] = '\0';
	    continue;
	}

	/*
	 * Verify we have proper access to this file.
	 * Note we can try writing files that do not exist.
	 * We also can read the main incoming folder if it doesn't exist.
	 */
	if (can_access(filename, acc_mode) != 0) {
	    if (errno != ENOENT
			|| (acc_mode == READ_ACCESS
			    && !streq(filename, incoming_folder))) {
		sprintf(errbuf, S_(ElmSelfolderCannotOpen,
			    "Cannot open \"%s\". [%s]"),
			    filename, strerror(errno));
		set_error(errbuf);
		filename[0] = '\0';
		continue;
	    }
	}

	break;

    }

    MoveCursor(LINES-3, strlen(prompt1));
    CleartoEOS();
    return 0;
}


int verify_create(filename)
const char *filename;
{
    char buf[SLEN], *msg;
    int target_in_folder_dir
		= (strncmp(filename, folders, strlen(folders)) == 0);

    if (elm_access(filename, ACCESS_EXISTS) == 0) {
	if (confirm_files && !target_in_folder_dir)
	    msg = S_(ElmSavecmdAppendFile, "Append to existing file \"%s\"?");
	else if (confirm_append)
	    msg = S_(ElmSavecmdAppendFolder, "Append to mail folder \"%s\"?");
	else
	    return TRUE;
    } else {
	if (confirm_folders && target_in_folder_dir)
	    msg = S_(ElmSavecmdCreateFolder, "Create new mail folder \"%s\"?");
	else if (confirm_create)
	    msg = S_(ElmSavecmdCreateFile, "Create new file \"%s\"?");
	else
	    return TRUE;
    }
    sprintf(buf, msg, nameof(filename));
    return enter_yn(buf, FALSE, LINES-2, FALSE);
}



int expand_filename(filename)
char *filename;
{
	/** Expands	~/	to the current user's home directory
			~user/	to the home directory of "user"
			=,+,%	to the user's folder's directory
			!	to the user's incoming mailbox
			>	to the user's received folder
			<	to the user's sent folder
			!!	to the last folder used 
			@alias	to the default folder directory for "alias"
			shell variables (begun with $)

	    Side effect: strips off trailing blanks

	    Returns 	1	upon proper expansions
			0	upon failed expansions
	 **/

	char temp_filename[SLEN], varname[SLEN], env_value[SLEN],
	     logname[SLEN], *ptr, *address, buffer[LONG_STRING];
	register int iindex;
	int too_long = FALSE;
	struct passwd *pass, *getpwnam();
	char *getenv();

	ptr = filename;
	while (*ptr == ' ') ptr++;	/* leading spaces GONE! */
	strcpy(temp_filename, ptr);

	/** New stuff - make sure no illegal char as last **/
	/** Strip off any trailing backslashes or blanks **/

	ptr = temp_filename + strlen(temp_filename) - 1;
	while (*ptr == '\n' || *ptr == '\r'
	       || *ptr == '\\' || *ptr == ' ' || *ptr == '\t') {
	    *ptr-- = '\0';
	}

	if (temp_filename[0] == '~') {
	  if (temp_filename[1] == '\0') {
		/**
		*** Workaround hack to make ~ work as well as ~/.
		**/
		strcpy(temp_filename+1, "/");
	  }
	  if(temp_filename[1] == '/')
	    sprintf(filename, "%s%s", user_home, temp_filename+1);
	  else {
	    for(ptr = &temp_filename[1], iindex = 0; *ptr && *ptr != '/'; ptr++, iindex++)
	      logname[iindex] = *ptr;
	    logname[iindex] = '\0';
	    if((pass = getpwnam(logname)) == NULL) {
	      dprint(3,(debugfile, 
		      "Error: Can't get home directory for %s (%s)\n",
		      logname, "expand_filename"));
	      error1(catgets(elm_msg_cat, ElmSet, ElmDontKnowHomeCursor,
			"Don't know what the home directory of \"%s\" is!"),
			logname);
	      return(0);
	    }
	    sprintf(filename, "%s%s", pass->pw_dir, ptr);
	  }

	}
	else if (temp_filename[0] == '=' || temp_filename[0] == '+' || 
	 	 temp_filename[0] == '%') {
	  sprintf(filename, "%s/%s", folders, temp_filename+1);
	}
	else if  ( temp_filename[0] == '@' ) {

	  /* try to expand alias */

	  if ((address = get_alias_address(&(temp_filename[1]),FALSE,&too_long))
	       != NULL) {

            if (address[0] == '!') {
	      /* Refuse group aliases? */
              address[0] = ' ';
	    }

	    /* get filename from address */
	    get_return_name(address, buffer, TRUE);

	    sprintf(filename, "%s/%s", folders, buffer);
	  }
          else {
	    error1(catgets(elm_msg_cat, ElmSet, ElmCannotExpand,
		   "Cannot expand alias '%s'!"), &(temp_filename[1]) );
	    return(0);
	  }
	}
	else if (temp_filename[0] == '$') {	/* env variable! */
	  for(ptr = &temp_filename[1], iindex = 0; isalnum(*ptr); ptr++, iindex++)
	    varname[iindex] = *ptr;
	  varname[iindex] = '\0';

	  env_value[0] = '\0';			/* null string for strlen! */
	  if (getenv(varname) != NULL)
	    strcpy(env_value, getenv(varname));

	  if (strlen(env_value) == 0) {
	    dprint(3,(debugfile, 
		    "Error: Can't expand environment variable $%s (%s)\n",
		    varname, "expand_filename"));
	    error1(catgets(elm_msg_cat, ElmSet, ElmDontKnowValueCursor,
		  "Don't know what the value of $%s is!"), varname);
	    return(0);
	  }

	  sprintf(filename, "%s/%s", env_value, ptr);

	} else if (strcmp(temp_filename, "!") == 0) {
	  strcpy(filename, incoming_folder);
	} else if (strcmp(temp_filename, ">") == 0) {
	  strcpy(filename, recvd_mail);
	} else if (strcmp(temp_filename, "<") == 0) {
	  strcpy(filename, sent_mail);
	} else if (strcmp(temp_filename, "!!") == 0) {
	  strcpy(filename, prev_fold);
	} else
	  strcpy(filename, temp_filename);
	  
	return(1);
}

