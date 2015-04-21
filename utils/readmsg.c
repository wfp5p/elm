

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.7 $   $State: Exp $
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
 * $Log: readmsg.c,v $
 * Revision 1.7  1997/10/20  20:24:37  wfp5p
 * Incomingfolders no longer set Magic mode on for all remaining folders.
 *
 * Revision 1.6  1996/03/14  17:30:11  wfp5p
 * Alpha 9
 *
 * Revision 1.5  1995/09/29  17:42:49  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.4  1995/09/11  15:19:36  wfp5p
 * Alpha 7
 *
 * Revision 1.3  1995/06/01  13:13:33  wfp5p
 * Readmsg was fixed to work correctly if called from within elm.  From Chip
 * Rosenthal <chip@unicom.com>
 *
 * Revision 1.2  1995/04/20  21:02:10  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:39  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This program extracts messages from a mail folder.  It is particularly
    useful when the user is composting a mail response in an external
    editor.  He or she can call this program to pull a copy of the original
    message (or any other message in the folder) into the editor buffer.

    One of the first things we do is look for a folder state file.
    If we are running as a subprocess to Elm this file should tell us
    what folder is currently being read, the seek offsets to all the
    messages in the folder, and what message(s) in the folder is/are
    selected.  We will load in all that info and use it for defaults,
    as applicable.

    If a state file does not exist, then the default is to show whatever
    messages the user specifies on the command line.  Unless specified
    otherwise, this would be from the user's incoming mail folder.

    Even if a state file exists, the user can override the defaults
    and select a different set of messages to extract, or select an
    entirely different folder.

    Messages can be selected on the command line as follows:

	readmsg [-options] *

	    Selects all messages in the folder.

	readmsg [-options] pattern ...

	    Selects messsage(s) which match "pattern ...".  The selected
	    message will contain the "pattern ..." somewhere within
	    the header or body, and it must be an exact (case sensitive)
	    match.  Normally selects only the first match.  The "-a"
	    selects all matches.

	readmsg [-options] sel ...

	    where:  sel == a number -- selects that message number
		    sel == $ -- selects last message in folder
		    sel == 0 -- selects last message in folder

    The undocumented "-I" option is a kludge to deal with an Elm race
    condition.  The problem is that Elm does piping/printing/etc. by
    running "readmsg|command" and placing the mail message selection
    into a folder state file.  However, if the "command" portion of
    the pipeline craps out, Elm might regain control before "readmsg"
    completes.  The first thing Elm does is unlink the folder state
    file.  Thus "readmsg" can't figure out what to do -- there is no
    state file or command line args to select a message.  In this
    case, "readmsg" normally gives a usage diagnostic message.  The
    "-I" option says to ignore this condition and silently terminate.

**/

#define INTERN
#include "elm_defs.h"
#include "s_readmsg.h"

/* level of headers to display */
#define HDRSEL_ALL	1
#define HDRSEL_WEED	2
#define HDRSEL_NONE	3

#define metachar(c)	(c == '=' || c == '+' || c == '%')

/* increment for growing dynamic lists */
#define ALLOC_INCR	256

/* program name for diagnostics */
static char *prog;

/*
 * The "folder_idx_list" is a list of seek offsets into the folder,
 * indexed by message number.  The "folder_size" value indicates the
 * number of messages in the folder, as well as the length of the index.
 * This index will be used if we need to access messages by message
 * number.  If possible, the index will be loaded from the external state
 * file, otherwise we will have to make a pass through the entire folder
 * to build up the index.  The index is not needed if we are printing
 * everything ("*" specified on the command line) or if we are searching
 * for pattern match.
 *
 * The "folder_clen_list" contains the content lengths for the messages.
 * This information is available *only* if we were spawned from Elm and
 * the content lengths have been retrieved from the folder state file.
 * If this information is available, it will prevent truncated messages
 * due to embedded "From" lines.  If content length information is not
 * available, this list will be left NULL.
 */
static int folder_size;
static long *folder_idx_list;
static long *folder_clen_list;

/*
 * Information on a folder message.
 */
struct mssg_info {
	long folder_pos;	/* start of message seek position	*/
	long content_length;	/* message body length, or 0 if unknown	*/
};


