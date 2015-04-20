
/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.11 $   $State: Exp $
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
 * $Log: read_rc.c,v $
 * Revision 1.11  1999/03/24  14:04:03  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.10  1996/08/08  19:49:29  wfp5p
 * Alpha 11
 *
 * Revision 1.9  1996/03/14  17:29:46  wfp5p
 * Alpha 9
 *
 * Revision 1.8  1996/03/13  14:38:01  wfp5p
 * Alpha 9 before Chip's big changes
 *
 * Revision 1.7  1995/09/29  17:42:23  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.6  1995/09/11  15:19:25  wfp5p
 * Alpha 7
 *
 * Revision 1.5  1995/07/18  19:00:03  wfp5p
 * Alpha 6
 *
 * Revision 1.4  1995/06/30  14:56:27  wfp5p
 * Alpha 5
 *
 * Revision 1.3  1995/06/21  15:27:09  wfp5p
 * editflush and confirmtagsave are new in the elmrc (Keith Neufeld)
 * The mlist code has a little bug fix.
 *
 * Revision 1.2  1995/04/28  15:08:51  wfp5p
 * Added patch to allow embedded date in folder names (from Mike Kenney
 *   <mike@wavelet.apl.washington.edu>).  This allows:
 *
 *               %h      month name ( 3 letter abbreviation )
 *               %y      last 2 digits of year
 *               %m      month number
 *               %d      day of the month
 *               %j      day of the year
 *
 * This patch also allows environment vars to be enclosed in {}.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"
#include "elm_globals.h"
#include "mime.h"
#define SO_INTERN	/* so space declared in "save_opts.h" is created */
#include "save_opts.h"
#include "s_elm.h"
#include <time.h>

#define metachar(c)	(c == '+' || c == '%' || c == '=')
#define ok_rc_char(c)   (isalnum(c) || c == '-' || c == '_')


#define ASSIGNMENT      0
#define WEEDOUT		1
#define ALTERNATIVES	2

#define SYSTEM_RC	0
#define LOCAL_RC	1

static int lineno = 0;
static int errors = 0;

static void do_expand_env(char *descr, char *dest, char *src, unsigned destlen)
{
    if (expand_env(dest, src, destlen) != 0) {
	ShutdownTerm();
	show_error(catgets(elm_msg_cat, ElmSet, ElmCannotInitErrorExpanding,
	    "Cannot initialize \"%s\" - error expanding \"%s\"."),
	    descr, src);
	leave(LEAVE_ERROR);
    }
}

