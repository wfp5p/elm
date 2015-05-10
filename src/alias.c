

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
 * $Log: alias.c,v $
 * Revision 1.6  1996/08/08  19:49:22  wfp5p
 * Alpha 11
 *
 * Revision 1.5  1996/05/09  15:51:14  wfp5p
 * Alpha 10
 *
 * Revision 1.4  1996/03/14  17:27:50  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:41:57  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/05/24  15:34:37  wfp5p
 * Change to deal with parenthesized comments in when eliminating members from
 * an alias. (from Keith Neufeld <neufeld@pvi.org>)
 *
 * Allow a shell escape from the alias screen (just like from
 * the index screen).  It does not put the shell escape onto the alias
 * screen menu. (from Keith Neufeld <neufeld@pvi.org>)
 *
 * Allow the use of "T" from the builtin pager. (from Keith Neufeld
 * <neufeld@pvi.org>)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This file contains alias stuff

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "port_stat.h"
#include "s_elm.h"
#include "s_aliases.h"
#include "ndbz.h"

#define	ECHOIT	1 	/* echo on for prompting */

/*
 * A simple macro to make it easier to remember how to do a simple
 * resync and not screw up whether or not to prompt on deletions.
 */

#define resync_aliases(newaliases)	delete_aliases(newaliases,TRUE)

static void get_aliases(int are_in_aliases);
static void get_realnames(char *aliasname, char *firstname, char *lastname,
			  char *comment, char *buffer);
void install_aliases(void);
static int get_one_alias(DBZ *db, int current);
static int open_system_aliases(void);
static int superceed_system(int this_alias, char *buffer);
static int get_aliasname(char *aliasname, char *buffer, int *duplicate);
static int open_user_aliases(void);
static int ask_accept(char *aliasname, char *firstname, char *lastname,
		      char *comment, char *address, char *buffer,
		      int replace, int replacement);
static int parse_aliases(char *buffer, char *remainder);
static int alias_help(void);

static int  is_system=0;		/* system file updating?     */

static int num_duplicates;
static DBZ *system_hash = NULL, *user_hash = NULL;

int get_is_system(void)
{
	return is_system;
}

/* return 1 on failure, 0 on success */
/* FIXME nothing actually pay attention to these return values */
int open_alias_files(int are_in_aliases)
{
	if(open_system_aliases() || open_user_aliases()) {
	    dprint(5, (debugfile,
		      "Reading alias data files...\n"));
	    get_aliases(are_in_aliases);
	    return 0;
	}
	return 1;
}

static int open_system_aliases(void)
{
/*
 *	Open the system alias file, if present,
 *	and if it has changed since last we read it.
 *
 *	Return 0 if hash file wasn't opened, otherwise 1
 */

	struct stat hst;
	static time_t system_ctime = 0, system_mtime = 0;

	/* If file hasn't changed, don't bother re-opening. */

	if (stat(system_data_file, &hst) == 0) {	/* File exists */
	    if (hst.st_ctime == system_ctime &&
	        hst.st_mtime == system_mtime) {		/* No changes */
	        return(0);
	    }

	   /*
	    * Re-open system hash table.  If we can't, just return.
	    */

	    if (system_hash != NULL)
	        dbz_close(system_hash);

	    if ((system_hash = dbz_open(system_data_file, O_RDONLY, 0)) == NULL)
	        return(0);

	    /* Remember hash file times. */

	    system_ctime = hst.st_ctime;
	    system_mtime = hst.st_mtime;

            return(1);
	}
	else {					/* File does not exist */
	    if (system_ctime == 0 && system_mtime == 0) {
	        return(0);			/* File never existed */
	    }
	    else {			/* Once existed, better re-read */

	       /*
	        * Since we no longer exist, we pretend we never existed.
	        */

	        system_ctime = 0;
	        system_mtime = 0;

	        return(1);
	    }
	}

}

static int open_user_aliases(void)
{
/*
 *	Open the user alias file, if present,
 *	and if it has changed since last we read it.
 *
 *	Return 0 if hash file wasn't opened, otherwise 1
 */

	struct stat hst;
	char fname[SLEN];
	static time_t user_ctime = 0, user_mtime = 0;

	/* If hash file hasn't changed, don't bother re-reading. */

	sprintf(fname, "%s/%s", user_home, ALIAS_DATA);

	if (stat(fname, &hst) == 0) {			/* File exists */
	    if (hst.st_ctime == user_ctime &&
	        hst.st_mtime == user_mtime) {		/* No changes */
	        return(0);
	    }

	   /*
	    * Open user hash table.  If we can't, just return.
	    */

	    if (user_hash != NULL)
	        dbz_close(user_hash);

	    if ((user_hash = dbz_open(fname, O_RDONLY, 0)) == NULL)
	        return(0);

	    /* Remember hash file times. */

	    user_ctime = hst.st_ctime;
	    user_mtime = hst.st_mtime;

            return(1);
	}
	else {					/* File does not exist */
	    if (user_ctime == 0 && user_mtime == 0) {
	        return(0);			/* File never existed */
	    }
	    else {			/* Once existed, better re-read */

	       /*
	        * Since we no longer exist, we pretend we never existed.
	        */

	        user_ctime = 0;
	        user_mtime = 0;

	        return(1);
	    }
	}

}