/*
 * Header weeding information.
 */

#define DFLT_WEED_LIST	"Subject: From: To: Cc: Apparently- Date:"

struct weed_header_info {
    char *hdrname;	/* string to compare against header	*/
    int hdrlen;		/* characters in "hdrname"		*/
    int hdrskip;	/* skip (instead of accept) on match?	*/
    struct weed_header_info *next;
};

/* head of linked list of weeded headers */
static struct weed_header_info Weedlist;

/* is default to accept headers NOT in weed list? */
static int weed_dflt_accept = FALSE;


/*
 * local procedures
 */
void usage_error P_((void));
void malloc_fail_handler P_((const char *, unsigned));
void load_folder_index P_((FILE *));
int print_patmatch_mssg P_((FILE *, const char *, int, int, int));
int print_mssg P_((FILE *, const struct mssg_info *, int, int));
void weed_headers P_((const char *, int, FILE *));
void setup_weed_info P_((const char *));
int get_mssg_info P_((struct mssg_info *, int));


extern char *optarg;			/* for parsing the ...		*/
extern int   optind;			/*  .. starting arguments	*/

void usage_error(void)
{
    fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet, ReadmsgUsage,
	"Usage: %s [-anhp] [-f Folder] [-w weedlist] {MessageNum ... | pattern | *}\n"),
	prog);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct folder_state fstate;	/* information from external state file	*/
    char folder_name[SLEN];	/* pathname to the mail folder		*/
    int fstate_valid;		/* does "fstate" have valid info?	*/
    int hdr_disp_level;		/* amount of headers to show		*/
    int do_page_breaks;		/* true to formfeed between messages	*/
    int do_all_matches;		/* true to show all mssgs which match pat*/
    int ign_no_request;		/* terminate if no actions requested	*/
    int exit_status;		/* normally zero, set to one on error	*/
    FILE *fp;			/* file stream for opened folder	*/
    struct mssg_info minfo;	/* information on message to extract	*/
    char *sel_weed_str;		/* header weeding specification		*/
    char buf[SLEN];
    int i;

    initialize_common();

    prog = argv[0];
    folder_name[0] = '\0';	/* no folder specified yet		*/
    folder_size = -1;		/* message index not loaded yet		*/
    sel_weed_str = DFLT_WEED_LIST; /* default header weeding spec	*/
    hdr_disp_level = HDRSEL_WEED; /* only display interesting headers	*/
    do_page_breaks = FALSE;	/* suppress formfeed between mssgs	*/
    do_all_matches = FALSE;	/* only show 1st mssg which matches pat	*/
    ign_no_request = FALSE;	/* no action requested is an error	*/
    exit_status = 0;		/* will set nonzero on error		*/

    /* see if an external folder state file exists */
    if (load_folder_state_file(&fstate) != 0) {
	fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet,
	    ReadmsgStateFileCorrupt,
	    "%s: Elm folder state file appears to be corrupt!\n"), prog);
	exit(1);
    }
    fstate_valid = (fstate.folder_name != NULL);

    /* crack the command line */
    while ((i = getopt(argc, argv, "af:hnpw:I")) != EOF) {
	switch (i) {
	case 'a' :
	    do_all_matches = TRUE;
	    break;
	case 'f' :
	    strcpy(folder_name, optarg);
	    if (metachar(folder_name[0]) && !expand(folder_name)) {
		fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet,
		    ReadmsgCannotExpandFolderName,
		    "%s: Cannot expand folder name \"%s\".\n"),
		    prog, folder_name);
		exit(1);
	    }
	    folder_size = -1;	/* zot out index info from extern state file */
	    break;
	case 'h' :
	    hdr_disp_level = HDRSEL_ALL;
	    break;
	case 'n' :
	    hdr_disp_level = HDRSEL_NONE;
	    break;
	case 'p' :
	    do_page_breaks = TRUE;
	    break;
	case 'w':
	    hdr_disp_level = HDRSEL_WEED;
	    sel_weed_str = optarg;
	    break;
	case 'I':
	    ign_no_request = TRUE;
	    break;
	default:
	    usage_error();
	}
    }

    /* figure out where the folder is */
    if (folder_name[0] == '\0') {
	if (fstate_valid)
	    strcpy(folder_name, fstate.folder_name);
	else
	    strcpy(folder_name, incoming_folder);
    }

    /* open up the message folder */
    if ((fp = fopen(folder_name, "r")) == NULL) {
	fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet,
	    ReadmsgFolderEmpty, "%s: Folder \"%s\" is empty.\n"),
	    prog, folder_name);
	exit(0);
    }

    /* if this is the folder in the state file then grab the index */
    if (fstate_valid && strcmp(folder_name, fstate.folder_name) == 0) {
	folder_size = fstate.num_mssgs;
	folder_idx_list = fstate.idx_list;
	folder_clen_list = fstate.clen_list;
    } else {
	fstate_valid = FALSE;
    }

    /* process the header weeding specification */
    if (hdr_disp_level == HDRSEL_WEED)
	setup_weed_info(sel_weed_str);

    /* if no selections on cmd line then show selected mssgs from state file */
    if (argc == optind) {
	if (!fstate_valid) {
		/* no applicable state file or it didn't select anything */
		if (ign_no_request)
		    exit(0);
		usage_error();
	}
	if (folder_size < 1) {
	    fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet, ReadmsgFolderEmpty,
		"%s: Folder \"%s\" is empty.\n"), prog, folder_name);
	    exit(0);
	}
	for (i = 0 ; i < fstate.num_sel ; ++i) {
	    if (get_mssg_info(&minfo, fstate.sel_list[i]) != 0)
		exit_status = 1;
	    else if (print_mssg(fp, &minfo, hdr_disp_level, do_page_breaks)!=0)
		exit_status = 1;
	}
	exit(exit_status);
    }

    /* see if we are trying to match a pattern */
    if (strchr("0123456789$*", argv[optind][0]) == NULL) {
	strcpy(buf, argv[optind]);
	while (++optind < argc)
		strcat(strcat(buf, " "), argv[optind]);
	if (print_patmatch_mssg(fp, buf, do_all_matches,
			hdr_disp_level, do_page_breaks) != 0)
	    exit_status = 1;
	exit(exit_status);
    }

    /* if we do not have an index from the state file then go build one */
    if (folder_size < 0)
	load_folder_index(fp);

    /* make sure there is something there to look at */
    if (folder_size < 1) {
	fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet, ReadmsgFolderEmpty,
	    "%s: Folder \"%s\" is empty.\n"), prog, folder_name);
	exit(0);
    }

    /* see if all messages should be shown */
    if (argc-optind == 1 && strcmp(argv[optind], "*") == 0) {
	for (i = 1 ; i <= folder_size ; ++i) {
  	    if (get_mssg_info(&minfo, i) != 0)
		exit_status = 1;
	    else if (print_mssg(fp, &minfo, hdr_disp_level, do_page_breaks)!=0)
		exit_status = 1;
	}
	exit(exit_status);
    }

    /* print out all the messages specified on the command line */
    for ( ; optind < argc ; ++optind) {

	/* get the message number */
	if (strcmp(argv[optind], "$") == 0 || strcmp(argv[optind], "0") == 0) {
	    i = folder_size;
	} else if ((i = atoi(argv[optind])) == 0) {
	    fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet,
		ReadmsgIDontUnderstand,
		"%s: I don't understand what \"%s\" means.\n"),
		prog, argv[optind]);
	    exit(1);
	}

	/*print it */
	if (get_mssg_info(&minfo, i) != 0)
	    exit_status = 1;
	else if (print_mssg(fp, &minfo, hdr_disp_level, do_page_breaks)!=0)
	    exit_status = 1;
    }

    exit(exit_status);
    /*NOTREACHED*/
}


