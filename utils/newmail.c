
static char rcsid[] = "@(#)$Id: newmail.c,v 1.7 1996/03/14 17:30:10 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.7 $   $State: Exp $
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
 * $Log: newmail.c,v $
 * Revision 1.7  1996/03/14  17:30:10  wfp5p
 * Alpha 9
 *
 * Revision 1.6  1995/09/29  17:42:46  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.5  1995/07/18  19:00:13  wfp5p
 * Alpha 6
 *
 * Revision 1.4  1995/06/14  19:59:42  wfp5p
 * Changes for alpha 3.
 * Changed the #ifdef for defining UTIMBUF
 *
 * Revision 1.3  1995/05/30  18:27:31  wfp5p
 * Added fix for utime() from Chip Rosenthal <chip@unicom.com> .
 *
 * Revision 1.2  1995/04/20  21:02:09  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:41  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This is actually two programs folded into one - 'newmail()' and
    'wnewmail()'.  They perform essentially the same function, to
    monitor the mail arriving in a set of/a mailbox or folder, but
    newmail is designed to run in background on a terminal, and
    wnewmail is designed to have a window of its own to run in.

    The main difference is that wnewmail checks for mail more often.

    The usage parameters are:

	-i <interval>  		how often to check for mail
				(default: 60 secs if newmail,
					  10 secs if wnewmail)

	<filename>		name of a folder to monitor
				(can prefix with '+'/'=', or can
			 	default to the incoming mailbox)

	<filename>=prefix	file to monitor, output with specified
				prefix when mail arrives.

    If we're monitoring more than one mailbox the program will prefix
    each line output (if 'newmail') or each cluster of mail (if 'wnewmail')
    with the basename of the folder the mail has arrived in.  In the 
    interest of exhaustive functionality, you can also use the "=prefix"
    suffix (eh?) to specify your own strings to prefix messages with.

    The output format is either:

	  newmail:
	     >> New mail from <user> - <subject>
	     >> Priority mail from <user> - <subject>

	     >> <folder>: from <user> - <subject>
	     >> <folder>: Priority from <user> - <subject>

	  wnewmail:
	     <user> - <subject>
	     Priority: <user> - <subject>

	     <folder>: <user> - <subject>
	     <folder>: Priority: <user> - <subject>\fR

**/

#define INTERN
#include "elm_defs.h"
#include "s_newmail.h"
#include "port_stat.h"

#ifdef I_TIME
#  include <time.h>
#endif
#ifdef I_SYSTIME
#  include <sys/time.h>
#endif
#ifdef BSD
#  include <sys/timeb.h>
#endif

#ifdef I_UTIME
#  include <utime.h>
#endif
#ifdef I_SYSUTIME
#  include <sys/utime.h>
#endif


/**********
   Since a number of machines don't seem to bother to define the utimbuf
   structure for some *very* obscure reason.... 

   Suprise, though, BSD has a different utime() entirely...*sigh*
**********/

/*if defined (BSD) && !defined(UTIMBUF) */
#if !defined(UTIMBUF)
struct utimbuf {
        time_t  actime;         /** access time       **/ 
        time_t  modtime;        /** modification time **/
       };

#endif


#define LINEFEED		(char) 10
#define BEGINNING		0			/* seek fseek(3S) */
#define DEFAULT_INTERVAL	60

#define MAX_FOLDERS		25		/* max we can keep track of */

/*
 * The "read_headers()" and "show_header()" routines use a (struct header_rec)
 * to hold the header information.  This structure does not have a flag to
 * save the message priority status.  We don't need the "encrypted" flag in
 * the structure, so we will use that instead.
 */
#define priority encrypted

#define metachar(c)	(c == '+' || c == '=' || c == '%')

long  bytes();

struct folder_struct {
	  char		foldername[SLEN];
	  char		prefix[NLEN];
	  long		filesize;
	  int		access_error;
       } folders[MAX_FOLDERS] = {0};