int read_rc_file(void)
{
	/** this routine does all the actual work of reading in the
	    .rc file... **/

	FILE *file;
	char buffer[SLEN], filename[SLEN], *cp;


	/* Establish some defaults in case elmrc is incomplete or not there.
	 * Defaults for other elmrc options were established in their
	 * declaration - in elm.h.  And defaults for sent_mail and recvd_mail
	 * are established after the elmrc is read in since these default
	 * are based on the folders directory name, which may be given
	 * in the emrc.
	 * Also establish alternative_editor here since it is based on
	 * the default editor and not on the one that might be given in the
	 * elmrc.
	 */

	default_weedlist();

	errors = 0;
	alternative_addresses = NULL; 	/* none yet! */

	raw_local_signature[0] = raw_remote_signature[0] =
		local_signature[0] = remote_signature[0] =
		raw_recvdmail[0] = raw_sentmail[0] =
		allowed_precedences[0] = '\0';
		/* no defaults for those */


	strncpy(raw_shell, (( (cp = getenv("SHELL")) == NULL)? default_shell : cp ), SLEN);
	do_expand_env("shell", shell, raw_shell, sizeof(shell));
	if (shell[0] != '/') {
	   sprintf(buffer, "/bin/%s", shell);
	   strcpy(shell, buffer);
	}

	strncpy(raw_pager, (((cp = getenv("PAGER")) == NULL)? default_pager : cp), SLEN);
	do_expand_env("pager", pager, raw_pager, sizeof(pager));

	strncpy(raw_temp_dir, ((cp = getenv("TMPDIR")) ? cp : default_temp), SLEN);
	do_expand_env("temp_dir", temp_dir, raw_temp_dir, sizeof(temp_dir));
	if (temp_dir[strlen (temp_dir)-1] != '/')
	    strcat(temp_dir, "/");

	strncpy(raw_editor, (((cp = getenv("EDITOR")) == NULL)? default_editor:cp), SLEN);
	strcpy(alternative_editor, raw_editor);
	do_expand_env("editor", editor, raw_editor, sizeof(editor));

	strcpy(raw_printout, default_printout);
	do_expand_env("printout", printout, raw_printout, sizeof(printout));

	sprintf(raw_folders, "~/%s", default_folders);
	do_expand_env("folders", folders, raw_folders, sizeof(folders));

	strcpy(e_editor, emacs_editor);
	strcpy(v_editor, default_editor);

	strcpy(raw_printout, default_printout);
	strcpy(printout, raw_printout);

	sprintf(raw_folders, "%s/%s", user_home, default_folders);
	strncpy(folders, raw_folders, SLEN);

	strcpy(to_chars, default_to_chars);

	strcpy(charset, default_charset);

#ifdef MIME_RECV
	strcpy(charset_compatlist, COMPAT_CHARSETS);
	strcpy(display_charset, default_display_charset);
#endif


	/* try system-wide rc file */
        if ((file = fopen(system_rc_file, "r")) != NULL) {
	  do_rc(file, SYSTEM_RC);
	  fclose(file);
	}

	/* Look for the elmrc file */
/*	sprintf(filename, "%s/%s", user_home, elmrcfile); */
        getelmrcName(filename,SLEN);

	if ((file = fopen(filename, "r")) == NULL) {
	  dprint(2, (debugfile, "Warning:User has no \".elm/elmrc\" file\n\n"));
	} else {
  	  do_rc(file, LOCAL_RC);
	  fclose(file);
	}

/* validate/correct config_options string */

	if (config_options[0]) {
	    register char *s, *t;
	    register opts_menu *o;
	    s = shift_lower(config_options);
	    for (t = config_options; *s; ++s) {
		if (*s == '_' || *s == '^') {
		    *t++ = *s;
		    continue;
		}
		o = find_cfg_opts(*s);
		if (o != NULL)
		    *t++ = *s; /* silently remove invalid options */
		}
	    *t = '\0';
	}
	do_expand_env("folders", folders, raw_folders, sizeof(folders));

	do_expand_env("temp_dir", temp_dir, raw_temp_dir, sizeof(temp_dir));
  	if (temp_dir[strlen (temp_dir)-1] != '/')
  	    strcat(temp_dir, "/");

	do_expand_env("shell", shell, raw_shell, sizeof(shell));

	do_expand_env("editor", editor, raw_editor, sizeof(editor));

	do_expand_env("calendar_file", calendar_file, raw_calendar_file,
	    sizeof(calendar_file));

	do_expand_env("printout", printout, raw_printout, sizeof(printout));

	do_expand_env("pager", pager, raw_pager, sizeof(pager));
	if (streq(pager, "builtin+") || streq(pager, "internal+"))
		clear_pages++;

	do_expand_env("local_signature", local_signature,
	    raw_local_signature, sizeof(local_signature));
	do_expand_env("remote_signature", remote_signature,
	    raw_remote_signature, sizeof(remote_signature));

	if (streq(local_signature, remote_signature) &&
	    (streq(shift_lower(local_signature), "on") ||
	    streq(shift_lower(local_signature), "off"))) {
	    errors++;

	    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmSignatureObsolete,
	"\"signature\" used in obsolete way in .elm/elmrc file. Ignored!\n\r\
\t(Signature should specify the filename to use rather than on/off.)\n\r\n"));

	    raw_local_signature[0] = raw_remote_signature[0] =
		local_signature[0] = remote_signature[0] = '\0';
	}

	allow_forms = (allow_forms?MAYBE:NO);

	if (bounceback > MAX_HOPS) {
	    errors++;
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmBouncebackGTMaxhops,
	"Warning: bounceback is set to greater than %d (max-hops). Ignored.\n\r"),
		     MAX_HOPS);
	    bounceback = 0;
	}

	if ((timeout != 0) && (timeout < 10)) {
	    errors++;
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmTimeoutLTTen,
		 "Warning: timeout is set to less than 10 seconds. Ignored.\n\r"));
	    timeout = 0;
	}

	if (readmsginc < 1) {
		errors++;
		fprintf(stderr, catgets(elm_msg_cat, ElmSet,
		    ElmReadMessageIncrement,
		    "Warning: readmsginc is set to less than 1.  Ignored.\n\r"));
		readmsginc = 1;
	}

	if (sleepmsg < 0) {
		errors++;
		fprintf(stderr, catgets(elm_msg_cat, ElmSet,
		    ElmSleepMessageInvalid,
		    "Warning: sleepmsg is set to less than 0.  Setting to 0.\n\r"));
		sleepmsg = 0;
	}

	/* If recvd_mail or sent_mail havent't yet been established in
	 * the elmrc, establish them from their defaults.
	 * Then if they begin with a metacharacter, replace it with the
	 * folders directory name.
	 */
	if(*raw_recvdmail == '\0') {
	  strcpy(raw_recvdmail, default_recvdmail);
	}

	do_expand_env("recvd_mail", recvd_mail, raw_recvdmail,
	    sizeof(recvd_mail));

	if(metachar(recvd_mail[0])) {
	  strcpy(buffer, &recvd_mail[1]);
	  sprintf(recvd_mail, "%s/%s", folders, buffer);
	}

	if(*raw_sentmail == '\0') {
	  sprintf(raw_sentmail, default_sentmail);
	  sprintf(sent_mail, default_sentmail);
	}

	do_expand_env("sent_mail", sent_mail, raw_sentmail, sizeof(sent_mail));

	if(metachar(sent_mail[0])) {
	  strcpy(buffer, &sent_mail[1]);
	  sprintf(sent_mail, "%s/%s", folders, buffer);
	}

        expandFilenamesInMagicList();

	if (debug > 10) 	/** only do this if we REALLY want debug! **/
	  dump_rc_results();

}

