

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.5 $   $State: Exp $
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
 * $Log: save_opts.c,v $
 * Revision 1.5  1996/10/28  16:58:08  wfp5p
 * Beta 1
 *
 * Revision 1.4  1996/03/14  17:29:49  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:25  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:29  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elmrc.h"
#include "save_opts.h"

extern char version_buff[];

/* local procedures */
static void err_pause P_((int));
static void output_option P_((FILE *, int));
static int find_opt P_((const char *));
static char *str_opt P_((int, int));


/*
 * This routine is used in the display_options() procedure.
 */
char *str_opt_nam(const char *optname, int dispmode)
{
    int optnum;
    optnum = find_opt(optname);
    return (optnum >= 0 ? str_opt(optnum, dispmode) : (char *)NULL);
}


/*
 * save_options() - Create an ".elm/elmrc" file with current settings.
 *
 * A backup copy of any existing "elmrc" file is made before the
 * new one is written.
 *
 * An "elmrc-info" file in the library directory directs the creation of
 * this file.  The "elmrc-info" file contains lines as follows:
 *
 *	; comment text		This line is deleted and does NOT
 *				appear in the user's elmrc file.
 *
 *	# comment text		This line is passed through to the
 *				user's elmrc file as a comment.
 *
 *	(blank line)		A blank line is written to the user's
 *				elmrc file.
 *
 *	option_name		An "option_name = option_value" line
 *				is written to the user's elmrc file.
 *
 * If an "elmrc-info" file is not available, then a raw alphabetical
 * dump of the option settings is made.
 */