static int add_alias(int replace, int to_replace)
{
/*
 *	Add an alias to the user alias text file.  If there
 *	are aliases tagged, the user is asked if he wants to
 *	create a group alias from the tagged files.
 *
 *	Return zero if alias not added in actuality.
 *
 *	If replace == FALSE, then we will ask for the new
 *	aliasname.
 *
 *	If replace == TRUE, then we are replacing the alias
 *	denoted by to_replace.
 *
 *	Note that even if replace == FALSE, if the user types
 *	in the name of a current alias then we can still do
 *	a replacement.
 */

	int i, ans;
	int tagged = 0;
	int leftoff = 0;
	char aliasname[SLEN], firstname[SLEN], lastname[SLEN];
	char address1[LONG_STRING], buffer[SLEN];
	char comment[LONG_STRING];
	char *ch_ptr;

/*
 *	See if there are any tagged aliases.
 */
	for (i=0; i < num_aliases; i++) {
	    if (ison(aliases[i]->status, TAGGED)) {
		if (tagged == 0) leftoff = i;
	        tagged++;
	    }
	}

	if (tagged == 1) {
	 /*
	  * There is only on alias tagged.  Ask the question
	  * but the default response is NO.
	  */
	    PutLine(LINES-2,0, catgets(elm_msg_cat,
	            AliasesSet, AliasesOneTagged,
	            "There is 1 alias tagged..."));
	    CleartoEOLN();
	    ans = enter_yn(catgets(elm_msg_cat, AliasesSet, AliasesCreateGroup,
			"Create group alias?"), FALSE, LINES-3, FALSE);
	}
	else if (tagged > 1) {
	 /*
	  * If multiple tagged aliases then we assume the user
	  * wants to create a group alias.  The default response
	  * is YES.
	  */
	    PutLine(LINES-2,0, catgets(elm_msg_cat,
	            AliasesSet, AliasesManyTagged,
	            "There are %d aliases tagged..."), tagged);
	    CleartoEOLN();
	    ans = enter_yn(catgets(elm_msg_cat, AliasesSet, AliasesCreateGroup,
			"Create group alias?"), TRUE, LINES-3, FALSE);
	} else {
	    /*
	     * Nothing tagged ... thus nothing to make a group of.
	     */
	    ans = FALSE;
	}

/*
 *	If requested above, create the group alias address.
 */
	if (ans) {
	    strcpy(address1, aliases[leftoff]->alias);
	    for (i=leftoff+1; i < num_aliases; i++) {
	        if (ison(aliases[i]->status, TAGGED)) {
	            strcat(address1, ",");
	            strcat(address1, aliases[i]->alias);
	        }
	    }
	}
	else {
	    tagged = 0;
	}

/*
 *	Only ask for an aliasname if we are NOT replacing the
 *	current alias.
 */
	if (replace) {
	    strcpy(aliasname, aliases[to_replace]->alias);
	/*
	 *  First, see if what we are replacing is a SYSTEM
	 *  alias.  If so, we need to ask a question.
	 */
	    if(aliases[to_replace]->type & SYSTEM) {
	        dprint(3, (debugfile,
	            "Aliasname [%s] is SYSTEM in add_alias\n", aliasname));
	    /*
	     *  If they don't want to superceed the SYSTEM alias then
	     *  just return.
	     */
	        if( ! superceed_system(to_replace, buffer)) {
	            ClearLine(LINES-2);
	            return(0);
	        }
	    }
	}
	else {
	    strcpy(buffer, catgets(elm_msg_cat,
	            AliasesSet, AliasesEnterAliasName, "Enter alias name: "));
	    PutLine(LINES-2,0, buffer);
	    CleartoEOLN();
	    *aliasname = '\0';
	    if ((replace = get_aliasname(aliasname, buffer, &to_replace)) < 0) {
	        dprint(3, (debugfile,
	            "Aliasname [%s] was rejected in add_alias\n", aliasname));
	        ClearLine(LINES-2);
	        return(0);
	    }
	}

/*
 *	If we are replacing an existing alias, we will assume that
 *	they might want to be just editing most of what is already
 *	there.  So we copy some defaults from the existing alias.
 */
	if (replace) {
	    strcpy(lastname, aliases[to_replace]->last_name);
	    strcpy(firstname, aliases[to_replace]->name);
	    ch_ptr = strstr(firstname, lastname);
	    *(ch_ptr-1) = '\0';
	    strcpy(comment, aliases[to_replace]->comment);
	}
	else {
	    *lastname = '\0';
	    *firstname = '\0';
	    *comment = '\0';
	}
	get_realnames(aliasname, firstname, lastname, comment, buffer);

/*
 *	Since there are no tagged aliases, we must ask for an
 *	address.  If we are replacing, a default address is
 *	presented.
 */
	if (tagged == 0) {
	    sprintf(buffer, catgets(elm_msg_cat,
	            AliasesSet, AliasesEnterAddress,
	            "Enter address for %s: "), aliasname);
	    PutLine(LINES-2, 0, buffer);
	    if (replace)
	        strcpy(address1, aliases[to_replace]->address);
	    else
	        *address1 = '\0';

	    if (enter_string(address1, sizeof(address1), -1, -1,
			ESTR_REPLACE) < 0 || address1[0] == '\0') {
		Raw(ON);
	        show_error(catgets(elm_msg_cat, AliasesSet, AliasesNoAddressSpec,
	                "No address specified!"));
	        return(0);
	    }
	    Raw(ON);

	    despace_address(address1);

	    clear_error();			/* Just in case */
	}

	if(ask_accept(aliasname, firstname, lastname, comment, address1,
	        buffer, replace, to_replace)) {
	 /*
	  * We can only clear the tags after we know that the
	  * alias was added.  This allows the user to back out
	  * and rethink without losing the tags.
	  */
	    if (tagged > 0) {
	        for (i=leftoff; i < num_aliases; i++) {
	            if (ison(aliases[i]->status, TAGGED)) {
	                clearit(aliases[i]->status, TAGGED);
	                show_msg_tag(i);
	            }
	        }
	    }
	    return(1);
	}
	else {
	    return(0);
	}

}

