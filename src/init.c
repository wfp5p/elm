

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
 * $Log: init.c,v $
 * Revision 1.5  1999/03/24  14:04:00  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.4  1996/03/14  17:29:39  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:15  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:10  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:36  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"
#include "elm_globals.h"
#include "patchlevel.h"
#include "s_elm.h"

#ifdef I_TIME
#  include <time.h>
#endif
#ifdef I_SYSTIME
#  include <sys/time.h>
#endif

#include <sys/stat.h>

static void check_elm_dir(void);
static void check_folder_dir(void);
static int get_ynq(int);
static void create_private_dir(const char *);


#ifndef ANSI_C
char *getlogin();
unsigned short getgid(), getuid();
#endif

/* FOO - need to update message catalog */
static char def_ans_quit[] = "q";	/*FOO*/

void initialize(char *requestedmfile)
{
    char buf[SLEN];

    sprintf(version_buff, "%s", VERSION);
    def_ans_yes = catgets(elm_msg_cat, ElmSet, ElmYes, "y");
    def_ans_no = catgets(elm_msg_cat, ElmSet, ElmNo, "n");
    nls_deleted = catgets(elm_msg_cat, ElmSet, ElmTitleDeleted, "[deleted]");
    nls_form = catgets(elm_msg_cat, ElmSet, ElmTitleForm, "Form");
    nls_message = catgets(elm_msg_cat, ElmSet, ElmTitleMessage, "Message");
    nls_to = catgets(elm_msg_cat, ElmSet, ElmTitleTo, "To");
    nls_from = catgets(elm_msg_cat, ElmSet, ElmTitleFrom, "From");
    nls_page = catgets(elm_msg_cat, ElmSet, ElmTitlePage, "  Page %d");
    nls_item = catgets(elm_msg_cat, ElmSet, Elmitem, "message");
    nls_items = catgets(elm_msg_cat, ElmSet, Elmitems, "messages");
    nls_Item = catgets(elm_msg_cat, ElmSet, ElmItem, "Message");
    nls_Items = catgets(elm_msg_cat, ElmSet, ElmItems, "Messages");
    nls_Prompt = catgets(elm_msg_cat, ElmSet, ElmPrompt, "Command: ");

    /* install the error trap to take if xmalloc() or friends fail */
    safe_malloc_fail_handler = malloc_failed_exit;

    /* save original user and group ids */
    userid = getuid();
    groupid = getgid();
    mailgroupid = getegid();
    if (mailgroupid != groupid)
	setegid(groupid);

    /* make all newly created files private */
    original_umask = umask(077);

#ifdef DEBUG
    if (debug) {		/* setup for dprint() statements! */
	char newfname[SLEN], filename[SLEN];
	sprintf(filename, "%s/%s", user_home, DEBUGFILE);
	if (elm_access(filename, ACCESS_EXISTS) == 0) {
	    sprintf(newfname,"%s/%s", user_home, OLDEBUG);
	    (void) rename(filename, newfname);
	}
	if ((debugfile = fopen(filename, "w")) == NULL) {
	    debug = 0;	/* otherwise 'leave' will try to log! */
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet,
			ElmCouldNotOpenDebugFile,
			"Could not open file %s for debug output!\n"),
			filename);
	    exit(1);
	}
	chown(filename, userid, groupid);
	fprintf(debugfile,
"Debug output of the ELM program (at debug level %d).  Version %s\n\n",
	      debug, version_buff);
    }