int  interval_time,		/* how long to sleep between checks */
     debug = 0,			/* include verbose debug output?    */
     in_window = 0,		/* are we running as 'wnewmail'?    */
     total_folders = 0,		/* # of folders we're monitoring    */
     print_prefix = 0,		/* force printing of prefix	    */
     current_folder = 0;	/* struct pointer for looping       */
FILE	*fd = NULL;		/* fd to use to read folders	    */

#ifdef PIDCHECK
int  parent_pid;		/* See if sucide should be attempt  */
#endif /* PIDCHECK */

extern int errno;

#if defined(BSD) && !defined(UTIMBUF)
        time_t utime_buffer[2];         /* utime command */
#else
        struct utimbuf utime_buffer;    /* utime command */
#endif

static char	*no_subj,	/* Pointer to No subject text	*/
		*priority_to,	/* pointer to Priority to text	*/
		*priority_text,	/* pointer to priority text	*/
		*To_text,	/* pointer To to text		*/
		*priority_mail,	/* pointer to priority mail	*/
		*mail_text,	/* pointer to mail text		*/
		*to_text,	/* pointer to to text		*/
		*from_text;	/* pointer to from text		*/

main(argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int   optind;
	char *ptr;
	int c, i, done;
	long lastsize,
	     newsize;			/* file size for comparison..      */
	register struct folder_struct *cur_folder;

	initialize_common();

	/* Get the No subject string */

	no_subj = catgets(elm_msg_cat, NewmailSet, NewmailNoSubject,
	   "(No Subject Specified)");
	priority_to = catgets(elm_msg_cat, NewmailSet,
	   NewmailInWinPriorityTo, "Priority to ");
	priority_text = catgets(elm_msg_cat, NewmailSet,
	      NewmailInWinPriority, "Priority ");
	To_text = catgets(elm_msg_cat, NewmailSet, NewmailInWinTo, "To ");
	priority_mail = catgets(elm_msg_cat, NewmailSet,
	   NewmailPriorityMail, "Priority mail ");
	mail_text = catgets(elm_msg_cat, NewmailSet, NewmailMail, "Mail ");
	to_text = catgets(elm_msg_cat, NewmailSet, NewmailTo, "to ");
	from_text = catgets(elm_msg_cat, NewmailSet, NewmailFrom, "from ");

#ifdef PIDCHECK				/* This will get the pid that         */
	parent_pid = getppid();		/* started the program, ie: /bin/sh   */
					/* If it dies for some reason (logout)*/
#endif /* PIDCHECK */			/* Then exit the program if PIDCHECK  */

	interval_time = DEFAULT_INTERVAL;

	/** let's see if the first character of the basename of the
	    command invoked is a 'w' (e.g. have we been called as
	    'wnewmail' rather than just 'newmail'?)
	**/

	for (i=0, ptr=(argv[0] + strlen(argv[0])-1); !i && ptr > argv[0]; ptr--)
	  if (*ptr == '/') {
	    in_window = (*(ptr+1) == 'w');
	    i++;
	  }

	if (ptr == argv[0] && i == 0 && argv[0][0] == 'w')
	  in_window = 1;

	while ((c = getopt(argc, argv, "di:w")) != EOF) {
	  switch (c) {
	    case 'd' : debug++;					break;
	    case 'i' : interval_time = atoi(optarg);		break;
	    case 'w' : in_window = 1;				break;
	    default  : usage(argv[0]);				exit(1);
	 }
	}

	if (interval_time < 10)
	  if (interval_time == 1)
	    fprintf(stderr, catgets(elm_msg_cat, NewmailSet, NewmailShort,
"Warning: interval set to 1 second.  I hope you know what you're doing!\n"));
	  else
	    fprintf(stderr, catgets(elm_msg_cat, NewmailSet, NewmailShortPlur,
"Warning: interval set to %d seconds.  I hope you know what you're doing!\n"),
		interval_time);

	/* now let's parse the foldernames, if any are given */

	if (optind >= argc) /* get default */
	  add_default_folder();
	else {
	  while (optind < argc)
	    add_folder(argv[optind++]);
	  pad_prefixes();			/* for nice output...*/
	}
	if (total_folders > 1)
		print_prefix = 1;

#ifdef AUTO_BACKGROUND
	if (! in_window) {
	  if (fork())	    /* automatically puts this task in background! */
	    exit(0);

	  (void) signal(SIGINT, SIG_IGN);
	  (void) signal(SIGQUIT, SIG_IGN);
	}
#endif
	(void) signal(SIGHUP, SIG_DFL);

	if (in_window && ! debug)
	  printf(catgets(elm_msg_cat, NewmailSet, NewmailIncommingMail,
	      "Incoming mail:\n"));

	while (1) {

#ifdef PIDCHECK
	if ( kill(parent_pid,0))
		exit(0);
#else
#ifndef AUTO_BACKGROUND		/* won't work if we're nested this deep! */
	  if (getppid() == 1) 	/* we've lost our shell! */
	    exit(0);
#endif /* AUTO_BACKGROUND */
#endif /* PIDCHECK */

	  if (! isatty(1))	/* we're not sending output to a tty any more */
	     exit(0);

	  dprint(1, (debugfile, "\n----\n"));

	  for (i = 0; i < total_folders; i++) {

	    cur_folder = &folders[i];
	    dprint(1, (debugfile, "[checking folder #%d: %s]\n",
		i, cur_folder->foldername));

	    if ((newsize = bytes(cur_folder->foldername)) == 
	        cur_folder->filesize) 	/* no new mail has arrived! */
	    	continue;

	    if ((fd = fopen(cur_folder->foldername,"r")) == NULL) {
	      if (errno == EACCES) {
		cur_folder->access_error++;
		if (cur_folder->access_error > 5) {
		  fprintf(stderr, catgets(elm_msg_cat, NewmailSet,
		      NewmailErrNoPerm,
		      "\nPermission to monitor \"%s\" denied!\n\n"),
			 cur_folder->foldername);
		  sleep(5);
		  exit(1);
		}
	      }
	      continue;
	    }

	    if ((newsize = bytes(cur_folder->foldername)) > 
	        cur_folder->filesize) {	/* new mail has arrived! */

	      dprint(1, (debugfile,
		  "\tnew mail has arrived!  old size = %ld, new size=%ld\n",
		  cur_folder->filesize, newsize));

	      /* skip what we've read already... */

	      if (fseek(fd, cur_folder->filesize, 
			BEGINNING) != 0)
	        perror("fseek()");

	      cur_folder->filesize = newsize;

	      /* read and display new mail! */
	      if (read_headers(cur_folder) && ! in_window)
	        printf("\n\r");
	      /* try to set the file access times back, ignore
		 failures */

#if defined(BSD) && !defined(UTIMBUF)
	      utime(cur_folder->foldername, utime_buffer);
#else
              utime(cur_folder->foldername, &utime_buffer);
#endif
	    }
	    else {	/* file SHRUNK! */

	      cur_folder->filesize = bytes(cur_folder->foldername);
	      lastsize = cur_folder->filesize;
	      done     = 0;

	      while (! done) {
	        sleep(1);	/* wait for the size to stabilize */
	        newsize = bytes(cur_folder->foldername);
	        if (newsize != lastsize)
	          lastsize = newsize;
		else
	          done++;
	      } 
	        
	      cur_folder->filesize = newsize;
	    }
	    (void) fclose(fd);			/* close it and ...         */
	  }

	  sleep(interval_time);
	}
}