void save_options(void)
{
    int i;
    char rcfname[SLEN], buf[SLEN], *optdone;
    FILE *fp_elmrc, *fp_rcinfo;

    err_pause(FALSE);

    /* locate the "elmrc" file */
/*    sprintf(rcfname, "%s/%s", user_home, elmrcfile); */
    getelmrcName(rcfname,SLEN);
    save_file_stats(rcfname);

    /* make backup copy of any existing "elmrc" */
/*    sprintf(buf, "%s/%s", user_home, old_elmrcfile);*/
    getelmrcName(buf,SLEN-5);
   strcpy(buf, ".old");

    if (elm_access(rcfname, ACCESS_EXISTS) != -1) {

      /* FIXME shouldn't this do more than write a debug */
        if (rename(rcfname, buf) < 0) {
	    dprint(2, (debugfile, "Unable to rename %s to %s\n", rcfname, buf));
	}

	chown(buf, userid, groupid);
    }

    /* create an "elmrc" file */
    if ((fp_elmrc = fopen(rcfname, "w")) == NULL) {
	err_pause(TRUE);
	show_error(catgets(elm_msg_cat, ElmrcSet, ElmrcCannotOpenElmrc,
		"Error: cannot open \"%s\" to save.  [%s]"),
		rcfname, strerror(errno));
	return;
    }

    /* the "elmrc-info" file describes the option settings */
    if ((fp_rcinfo = fopen(system_rcinfo_file, "r")) == NULL) {
	err_pause(TRUE);
	show_error(catgets(elm_msg_cat, ElmrcSet, ElmrcCannotOpenRcinfo,
	      "Warning: cannot open \"%s\" file.  [%s]"),
	      basename(system_rcinfo_file), strerror(errno));
    }

    /* sanity check to ensure the options list is sorted properly */
    for (i = 1 ; i < NUMBER_OF_SAVEABLE_OPTIONS ; ++i) {
	if (strcmp(save_info[i-1].name, save_info[i].name) > 0) {
	    err_pause(TRUE);
	    show_error(catgets(elm_msg_cat, ElmrcSet, ElmrcSortOrderCorrupt,
		    "INTERNAL ERROR: save_info[] sort order corrupt at \"%s\"."),
		    save_info[i].name);
	    /* a raw dump probably will work alright */
	    if (fp_rcinfo != NULL) {
		(void) fclose(fp_rcinfo);
		fp_rcinfo = NULL;
	    }
	    break;
	}
    }

    /* emit header */
    fprintf(fp_elmrc, catgets(elm_msg_cat, ElmrcSet, ElmrcOptionsFile,
	    "#\n# %s - options file for the ELM mail system\n#\n"),
	    elmrcfile);
    fprintf(fp_elmrc, catgets(elm_msg_cat, ElmrcSet, ElmrcSavedAutoFor,
	    "# Saved automatically by ELM %s for %s\n#\n\n"),
	    version_buff, user_fullname);

    /* if "elmrc-info" is missing then just do a raw dump */
    if (fp_rcinfo == NULL) {
	/* should be a mssg on the display explaining why we are doing this */
	err_pause(TRUE);
	show_error(catgets(elm_msg_cat, ElmrcSet, ElmrcSavedRaw,
		"Due to problems - options being saved as raw, uncommented dump."));
	for (i = 0 ; i < NUMBER_OF_SAVEABLE_OPTIONS ; ++i) {
	    if (!(save_info[i].flags & FL_SYS))
		output_option(fp_elmrc, i);
	}
	goto done;
    }

    /* create vector to note options that have been processed */
    optdone = (char *) safe_malloc(NUMBER_OF_SAVEABLE_OPTIONS);
    (void) bzero(optdone, NUMBER_OF_SAVEABLE_OPTIONS);

    /* process the "elmrc-info" file */
    while (fgets(buf, sizeof(buf), fp_rcinfo) != NULL) {

	for (i = strlen(buf)-1 ; i >= 0 && isspace(buf[i]) ; --i)
	    ;
	buf[i+1] = '\0';

	switch (buf[0]) {

	case ';':		/* source file comment to delete */
	    break;

	case '#':		/* comment to pass through */
	    fputs(buf, fp_elmrc);
	    /*FALLTHRU*/

	case '\0':		/* pass through blank lines */
	    putc('\n', fp_elmrc);
	    break;

	default:		/* should be name of an option setting */
	    if ((i = find_opt(buf)) < 0) {
		err_pause(TRUE);
		show_error(catgets(elm_msg_cat, ElmrcSet, ElmrcUnknownOptionWarning,
			"Warning: unknown option \"%s\" in \"%s\" file."),
			buf, basename(system_rcinfo_file));
		fprintf(fp_elmrc,
			catgets(elm_msg_cat, ElmrcSet, ElmrcUnknownOptionMssg,
			"# WARNING - unknown option in \"%s\"\n"),
			system_rcinfo_file);
		fprintf(fp_elmrc, "### %s = ?unknown?\n", buf);
	    } else if (!(save_info[i].flags & FL_SYS)) {
		output_option(fp_elmrc, i);
		optdone[i] = TRUE;
	    }
	    break;

	}

    }

    /* see if there are any options missing from "elmrc-info" */
    for (i = 0 ; i < NUMBER_OF_SAVEABLE_OPTIONS ; ++i) {
	if (!optdone[i] && !(save_info[i].flags & FL_SYS)) {
	    switch (save_info[i].flags & DT_MASK) {
	    case DT_MLT:
	    case DT_SYN:
	    case DT_NOP:
		break;
	    default:
		err_pause(TRUE);
		show_error(catgets(elm_msg_cat, ElmrcSet, ElmrcMissingOptionWarning,
			"Warning: option \"%s\" missing from \"%s\" file."),
			buf, basename(system_rcinfo_file));
		fprintf(fp_elmrc,
			catgets(elm_msg_cat, ElmrcSet, ElmrcMissingOptionMssg,
			"# WARNING - following option missing from \"%s\"\n"),
			system_rcinfo_file);
		output_option(fp_elmrc, i);
		break;
	    }
	}
    }

    free((malloc_t)optdone);
    fclose(fp_rcinfo);
done:
    fclose(fp_elmrc);
    restore_file_stats(rcfname);
    err_pause(TRUE);
    show_error(catgets(elm_msg_cat, ElmrcSet, ElmrcOptionsSavedIn,
	    "Options saved in file %s."), rcfname);
}