int do_rc(FILE *file, int lcl)
{
	static int prev_type = 0;
	int x;
	char buffer[SLEN], word1[SLEN], word2[SLEN];

	if (!file) return;
	lineno=0;

	while (x = mail_gets(buffer, SLEN, file)) {
	    lineno++;
	    no_ret(buffer);	 	/* remove return */
	    if (buffer[0] == '#'        /* comment       */
	     || x < 2)     /* empty line    */
	      continue;

	    if(breakup(buffer, word1, word2) == -1)
	        continue;		/* word2 is null - let default value stand */

	    if(strcmp(word1, "warnings") == 0)
		continue;		/* grandfather old keywords */

	    strcpy(word1, shift_lower(word1));	/* to lower case */
	    x = do_set(file, word1, word2, lcl);

	    if (x == 0) {
		if (prev_type == DT_ALT) {
		    alternatives(buffer);
		} else if (prev_type == DT_WEE) {
		    weedout(buffer);
		}
	        else if (prev_type == DT_INC)
		 {
		    add_incoming(buffer);
		 }
	         else {
		    errors++;
		    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmBadSortKeyInElmrc,
"I can't understand sort key \"%s\" in line %d in your \".elm/elmrc\" file\n\r"),
			word1, lineno);
		}
	    } else
		prev_type = x;
	}
}

/*
 * set the named parameter according to save_info structure.
 * This routine may call itself (DT_SYN or DT_MLT).
 * Also tags params that were set in "local" (personal) RC file
 * so we know to save them back out in "o)ptions" screen.
 * Uses an internal table to decode sort-by params...should be coupled
 * with sort_name(), etc...but...
 */
