
static char rcsid[] = "@(#)$Id: args.c,v 1.8 1999/03/24 14:03:57 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.8 $   $State: Exp $
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
 * $Log: args.c,v $
 * Revision 1.8  1999/03/24  14:03:57  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.7  1997/10/20  20:24:30  wfp5p
 * Incomingfolders no longer set Magic mode on for all remaining folders.
 *
 * Revision 1.6  1996/05/09  15:51:16  wfp5p
 * Alpha 10
 *
 * Revision 1.5  1996/03/14  17:27:52  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1996/03/13  14:37:57  wfp5p
 * Alpha 9 before Chip's big changes
 *
 * Revision 1.3  1995/09/29  17:41:59  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/04/20  21:01:45  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** starting argument parsing routines for ELM system...

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "patchlevel.h"
#include "s_elm.h"

extern char *optarg;		/* optional argument as we go */
extern int   optind;			/* argnum + 1 when we leave   */

char *
parse_arguments(argc, argv, to_whom)
int argc;
char *argv[], *to_whom;
{
	char *bp;
	int i, len;
	static char req_mfile[SLEN];

	opmode = OPMODE_READ;
	sendmail_verbose = FALSE;
        TreatAsSpooled = FALSE;
	to_whom[0] = '\0';
	batch_subject = NULL;
	included_file = NULL;

        while ((i = getopt(argc, argv, "?acd:f:hi:kKMmr:Ss:tVvz")) != EOF) {
	   switch (i) {
	     case 'a' : arrow_cursor = TRUE;			break;
	     case 'c' : fprintf(stderr, catgets(elm_msg_cat, ElmSet,
				ElmUseCheckaliasInstead,
				"Error - the \"-c\" option is obsolete.\n\
				Use the \"checkalias\" command instead\n"));
		        exit(1);
		        break;
	     case 'd' : debug = atoi(optarg);			break;
	     case 'f' : strncpy(req_mfile, optarg, SLEN);	break;
	     case '?' : /*FALLTHRU*/
	     case 'h' : args_help();
	     case 'i' : included_file = optarg;
	     	        opmode = OPMODE_SEND;
	     	        break;
	     case 'k' : /* obsolete - was HP terminal in 2.4 */	break;
	     case 'K' : hp_softkeys = TRUE;			break;
             case 'M' : TreatAsSpooled = TRUE;			break;
	     case 'm' : mini_menu = FALSE;			break;
 	      case 'r' :  setelmrcName(optarg);break;
 	     case 'S' : opmode = OPMODE_SEND;			break;
	     case 's' : batch_subject = optarg;
	     	        opmode = OPMODE_SEND;
	     	        break;
	     case 't' : use_tite = FALSE;			break;
             case 'V' : sendmail_verbose = TRUE; 		break;
	     case 'v' : args_version();
	     case 'z' : opmode = OPMODE_READ_NONEMPTY;		break;
	    }
	 }

#ifndef DEBUG
	if (debug)
	  fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmArgsIngoringDebug,
"Warning: system created without debugging enabled - request ignored\n"));
	debug = 0;
#endif

	/* address arguments indicate we are sending */
	*(bp = to_whom) = '\0';
	len = SLEN;
	for ( ; optind < argc ; ++optind) {
	  i = strlen(argv[optind]);
	  if (1+i+1 > len) {
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet,
		    ElmArgsTooManyAddresses,
		    "\n\rToo many addresses, or addresses too long!\n\r"));
	    exit(1);
	  }
	  if (bp > to_whom) {
	    *bp++ = ' ';
	    --len;
	  }
	  (void) strcpy(bp, argv[optind]);
	  bp += i;
	  len -= i;
	  opmode = OPMODE_SEND;
	}

	if (!isatty(STDIN_FILENO)) {
	  if (opmode == OPMODE_SEND) {
	    opmode = OPMODE_SEND_BATCH;
	    if (batch_subject == NULL)
	      batch_subject = DEFAULT_BATCH_SUBJECT;
	    if (included_file != NULL) {
	      fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmArgsInclFileBatch,
		"\n\rCan't specify an included file in batch mode!\n\r"));
	      exit(1);
	    }
	  } else {
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmArgsNoFileBatch,
	    	    "\n\rMust specify a recipient in batch mode.\n\r"));
	    exit(1);
	  }
	}

	return req_mfile;
}

args_help()
{
	/**  print out possible starting arguments... **/

	fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmArgsHelp1,
	  "\nPossible Starting Arguments for ELM program:\n\n\r\
\targ\t\t\tMeaning\n\r\
\t -a \t\tArrow - use the arrow pointer regardless\n\r\
\t -c \t\tCheckalias - check the given aliases only\n\r\
\t -dn\t\tDebug - set debug level to 'n'\n\r\
\t -fx\t\tFolder - read folder 'x' rather than incoming mailbox\n\r\
\t -h \t\tHelp - give this list of options\n\r\
\t -ix\t\tInclude prepared file 'x' in edit buffer for send\n\r"));
	fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmArgsHelp2,
	  "\t -K \t\t- Enable use of HP 2622 softkeys\n\r\
\t -M \t\tMagic mode - treat all folders as spool files.\n\r\
\t -m \t\tMenu - Turn off menu, using more of the screen\n\r\
\t -rx \t\tRcfile - Use 'x' as the elmrc instead of the default\n\r\
\t -S \t\tSend-only mode\n\r\
\t -sx\t\tSubject 'x' - for batch mailing\n\r\
\t -t \t\tTiTe - don't use termcap/terminfo ti/te entries.\n\r\
\t -V \t\tEnable sendmail voyeur mode.\n\r\
\t -v \t\tPrint out ELM version information.\n\r\
\t -z \t\tZero - don't enter ELM if no mail is pending\n\r\
\n\n"));
	exit(1);
}