int
read_headers(cur_folder)
register struct folder_struct *cur_folder;
{
	/** read the headers, output as found given current_folder,
	    the prefix of that folder, and whether we're in a window
	    or not.
	**/

	struct header_rec hdr;		/* holds header info on curr mssg */
	char buffer[SLEN];		/* message line buffer		  */
	char *fld_name, *fld_val;	/* field name and value pointers  */
	register int in_header;		/* TRUE when processing a hdr	  */
	int count;			/* count of messages done	  */
	int init_header;		/* TRUE to reset hdr for new mssg */
#ifdef MMDF
	int newheader = 0;		/* count lead/trail ^A^A^A^A	  */
#endif /* MMDF */

	count = 0;
	init_header = TRUE;

	/*
	 * Go through every line of the mailbox.
	 */
	while (mail_gets(buffer, SLEN, fd) != 0) {

	  /*
	   * Initialize header record for a new message.
	   */
	  if (init_header) {
	    hdr.to[0] = '\0';
	    hdr.from[0] = '\0';
	    hdr.subject[0] = '\0';
	    hdr.priority = FALSE;
	    in_header = FALSE;
	    init_header = FALSE;
	  }

	  /*
	   * Search for the start of a message.
	   */
	  if (!in_header) {
	    in_header = 
#ifdef MMDF
	      (strcmp(buffer, MSG_SEPARATOR) == 0 && (newheader = !newheader));
#else
	      strbegConst(buffer, "From ") && real_from(buffer, &hdr);
#endif /* MMDF */
	    continue;
	  }

	  /*
	   * Look for blank line at the end of the message header.
	   */
	  if (buffer[0] == LINEFEED) {
	    count++;
	    show_header(&hdr, cur_folder);
	    init_header = TRUE;
	    continue;
	  }

#ifdef MMDF
	  /*
	   * With MMDF we still need to locate the "From_" header.
	   */
	  if (real_from(buffer, &hdr))
	    continue;
#endif /* MMDF */

	  if (header_cmp(buffer, "From", (char *) NULL))  {
	    parse_arpa_who(buffer+5, hdr.from);
	    continue;
	  }

	  /*
	   * Split up the header into a field name and field value.
	   */
	  fld_name = fld_val = buffer;
	  while (*fld_val != '\0' && !isspace(*fld_val))
	    ++fld_val;
	  if (*fld_val != '\0') {
	    *fld_val = '\0';
	    do {
	      ++fld_val;
	    } while (isspace(*fld_val));
	  }

	  if (istrcmp(fld_name, ">From") == 0) {
	    forwarded(fld_val, hdr.from);
	    continue;
	  }

	  if (istrcmp(fld_name, "To:") == 0) {
	    figure_out_addressee(fld_val, user_name, hdr.to);
	    continue;
	  }

	  if (istrcmp(fld_name, "Subject:") == 0) {
	    strfcpy(hdr.subject, fld_val, sizeof(hdr.subject));
	    continue;
	  }

	  if (istrcmp(fld_name, "Re:") == 0) {
	    if (hdr.subject[0] == '\0')
	      strfcpy(hdr.subject, fld_val, sizeof(hdr.subject));
	    continue;
	  }

	  if (istrcmp(fld_name, "Importance:") == 0) {
	    if (atoi(fld_val) >= 2)
	      hdr.priority = TRUE;
	    continue;
	  }

	  if (istrcmp(fld_name, "Priority:") == 0) {
	    if (strincmp(fld_val, "normal", 6) != 0 &&
	    		strincmp(fld_val, "non-urgent", 10) != 0)
	      hdr.priority = TRUE;
	    continue;
	  }

	  /*
	   * If we reach this point, it must be an unknown
	   * or boring header.  Skip it.
	   */

	}

	return(count);
}