/*
 * err_pause() - Pause to prevent error messages from getting overwritten.
 *
 * Call with err_flag=FALSE to reset to "no message displayed".  Then
 * call before each error message with err_flag=TRUE.  This will pause
 * if the following error message will wipe out a current message.
 */
static void err_pause(int err_flag)
{
    static char pause_mssg[] = "press ENTER to continue";
    static int error_displayed;
    int i;

    if (!err_flag) {
	error_displayed = FALSE;
	return;
    }

    if (error_displayed) {
	MoveCursor(LINES-1 , 0);
	CleartoEOLN();
	PutLine(LINES-1, (COLS-(sizeof(pause_mssg)-1))/2, pause_mssg);
	while ((i = ReadCh()) != '\r' && i != '\n')
	    putchar('\007');
	ClearLine(LINES-1);
    }

    error_displayed = TRUE;
}


static void output_option(FILE *fp_elmrc, int optnum)
{
    int local_value, len, i;
    char *optval, *w;
    struct addr_rec *alts;

    local_value = !!(save_info[optnum].flags & FL_LOCAL);
    optval = NULL;

    switch(save_info[optnum].flags & DT_MASK) {

    case DT_MLT:
    case DT_SYN:
    case DT_NOP:
	/* these items are not saved */
	break;

    case DT_ASR:
    case DT_SRT:
	optval = str_opt(optnum, TRUE);
	break;

    case DT_STR:
    case DT_CHR:
    case DT_BOL:
    case DT_NUM:
	optval = str_opt(optnum, FALSE);
	break;

    case DT_WEE:
	fprintf(fp_elmrc, "%s%s =",
		(local_value ? "" : "### "), save_info[optnum].name);
	len = strlen(save_info[optnum].name) + 6;

	i = 0;
	while (i < weedcount && strcasecmp(weedlist[i], "*end-of-defaults*") != 0)
	    i++;
	while (i < weedcount && strcasecmp(weedlist[i], "*end-of-defaults*") == 0)
	    i++;
	if (i == 1) {
	    /* end-of-defaults in the first position means
	    ** that there are no defaults, i.e.
	    ** a clear-weed-list has been done.
	    */
	    fprintf(fp_elmrc, " \"*clear-weed-list*\"");
	    len += 20;
	}

	while (i <= weedcount) {
	    w = (i < weedcount) ? weedlist[i] : "*end-of-user-headers*";
	    if (strlen(w) + len > 72) {
		if (local_value)
		    fprintf(fp_elmrc, "\n\t");
		else
		    fprintf(fp_elmrc, "\n###\t");
		len = 8;
	    } else {
		fprintf(fp_elmrc, " ");
		++len;
	    }
	    fprintf(fp_elmrc, "\"%s\"", w);
	    len += strlen(w) + 3;
	    i++;
	}
	putc('\n', fp_elmrc);
	break;

    case DT_INC:
	fprintf(fp_elmrc, "%s%s =",
		(local_value ? "" : "### "), save_info[optnum].name);
	len = strlen(save_info[optnum].name) + 6;

	i = 0;
	while (i <= magiccount) {
	    w = (i < magiccount) ? magiclist[i] : "*end-of-incoming-folders*";
	    if (strlen(w) + len > 72) {
		if (local_value)
		    fprintf(fp_elmrc, "\n\t");
		else
		    fprintf(fp_elmrc, "\n###\t");
		len = 8;
	    } else {
		fprintf(fp_elmrc, " ");
		++len;
	    }
	    fprintf(fp_elmrc, "\"%s\"", w);
	    len += strlen(w) + 3;
	    i++;
	}
	putc('\n', fp_elmrc);
	break;

    case DT_ALT:
	len = 0;
	if (!local_value)
	    fprintf(fp_elmrc, "### ");
	fprintf(fp_elmrc, "%s =", save_info[optnum].name);
	len = strlen(save_info[optnum].name) + 6;
	for (alts = *SAVE_INFO_ALT(optnum) ; alts ; alts = alts->next) {
	    if (strlen(alts->address) + len > 72) {
		if (local_value)
		    fprintf(fp_elmrc, "\n\t");
		else
		    fprintf(fp_elmrc, "\n###\t");
		len = 8;
	    }
	    else {
		fprintf(fp_elmrc, " ");
		++len;
	    }
	    fprintf(fp_elmrc, "%s", alts->address);
	    len += strlen(alts->address);
	}
	fprintf(fp_elmrc,"\n");
	break;

    }

    if (optval != NULL) {
	fprintf(fp_elmrc, "%s%s = %s\n",
	    (local_value ? "" : "### "), save_info[optnum].name, optval);
    }

}