static int add_current_alias(void)
{
/*
 *	Alias the current message to the specified name and
 *	add it to the alias text file, for processing as
 *	the user leaves the program.
 *
 *	Returns non-zero iff alias actually added to file.
 */

	char aliasname[SLEN], firstname[SLEN], lastname[SLEN];
	char comment[SLEN], address1[LONG_STRING], buffer[SLEN];
	char comment_buff[LONG_STRING];
	char *chspace, *bufptr;
	struct header_rec *current_header;

	static char bad_punc[] = ",.:;";
	char *punc_ptr;
	int i, match;
	int replace, to_replace;

	if (curr_folder.curr_mssg == 0) {
	 dprint(4, (debugfile,
		"Add current alias called without any current message!\n"));
	 show_error(catgets(elm_msg_cat, AliasesSet, AliasesNoMessage,
		"No message to alias to!"));
	 return(0);
	}
	current_header = curr_folder.headers[curr_folder.curr_mssg-1];

	strcpy(buffer, catgets(elm_msg_cat, AliasesSet, AliasesCurrentMessage,
		"Current message address aliased to: "));
	PutLine(LINES-2,0, buffer);
	CleartoEOLN();
	*aliasname = '\0';
	if ((replace = get_aliasname(aliasname, buffer, &to_replace)) < 0) {
	    dprint(3, (debugfile,
	        "Aliasname [%s] was rejected in add_current_alias\n",
	        aliasname));
	    ClearLine(LINES-2);
	    return(0);
	}

	/* use full name in current message for default comment */
	tail_of(current_header->from, comment_buff, current_header->to);
	if(strchr(comment_buff, (int)'!') || strchr(comment_buff, (int)'@'))
	  /* never mind - it's an address not a full name */
	  *comment_buff = '\0';

/*
 *	Try to break up the From: comment into firstname, lastname, and
 *	any other text.  This is based on the fact that many address
 *	comments are pretty straightforward.  This will break on many
 *	situations.  Should handle:
 *		(Robert Howard)
 *		(Robert L. Howard)
 *		(Robert Howard, Georgia Tech)
 *	pretty well.  Will break on:
 *		(The Voice of Reason)
 *		and others....
 */

	*firstname = '\0';
	*lastname = '\0';
	*comment = '\0';
	if (strlen(comment_buff) != 0) {	/* There is something. */
	    bufptr = comment_buff;
	    while (*bufptr == ' ') bufptr++;	/* Always strip leading WS */
	    if ((chspace = strchr(bufptr, ' ')) != NULL) {
	   /*
	    *   A space means that there is at least (firstname lastname)
	    *   Get firstname and move bufptr.
	    */
	        *chspace = '\0';
	        strcpy(firstname, bufptr);
	        bufptr = chspace + 1;			/* Move the pointer */
	        while (*bufptr == ' ') bufptr++;
	    }

above:	    if ((chspace = strchr(bufptr, ' ')) != NULL) {
	   /*
	    *   Another space means a third+ word.  We either have:
	    *       1. Word 3+ is a comment, or
	    *       2. Word 2 is a middle initial (word 3 is lastname).
	    *   Check and see.
	    */
	        *chspace = '\0';
	        if ((strlen(bufptr) == 1) ||
	            (strlen(bufptr) == 2  && *(bufptr+1) == '.')) {
	       /*
	        *   If the second word is either a single
	        *   character or a character followed by '.' it was
	        *   probably a middle initial.  Add it to firstname
	        *   and shift.
	        */
	            strcat(firstname, " ");
	            strcat(firstname, bufptr);
	            bufptr = chspace + 1;		/* Move the pointer */
	            while (*bufptr == ' ') bufptr++;
	            goto above;
	        }
	        strcpy(lastname, bufptr);
	        bufptr = chspace + 1;			/* Move the pointer */
	        while (*bufptr == ' ') bufptr++;
	        strcpy(comment, bufptr);
	    }
	    else {
	   /*
	    *   Only a lastname left.
	    */
	        strcpy(lastname, bufptr);
	    }

	/*
	 *  Finally, get any puctuation characters off the end of
	 *  lastname.
	 */
	    match = TRUE;
	    for (i = strlen(lastname) - 1; match && i>0; i--) {
	        match = FALSE;
	        for (punc_ptr = bad_punc; *punc_ptr != '\0'; punc_ptr++) {
	            if (lastname[i] == *punc_ptr) {
	                lastname[i] = '\0';
	                match = TRUE;
	                break;
	            }
	        }
	    }
	}

	get_realnames(aliasname, firstname, lastname, comment, buffer);

	/* grab the return address of this message */
	get_return(address1, curr_folder.curr_mssg-1);
	strcpy(address1, strip_parens(address1));	/* remove parens! */

	return(ask_accept(aliasname, firstname, lastname, comment, address1,
	        buffer, replace, to_replace));

}

static int add_to_alias_text(char *aliasname, char *firstname, char *lastname,
			     char *comment, char *address)
{
/*
 *	Add the data to the user alias text file.
 *
 *	Return zero if we succeeded, 1 if not.
 */

	FILE *file;
	char fname[SLEN];
	char buffer[SLEN];
	int  err;

	sprintf(fname,"%s/%s", user_home, ALIAS_TEXT);

	save_file_stats(fname);
	if ((file = fopen(fname, "a")) == NULL) {
	  err = errno;
	  dprint(2, (debugfile,
		 "Failure attempting to add alias to file %s within %s",
		   fname, "add_to_alias_text"));
	  dprint(2, (debugfile, "** %s **\n", strerror(err)));
	  show_error(catgets(elm_msg_cat, AliasesSet, AliasesCouldntOpenAdd,
		 "Couldn't open %s to add new alias!"), fname);
	  return(1);
	}

	if (strlen(firstname) == 0) {
	    strcpy(buffer, lastname);
	}
	else {
	    sprintf(buffer, "%s; %s", lastname, firstname);
	}
	if (strlen(comment) != 0) {
	    strcat(buffer, ", ");
	    strcat(buffer, comment);
	}
	if (fprintf(file,"%s = %s = %s\n", aliasname, buffer, address) == EOF) {
	    err = errno;
	    dprint(2, (debugfile,
		       "Failure attempting to write alias to file within %s add_to_alias_text",
		       fname));
	    dprint(2, (debugfile, "** %s **\n", strerror(err)));
	    show_error(catgets(elm_msg_cat, AliasesSet, AliasesCouldntWrite,
		   "Couldn't write alias to file %s!"), fname);
	    fclose(file);
	    return(1);
	}

	fclose(file);

	restore_file_stats(fname);

	return(0);
}