int do_set(FILE *file, char *word1, char *word2, int lcl)
{
	register int x, y;

	for (x=0; x < NUMBER_OF_SAVEABLE_OPTIONS; ++x) {
	    y = strcmp(word1, save_info[x].name);
	    if (y <= 0)
		break;
	}

	if (y != 0)
	    return(0);

	if (save_info[x].flags & FL_SYS && lcl == LOCAL_RC)
	    return(0);

	if (lcl == LOCAL_RC)
	    save_info[x].flags |= FL_LOCAL;

	switch (save_info[x].flags & DT_MASK) {
	    case DT_SYN:
		if (*(SAVE_INFO_SYN(x)) == '!')	/* antonym */
		    return(do_set(file, SAVE_INFO_SYN(x) + 1,
			    is_it_on(word2) ? "OFF" : "ON", lcl));
		else				/* synonym */
		    return(do_set(file, SAVE_INFO_SYN(x), word2, lcl));

	    case DT_MLT:
		y=0;
		{
		    register char **s;
		    for (s = SAVE_INFO_MLT(x); *s; ++s)
			if (**s == '!')		/* negated multiple */
			    y |= do_set(file, *s + 1,
				    is_it_on(word2) ? "OFF" : "ON", lcl);
			else			/* asserted multiple */
			    y |= do_set(file, *s, word2, lcl);
		}

		return(y); /* we shouldn't "or" the values into "y" */

	    case DT_STR:
		strcpy(SAVE_INFO_STR(x), word2);
		if (save_info[x].flags & FL_NOSPC) {
		    register char *s;
		    for (s = SAVE_INFO_STR(x); *s; ++s)
			if (*s == '_') *s=' ';
		    }
		break;

	    case DT_CHR:
		*SAVE_INFO_CHR(x) = word2[0];
		break;

	    case DT_NUM:
		*SAVE_INFO_NUM(x) = atoi(word2);
		break;

	    case DT_BOL:
		if (save_info[x].flags & FL_OR)
		    *SAVE_INFO_BOL(x) |= is_it_on(word2);
		else if (save_info[x].flags & FL_AND)
		    *SAVE_INFO_BOL(x) &= is_it_on(word2);
		else
		    *SAVE_INFO_BOL(x) = is_it_on(word2);
		break;

	    case DT_SRT:
		{ static struct { char *kw; int sv; } srtval[]={
		    {"sent", SENT_DATE},
		    {"received", RECEIVED_DATE},
		    {"recieved", RECEIVED_DATE},
		    {"rec", RECEIVED_DATE},
		    {"from", SENDER},
		    {"sender", SENDER},
		    {"size", SIZE},
		    {"lines", SIZE},
		    {"subject", SUBJECT},
		    {"mailbox", MAILBOX_ORDER},
		    {"folder", MAILBOX_ORDER},
		    {"status", STATUS},
		    {NULL, 0} };
		    char *s = word2;
		    int f;

		    f = 1;
		    strcpy(word2, shift_lower(word2));
		    if (strncmp(s, "rev-", 4) == 0 ||
			strncmp(s, "reverse-", 8) == 0) {
			f = -f;
			s = strchr(s, '-') + 1;
		    }

		    for (y= 0; srtval[y].kw; y++) {
			if (streq(s, srtval[y].kw))
			    break;
		    }
		    if (srtval[y].kw) {
			*SAVE_INFO_SRT(x) = f > 0 ? srtval[y].sv : -srtval[y].sv;
		    } else {
			errors++;
			fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmBadSortKeyInElmrc,
	  "I can't understand sort key \"%s\" in line %d in your \".elm/elmrc\" file\n\r"),
			    word2, lineno);
		    }
		}
		break;

	    case DT_ASR:
		{ static struct { char *kw; int sv; } srtval[]={
		    {"alias", ALIAS_SORT},
		    {"name", NAME_SORT},
		    {"text", TEXT_SORT},
		    {NULL, 0} };
		    char *s = word2;
		    int f;

		    f = 1;
		    strcpy(word2, shift_lower(word2));
		    if (strncmp(s, "rev-", 4) == 0 ||
			strncmp(s, "reverse-", 8) == 0) {
			f = -f;
			s = strchr(s, '-') + 1;
		    }

		    for (y= 0; srtval[y].kw; y++) {
			if (streq(s, srtval[y].kw))
			    break;
		    }
		    if (srtval[y].kw) {
			*SAVE_INFO_SRT(x) = f > 0 ? srtval[y].sv : -srtval[y].sv;
		    } else {
			errors++;
			fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmBadAliasSortInElmrc,
	"I can't understand alias sort key \"%s\" in line %d in your \".elm/elmrc\" file\n\r"),
			    word2, lineno);
		    }
		}
		break;

	    case DT_ALT:
		alternatives(word2);
		break;

	    case DT_WEE:
		weedout(word2);
		break;

	    case DT_INC:
	        add_incoming(word2);
	        break;


	    case DT_NOP:
		/*
		 * This is an obsolete option - ignored for backward
		 * compatiblity with old elmrc files.  Silently ignore it.
		 */
		break;

	    }

	return(save_info[x].flags & DT_MASK);
}

