

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.9 $   $State: Exp $
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
 * $Log: from.c,v $
 * Revision 1.9  1996/05/09  15:51:39  wfp5p
 * Alpha 10
 *
 * Revision 1.8  1996/03/14  17:30:08  wfp5p
 * Alpha 9
 *
 * Revision 1.7  1995/09/29  17:42:42  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.6  1995/09/11  15:19:35  wfp5p
 * Alpha 7
 *
 * Revision 1.5  1995/07/18  19:00:12  wfp5p
 * Alpha 6
 *
 * Revision 1.4  1995/06/08  13:41:38  wfp5p
 * A few mostly cosmetic changes
 *
 * Revision 1.3  1995/05/01  13:46:27  wfp5p
 * Changes to make frm -q look right
 *
 * Revision 1.2  1995/04/20  21:02:07  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:40  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** print out whom each message is from in the pending folder or specified
    one, including a subject line if available.

**/

#define INTERN
#include "elm_defs.h"
#include "elm_globals.h"
#include "mailfile.h"
#include "s_from.h"
#include "port_stat.h"

#define metachar(c)	(c == '=' || c == '+' || c == '%')

/* for explain(), positive and negative */
#define POS	1
#define NEG	0

/* defines for selecting messages by Status: */
#define NEW_MSG		0x1
#define OLD_MSG		0x2
#define READ_MSG	0x4
#define UNKNOWN		0x8

#define ALL_MSGS	0xf

/* exit statuses */
#define	EXIT_SELECTED	0	/* Selected messages present */
#define	EXIT_MAIL	1	/* Mail present, but no selected messages */
#define	EXIT_NO_MAIL	2	/* No messages at all */
#define	EXIT_ERROR	3	/* Error */

FILE *mailfile;

int   number = FALSE,	/* should we number the messages?? */
      veryquiet = FALSE,/* should we be print any output at all? */
      quiet = FALSE,	/* only print mail/no mail and/or summary */
      selct = FALSE,	/* select these types of messages */
      tidy  = FALSE,    /* tidy output with long 'from's */
      useMlists = FALSE,/* include "mailing list" info in the report */
      summarize = FALSE,/* print a summary of how many messages of each type */
      verbose = FALSE;	/* and should we prepend a header? */

int columns = 0;	/* columns on the screen */

char infile[SLEN];	/* current file name */

static int from_forwarded(char *buffer, char *who);
static int usage(char *prog);
static char *whos_mail(char *filename);
static int print_help(void);
static int read_headers(int user_mailbox, int *total_msgs, int *selected);
static char *explain(int selection, int how_to_say);