add_folder(name)
char *name;
{
	/* add the specified folder to the list of folders...ignore any
	   problems we may having finding it (user could be monitoring
	   a mailbox that doesn't currently exist, for example)
	*/

	char *cp, buf[SLEN];

	if (current_folder >= MAX_FOLDERS) {
	  fprintf(stderr, catgets(elm_msg_cat, NewmailSet, NewmailErrMaxFolders,
            "Sorry, but I can only keep track of %d folders.\n"), MAX_FOLDERS);
	  exit(1);
	}

	/* now let's rip off the suffix "=<string>" if it's there... */

	for (cp = name + strlen(name); cp > name+1 && *cp != '=' ; cp--)
	  /* just keep stepping backwards */ ;

	/* if *cp isn't pointing to the first character we'e got something! */

	if (cp > name+1) {

	  *cp++ = '\0';		/* null terminate the filename & get prefix */

	  if (metachar(*cp)) cp++;

	  strcpy(folders[current_folder].prefix, cp);
	  print_prefix = 1;
	}
	else {			/* nope, let's get the basename of the file */
	  for (cp = name + strlen(name); cp > name && *cp != '/'; cp--)
	    /* backing up a bit... */ ;

	  if (metachar(*cp)) cp++;
	  if (*cp == '/') cp++;

	  strcpy(folders[current_folder].prefix, cp);
	}

	/* and next let's see what kind of weird prefix chars this user
	   might be testing us with.  We can have '+'|'='|'%' to expand
	   or a file located in the incoming mail dir...
	*/

	if (metachar(name[0]))
	  expand_filename(name, folders[current_folder].foldername);
	else if (access(name, 00) == -1) {
	  /* let's try it in the mail home directory */
	  sprintf(buf, "%s%s", mailhome, name);
	  if (access(buf, 00) != -1) 		/* aha! */
	    strcpy(folders[current_folder].foldername, buf);
	  else
	    strcpy(folders[current_folder].foldername, name);
	}
	else
	  strcpy(folders[current_folder].foldername, name);

	/* now let's try to actually open the file descriptor and grab
	   a size... */

	if ((fd = fopen(folders[current_folder].foldername, "r")) == NULL)
          if (errno == EACCES) {
	    fprintf(stderr, catgets(elm_msg_cat, NewmailSet, NewmailErrNoPerm,
		      "\nPermission to monitor \"%s\" denied!\n\n"),
			 folders[current_folder].foldername);
	    exit(1);
	  }

	folders[current_folder].filesize = 
	      bytes(folders[current_folder].foldername);

	/* and finally let's output what we did */

	  dprint(1, (debugfile, "folder %d: \"%s\" <%s> %s, size = %ld\n",
	      current_folder,
	      folders[current_folder].foldername,
	      folders[current_folder].prefix,
	      fd == NULL? "not found" : "opened",
	      folders[current_folder].filesize));

	if (fd != NULL) /* Close it only if we succeeded in opening it */
		(void) fclose(fd);

	/* and increment current-folder please! */

	current_folder++;
	total_folders++;
}