int delete_from_alias_text(char **name, int num_to_delete)
{
/*
 *	Delete the data from the user alias text file.
 *
 *	Return zero if we succeeded, 1 if not.
 */

	FILE *file, *tmp_file;

	char fname[SLEN], tmpfname[SLEN];
	char line_in_file[LONG_STRING];
	char rest_of_line[LONG_STRING];
	char *s, *rest;

	int i;
	int num_aliases;
	int delete_continues;
	int err;

	delete_continues = FALSE;

	for (i=0; i < num_to_delete; i++)
	  strcat(name[i], ",");

	sprintf(fname,"%s/%s", user_home, ALIAS_TEXT);
	sprintf(tmpfname,"%s/%s.t", user_home, ALIAS_TEXT);

	save_file_stats(fname);

	if ((file = fopen(fname, "r")) == NULL) {
	  err = errno;
	  dprint(2, (debugfile,
		 "Failure attempting to delete alias from file %s within %s",
		   fname, "delete_from_alias_text"));
	  dprint(2, (debugfile, "** %s **\n", strerror(err)));
	  show_error(catgets(elm_msg_cat, AliasesSet, AliasesCouldntOpenDelete,
		 "Couldn't open %s to delete alias!"), fname);
	  return(1);
	}

	if ((tmp_file = file_open(tmpfname, "w")) == NULL) {
	  err = errno;
	  dprint(2, (debugfile,
		 "Failure attempting to open temp file %s within %s",
		   tmpfname, "delete_from_alias_text"));
	  dprint(2, (debugfile, "** %s **\n", strerror(err)));
	  show_error(catgets(elm_msg_cat, AliasesSet, AliasesCouldntOpenTemp,
	  	 "Couldn't open temp file %s to delete alias!"), tmpfname);
	  return(1);
	}

	while (mail_gets(line_in_file, sizeof(line_in_file), file) != 0)
	{
	  if (! whitespace(line_in_file[0])) {
	    delete_continues = FALSE;
	    if (line_in_file[0] != '#') {
	      num_aliases = parse_aliases(line_in_file, rest_of_line);
	      if (num_aliases > 0) {
	        for (i=0; i < num_to_delete && num_aliases; i++) {
	          if ( ((s = strstr(line_in_file, name[i])) == line_in_file) ||
		       ((s != NULL) && (*(s-1) == ',')) ) {
/*
 *	Collapse the to be deleted alias out of line_in_file
 */
	            rest = strchr(s, (int)',');
	            for (++rest; *rest; rest++)
	              *s++ = *rest;
	            *s = '\0';
	            num_aliases--;
	          }
	        }
	        if (num_aliases) {
	          *(line_in_file + strlen(line_in_file) - 1) = ' ';
	          strcat(line_in_file, rest_of_line);
	        }
	        else {
	          delete_continues = TRUE;
	        }
	      }
	    }
	  }
	  if (! delete_continues) {
	    if (fprintf(tmp_file,"%s", line_in_file) == EOF) {
	      err = errno;
	      dprint(2, (debugfile,
		"Failure attempting to write to temp file %s within %s",
		tmpfname, "delete_from_alias_text"));
	      dprint(2, (debugfile, "** %s **\n", strerror(err)));
	      show_error(catgets(elm_msg_cat, AliasesSet, AliasesCouldntWriteTemp,
		"Couldn't write to temp file %s!"), tmpfname);
	      fclose(file);
	      fclose(tmp_file);
	      unlink(tmpfname);
	      return(1);
	    }
	  }
	}
	fclose(file);
	fclose(tmp_file);
	if (rename(tmpfname, fname) != 0)
	{
		show_error(catgets(elm_msg_cat, AliasesSet, AliasesCouldntRenameTemp,
			"Couldn't rename temp file %s after deleting alias!"), tmpfname);
		return(1);
	}

	restore_file_stats(fname);

	return(0);
}