int main(int argc, char *argv[])
{
	char *cp;
	char *default_list[2];
	int  output_files = FALSE;
	int  user_mailbox = FALSE, doing_default_incoming, c;
        int TreatAsSpool = 0;
	int total_msgs = 0, selected_msgs = 0;
	int file_exists;
	struct stat statbuf;

	extern int optind;
	extern char *optarg;

	char *rawarg;

	initialize_common();

	/*
	 * check the first character of the command basename to
	 * use as the selection criterion.
	 */
	cp = argv[0] + strlen(argv[0]) - 1;
	while (cp != argv[0] && cp[-1] != '/')
	  cp--;
	switch (*cp) {
	  case 'n': selct |= NEW_MSG;  break;
	  case 'u':
	  case 'o': selct |= OLD_MSG;  break;
	  case 'r': selct |= READ_MSG; break;
	}

	while ((c = getopt(argc, argv, "lMhnQqSs:tv")) != EOF)
	  switch (c) {
	    case 'l': useMlists++;	break;
	    case 'n': number++;	break;
            case 'M': TreatAsSpool++;break;
	    case 'Q': veryquiet++;	break;
	    case 'q': quiet++;	break;
	    case 'S': summarize++; break;
	    case 't': tidy++;      break;
	    case 'v': verbose++;	break;
	    case 's': if (optarg[1] == '\0') {
			     switch (*optarg) {
			       case 'n':
			       case 'N': selct |= NEW_MSG;  break;
			       case 'o':
			       case 'O':
			       case 'u':
			       case 'U': selct |= OLD_MSG;  break;
			       case 'r':
			       case 'R': selct |= READ_MSG; break;
			       default:       usage(argv[0]);
					      exit(EXIT_ERROR);
			     }
			   } else if (strcasecmp(optarg,"new") == 0)
			     selct |= NEW_MSG;
			   else if (strcasecmp(optarg,"old") == 0)
			     selct |= OLD_MSG;
			   else if (strcasecmp(optarg,"unread") == 0)
			     selct |= OLD_MSG;
			   else if (strcasecmp(optarg,"read") == 0)
			     selct |= READ_MSG;
			   else {
			     usage(argv[0]);
			     exit(EXIT_ERROR);
			   }
			   break;
	    case 'h': print_help();
			   exit(EXIT_ERROR);
	    case '?': usage(argv[0]);
			   printf(catgets(elm_msg_cat,
					  FromSet,FromForMoreInfo,
				"For more information, type \"%s -h\"\n"),
				   argv[0]);
			   exit(EXIT_ERROR);
	  }

	if (quiet && verbose) {
	  fprintf(stderr,catgets(elm_msg_cat,FromSet,FromNoQuietVerbose,
				 "Can't have quiet *and* verbose!\n"));
	  exit(EXIT_ERROR);
	}

	if (veryquiet) {
	  if (freopen("/dev/null", "w", stdout) == NULL) {
	    fprintf(stderr,catgets(elm_msg_cat,FromSet,FromCantOpenDevNull,
			"Can't open /dev/null for \"very quiet\" mode.\n"));
	    exit(EXIT_ERROR);
	  }
	}

	/* default is all messages */
	if (selct == 0 || selct == (NEW_MSG|OLD_MSG|READ_MSG))
	  selct = ALL_MSGS;

#ifdef TIOCGWINSZ
	{
	  struct winsize w;
	  if (ioctl(1,TIOCGWINSZ,&w) != -1 && w.ws_col > 0)
	    columns = w.ws_col;
	}
#endif
	if (columns == 0) columns = 80;

	if ((argc -= optind) == 0) {
	    argc = 1;
	    argv = default_list;
	    default_list[0] = incoming_folder;
	    default_list[1] = NULL;
	    doing_default_incoming = TRUE;
	} else {
	    argv += optind;
	    doing_default_incoming = FALSE;
	}

	while (*argv != NULL) {

	  (void) strfcpy(infile, rawarg = *argv++, sizeof(infile));

	  if (argc > 1 && verbose)
	    printf("%s%s:\n", (output_files++ > 0 ? "\n" : ""), infile);

	  if (metachar(infile[0]) && expand(infile) == 0) {
	     fprintf(stderr,catgets(elm_msg_cat,
				    FromSet,FromCouldntExpandFilename,
				    "%s: couldn't expand filename %s!\n"),
		     argv[0], infile);
	     exit(EXIT_ERROR);
	  }

	  /* see if this is some user's mailbox */
	  user_mailbox = (
		TreatAsSpool
		|| streq(infile, incoming_folder)
		|| strncmp(infile, mailhome, strlen(mailhome)) == 0
	  );

	  /* pardon the oversimplification here */
	  file_exists = (stat(infile, &statbuf) == 0);
	  if (file_exists && !S_ISREG(statbuf.st_mode)) {
	    printf(catgets(elm_msg_cat,FromSet,FromNotRegularFile,
			   "\"%s\" is not a regular file!\n"), infile);
	    continue;
	  }

	  if ((mailfile = fopen(infile,"r")) == NULL) {

	    if (doing_default_incoming) {
		if (verbose)
		  printf(catgets(elm_msg_cat,FromSet,FromNoMail,"No mail.\n"));
		continue;
	    }

	    if (infile[0] == '/' || file_exists == TRUE)  {
	      printf(catgets(elm_msg_cat,FromSet,FromCouldntOpenFolder,
			     "Couldn't open folder \"%s\".\n"), infile);
	      continue;
	    }

	    /* only try mailhome if file not found */
	    sprintf(infile, "%s%s", mailhome, rawarg);
	    if ((mailfile = fopen(infile,"r")) == NULL) {
	      printf(catgets(elm_msg_cat,
			     FromSet,FromCouldntOpenFolderPlural,
			     "Couldn't open folders \"%s\" or \"%s\".\n"),
		     rawarg, infile);
	      continue;
	    }
	    user_mailbox = TRUE;

	  }

	  read_headers(user_mailbox, &total_msgs, &selected_msgs);

	  /*
	   * we know what to say; now we have to figure out *how*
	   * to say it!
	   */

	  /* no messages at all? */
	  if (total_msgs == 0) {
	    if (user_mailbox)
	      printf(catgets(elm_msg_cat,FromSet,FromStringNoMail,
			     "%s no mail.\n"), whos_mail(infile));
	    else
	      if (!summarize)
		printf(catgets(elm_msg_cat,FromSet,FromNoMesgInFolder,
			       "No messages in that folder!\n"));
	  }
	  else
	    /* no selected messages then? */
	    if (selected_msgs == 0) {
	      if (user_mailbox)
		printf(catgets(elm_msg_cat,FromSet,FromNoExplainMail,
			       "%s no%s mail.\n"), whos_mail(infile),
		       explain(selct,NEG));
	      else
		if (!summarize)
		  printf(catgets(elm_msg_cat,
				 FromSet,FromNoExplainMessages,
				 "No%s messages in that folder.\n"),
			 explain(selct,NEG));
	    }
	    else
	      /* there's mail, but we just want a one-liner */
	      if (quiet && !summarize) {
		if (user_mailbox)
		  printf(catgets(elm_msg_cat,FromSet,FromStringStringMail,
				 "%s%s mail.\n"), whos_mail(infile),
			 explain(selct,POS));
		else
		  printf(catgets(elm_msg_cat,FromSet,FromThereAreMesg,
				 "There are%s messages in that folder.\n"),
			  explain(selct,POS));
	      }
	  fclose(mailfile);

	} /* for each arg */

	/*
	 * return "shell true" (0) if there are selected messages;
	 * 1 if there are messages, but no selected messages;
	 * 2 if there are no messages at all.
	 */
	if (selected_msgs > 0)
	  exit(EXIT_SELECTED);
	else if (total_msgs > 0)
	  exit(EXIT_MAIL);
	else
	  exit(EXIT_NO_MAIL);
}