add_default_folder()
{

	/* this routine will add the users home mailbox as the folder
	 * to monitor.  Since there'll only be one folder we'll never
	 * prefix it either...
	 */
	strcpy(folders[0].foldername, incoming_folder);
	
	fd = fopen(folders[0].foldername, "r");
	folders[0].filesize = bytes(folders[0].foldername);

	dprint(1, (debugfile, "default folder: \"%s\" <%s> %s, size = %ld\n",
	      folders[0].foldername,
	      folders[0].prefix,
	      fd == NULL? "not found" : "opened",
	      folders[0].filesize));

	total_folders = 1;
	if (fd != NULL) /* Close it only if we succeeded in opening it */
	  fclose(fd);
}


forwarded(buffer, who)
char *buffer, *who;
{
	/** change 'from' and date fields to reflect the ORIGINATOR of 
	    the message by iteratively parsing the >From fields...
	    The leading >From will already be stripped off the line. **/

	char machine[SLEN], buff[SLEN];

	machine[0] = '\0';
	sscanf(buffer, "%s %*s %*s %*s %*s %*s %*s %*s %*s %s",
	            who, machine);

	if(machine[0] == '\0')	/* try for address with timezone in date */
	sscanf(buffer, "%s %*s %*s %*s %*s %*s %*s %*s %s",
	            who, machine);

	if (machine[0] == '\0') /* try for srm address */
	  sscanf(buffer, "%s %*s %*s %*s %*s %*s %*s %s",
	            who, machine);

	if (machine[0] == '\0')
	  sprintf(buff,"anonymous");
	else
	  sprintf(buff,"%s!%s", machine, who);

	strncpy(who, buff, SLEN);
}

show_header(hdr, cur_folder)
struct header_rec *hdr;
struct folder_struct *cur_folder;
{
	char from_line[SLEN];
	char prefix[SLEN];
	int used_to_line;