void alias(void)
{
/*
 *	Work with alias commands...
 */

	char name[NLEN], *address, buffer[SLEN];
	char *commap;
	static int  newaliases = 0;
	int  ch, i;
	int nutitle = 0;
	int too_long;

/*
 *	We're going to try to match the way elm does it at
 * 	he main menu.  I probably won't be able to use any
 *	main menu routines, but I will "borrow" from them. RLH
 */

	alias_main_state();		/* Save globals for return to main menu */

	open_alias_files(FALSE);	/* First, read the alias files. RLH */

	alias_screen(newaliases);

	while (1) {

	  redraw = 0;
	  nucurr = 0;
	  nufoot = 0;

	  prompt(nls_Prompt);
	  CleartoEOLN();
	  ch = GetKey(0);

	  MoveCursor(LINES-3,strlen(nls_Prompt)); CleartoEOS();

	  dprint(3, (debugfile, "\n-- Alias command: %c\n\n", ch));

	  switch (ch) {
	    case '?': redraw += alias_help();			break;

#ifdef ALLOW_SUBSHELL
	    case '!' : WriteChar('!');
	      alias_main_state(); /** reload index screen vars **/
	      redraw += subshell();
	      alias_main_state(); /** reload alias screen vars **/
	      break;
#endif /* ALLOW_SUBSHELL */

	    case '$': PutLine(-1, -1, catgets(elm_msg_cat,
					AliasesSet, AliasesResync,
					"Resynchronize aliases..."));
	           /*
	            * Process deletions and then see if we need to
	            * re-run the "newalias" routine.
	            */
		      if (resync_aliases(newaliases)) {
		        install_aliases();
	                newaliases = 0;
		        redraw++;
		      }
		      break;

	    case 'a': PutLine(-1, -1, catgets(elm_msg_cat,
					AliasesSet, AliasesAddCurrent,
					"Add address from current message..."));
		      clear_error();
		      if (add_current_alias()) {
		          newaliases++;
		          nutitle++;
		      }
		      break;

	    case 'c':
	              if (curr_alias > 0) {
			  PutLine(-1, -1, catgets(elm_msg_cat,
	                              AliasesSet, AliasesReplaceCurrent,
	                              "Replace current alias in database..."));
		          clear_error();
		          if (add_alias(TRUE, curr_alias-1)) {
		              newaliases++;
		              nutitle++;
		          }
	              }
	              else {
		          show_error(catgets(elm_msg_cat,
	                          AliasesSet, AliasesNoneToReplace,
				  "Warning: no aliases to replace!"));
	              }
		      break;

	    case 'e': PutLine(LINES-3, strlen(nls_Prompt),
	                  catgets(elm_msg_cat, AliasesSet, AliasesEdit,
	                      "Edit %s..."), ALIAS_TEXT);
	           /*
	            * Process aliases.text for deletions, etc.  You
	            * have to do this *before* checking current because
	            * all aliases could be marked for deletion.
	            */
	              (void) resync_aliases(newaliases);
	              if (edit_aliases_text()) {
	                  newaliases = 0;
	              }
		      redraw++;
		      break;

	    case 'm':
	              if (curr_alias > 0) {
			  PutLine(-1, -1, catgets(elm_msg_cat,
					  AliasesSet, AliasesMail, "Mail..."));
	                  redraw += a_sendmsg();
	              }
	              else {
		          show_error(catgets(elm_msg_cat,
	                          AliasesSet, AliasesNoneToMail,
				  "Warning: no aliases to send mail to!"));
	              }
		      break;

	    case 'n': PutLine(-1, -1, catgets(elm_msg_cat,
					AliasesSet, AliasesAddNew,
					"Add a new alias to database..."));
		      clear_error();
		      if (add_alias(FALSE, -1)) {
		          newaliases++;
		          nutitle++;
		      }
		      break;

	    case 'q':
	    case 'Q':
	    case 'i':
	    case 'I':
	    case 'r':
	    case 'R': PutLine(-1, -1, catgets(elm_msg_cat,
	    				AliasesSet, AliasesAddReturn,
					"Return to main menu..."));
	           /*
	            * leaving the alias system.  Must check for
	            * pending deletes, etc.  prompt is set to FALSE
	            * on uppercase letters so that deletions are
	            * NOT queried.
	            */
	              if (delete_aliases(newaliases, islower(ch))) {
	                install_aliases();
	                newaliases = 0;
	              }
		      clear_error();
		      alias_main_state();		/* Done with aliases. */
		      return;

	    case RETURN:
	    case LINE_FEED:
	    case ' ':
	    case 'v':
		      if (newaliases) {		/* Need this ?? */
		          show_error(catgets(elm_msg_cat,
	                          AliasesSet, AliasesNotInstalled,
				  "Warning: new aliases not installed yet!"));
	              }

	              if (curr_alias > 0) {
	                  if (aliases[curr_alias-1]->type & GROUP) {
	                      PutLine(LINES-1, 0, catgets(elm_msg_cat,
	                              AliasesSet, AliasesGroupAlias,
				      "Group alias: %-60.60s"),
	                          aliases[curr_alias-1]->address);
		          }
		          else {
	                      PutLine(LINES-1, 0, catgets(elm_msg_cat,
	                              AliasesSet, AliasesAliasedAddress,
				      "Aliased address: %-60.60s"),
	                          aliases[curr_alias-1]->address);
		          }
		      }
	              else {
		          show_error(catgets(elm_msg_cat,
	                          AliasesSet, AliasesNoneToView,
				  "Warning: no aliases to view!"));
		      }
		      break;

	    case 'x':
	    case 'X': PutLine(-1, -1, catgets(elm_msg_cat,
	    				AliasesSet, AliasesAddReturn,
					"Return to main menu..."));
	              exit_alias();
		      clear_error();
		      alias_main_state();		/* Done with aliases. */
		      return;

	    case 'f':
	    case 'F':
	              if (curr_alias > 0) {
		          clear_error();
		          strcpy(name, aliases[curr_alias-1]->alias);
		          if (ch == 'F') {
		              strcpy(buffer, catgets(elm_msg_cat,
	                              AliasesSet, AliasesFullyExpanded,
				      "Fully expand alias: "));
		              PutLine(LINES-2, 0, buffer);
			      if (enter_string(name, sizeof(name), -1, -1,
					  ESTR_REPLACE) < 0 || name[0] == '\0')
				break;
		          }
	                  too_long = FALSE;
		          address = get_alias_address(name, TRUE, &too_long);
		          if (address != NULL) {
		              while (TRUE) {
	                          ClearScreen();
			          PutLine(2,0, catgets(elm_msg_cat,
	                                  AliasesSet, AliasesAliasedFull,
					  "Aliased address for:\t%s\n\r"),
	                              name);
		                  i = 4;
		                  while (i < LINES-2) {
		                      if ((commap = strchr(address, (int)','))
	                                          == NULL) {
		                          PutLine(i, 4, address);
		                          break;
		                      }
		                      *commap = '\0';
		                      PutLine(i++, 4, address);
		                      address = commap+2;
		                  }
	                          PutLine(LINES-1, 0, catgets(elm_msg_cat,
	                                  AliasesSet, AliasesPressReturn,
					  "Press <return> to continue."));
			          (void) ReadCh();
		                  if (commap == NULL) {
			              redraw++;
		                      break;
		                  }
		              }
		          }
	                  else if (! too_long) {
			      show_error(catgets(elm_msg_cat,
	                              AliasesSet, AliasesNotFound,
				      "Not found."));
		          }
		      }
	              else {
		          show_error(catgets(elm_msg_cat,
	                          AliasesSet, AliasesNoneToView,
				  "Warning: no aliases to view!"));
		      }
		      break;

	  case KEY_REDRAW:
		      redraw = 1;
		      break;

	 /*
	  * None of the menu specific commands were chosen, therefore
	  * it must be a "motion" command (or an error).
	  */
	    default	: motion(ch);

	  }

	  if (redraw) {			/* Redraw screen if necessary */
	      alias_screen(newaliases);
	      nutitle = 0;
	  }

	  if (nutitle) {		/* Redraw title if necessary */
	      alias_title(newaliases);
	      nutitle = 0;
	  }

	  check_range();

	  if (nucurr == NEW_PAGE)
	    show_headers();
	  else if (nucurr == SAME_PAGE)
	    show_current();
	  else if (nufoot) {
	    if (mini_menu) {
	      MoveCursor(LINES-7, 0);
              CleartoEOS();
	      show_alias_menu();
	    }
	    else {
	      MoveCursor(LINES-4, 0);
	      CleartoEOS();
	    }
	    show_last_error();	/* for those operations that have to
				 * clear the footer except for a message.
				 */
	  }
	}			/* BIG while loop... */
}

void install_aliases(void)
{
/*
 *	Run the 'newalias' program and update the
 *	aliases before going back to the main program!
 *
 *	No return value.....
 */

	int na;
	char itextfile[SLEN], odatafile[SLEN];

	show_error(catgets(elm_msg_cat, AliasesSet, AliasesUpdating,
		"Updating aliases..."));
	if (sleepmsg > 0)
		sleep(sleepmsg);

	sprintf(itextfile, "%s/%s", user_home, ALIAS_TEXT);
	sprintf(odatafile, "%s/%s", user_home, ALIAS_DATA);

/*
 *	We need to unlimit everything since aliases are
 * 	eing read in from scratch.
 */
	selected = 0;

	na = do_newalias(itextfile, odatafile, TRUE, FALSE);
	if (na >= 0) {
	    show_error(catgets(elm_msg_cat, AliasesSet, AliasesReReading,
		  "Processed %d aliases.  Re-reading the database..."), na);
	    if (sleepmsg > 0)
		sleep(sleepmsg);
	    open_alias_files(TRUE);
	    set_error(catgets(elm_msg_cat, AliasesSet, AliasesUpdatedOK,
		  "Aliases updated successfully."));
	}
}