static int read_headers(int user_mailbox, int *total_msgs, int *selected)
{
	/** Read the headers, output as found.  User-Mailbox is to guarantee
	    that we get a reasonably sensible message from the '-v' option
	 **/

	struct header_rec hdr;
	char *buffer;
	char to_whom[SLEN], from_whom[SLEN], subject[SLEN];
	char who[SLEN];
	char outbuf[SLEN], *bp;
	char all_to[LONG_STRING];
	int in_header = FALSE, count = 0, selected_msgs = 0;
	int in_to_list = FALSE;
	int expect_header = 0;
	long content_length, last_offset;
	int status, i;
	int indent, width;
	int summary[ALL_MSGS];
	int line_bytes;
	struct mailFile mailFile;
	int flush_lines = FALSE;
	extern struct addrs patterns;
	extern struct addrs mlnames;
	static struct addrs allto;
	static struct addrs user;
	static char *to_me = NULL;
	static char *to_many = NULL;
	static char *cc_me = NULL;	/* not implemented */
	static int initialized = FALSE;
	FAST_COMP_DECLARE;

	if (!initialized) {
	  if (useMlists) {
	    mlist_init();
	    allto.len = allto.max = 0; allto.str = NULL;
	    user.len  = user.max  = 0; user.str = NULL;
	    mlist_push(&user, user_name);
	    /* search for to_me/to_many override */
	    for (i=0; i < patterns.len; i++) {
	      if (patterns.str[i] == NULL)
		continue;
	      if (strcmp(patterns.str[i], TO_ME_TOKEN) == 0) {
		to_me = mlnames.str[i];
	      }
	      else if (strcmp(patterns.str[i], TO_MANY_TOKEN) == 0) {
		to_many = mlnames.str[i];
	      }
	      else if (strcmp(patterns.str[i], CC_ME_TOKEN) == 0) {
		cc_me = mlnames.str[i];
	      }
	    }
	    if (to_me == NULL)   to_me   = TO_ME_DEFAULT;
	    if (to_many == NULL) to_many = TO_MANY_DEFAULT;
	    if (cc_me == NULL)   cc_me   = CC_ME_DEFAULT;
	  }
	  initialized = 1;
	}

	/* amount to indent subject if who is too long */
	indent = 22;	/* who field width plus blanks */
	if (number) indent += 5;
	if (useMlists) indent += 15;
	width = columns - indent;

	for (i=0; i<ALL_MSGS; i++)
	  summary[i] = 0;

	mailFile_attach(&mailFile, mailfile);

	while ((line_bytes = mailFile_gets(&buffer, &mailFile)) != 0) {
	  if (expect_header && buffer[0] == '\n') continue;
	  flush_lines = (buffer[line_bytes-1] != '\n');

	  /* preload first char of line for fast string comparisons */
	  fast_comp_load(buffer[0]);

	  if (fast_strbegConst(buffer, "From ") && real_from(buffer, &hdr)) {
	    strcpy(from_whom, hdr.from);
	    subject[0] = '\0';
	    to_whom[0] = '\0';
	    all_to[0] = '\0';
	    in_header = TRUE;
	    expect_header = FALSE;
	    content_length = 0;
	    if (user_mailbox)
	      status = NEW_MSG;
	    else
	      status = READ_MSG;
	  }
	  else if (expect_header) {
	    /* didn't find a header where we expected, so go back */
	    /* and search for a new header */
	    dprint(1, (debugfile, "Error -- didn't find a header: %s\n", buffer));
	    mailFile_seek(&mailFile, last_offset);
	    expect_header=FALSE;
	    continue;
	  }
	  else if (in_header) {
	    if (!isspace(buffer[0]))
	      in_to_list = FALSE;
		    if (fast_strbegConst(buffer,">From "))
  	     from_forwarded(buffer, from_whom); /* return address */
	    else if (fast_header_cmp(buffer,"Subject", (char *)NULL) ||
		     fast_header_cmp(buffer,"Re", (char *)NULL)) {
	      if (subject[0] == '\0') {
	        remove_header_keyword(buffer);
		 strncpy(subject, buffer, sizeof(subject)-1);
		 subject[sizeof(subject)-1] = '\0';
	      }
	    }
	    else if (fast_header_cmp(buffer,"From", (char *)NULL))
	      parse_arpa_who(buffer+5, from_whom);
	    else if (fast_header_cmp(buffer, ">From", (char *)NULL))
	      parse_arpa_who(buffer+6, from_whom);
	    else if (fast_header_cmp(buffer, "To", (char *)NULL)) {
	      strfcat(all_to, buffer+3, LONG_STRING);
	      figure_out_addressee(buffer+3, user_name, to_whom);
	      in_to_list = TRUE;
	    }
	    else if (useMlists && fast_header_cmp(buffer, "Apparently-To", NULL)) {
	      strfcat(all_to, buffer+14, LONG_STRING);
	      in_to_list = TRUE;
	    }
	    else if (useMlists && fast_header_cmp(buffer, "Cc", (char *)NULL)) {
	      strfcat(all_to, buffer+3, LONG_STRING);
	      in_to_list = TRUE;
	    }
	    else if (fast_header_cmp(buffer, "Status", (char *)NULL)) {
	      remove_header_keyword(buffer);
	      switch (*buffer) {
		case 'N': status = NEW_MSG;	break;
		case 'O': status = OLD_MSG;	break;
		case 'R': status = READ_MSG;	break;
		default:  status = UNKNOWN;	break;
	      }
	      if (buffer[0] == 'O' && buffer[1] == 'R')
		status = READ_MSG;
	    }
	    else if (fast_header_cmp(buffer, "Content-Length", (char *)NULL)) {
	      remove_header_keyword(buffer);
	      content_length = atoi(buffer);
	    }
	    else if (useMlists && isspace(buffer[0]) && in_to_list)
		strfcat(all_to, buffer, LONG_STRING);
	    else if (buffer[0] == '\n') {
	      in_header = FALSE;
	      count++;
	      summary[status]++;
	      if (content_length > 0) {
		last_offset = mailFile_tell(&mailFile);
		mailFile_seek(&mailFile, last_offset + content_length);
		expect_header = TRUE;
	      }

	      if ((status & selct) != 0) {

		/* what a mess! */
		if (verbose && selected_msgs == 0) {
		  if (user_mailbox) {
		    if (selct == ALL_MSGS)
		      printf(catgets(elm_msg_cat,FromSet,FromFollowingMesg,
				     "%s the following mail messages:\n"),
			      whos_mail(infile));
		    else
		      printf(catgets(elm_msg_cat,FromSet,FromStringStringMail,
				     "%s%s mail.\n"), whos_mail(infile),
			     explain(selct,POS));
		  }
		  else
		    printf(catgets(elm_msg_cat,
				   FromSet,FromFolderContainsFollowing,
			"Folder contains the following%s messages:\n"),
			    explain(selct,POS));
		}

		selected_msgs++;
		if (! quiet) {
		  if (tail_of(from_whom, who, to_whom) == 1) {
		    char buf[SLEN];
		    strcpy(buf, "To ");
		    strcat(buf, who);
		    strcpy(who, buf);
		  }

		  bp = outbuf;

		  if (number) {
		    sprintf(bp, "%3d: ", count);
		    bp += strlen(bp);
		  }

		  if (useMlists) {
		    parseaddrs(all_to, &allto, FALSE);
		    if (addrmatch(&allto, &user) >= 0) {
		      if (allto.len == 1)
			sprintf(bp, "%-14.14s", to_me);
		      else
			sprintf(bp, "%-14.14s", to_many);
		    } else {
		      int match = addrmatch(&allto, &patterns);
		      if (match >= 0) {
			sprintf(bp, "%-14.14s", mlnames.str[match]);
		      }
		      else {
			if (allto.len > 0)
			  sprintf(bp, "(%-12.12s)", allto.str[0]);
			else
			  sprintf(bp, "***           ");
		      }
		    }
		    bp += strlen(bp);
		    *bp++ = ' ';
		  }

		  /***
		  *	Print subject on next line if the Who part blows
		  *	the alignment
		  ***/

		  sprintf(bp, "%-20s  ", who);
		  bp += strlen(bp);

		  if (tidy && strlen(who) > 20) {
		    outbuf[columns-1] = '\0';
		    puts(outbuf);
		    if (*subject != '\0') {
		      subject[columns-indent-1] = '\0';
		      printf("%*s%s\n", indent, "", subject);
		    }
		  }
		  else {
		    sprintf(bp, "%s", subject);
		    outbuf[columns-1] = '\0';
		    puts(outbuf);
		  }
		}
	      }
	    }
	  }
	  /* throw away lines until we get a NL */
	  if (flush_lines) {
	    do {
	      line_bytes = mailFile_gets(&buffer, &mailFile);
	    } while (line_bytes > 0 && buffer[line_bytes-1] != '\n');
	  }
	}
	mailFile_detach(&mailFile);

	*selected = selected_msgs;
	*total_msgs = count;

	/* print a message type summary */

	if (summarize) {
	  int output=FALSE, unknown = 0;

	  if (user_mailbox)
	    printf("%s ", whos_mail(infile));
	  else
	    printf(catgets(elm_msg_cat,FromSet,FromFolderContains,
			   "Folder contains "));

	  for (i=0; i<ALL_MSGS; i++) {
	    if (summary[i] > 0) {
	      if (output)
		printf(", ");
	      switch (i) {
		case NEW_MSG:
		case OLD_MSG:
		case READ_MSG:
		  printf("%d%s ",summary[i], explain(i,POS));
		  if (summary[i] == 1)
		       printf("%s",catgets(elm_msg_cat,
				   FromSet,FromMessage,"message"));
		  else
		       printf("%s",catgets(elm_msg_cat,
				   FromSet,FromMessagePlural,"messages"));

		  output = TRUE;
		  break;
		default:
		  unknown += summary[i];
	      }
	    }
	  }
	  if (unknown)
	  {
	       printf("%d ",unknown);

	       if (unknown == 1)
		    printf("%s",catgets(elm_msg_cat,
					FromSet,FromMessage,"message"));
	       else
		    printf("%s",catgets(elm_msg_cat,
					FromSet,FromMessagePlural,"messages"));

	       printf("%s "," of unknown status");
	       output = TRUE;
	  }

	  if (output)
	    printf(".\n");
	  else
	    printf(catgets(elm_msg_cat,FromSet,FromNoMessages,
                       "no messages.\n"));
	}
}