#endif

    /* load in the system- and user- rc files */
    read_rc_file();

    /* verify required directories exist */
    sprintf(buf, "%s/.elm", user_home);
    if (access(buf, ACCESS_EXISTS) != 0) {
	fprintf(stderr, "\
\n\
Notice:  ELM requires an \".elm\" subdirectory off your home directory\n\
to hold information such as your configuration preferences (the\n\
\"elmrc\" file) and aliases.\n");
	create_private_dir(buf);
    }
    if (access(folders, ACCESS_EXISTS) != 0) {
	fprintf(stderr, "\
\n\
Notice:  ELM requires a \"%s\" directory\n\
in which to store your mail folders.\n", folders);
	create_private_dir(folders);
    }

    /*
     * Locking problems are common when Elm is misconfigured/misinstalled.
     * Sometimes, rather than fixing the problem, people will set
     * Elm setuid=root.  This is *extremely* dangerous.  Make sure
     * this isn't happening.  This test needed to be deferred until
     * after the system rcfile was read in.
     */
    if (getuid() != geteuid() && !allow_setuid) {
	fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmInstalledSetuid,
"\n\
This version of Elm has been installed setuid=%d.  This is dangerous!\n\
Elm is NOT designed to run in this mode, and to do so can introduce\n\
grave security hazards.  See the description of \"allow_setuid\" in the\n\
Elm Reference Guide for further information.\n\n"),
		geteuid());
	exit(1);
    }

    /* figure out what folder we are reading and verify access */
    if (OPMODE_IS_READMODE(opmode)) {

	/* determine the mail file to read */
	if (*requestedmfile == '\0')
	    strcpy(requestedmfile, incoming_folder);
	else if (!expand_filename(requestedmfile))
	    exit(0);

	/* if "check_size" is set then we don't want to read empty mbox */
	if (opmode == OPMODE_READ_NONEMPTY) {
	    if (check_mailfile_size(requestedmfile) != 0)
		exit(0);
	}

	/* check for permissions only if not send only mode file */
	if (can_access(requestedmfile, READ_ACCESS) != 0) {
	    if (errno != ENOENT || !streq(requestedmfile, incoming_folder)) {
		fprintf(stderr, catgets(elm_msg_cat, ElmSet,
			    ElmCantOpenFolderRead,
			    "Can't open '%s' for reading! [%s]"),
			    requestedmfile, strerror(errno));
		putc('\n', stderr);
		exit(1);
	    }
	}

    }

    /*
     * Now that we are about to muck with the tty and mail folder,
     * it's a good time to get the signal handlers installed.
     */
    initialize_signals();

    /*
     * Initialize the keyboard and display.
     */
    if (OPMODE_IS_INTERACTIVE(opmode)) {
	if (InitScreen() < 0)
	    exit(1);
	Raw(ON);
	EnableFkeys(ON);
	ClearScreen();
    }

    /*
     * ==> From this point on, curses may be
     * ==> used and graceful exit is required.
     */

    /* final preparations to read mailbox */
    if (OPMODE_IS_READMODE(opmode)) {

	/* initialize the options menu */
	init_opts_menu();

	/* with mini menu, this allows for 5 messages */
	if (LINES < 18) {
	    ShutdownTerm();
	    error(catgets(elm_msg_cat, ElmSet, ElmWindowSizeTooSmall,
		    "The window is too small to run Elm!"));
	    exit(1);
	}

    }

#ifdef DEBUG
    if (debug >= 2 && debug < 10) {
	fprintf(debugfile, "\n\
host_name        = \"%s\"\n\
user_name        = \"%s\"\n\
user_fullname    = \"%s\"\n\
user_home        = \"%s\"\n\
editor           = \"%s\"\n\
recvd_mail       = \"%s\"\n\
filename         = \"%s\"\n\
folders          = \"%s\"\n\
printout         = \"%s\"\n\
sent_mail        = \"%s\"\n\
prefix           = \"%s\"\n\
shell            = \"%s\"\n\
local_signature  = \"%s\"\n\
remote_signature = \"%s\"\n\
\n",
host_name, user_name, user_fullname, user_home, editor, recvd_mail,
curr_folder.filename, folders, printout, sent_mail, prefixchars, shell,
local_signature, remote_signature);
    }
#endif

}

static void create_private_dir(const char *path)
{
    int ans, ch;

    if (!OPMODE_IS_INTERACTIVE(opmode)) {
	fputs("\nPlease run the ELM program interactively so I may set this up for you.\n", stderr);
	exit(1);
    }

    for (;;) {

	fprintf(stderr, "\nMay I create this directory for you (yes/no/quit) ? [%c] : ", *def_ans_yes);
	fflush(stderr);
	fflush(stdout);

	ans = getchar();
	ch = ans;
	while (ch != EOF && ch != '\r' && ch != '\n')
	    ch = getchar();

	if (ans == '\r' || ans == '\n')
	    ans = *def_ans_yes;

	if (ans == *def_ans_yes)
	    break;

	if (ans == *def_ans_no) {
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmDirNoticeNo,
		    "Very well, but you may run into difficulties later.\n"));
	    fflush(stderr);
	    if (sleepmsg > 0)
		sleep(sleepmsg * 2);
	    return;
	}

	if (ans == *def_ans_quit) {
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmDirNoticeQuit,
			"OK.  Bailing out of ELM.\n"));
	    exit(0);
	}

	fputs("\007Please answer \"yes\", \"no\", or \"quit.\"\n", stderr);

    }

    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmDirNoticeYes,
		"Great!  I'll create \"%s\" for you now.\n"), path);
    fflush(stderr);
    if (sleepmsg > 0)
	sleep(sleepmsg * 2);

    if (mkdir(path, 0700) != 0) {
	fprintf(stderr, "Cannot create \"%s\" directory. [%s]\r\n",
		path, strerror(errno));
	exit(1);
    }
    if (chown(path, userid, groupid) != 0) {
	fprintf(stderr, "Cannot set \"%s\" ownership. [%s]\r\n",
		path, strerror(errno));
	exit(1);
    }

}