static int alias_help(void)
{
/*
 *	Help section for the alias menu...
 *
 *	Return non-0 if main part of screen overwritten, else 0
 */

	int  ch;
	int  redraw=0;
	char *alias_prompt;


	if (mini_menu)
		alias_prompt = catgets(elm_msg_cat, AliasesSet, AliasesShortKey,
			"Key: ");
	else
		alias_prompt = catgets(elm_msg_cat, AliasesSet, AliasesLongKey,
			"Key you want help for: ");

	MoveCursor(LINES-3, 0);
	CleartoEOS();
	if (mini_menu) {
	  CenterLine(LINES-3, catgets(elm_msg_cat, AliasesSet, AliasesKeyMenu,
 "Press the key you want help for, '?' for a key list, or '.' to exit help"));
	}

	lower_prompt(alias_prompt);

	while ((ch = ReadCh()) != '.') {
	  switch(ch) {
	    case '?' : display_helpfile("alias");
		       redraw++;
		       return(redraw);

	    case '$': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpDollar,
"$ = Force resynchronization of aliases, processing additions and deletions."));
		      break;

	    case '/': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpSlash,
			"/ = Search for specified name or alias in list."));
		      break;

	    case RETURN:
	    case LINE_FEED:
	    case ' ':
	    case 'v': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpv,
	    "v = View the address for the currently selected alias."));
		      break;

	    case 'a': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpa,
	    "a = Add (return) address of current message to alias database."));
		      break;

	    case 'c': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpc,
"c = Change current user alias, modifying alias database at next resync."));
		      break;

	    case 'd': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpd,
	    "d = Mark the current alias for deletion from alias database."));
		      break;

	    case ctrl('D'): show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpCtrlD,
	    "^D = Mark for deletion user aliases matching specified pattern."));
		      break;

	    case 'e': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpe,
	    "e = Edit the alias text file directly (will run newalias)."));
		      break;

	    case 'f': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpf,
		"f = Display fully expanded address of current alias."));
		      break;

	    case 'l': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpl,
	    "l = Limit displayed aliases on the specified criteria."));
		      break;

	    case ctrl('L'): show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpCtrlL,
		      "^L = Rewrite the screen."));
	    	      break;

	    case 'm': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpm,
	    "m = Send mail to the current or tagged aliases."));
		      break;

	    case 'n': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpn,
"n = Add a new user alias, adding to alias database at next resync."));
		      break;

	    case 'r':
	    case 'q':
	    case 'i': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpi,
		      "r,q,i = Return from alias menu (with prompting)."));
	   	      break;

	    case 'R':
	    case 'Q':
	    case 'I': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpQ,
		      "R,Q,I = Return from alias menu (no prompting)."));
	   	      break;

	    case 't': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpt,
		      "t = Tag current alias for further operations."));
		      break;

	    case 'T': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpT,
		      "T = Tag current alias and go to next alias."));
		      break;

	    case ctrl('T'): show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpCtrlT,
	    "^T = Tag aliases matching specified pattern."));
		      break;

	    case 'u': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpu,
	    "u = Unmark the current alias for deletion from alias database."));
		      break;

	    case ctrl('U'): show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpCtrlU,
"^U = Mark for undeletion user aliases matching specified pattern."));
		      break;

	    case 'x':
	    case 'X': show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpX,
	    "x = Exit from alias menu, abandoning any pending deletions."));
	   	      break;

	    default : show_error(catgets(elm_msg_cat, AliasesSet, AliasesHelpNoHelp,
			"That key isn't used in this section."));
	   	      break;
	  }
	  lower_prompt(alias_prompt);
	}

	/* Remove help lines */
	MoveCursor(LINES-3, 0);	CleartoEOS();
	return(redraw);
}

static void get_aliases(int are_in_aliases)
{
/*
 *	Get all the system and user alias info
 *
 *	If we get this far, we must be needing to re-read from
 *	at least one data file.  Unfortunately that means we
 *	really need to read both since the aliases may be sorted
 *	and all mixed up...  :-(
 */

	int dups = 0;

	curr_alias = 0;
	num_duplicates = 0;
/*
 *	Read from user data file if it is open.
 */
	if (user_hash != NULL) {
	    dprint(6, (debugfile,
		      "About to read user data file = %s.\n",
	              user_hash->dbz_basefname));
	    fseek(user_hash->dbz_basef, 0L, 0);
	    while (get_one_alias(user_hash, curr_alias)) {
		dprint(8, (debugfile, "%d\t%s\t%s\n", curr_alias+1,
				       aliases[curr_alias]->alias,
				       aliases[curr_alias]->address));

		curr_alias++;
	    }
	}
	num_aliases = curr_alias;		/* Needed for find_alias() */

/*
 *	Read from system data file if it is open.
 */
	if (system_hash != NULL) {
	    dprint(6, (debugfile,
		      "About to read system data file = %s.\n",
	              system_hash->dbz_basefname));
	    fseek(system_hash->dbz_basef, 0L, 0);
	    while (get_one_alias(system_hash, curr_alias)) {
	    /*
	     *  If an identical user alias is found, we may
	     *  not want to display it, so we had better mark it.
	     */
		if (find_alias(aliases[curr_alias]->alias, USER) >= 0) {
		    setit(aliases[curr_alias]->type, DUPLICATE);
		    dups++;
		    setit(aliases[curr_alias]->status, URGENT);
				    /* Not really, I want the U for User */
		    dprint(6, (debugfile,
			       "System alias %s is same as user alias.\n",
			       aliases[curr_alias]->alias));
		}
		dprint(8, (debugfile, "%d\t%s\t%s\n", curr_alias+1,
				       aliases[curr_alias]->alias,
				       aliases[curr_alias]->address));

		curr_alias++;
	    }
	    num_duplicates = dups;
	}
	num_aliases = curr_alias - num_duplicates;

	if (OPMODE_IS_READMODE(opmode) && num_aliases > 0) {
	    curr_alias = 0;
	    sort_aliases((num_aliases+num_duplicates), FALSE, are_in_aliases);
	    curr_alias = 1;
	    if (are_in_aliases) {
	        (void) get_page(curr_alias);
	    }
	}

}