/*
 * Scan through the entire folder and build an index of seek offsets.
 */
void load_folder_index(FILE *fp)
{
    long offset;
    int alloc_size;
    char buf[SLEN];
    int buf_len, last_buf_len, was_empty_line;

    /* zero out the folder seek offsets index */
    folder_size = 0;
    alloc_size = 0;
    folder_idx_list = NULL;
    /* pretend there was an empty line before the first line of the mailbox */
    last_buf_len = 1;

    /* we are being lazy - we don't bother figuring out content lengths */
    folder_clen_list = NULL;

    /* go through the folder a line at a time looking for messages */
    rewind(fp);
    while (offset = ftell(fp), (buf_len = mail_gets(buf, sizeof(buf), fp)) != 0) {
	was_empty_line = last_buf_len == 1;
	last_buf_len = buf_len;

	if (was_empty_line && strbegConst(buf, "From ") &&
	    real_from(buf, (struct header_rec *)NULL))
	{
	    if (folder_size >= alloc_size) {
		alloc_size += ALLOC_INCR;
		folder_idx_list = (long *) safe_realloc((char *)folder_idx_list,
		    alloc_size*sizeof(long));
	    }
	    folder_idx_list[folder_size++] = offset;
	}
    }

}


/*
 * Scan through a folder and print message(s) which match the pattern.
 */