static int from_forwarded(char *buffer, char *who)
{
	/** change 'from' and date fields to reflect the ORIGINATOR of
	    the message by iteratively parsing the >From fields... **/

	char machine[SLEN], buff[SLEN], holding_from[SLEN];

	machine[0] = '\0';
	holding_from[0] = '\0';
	sscanf(buffer, "%*s %s %*s %*s %*s %*s %*s %*s %*s %*s %s",
	            holding_from, machine);

	if(machine[0] == '\0')	/* try for address with timezone in date */
	sscanf(buffer, "%*s %s %*s %*s %*s %*s %*s %*s %*s %s",
	            holding_from, machine);

	if (machine[0] == '\0') /* try for srm address */
	  sscanf(buffer, "%*s %s %*s %*s %*s %*s %*s %*s %s",
	            holding_from, machine);

	if (machine[0] == '\0')
	  sprintf(buff, holding_from[0] ? holding_from :
		  catgets(elm_msg_cat,FromSet,FromAnon, "anonymous"));
	else
	  sprintf(buff,"%s!%s", machine, holding_from);

	strncpy(who, buff, SLEN);
}

/*
 * Return an appropriate string as to whom this mailbox belongs.
 */
static char *whos_mail(char *filename)
{
	static char whos_who[SLEN];
	char *mailname;

	if (strncmp(filename, mailhome, strlen(mailhome)) == 0) {
	  mailname = filename + strlen(mailhome);
	  if (*mailname == '/')
	    mailname++;
	  if (strcmp(mailname, user_name) == 0)
	    strcpy(whos_who,catgets(elm_msg_cat,
				    FromSet,FromYouHave,"You have"));
	  else {
	    strcpy(whos_who, mailname);
	    strcat(whos_who,catgets(elm_msg_cat,FromSet,FromHas, " has"));
	  }
	}
	else
	/* punt... */
	     strcpy(whos_who,catgets(elm_msg_cat,
				     FromSet,FromYouHave,"You have"));

	return whos_who;
}