int weedout(char *string)
{
	/** This routine is called with a list of headers to weed out.   **/

	char *strptr, *header, *p;
	int finished;

	finished = FALSE;
	strptr = string;
	while (!finished && (header = strtokq(strptr, "\t ,", TRUE)) != NULL) {
	  strptr = NULL;

	  if (!*header)
	    continue;

	  for (p = header; *p; ++p) {
	    if (*p == '_')
	      *p = ' ';
	  }

	  if (! strcasecmp(header, "*end-of-user-headers*"))
	    break;

	  if (! strcasecmp(header, "*end-of-defaults*"))
	    finished = TRUE;

	  if (! strcasecmp(header, "*clear-weed-list*")) {
	    while (weedcount)
	      free(weedlist[--weedcount]);
	    header = "*end-of-defaults*";
	  }

	  if (matchInList(weedlist,weedcount,header,TRUE))
	    continue;

	  if (weedcount > MAX_IN_WEEDLIST) {
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmTooManyWeedHeaders,
		   "Too many weed headers!  Leaving...\n\r"));
	    exit(1);
	  }
	  weedlist[weedcount++] = safe_strdup(header);
	}
}

int alternatives(char *string)
{
	/** This routine is called with a list of alternative addresses
	    that you may receive mail from (forwarded) **/

	char *strptr, *address;
	struct addr_rec *current_record, *previous_record;

	previous_record = alternative_addresses;	/* start 'er up! */
	/* move to the END of the alternative addresses list */

	if (previous_record != NULL)
	  while (previous_record->next != NULL)
	    previous_record = previous_record->next;

	strptr = (char *) string;

	while ((address = strtok(strptr, "\t ,\"'")) != NULL) {
	  if (previous_record == NULL) {
	    previous_record = (struct addr_rec *)
		safe_malloc(sizeof(struct addr_rec));

	    strcpy(previous_record->address, address);
	    previous_record->next = NULL;
	    alternative_addresses = previous_record;
	  }
	  else {
	    current_record = (struct addr_rec *)
		safe_malloc(sizeof(struct addr_rec));

	    strcpy(current_record->address, address);
	    current_record->next = NULL;
	    previous_record->next = current_record;
	    previous_record = current_record;
	  }
	  strptr = (char *) NULL;
	}
}

int add_incoming(char *string)
{
  /** This routine is called with a list of folder names **/

  char *strptr,*f;
  char filename[SLEN];
  int finished;

  finished = FALSE;
  strptr = string;

  while ((f = strtokq(strptr, "\t ,", TRUE)) != NULL)
  {
     strptr = NULL;

     if (!*f)
      continue;

    if (!strcasecmp(f, "*end-of-incoming-folders*"))
      break;

    strcpy (filename, f);

    if (matchInList(magiclist,magiccount,filename,FALSE))
      continue;

/* 7! need to put an error call if magiccount > MAX_IN_WEEDLIST */

    magiclist[magiccount++] =  safe_strdup(filename);

    }

}

int expandFilenamesInMagicList(void)
{

   int i;
   char filename[SLEN];

   for (i=0; i < magiccount; i++)
   {
      strcpy(filename,magiclist[i]);
      expand_filename(filename);
      free(magiclist[i]);
      magiclist[i] = safe_strdup(filename);
   }
}

int default_weedlist(void)
{
	/** Install the default headers to weed out!  Many gracious
	    thanks to John Lebovitz for this dynamic method of
	    allocation!
	**/

	static char *default_list[] = { ">From", "In-Reply-To:",
		       "References:", "Newsgroups:", "Received:",
		       "Apparently-To:", "Message-Id:", "Content-Type:",
		       "Content-Length", "MIME-Version",
		       "Content-Transfer-Encoding",
		       "From", "X-Mailer:", "Status:",
		       "*end-of-defaults*", NULL
		     };

	for (weedcount = 0; default_list[weedcount] != (char *) 0;weedcount++)
	  weedlist[weedcount] = safe_strdup(default_list[weedcount]);
}

int matchInList(char *list[], int count, const char *buffer, int ignoreCase)
{
   	/** returns true iff the first 'n' characters of 'buffer'
	    match an entry of the list **/

	register int i;

	for (i=0;i < count; i++)
        {
	   if (ignoreCase)
	   {
	      if (strncasecmp(buffer, list[i], strlen(list[i])) == 0)
	         return(1);
	   }
	   else
 	   {
	      if (strncmp(buffer, list[i], strlen(list[i])) == 0)
	         return(1);
           }
	}

	return(0);
}