int print_patmatch_mssg(FILE *fp, const char *pat, int do_all_matches, int hdr_disp_level, int do_page_breaks)
{
    long offset;
    struct mssg_info minfo;
    char buf[VERY_LONG_STRING];
    int look_for_pat;
    int buf_len, last_buf_len, was_empty_line;

    /*
     * This flag ensures that we don't reprint a message if a single
     * message matches the pattern several times.  We turn it on when
     * we get to the top of a message.
     */
    look_for_pat = FALSE;
    /* pretend there was an empty line before the first line of the mailbox */
    last_buf_len = 1;

    rewind(fp);
    while (offset = ftell(fp), (last_buf_len = mail_gets(buf, sizeof(buf), fp)) != 0) {

	was_empty_line = last_buf_len == 1;
	last_buf_len = buf_len;
	if (was_empty_line && strbegConst(buf, "From ") &&
	    real_from(buf, (struct header_rec *)NULL))
	{
	    look_for_pat = TRUE;
	}

	if (look_for_pat && strstr(buf, pat) != NULL) {
	    minfo.folder_pos = ftell(fp);
	    minfo.content_length = 0L;		/* we don't know this */
	    if (print_mssg(fp, &minfo, hdr_disp_level, do_page_breaks) != 0)
		return -1;
	    fseek(fp, minfo.folder_pos, 0);
	    if (!do_all_matches)
		break;
	    look_for_pat = FALSE;
	}

    }

    return 0;
}


/*
 * Print the message at the indicated location.
 */
int print_mssg(FILE *fp, const struct mssg_info *minfo, int hdr_disp_level,
	       int do_page_breaks)
{
    char buf[SLEN];
    int first_line, is_seperator, in_header, buf_len, last_buf_len, newlines, was_empty_line;
    long curr_offset, stop_offset;
    static int num_mssgs_listed = 0;

    in_header = TRUE;
    first_line = TRUE;
    stop_offset = 0L;

    if (num_mssgs_listed++ == 0)
	; /* no seperator before first message */
    else if (do_page_breaks)
	putchar('\f');
    else
	puts("\n------------------------------------------------------------------------------\n");

    /* move to the beginning of the selected message */
    if (fseek(fp, minfo->folder_pos, 0) != 0) {
	fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet, ReadmsgCannotSeek,
	    "%s: Cannot seek to selected message. [offset=%ld]\n"),
	    prog, minfo->folder_pos);
	return -1;
    }

    /* we will chop off newlines at the end of the message */
    newlines = 0;
    /* pretend there was an empty line before the first line of the mailbox */
    last_buf_len = 1;

    /* print the message a line at a time */
    while (curr_offset = ftell(fp), (buf_len = mail_gets(buf, SLEN, fp)) != 0) {

	was_empty_line = last_buf_len == 1;
	last_buf_len = buf_len;

	if (stop_offset == 0L || curr_offset >= stop_offset) {
	    is_seperator = was_empty_line && strbegConst(buf, "From ") &&
		       real_from(buf, (struct header_rec *)NULL);

	} else {
	    is_seperator = FALSE;
	}

	/* the first line of the message better be the seperator */
	/* next time we encounter the seperator marks the end of the message */
	if (first_line) {
	    if (!is_seperator) {
		fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet,
		    ReadmsgCannotFindStart,
		    "%s: Cannot find start of selected message. [offset=%ld]\n"),
		    prog, minfo->folder_pos);
		return -1;
	    }
	    first_line = FALSE;
	} else {
	    if (is_seperator)
		break;
	}

	/* just accumulate newlines */
	if (buf[0] == '\n' && buf[1] == '\0') {
	    if (in_header) {
		if (minfo->content_length > 0)
		    stop_offset = curr_offset + minfo->content_length;
		in_header = FALSE;
	    }
	    ++newlines;
	    continue;
	}

	if (in_header) {
	    switch (hdr_disp_level) {
	    case HDRSEL_NONE:
		break;
	    case HDRSEL_WEED:
		weed_headers(buf, buf_len, stdout);
		break;
	    case HDRSEL_ALL:
	    default:
		fwrite(buf, 1, buf_len, stdout);
		break;
	    }
	} else {
	    while (--newlines >= 0)
		putchar('\n');
	    newlines = 0;
	    fwrite(buf, 1, buf_len, stdout);
	}

    }

    /* make sure we didn't do something like seek beyond the */
    /* end of the folder and thus never get anything to print */
    if (first_line) {
	fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet, ReadmsgCannotFindStart,
	    "%s: Cannot find start of selected message. [offset=%ld]\n"),
	    prog, minfo->folder_pos);
	return -1;
    }

    return 0;
}