static int usage(char *prog)
{
     printf(catgets(elm_msg_cat,FromSet,FromUsage,
	"Usage: %s [-l] [-n] [-v] [-t] [-s {new|old|read}] [filename | username] ...\n"),
	    prog);
}

static int print_help(void)
{

     printf(catgets(elm_msg_cat,FromSet,FromHelpTitle,
 "frm -- list from and subject lines of messages in mailbox or folder\n"));

     usage("frm");
     printf(catgets(elm_msg_cat,FromSet,FromHelpText,
"\noption summary:\n\
-h\tprint this help message.\n\
-l\tinclude information about who each message is to (mailing list info).\n\
-n\tdisplay the message number of each message printed.\n\
-Q\tvery quiet -- no output is produced.  This option allows shell\n\
\tscripts to check frm's return status without having output.\n\
-q\tquiet -- only print summaries for each mailbox or folder.\n\
-S\tsummarize the number of messages in each mailbox or folder.\n\
-s status only -- select messages with the specified status.\n\
\t'status' is one of \"new\", \"old\", \"unread\" (same as \"old\"),\n\
\tor \"read\".  Only the first letter need be specified.\n\
-t\ttry to align subjects even if 'from' text is long.\n\
-v\tprint a verbose header.\n"));

}

/* explanation of messages visible after selection */
/* usage: "... has the following%s messages ...", explain(selct,POS) */