args_version()
{
	/** print out version information **/

	printf("\nElm Version and Identification Information:\n\n");
	printf("\tElm %s PL%s, of %s\n",VERSION,PATCHLEVEL,VERS_DATE);
	printf("\t(C) Copyright 1988-1999 USENET Community Trust\n");
	printf("\tBased on Elm 2.0, (C) Copyright 1986,1987 Dave Taylor\n");
	printf("\t----------------------------------\n");
	printf("\tConfigured %s\n", CONFIGURE_DATE);
	printf("\t----------------------------------\n");

#ifdef MMDF
	printf("\tUse MMDF Mail Transport Agent/Mailbox Delimiters: MMDF\n");
#else /* MMDF */
	printf("\tUse UNIX Mailbox Delimiters and %s Mail Transport Agent: not MMDF\n", mailer);
#endif /* MMDF */

#ifdef DONT_ADD_FROM
	printf("\tLet the MTA add the From: header: DONT_ADD_FROM\n");
#else /* DONT_ADD_FROM */
	printf("\tElm will add the From: header: not DONT_ADD_FROM\n");
#endif /* DONT_ADD_FROM */

	printf("\tFollowing mail spool locking protocols will be used:");
#ifdef USE_DOTLOCK_LOCKING
	printf(" USE_DOTLOCK_LOCKING (.lock)");
#endif
#ifdef USE_FLOCK_LOCKING
	printf(" USE_FLOCK_LOCKING");
#endif
#ifdef USE_FCNTL_LOCKING
	printf(" USE_FCNTL_LOCKING");
#endif
	printf("\n");

#ifdef USE_EMBEDDED_ADDRESSES
	printf("\tFrom: and Reply-To: addresses are good: USE_EMBEDDED_ADDRESSES\n");
#else /* USE_EMBEDDED_ADDRESSES */
	printf("\tFrom: and Reply-To: addresses ignored: not USE_EMBEDDED_ADDRESSES\n");
#endif /* USE_EMBEDDED_ADDRESSES */

#ifdef MIME_RECV
	printf("\tSupport Multipurpose Internet Mail Extensions: MIME_RECV\n");
#else /* MIME_RECV */
	printf("\tIgnore Multipurpose Internet Mail Extensions: not MIME_RECV\n");
#endif /* MIME_RECV */

#ifdef INTERNET
	printf("\tPrefers Internet address formats: INTERNET\n");
#else /* INTERNET */
	printf("\tInternet address formats not used: not INTERNET\n");
#endif /* INTERNET */

#ifdef DEBUG
	printf("\tDebug options are available: DEBUG\n");
#else /* DEBUG */
	printf("\tNo debug options are available: not DEBUG\n");
#endif /* DEBUG */

        printf("\tLib dir is: %s\n",system_help_dir);
   
#ifdef CRYPT
	printf("\tCrypt function enabled: CRYPT\n");
#else /* CRYPT */
	printf("\tCrypt function disabled: not CRYPT\n");
#endif /* CRYPT */

#ifdef ALLOW_MAILBOX_EDITING
	printf("\tMailbox editing included: ALLOW_MAILBOX_EDITING\n");
#else /* ALLOW_MAILBOX_EDITING */
	printf("\tMailbox editing not included: not ALLOW_MAILBOX_EDITING\n");
#endif /* ALLOW_MAILBOX_EDITING */

#ifdef ALLOW_STATUS_CHANGING
	printf("\tStatus changing included: ALLOW_STATUS_CHANGING\n");
#else /* ALLOW_STATUS_CHANGING */
	printf("\tStatus changing not included: not ALLOW_STATUS_CHANGING\n");
#endif /* ALLOW_STATUS_CHANGING */

#ifdef ALLOW_SUBSHELL
	printf("\tSubshell menu items included: ALLOW_SUBSHELL\n");
#else /* ALLOW_SUBSHELL */
	printf("\tSubshell menu items not included: not ALLOW_SUBSHELL\n");
#endif /* ALLOW_SUBSHELL */
   
#ifdef HAS_SETEGID
        printf("\tUse setegid(): HAS_SETEGID\n");
#else
        printf("\tUse setgid(): not HAS_SETEGID\n");
#endif
   

#ifdef ISPELL
	printf("\tSpell checking feature enabled: ISPELL\n");
	printf("\t\t(Default spelling checker is %s options '%s')\n", ISPELL_PATH, ISPELL_OPTIONS);
#else /* ISPELL */
	printf("\tSpell checking feature disabled: not ISPELL\n");
#endif /* ISPELL */

#ifdef ENABLE_CALENDAR
	printf("\tCalendar file feature enabled: ENABLE_CALENDAR\n");
	printf("\t\t(Default calendar file is %s)\n",dflt_calendar_file);
#else /* ENABLE_CALENDAR */
	printf("\tCalendar file feature disabled: not ENABLE_CALENDAR\n");
#endif /* ENABLE_CALENDAR */

	printf("\n\n");
	exit(1);

}