/*
 * Weed out and display selected headers.
 */
void weed_headers(const char *buf, int buflen, FILE *fp)
{
    struct weed_header_info *w;
    static int accept_header = FALSE;

    /* if this is a new header, see if we want to accept it */
    if (!isspace(buf[0])) {
	accept_header = weed_dflt_accept;   
	for (w = Weedlist.next ; w != NULL ; w = w->next) {  
	    if (strncasecmp(buf, w->hdrname, w->hdrlen) == 0) {
		accept_header = !w->hdrskip;
		break;
	    }
	}
    }

    /* output accepted headers */
    if (accept_header)
	fwrite(buf, 1, buflen, fp);
}


/* 
 * Initialize the header weeding info.
 *
 * The header weeding info is a linked list of (struct weed_header_info).
 */
void setup_weed_info(const char *sel_weed_str)
{
    struct weed_header_info *w;
    char *weed_str, *fld, *s;

    /* make a copy we can scribble on safely */
    weed_str = safe_strdup(sel_weed_str);

    w = &Weedlist;
    while ((fld = strtok(weed_str, " \t\n,")) != NULL) {
	weed_str = NULL;

	/* convert "_" to " " */
	for (s = fld ; *s != '\0' ; ++s) {
	    if (*s == '_')
		*s = ' ';
	}

	/* add weeding info to end of list */
	w->next = (struct weed_header_info *)
	    safe_malloc(sizeof(struct weed_header_info));
	w = w->next;
	if (w->hdrskip = (*fld == '!'))
	    ++fld;
	w->hdrname = fld;
	w->hdrlen = strlen(fld);
	w->next = NULL;

	/*
	 * The default weed action is the opposite of the last list entry.
	 * That is, if the list ends in "foo" (an accept action) then the
	 * default is to reject things not in the weed list.  If the list
	 * ends in "!foo" (a reject action) then the default is to accept
	 * things not in the weed list.
	 *
	 * If this doesn't make sense to you, consider the following
	 * two example commands...and what you'd like them to do:
	 *
	 *	readmsg -w "From: Date: Subject:"
	 *	readmsg -w "!From_ !Received: !Status:"
	 *
	 */
	weed_dflt_accept = w->hdrskip;

    }

}


/*
 * Retrieve mailbox location information on a specific message.
 */
int get_mssg_info(struct mssg_info *minfo_p, int mssgno)
{
    if (mssgno < 1 || mssgno > folder_size) {
	fprintf(stderr, catgets(elm_msg_cat, ReadmsgSet,
	    ReadmsgCannotFindMessage, "%s: Cannot find message number %d.\n"),
	    prog, mssgno);
	return -1;
    }
    minfo_p->folder_pos = folder_idx_list[mssgno-1];
    minfo_p->content_length = 
	(folder_clen_list == NULL ? 0L : folder_clen_list[mssgno-1]);
    return 0;
}