static int get_one_alias(DBZ *db, int current)
{
/*
 *	Get an alias (name, address, etc.) from the data file
 */

	int new_max;
	struct alias_rec *a;

	if ((a = fetch_alias(db, (char *)NULL)) == NULL)
	    return 0;

	if (current >= max_aliases) {
	    new_max = max_aliases + KLICK;
	    if (max_aliases == 0) {
		aliases = (struct alias_rec **)
		    safe_malloc(new_max * sizeof(struct alias_rec *));
	    } else {
		aliases = (struct alias_rec **) safe_realloc((malloc_t)aliases,
		    new_max * sizeof(struct alias_rec *));
	    }
	    while (max_aliases < new_max)
		aliases[max_aliases++] = NULL;
	}

	if (aliases[current] != NULL)
	    free((malloc_t)aliases[current]);
	aliases[current] = a;
	return 1;
}


void alias_main_state(void)
{
/*	Save the globals that are shared for both menus
 *	so that we can return to the main menu without
 *	"tragedy".
 */

	static int alias_last = -1, alias_selected = 0, alias_page = 0;
	static int main_last = -1, main_selected = 0,  main_page = 0;

	if (inalias) {			/* Restore the settings */
	    alias_last = last_current;
	    alias_selected = selected;
	    alias_page = header_page;

	    last_current = main_last;
	    selected = main_selected;
	    header_page = main_page;

	    nls_item = catgets(elm_msg_cat, ElmSet, Elmitem, "message");
	    nls_items = catgets(elm_msg_cat, ElmSet, Elmitems, "messages");
	    nls_Item = catgets(elm_msg_cat, ElmSet, ElmItem, "Message");
	    nls_Items = catgets(elm_msg_cat, ElmSet, ElmItems, "Messages");
	    nls_Prompt = catgets(elm_msg_cat, ElmSet, ElmPrompt, "Command: ");

	    (void) define_softkeys(SOFTKEYS_MAIN);

	    dprint(3, (debugfile, "Leaving alias mode\n"));
	    inalias = FALSE;
	}
	else {
	    main_last = last_current;
	    main_selected = selected;
	    main_page = header_page;

	    last_current = alias_last;
	    selected = alias_selected;
	    header_page = alias_page;

	    nls_item = catgets(elm_msg_cat, AliasesSet, Aliasesitem, "alias");
	    nls_items = catgets(elm_msg_cat, AliasesSet, Aliasesitems, "aliases");
	    nls_Item = catgets(elm_msg_cat, AliasesSet, AliasesItem, "Alias");
	    nls_Items = catgets(elm_msg_cat, AliasesSet, AliasesItems, "Aliases");
	    nls_Prompt = catgets(elm_msg_cat, AliasesSet, AliasesPrompt, "Alias: ");

	    (void) define_softkeys(SOFTKEYS_ALIAS);

	    dprint(3, (debugfile, "Entered alias mode\n"));
	    inalias = TRUE;
	}
}

static int parse_aliases(char *buffer, char *remainder)
{
/*
 *	This routine will parse out the individual aliases present
 *	on the line passed in buffer.  This involves:
 *
 *	1. Testing for an '=' to make sure this is an alias entry.
 *
 *	2. Setting remainder to point to the rest of the line starting
 *	   at the '=' (for later rewriting if needed).
 *
 *	3. Parsing the aliases into an string padded with ',' at
 *	   the end.
 *
 *	4. Returning the number of aliases found (0 if test #1 fails).
 */

	char *s;
	int number;

/*	Check to see if an alias */

	if ((s = strchr(buffer, (int)'=')) == NULL)
	  return (0);

	strcpy(remainder, s);		/* Save the remainder of the line */

/*	Terminate the list of aliases with a ',' */

	while (--s >= buffer && whitespace(*s)) ;
	*++s = ',';
	*++s = '\0';

/*	Lowercase everything */

	s = shift_lower(buffer);
	strcpy(buffer, s);

/*	Now, count the aliases */

	number = 0;
	for (s = buffer; *s; s++)
	  if (*s == ',')
	    number++;

	return (number);
}

static int get_aliasname(char *aliasname, char *buffer, int *duplicate)
{

/*
 *	Have the user enter an aliasname, check to see if it
 *	is legal, then check for duplicates.  If a duplicate
 *	is found offer to replace existing alias.
 *
 *	Return values:
 *
 *	-1	Either the aliasname was zero length, had bad
 *		characters and was a duplicate which the user
 *		chose not to replace.
 *
 *	0	A new alias was entered successfully.
 *
 *	1	The entered alias was an existing USER alias
 *		that the user has chosen to replace.  In this
 *		case the alias to replace is passed back in
 *		in the variable 'duplicate'.
 */

	int loc;

	do {
	    if (enter_string(aliasname, SLEN,
				LINES-2, strlen(buffer), ESTR_REPLACE) < 0
			|| aliasname[0] == '\0')
	        return(-1);
	} while (check_alias(aliasname) == -1);

	clear_error();			/* Just in case */
/*
 *	Check to see if there is already a USER alias by this name.
 */
	if ((loc = find_alias(aliasname, USER)) >= 0) {
	    dprint(3, (debugfile,
	         "Attempt to add a duplicate alias [%s] in get_aliasname\n",
	         aliases[loc]->alias));
	    if (aliases[loc]->type & GROUP )
	        PutLine(LINES-2,0, catgets(elm_msg_cat,
	                AliasesSet, AliasesAlreadyGroup,
	                "Already a group with name %s."), aliases[loc]->alias);
	    else
	        PutLine(LINES-2,0, catgets(elm_msg_cat,
	                AliasesSet, AliasesAlreadyAlias,
	                "Already an alias for %s."), aliases[loc]->alias);
	    CleartoEOLN();
	 /*
	  * If they don't want to replace the alias by that name
	  * then just return.
	  */
	    if (!enter_yn(catgets(elm_msg_cat, AliasesSet,
			AliasesReplaceExisting,
			"Replace existing alias?"), FALSE, LINES-3, FALSE))
	        return(-1);
	    *duplicate = loc;
	    return(1);
	}
/*
 *	If they have elected to replace an existing alias then
 *	we assume that they would also elect to superceed a
 *	system alias by that name (since they have already
 *	done so).  So we don't even bother to check or ask.
 *
 *	Of course we do check if there was no USER alias match.
 */
	if ((loc = find_alias(aliasname, SYSTEM)) >= 0) {
	    dprint(3, (debugfile,
	      "Attempt to add a duplicate system alias [%s] in get_aliasname\n",
	      aliases[loc]->address));

	    if( ! superceed_system(loc, buffer))
	        return(-1);
	}
	return(0);

}