static char *explain(int selection, int how_to_say)
{
	switch (selection) {
	  case NEW_MSG:
	    return catgets(elm_msg_cat,FromSet,FromNew," new");
	  case OLD_MSG:
	    return catgets(elm_msg_cat,FromSet,FromUnread," unread");
	  case READ_MSG:
	    return catgets(elm_msg_cat,FromSet,FromRead," read");
	  case (NEW_MSG|OLD_MSG):
	    if (how_to_say == POS)
	      return catgets(elm_msg_cat,FromSet,FromNewAndUnread,
			     " new and unread");
	    else
	      return catgets(elm_msg_cat,FromSet,FromNewOrUnread,
			     " new or unread");
	  case (NEW_MSG|READ_MSG):
	    if (how_to_say == POS)
	      return catgets(elm_msg_cat,FromSet,FromNewAndRead,
			     " new and read");
	    else
	      return catgets(elm_msg_cat,FromSet,FromNewOrRead,
			     " new or read");
	  case (READ_MSG|OLD_MSG):
	    if (how_to_say == POS)
	      return catgets(elm_msg_cat,FromSet,FromReadAndUnread,
			     " read and unread");
	    else
	      return catgets(elm_msg_cat,FromSet,FromReadOrUnread,
			     " read or unread");
	  case ALL_MSGS:
	    return "";
	  default:
	    return catgets(elm_msg_cat,FromSet,FromUnknown," unknown");
	}
}