int breakup(char *buffer, char *word1, char *word2)
{
	/** This routine breaks buffer down into word1, word2 where
	    word1 is alpha characters only, and there is an equal
	    sign delimiting the two...
		alpha = beta
	    For lines with more than one 'rhs', word2 is set to the
	    entire string.
	    Return -1 if word 2 is of zero length, else 0.
	**/

	register int i;

	for (i=0;buffer[i] != '\0' && ok_rc_char(buffer[i]); i++)
	  if (buffer[i] == '_')
	    word1[i] = '-';
	  else
	    word1[i] = tolower(buffer[i]);

	word1[i++] = '\0';	/* that's the first word! */

	/** spaces before equal sign? **/

	while (whitespace(buffer[i])) i++;
	if (buffer[i] == '=') i++;

	/** spaces after equal sign? **/

	while (whitespace(buffer[i])) i++;

	if (buffer[i] != '\0')
	  strcpy(word2, (char *) (buffer + i));
	else
	  word2[0] = '\0';

	/* remove trailing spaces from word2! */
	i = strlen(word2) - 1;
	while(i && (whitespace(word2[i]) || word2[i] == '\n'))
	  word2[i--] = '\0';

	return(*word2 == '\0' ? -1 : 0 );

}


static char *monthnames[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

static char *daynames[7] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};


/*
**
** Read the environment variable from the source buffer (src) to the
** destination buffer (dst).  Variable ends with a null character, a
** closing brace ( } ), a path separator ('/'), or a percent sign ( % ).
**
** Returns:  number of characters read from source
**
*/
static int read_env_var(register char *dst, const char *src)
{
    register int	nread = 0;

    while(*src && !(*src == '/' || *src == '}' || *src == '%'))
    {
  	*dst++ = *src++;
	nread++;
    }

    *dst = 0;
    if(*src == '}')	/* Advance past the closing brace */
	nread++;

    return nread;
}

/*
** Copy a string from the source buffer (src) to the destination buffer
** (dst) performing various expansions along the way:
**
**   Source string		-->		Destination string
**   -------------				------------------
**
**	$VAR					  value of $VAR from
**						  the environment
**
**	${VAR}					  same as above.
**
**	~					  user's home directory
**
**	%h					  current month name
**						  (3 letter abbreviation)
**
**	%y					  current year (modulo 100)
**
**      %Y                                        current year
**
**	%m					  current month number (01-12)
**
**	%j					  day of the year (001-366)
**
**	%d					  day of the month (01-31)
**
**
** Returns:  0 if successful, -1 if error occurs
**
** Example:
**
**	JUNK = bar in the current environment:
**	Current date = 1/1/95
**
**	~/$JUNK/mail.%h%y  -->  /usr/whoever/bar/mail.Jan95
**
**      if you want to embed "$JUNK" within a string, use '{}':
**
**	~/${JUNK}xx/mail.%h%y  -->  /usr/whoever/barxx/mail.Jan95
**
**
*/
int expand_env(register char *dst, const char *src, int len)
{
    int			n;
    register char	*var, *env;
    time_t		now;
    struct tm		*tm;


    /*
    ** Buffer for environment variables
    */
    var = (char *) safe_malloc(strlen(src) + 1);

    time(&now);
    tm = localtime(&now);

    len--;	/* Leave room for trailing '\0' */

    while(*src)
    {
	switch(*src)
	{

	    /*
	    ** Environment variable expansion
	    */
	    case '$':
		src++;
	 	if(*src == '{')
		    src++;
	        n = read_env_var(var, src);
		src += n;
	        if((env = getenv(var)) == 0)
	        {
	            free(var);
	            return -1;
	        }
	        n = strlen(env);
	        strncpy(dst, env, (n > len ? len : n));
	        dst += n;
	        len -= n;
		break;
	    case '%':
		src++;

		/*
		** Handle date expansion
		*/
	        switch(*src)
	        {
		    case 'h':	/* Month name */
		        strncpy(dst, monthnames[tm->tm_mon],
                           (3 > len ? len : 3));
		        dst += 3;
		        len -= 3;
		        break;
		    case 'a':	/* Day of the week */
		        strncpy(dst, daynames[tm->tm_wday],
                           (3 > len ? len : 3));
		        dst += 3;
		        len -= 3;
		        break;
		    case 'y':	/* Last 2 digits of the year */
		        if(len < 2)
		        {
			    free(var);
			    return -1;
		        }
		        sprintf(dst, "%02d", tm->tm_year % 100);
		        dst += 2;
		        len -= 2;
  	                break;
		     case 'Y':	/* Last 4 digits of the year */
		        if(len < 4)
		        {
			    free(var);
			    return -1;
		        }
		        sprintf(dst, "%04d", tm->tm_year+1900);
		        dst += 4;
		        len -= 4;
		        break;
		    case 'm':	/* Month number (1-12) */
		        if(len < 2)
		        {
			    free(var);
			    return -1;
		        }
		        sprintf(dst, "%02d", tm->tm_mon+1);
		        dst += 2;
		        len -= 2;
		        break;
		    case 'd':	/* Day of the month */
		        if(len < 2)
		        {
			    free(var);
			    return -1;
		        }
		        sprintf(dst, "%02d", tm->tm_mday);
		        dst += 2;
		        len -= 2;
		        break;
		    case 'j':	/* Julian day */
		        if(len < 3)
		        {
			    free(var);
			    return -1;
		        }
		        sprintf(dst, "%03d", tm->tm_yday+1);
		        dst += 3;
		        len -= 3;
		        break;
	            default:
		        len -= 2;
		        *dst++ = '%';
		        *dst++ = *src;
		        break;
	        }
	        src++;
		break;

	    /*
	    ** Home directory expansion
	    */
	    case '~':
	        if((env = getenv("HOME")) == 0)
	        {
	            free(var);
	            return -1;
	        }
	        n = strlen(env);
	        strncpy(dst, env, (n > len ? len : n));
	        dst += n;
	        len -= n;
	        src++;
		break;
	    default:
                len--;
                *dst++ = *src++;
		break;
	}
    }

    free(var);
    *dst = 0;
    return 0;
}


