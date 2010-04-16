

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.5 $   $State: Exp $
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
 * $Log: answer.c,v $
 * Revision 1.5  1999/03/24  14:04:12  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.4  1996/03/14  17:30:06  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:41  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/04/20  21:02:04  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:40  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This program is a phone message transcription system, and
    is designed for secretaries and the like, to allow them to
    painlessly generate electronic mail instead of paper forms.

    Note: this program ONLY uses the local alias file, and does not
	  even read in the system alias file at all.

**/

#define INTERN
#include "elm_defs.h"
#include "s_answer.h"
#include "ndbz.h"

#define  ELM		"elm"		/* where the elm program lives */

int user_data;		/* fileno of user data file   */
DBZ *hash;		/* dbz file for same */

char *get_alias_address(), *get_token(), *strip_parens(), *shift_lower();

static char *quit_word, *exit_word, *done_word, *bye_word;

main(argc, argv)
int argc;
char *argv[];
{
	FILE *fd;
	char *address, buffer[LONG_STRING], tempfile[SLEN], *cp;
	char  name[SLEN], recip_name[SLEN], in_line[SLEN];
	int   msgnum = 0, eof, allow_name = 0, phone_slip = 0;
	int   ans_pid = getpid();

	initialize_common();
	
	quit_word = catgets(elm_msg_cat, AnswerSet, AnswerQuitWord, "quit");
	exit_word = catgets(elm_msg_cat, AnswerSet, AnswerExitWord, "exit");
	done_word = catgets(elm_msg_cat, AnswerSet, AnswerDoneWord, "done");
	bye_word = catgets(elm_msg_cat, AnswerSet, AnswerByeWord, "bye");
/*
 *	simplistic crack arguments, looking for -u/-p
 *	-u = allow user names not in alias table
 *	-p = prompt for phone slip messages
 */
	for (msgnum = 1; msgnum < argc; msgnum++) {
	  if (strcasecmp(argv[msgnum], "-u") == 0)
	    allow_name = 1;
	  if (strcasecmp(argv[msgnum], "-p") == 0)
	    phone_slip = 1;
	  if (strcasecmp(argv[msgnum], "-pu") == 0) {
	    allow_name = 1;
	    phone_slip = 1;
	  }
	  if (strcasecmp(argv[msgnum], "-up") == 0) {
	    allow_name = 1;
	    phone_slip = 1;
	  }
	}

	open_alias_file();

	while (1) {
	  if (msgnum > 9999) msgnum = 0;
	
	  printf("\n-------------------------------------------------------------------------------\n");

prompt:   printf(catgets(elm_msg_cat, AnswerSet, AnswerMessageTo, "\nMessage to: "));
	  if (fgets(recip_name, SLEN, stdin) == NULL) {
		putchar('\n');
		exit(0);
	  }
	  if(recip_name[0] == '\0')
	    goto prompt;
	  
	  cp = &recip_name[strlen(recip_name)-1];
	  if(*cp == '\n') *cp = '\0';
	  if(recip_name[0] == '\0')
		goto prompt;

	  if ((strcasecmp(recip_name, quit_word) == 0) ||
	      (strcasecmp(recip_name, exit_word) == 0) ||
	      (strcasecmp(recip_name, done_word) == 0) ||
	      (strcasecmp(recip_name, bye_word)  == 0))
	     exit(0);

	  if (translate(recip_name, name) == 0)
	    goto prompt;

	  address = get_alias_address(name, 1, 0);

	  if (address == NULL || strlen(address) == 0) {
	    if (allow_name)
	      address =  name;
	    else {
	      printf(catgets(elm_msg_cat, AnswerSet, AnswerSorryNotFound,
		     "Sorry, could not find '%s' [%s] in list!\n"),
		     recip_name, name);
	      goto prompt;
	    }
	  }

	  printf("address '%s'\n", address);

	  sprintf(tempfile, "%sans.%d.%d", default_temp, ans_pid, msgnum++);

	  if ((fd = fopen(tempfile,"w")) == NULL)
	    exit(printf(catgets(elm_msg_cat, AnswerSet, AnswerCouldNotOpenWrite,
			"** Fatal Error: could not open %s to write\n"),
		 tempfile));


	/** Enter standard phone message fields **/
	  if (phone_slip) {
	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerCaller, "Caller: "));
	    printf("\n%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerOf, "of:     "));
	    printf("%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerPhone, "Phone:  "));
	    printf("%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s\n",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerTelephoned, "TELEPHONED         - "));
	    printf("\n%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerCalledToSeeYou, "CALLED TO SEE YOU  - "));
	    printf("%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerWantsToSeeYou, "WANTS TO SEE YOU   - "));
	    printf("%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerReturnedYourCall, "RETURNED YOUR CALL - "));
	    printf("%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerPleaseCall, "PLEASE CALL        - "));
	    printf("%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerWillCallAgain, "WILL CALL AGAIN    - "));
	    printf("%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);

	    strcpy(buffer, catgets(elm_msg_cat, AnswerSet, AnswerUrgent, "*****URGENT******  - "));
	    printf("%s",buffer);
	    fflush(stdout);
	    fgets(in_line, SLEN, stdin);
	    if (strlen(in_line) > 1)
	      fprintf(fd,"%s%s",buffer,in_line);
	  }

	  printf(catgets(elm_msg_cat, AnswerSet, AnswerEnterMessage,
		"\n\nEnter message for %s ending with a blank line.\n\n"), 
		 recip_name);

	  fprintf(fd,"\n\n");

	  do {
	   printf("> ");
	   if (! (eof = (fgets(buffer, SLEN, stdin) == NULL))) 
	     fprintf(fd, "%s", buffer);
	  } while (! eof && strlen(buffer) > 1);
	
	  fclose(fd);
 
	  sprintf(buffer, catgets(elm_msg_cat, AnswerSet, AnswerElmCommand,
	     "( ( %s -s \"While You Were Out\" %s < %s ; %s %s) & ) > /dev/null"),
	     ELM, strip_parens(address), tempfile, remove_cmd, tempfile);

	  system(buffer);
	}
}

int
translate(fullname, name)
char *fullname, *name;
{
	/** translate fullname into name..
	       'first last'  translated to first_initial - underline - last
	       'initial last' translated to initial - underline - last
	    Return 0 if error.
	**/
	register int i, lastname = 0, len;

	for (i=0, len = strlen(fullname); i < len; i++) {

	  fullname[i] = tolower(fullname[i]);

	  if (fullname[i] == ' ') 
	    if (lastname) {
	      printf(catgets(elm_msg_cat, AnswerSet, AnswerCannotHaveMoreNames,
	      "** Can't have more than 'FirstName LastName' as address!\n"));
	      return(0);
	    }
	    else
	      lastname = i+1;
	
	}

	if (lastname) 
	  sprintf(name, "%c_%s", fullname[0], (char *) fullname + lastname);
	else
	  strcpy(name, fullname);

	return(1);
}

	    
open_alias_file()
{
	/** open the user alias file **/

	char fname[SLEN];

	sprintf(fname,  "%s/.elm/aliases", getenv("HOME")); 

	if ((hash = dbz_open(fname, O_RDONLY, 0)) == NULL) 
	  exit(printf("** Fatal Error: Could not open %s!\n", fname));

	if ((user_data = open(fname, O_RDONLY)) == -1) 
	  return;
}

char *get_alias_address(name, mailing, depth)
char *name;
int   mailing, depth;
{
	/** return the line from either datafile that corresponds 
	    to the specified name.  If 'mailing' specified, then
	    fully expand group names.  Returns NULL if not found.
	    Depth is the nesting depth, and varies according to the
	    nesting level of the routine.  **/

	static char buffer[VERY_LONG_STRING];
	static char sprbuffer[VERY_LONG_STRING];
	datum  key, value;
	int    loc;
	struct alias_disk_rec entry;

	name = shift_lower(name);
	key.dptr = name;
	key.dsize = strlen(name);
	value = dbz_fetch(hash, key);
	if (value.dptr == NULL)
	    return( (char *) NULL); /* not found */

	bcopy(value.dptr, (char *) &loc, sizeof(loc));
	loc -= sizeof(entry);
	lseek(user_data, loc, SEEK_SET);
	read(user_data, (char *) &entry, sizeof(entry));
	read(user_data, buffer, entry.length > VERY_LONG_STRING ? VERY_LONG_STRING : entry.length);
	if ((entry.type & GROUP) != 0 && mailing) {
	    if (expand_group(sprbuffer, buffer + entry.address,
			     depth) < 0)
		return NULL;
	} else {
	    sprintf(sprbuffer, "%s (%s)", buffer + entry.address,
		    buffer + entry.name);
	}
	return sprbuffer;
}

int expand_group(target, members, depth)
char *target;
char *members;
int   depth;
{
	/** given a group of names separated by commas, this routine
	    will return a string that is the full addresses of each
	    member separated by spaces.  Depth is the current recursion
	    depth of the expansion (for the 'get_token' routine) **/

	char   buf[VERY_LONG_STRING], *word, *address, *bufptr;

	strcpy(buf, members); 	/* parameter safety! */
	target[0] = '\0';	/* nothing in yet!   */
	bufptr = (char *) buf;	/* grab the address  */
	depth++;		/* one more deeply into stack */

	while ((word = (char *) get_token(bufptr, "!, ", depth)) != NULL) {
	  if ((address = (char *) get_alias_address(word, 1, depth)) == NULL) {
	    fprintf(stderr, catgets(elm_msg_cat, AnswerSet, AnswerNotFoundForGroup,
		"Alias %s not found for group expansion!\n"), word);
	    return -1;
	  }
	  else if (strcmp(target,address) != 0) {
	    sprintf(target + strlen(target), " %s", address);
	  }

	  bufptr = NULL;
	}
	return 0;
}

print_long(buffer, init_len)
char *buffer;
int   init_len;
{
	/** print buffer out, 80 characters (or less) per line, for
	    as many lines as needed.  If 'init_len' is specified, 
	    it is the length that the first line can be.
	**/

	register int i, loc=0, space, length, len; 

	/* In general, go to 80 characters beyond current character
	   being processed, and then work backwards until space found! */

	length = init_len;

	do {
	  if (strlen(buffer) > loc + length) {
	    space = loc + length;
	    while (buffer[space] != ' ' && space > loc + 50) space--;
	    for (i=loc;i <= space;i++)
	      putchar(buffer[i]);
	    putchar('\n');
	    loc = space;
	  }
	  else {
	    for (i=loc, len = strlen(buffer);i < len;i++)
	      putchar(buffer[i]);
	    putchar('\n');
	    loc = len;
	  }
	  length = 80;
	} while (loc < strlen(buffer));
}

/****
     The following is a newly chopped version of the 'strtok' routine
  that can work in a recursive way (up to 20 levels of recursion) by
  changing the character buffer to an array of character buffers....
****/

#define MAX_RECURSION		20		/* up to 20 deep recursion */

#undef  NULL
#define NULL			(char *) 0	/* for this routine only   */


char *get_token(string, sepset, depth)
char *string, *sepset;
int  depth;
{

	/** string is the string pointer to break up, sepstr are the
	    list of characters that can break the line up and depth
	    is the current nesting/recursion depth of the call **/

	register char	*p, *q, *r;
	static char	*savept[MAX_RECURSION];

	/** is there space on the recursion stack? **/

	if (depth >= MAX_RECURSION) {
	 fprintf(stderr, catgets(elm_msg_cat, AnswerSet, AnswerRecursionTooDeep,
		"Error: Get_token calls nested greater than %d deep!\n"),
			MAX_RECURSION);
	 exit(1);
	}

	/* set up the pointer for the first or subsequent call */
	p = (string == NULL)? savept[depth]: string;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, sepset);	/* skip leading separators */

	if (*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if ((r = strpbrk(q, sepset)) == NULL)	/* move past token */
		savept[depth] = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept[depth] = ++r;
	}
	return(q);
}