/*
 * find_opt() - Return the index of saved option "optname" (or -1 on error).
 *
 * Does a quick binary search through save_info[] for the indicated option.
 * This assumes that save_info[] is sorted by option name.  It's supposed
 * to be, and if it isn't we are going to be hosed badly.
 */
static int find_opt(const char *optname)
{
    int min_idx, max_idx, idx, d;

    min_idx = 0;
    max_idx = NUMBER_OF_SAVEABLE_OPTIONS-1;

    while (min_idx <= max_idx) {
	idx = (min_idx+max_idx)/2;
	if ((d = strcmp(optname, save_info[idx].name)) == 0)
	    return idx;
	if (d < 0)
	    max_idx = idx-1;
	else
	    min_idx = idx+1;
    }

    return -1;
}


/*
 * str_opt() - Return printable/saveable option value.
 *
 * Returns "*ERROR*" on error (bad option number or error in option type).
 *
 * The "optnum" is an option number, i.e. an index from save_info[].
 * The "longname" selects either a descriptive string or a code value
 *	for the options that support it.
 *
 * A possible point of confusion is that the sort description strings
 * themselves have long and short values.  When we are asked for the
 * long version here, we actually return the short name.  (When we
 * are asked for the short version here, we return a number.)
 */
static char *str_opt(int optnum, int longname)
{
    static char buf[SLEN];
    char *retval, *t;

    /* initialize to failure */
    retval = strcpy(buf, "*ERROR*");

    if (optnum < 0 || optnum >= NUMBER_OF_SAVEABLE_OPTIONS)
	return retval;

    switch (save_info[optnum].flags & DT_MASK) {

    case DT_STR:
	if (save_info[optnum].flags & FL_NOSPC) {
	    retval = strcpy(buf, SAVE_INFO_STR(optnum));
	    for (t = buf ; *t != '\0' ; ++t) {
		if (*t == ' ')
		    *t = '_';
	    }
	} else {
	    retval = SAVE_INFO_STR(optnum);
	}
	break;

    case DT_CHR:
	sprintf(retval = buf, "%c", *SAVE_INFO_CHR(optnum));
	break;

    case DT_BOL:
	retval = (*SAVE_INFO_BOL(optnum) ? "ON" : "OFF");
	break;

    case DT_SRT:
	if (longname)
	    retval = sort_name(FALSE);
	else
	    sprintf(retval = buf, "%d", *SAVE_INFO_NUM(optnum));
	break;

    case DT_ASR:
	if (longname)
	    retval = alias_sort_name(FALSE);
	else
	    sprintf(retval = buf, "%d", *SAVE_INFO_NUM(optnum));
	break;

    case DT_NUM:
	if (longname && strcmp(save_info[optnum].name, "userlevel")==0)
	    retval = level_name(*SAVE_INFO_NUM(optnum));
	else
	    sprintf(retval = buf, "%d", *SAVE_INFO_NUM(optnum));
	break;

    default:
	/* huh - unknown type??  preserve error retval */
	break;

    }

    return retval;
}