static int superceed_system(int this_alias, char *buffer)
{

	PutLine(LINES-2, 0, catgets(elm_msg_cat,
	        AliasesSet, AliasesSystemAlias, "System (%6s) alias for %s."),
	    alias_type(aliases[this_alias]->type), aliases[this_alias]->alias);
/*
 *	If they don't want to superceed the SYSTEM alias then
 *	return a FALSE.
 */
	return enter_yn(catgets(elm_msg_cat, AliasesSet, AliasesSuperceed,
	        "Superceed?"), FALSE, LINES-3, FALSE);
}

static void get_realnames(char *aliasname, char *firstname, char *lastname,
			  char *comment, char *buffer)
{
	/* FOO - this is not handling enter_string() aborts properly */

	sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesEnterLastName,
		"Enter last name for %s: "), aliasname);
	PutLine(LINES-2, 0, buffer);
	enter_string(lastname, SLEN, -1, -1, ESTR_REPLACE);

	sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesEnterFirstName,
		"Enter first name for %s: "), aliasname);
	PutLine(LINES-2, 0, buffer);
	enter_string(firstname, SLEN, -1, -1, ESTR_REPLACE);

	if (strlen(lastname) == 0) {
	    if (strlen(firstname) == 0) {
	        strcpy(lastname, aliasname);
	    }
	    else {
	        strcpy(lastname, firstname);
	        *firstname = '\0';
	    }
	}

	sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesEnterComment,
		"Enter optional comment for %s: "), aliasname);
	PutLine(LINES-2, 0, buffer);
	enter_string(comment, SLEN, -1, -1, ESTR_REPLACE);

}

static int ask_accept(char *aliasname, char *firstname, char *lastname,
	       char *comment, char *address, char *buffer,
	       int replace, int replacement)
{
	int ans;
	char *(old_alias[1]);
/*
 *	If firstname == lastname, they probably just took all
 *	the deafaults.  We *assume* they don't want lastname
 *	entered twice, so we will truncate it.
 */
	if (strcmp(firstname, lastname) == 0) {
	    *firstname = '\0';
	}

	if (strlen(firstname) == 0) {
	    strcpy(buffer, lastname);
	}
	else {
	    sprintf(buffer, "%s %s", firstname, lastname);
	}
	PutLine(LINES-1,0, catgets(elm_msg_cat, AliasesSet, AliasesAddressAs,
	        "Messages addressed as: %s (%s)"), address, buffer);
	if (strlen(comment) != 0) {
	    strcat(buffer, ", ");
	    strcat(buffer, comment);
	}

	PutLine(LINES-2,0, catgets(elm_msg_cat, AliasesSet, AliasesAddressTo,
	        "New alias: %s is '%s'."), aliasname, buffer);
	CleartoEOLN();
/*
 *	Kludge Alert:  Spaces are padded to the front of the prompt
 *	to write over the previous question.  Should probably record
 *	the end of the line, move to it, and CleartoEOLN() it.
 */
	ans = enter_yn(catgets(elm_msg_cat, AliasesSet, AliasesAcceptNew,
		"      Accept new alias?"), TRUE, LINES-3, FALSE);
	if(ans) {
	    if (replace) {
	        old_alias[0] = aliases[replacement]->alias;
	    /*
	     *  First, clear flag if this is marked to be deleted.
	     *  This prevents the problem where they marked it for
	     *  deletion and then figured out that it could be
	     *  c)hanged but didn't explicitly U)ndelete it.  Without
	     *  this test, the resync action would then delete
	     *  the new alias we just so carefully added to the
	     *  text file.
	     */
	        if (ison(aliases[replacement]->status, DELETED)) {
	            clearit(aliases[replacement]->status, DELETED);
	        }
	    /*
	     *  Changed aliases are given the NEW flag.
	     */
	        setit(aliases[replacement]->status, NEW);
	        show_msg_status(replacement);
	    /*
	     *  Now we can delete it...
	     */
	        delete_from_alias_text(old_alias, 1);
	    /*
	     *  Kludge Alert:  We need to get the trailing comma
	     *  (added in delete_from_alias_text()) off of the
	     *  alias since the display won't be re-sync'd right
	     *  away.
	     */
	        *((old_alias[0])+strlen(old_alias[0])-1) = '\0';
	    }
	    add_to_alias_text(aliasname, firstname, lastname, comment, address);
	}
	ClearLine(LINES-2);
	ClearLine(LINES-1);
	return ans;
}

/* Check whether an address is aliased; if so return the alias, otherwise
 * return NULL. */
char *address_to_alias(const char *address)
{
	int i;
	char return_address[SLEN];

	(void) open_alias_files(FALSE);

	get_return_address(address, return_address);

	for (i = 0; i < num_aliases; i++) {
	    if (strcasecmp(return_address, aliases[i]->address) == 0)
		return aliases[i]->alias;
	}
	return NULL;
}

int find_alias(char *word, int alias_type)
{
	/** find word and return loc, or -1 **/
	int loc = -1;

	/** cannot be an alias if its longer than NLEN chars **/
	if (strlen(word) > NLEN)
	    return(-1);

	while (++loc < (num_aliases+num_duplicates)) {
	    if ( aliases[loc]->type & alias_type ) {
	        if (strcasecmp(word, aliases[loc]->alias) == 0)
	            return(loc);
	    }
	}

	return(-1);				/* Not found */
}