	if (hdr->from[0] == '\0')
	  strcpy(hdr->from, user_name);
	if (hdr->subject[0] == '\0')
	  strcpy(hdr->subject, no_subj);
	else {
	  used_to_line = strlen(hdr->subject) - 1;
	  if (hdr->subject[used_to_line] == '\n')
	    hdr->subject[used_to_line] = '\0'; /* strip off trailing new line, we add our own */
	}

	used_to_line = tail_of(hdr->from, from_line, hdr->to);
	prefix[0] = '\0';

	if (! in_window)
	  strcat(prefix, ">> ");

	if (print_prefix) {
	  strcat(prefix, cur_folder->prefix);
	  strcat(prefix, ": ");
	}

	if (in_window) {
	  if (hdr->priority && used_to_line)
	    strcat(prefix, priority_to);
	  else if (hdr->priority)
	    strcat(prefix, priority_text);
	  else if (used_to_line)
	    strcat(prefix, To_text);
	  printf("\007%s%s -- %s\n", prefix, from_line, hdr->subject);
	} else {
	  if (hdr->priority)
	    strcat(prefix, priority_mail);
	  else
	    strcat(prefix, mail_text);
	  if (used_to_line)
	    strcat(prefix, to_text);
	  else
	    strcat(prefix, from_text);
	  printf("\n\r%s%s - %s\n\r", prefix, from_line, hdr->subject);
	}
}

long
bytes(name)
char *name;
{
	/** return the number of bytes in the specified file.  This
	    is to check to see if new mail has arrived....  **/

	int ok = 1;
	extern int errno;	/* system error number! */
	struct stat buffer;

	if (stat(name, &buffer) != 0)
	  if (errno != 2) {
	    MCfprintf(stderr, catgets(elm_msg_cat, NewmailSet, NewmailErrFstat,
	      "Error %d attempting fstat on %s"), errno, name);
	    exit(1);
	  }
	  else
	    ok = 0;
	
	/* retain the access times for later use */

#if defined(BSD) && !defined(UTIMBUF)
        utime_buffer[0]     = buffer.st_atime;
        utime_buffer[1]     = buffer.st_mtime;
#else
        utime_buffer.actime = buffer.st_atime;
        utime_buffer.modtime= buffer.st_mtime;
#endif

	return(ok ? buffer.st_size : 0);
}


usage(name)
char *name;
{
	/* print a nice friendly usage message */

	fprintf(stderr, catgets(elm_msg_cat, NewmailSet, NewmailArgsHelp1,
"\nUsage: %s [-d] [-i interval] [-w] {folders}\n\
\targ\t\t\tMeaning\n\r\
\t -d  \tturns on debugging output\n\
\t -i D\tsets the interval checking time to 'D' seconds\n\
\t -w  \tforces 'window'-style output, and bypasses auto-background\n\n"),
	name);

	fprintf(stderr, catgets(elm_msg_cat, NewmailSet, NewmailArgsHelp2,
"folders can be specified by relative or absolute path names, can be the name\n\
of a mailbox in the incoming mail directory to check, or can have one of the\n\
standard Elm mail directory prefix chars (e.g. '+', '=' or '%%').\n\
Furthermore, any folder can have '=string' as a suffix to indicate a folder\n\
identifier other than the basename of the file\n\n"));

}


expand_filename(name, store_space)
char *name, *store_space;
{
	strcpy(store_space, name);
	if (expand(store_space) == 0) {
	  fprintf(stderr, catgets(elm_msg_cat, NewmailSet, NewmailErrExpand,
	    "Sorry, but I couldn't expand \"%s\"\n"),name);
	  exit(1);
	}
}

pad_prefixes()
{
	/** This simple routine is to ensure that we have a nice
	    output format.  What it does is whip through the different
	    prefix strings we've been given, figures out the maximum
	    length, then space pads the other prefixes to match.
	**/

	register int i, j, len = 0;

	for (i=0; i < total_folders; i++)
	  if (len < (j=strlen(folders[i].prefix)))
	    len = j;
	
	for (i=0; i < total_folders; i++)
	  for (j = strlen(folders[i].prefix); j < len; j++)
	    strcat(folders[i].prefix, " ");
}