#define on_off(s)	(s == 1? "ON " : "OFF")
int dump_rc_results(void)
{
	register int i, j, len = 0;
	char buf[SLEN], *s;

	for (i = 0; i < NUMBER_OF_SAVEABLE_OPTIONS; i++) {
		extern char *sort_name();

	    switch (save_info[i].flags & DT_MASK) {
		case DT_SYN:
		case DT_MLT:
		case DT_NOP:
		    break;
		case DT_ALT:
		    break; /* not dumping addresses to debug file */
		case DT_WEE:
		    fprintf(debugfile, "\nAnd we're skipping the following headers:\n\t");

		    for (len = 8, j = 0; j < weedcount; j++) {
			if (weedlist[j][0] == '*') continue;	/* skip '*end-of-defaults*' */
			if (len + strlen(weedlist[j]) > 80) {
			    fprintf(debugfile, " \n\t");
			    len = 8;
			}
			fprintf(debugfile, "%s  ", weedlist[j]);
			len += strlen(weedlist[j]) + 3;
		    }
		    fprintf(debugfile, "\n\n");
		    break;

		default:
		    switch (save_info[i].flags&DT_MASK) {

		    case DT_STR:
			s = SAVE_INFO_STR(i);
			break;

		    case DT_NUM:
			sprintf(buf, "%d", *SAVE_INFO_NUM(i));
			s = buf;
			break;

		    case DT_CHR:
			sprintf(buf, "%c", *SAVE_INFO_CHR(i));
			s = buf;
			break;

		    case DT_BOL:
			s = on_off(*SAVE_INFO_BOL(i));
			break;

		    case DT_SRT:
			s = sort_name(FALSE);
			break;

		    case DT_ASR:
			s = alias_sort_name(FALSE);
			break;
		    }

		    fprintf(debugfile, "%s = %s\n", save_info[i].name, s);
		    break;
	    }
	}
	fprintf(debugfile, "\n\n");
}

int is_it_on(char *word)
{
	/** Returns TRUE if the specified word is either 'ON', 'YES'
	    or 'TRUE', and FALSE otherwise.   We explicitly translate
	    to lowercase here to ensure that we have the fastest
	    routine possible - we really DON'T want to have this take
	    a long time or our startup will be major pain each time.
	**/

	static char mybuffer[NLEN];
	register int i, j;

	for (i=0, j=0; word[i] != '\0'; i++)
	  mybuffer[j++] = tolower(word[i]);
	mybuffer[j] = '\0';

	return(  (strncmp(mybuffer, "on",   2) == 0) ||
		 (strncmp(mybuffer, "yes",  3) == 0) ||
		 (strncmp(mybuffer, "true", 4) == 0)
	      );
}
