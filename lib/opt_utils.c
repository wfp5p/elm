
static char rcsid[] = "@(#)$Id: opt_utils.c,v 1.3 1996/03/14 17:27:43 wfp5p Exp $";

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
 * $Log: opt_utils.c,v $
 * Revision 1.3  1996/03/14  17:27:43  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:23  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This file contains routines that might be needed for the various
     machines that the mailer can run on.  Please check the Makefile
     for more help and/or information. 

**/

#include "elm_defs.h"
#include "s_error.h"

#ifdef PWDINSYS
#  include <sys/pwd.h>
#else
#  include <pwd.h>
#endif

#ifndef HAS_CUSERID

char *cuserid(usrname)
     char *usrname;
{
	/** Added for compatibility with Bell systems, this is the last-ditch
	    attempt to get the users login name, after getlogin() fails.  It
	    instantiates "usrname" to the name of the user...(it also tries
	    to use "getlogin" again, just for luck)
	**/
	/** This wasn't really compatible.  According to our man page, 
	 ** It was inconsistent.  If the parameter is NULL then you return
	 ** the name in a static area.  Else the ptr is supposed to be a
	 ** pointer to l_cuserid bytes of memory [probally 9 bytes]...
	 ** It's not mention what it should return if you copy the name
	 ** into the array, so I chose NULL.
	 ** 					Sept 20, 1988
	 **					**WJL**
	 **/

  struct passwd *password_entry;
  char   *name, *getlogin();
  register returnonly = 0;
  
  if (usrname == NULL) ++returnonly;
  
  if ((name = getlogin()) != NULL) {
    if (returnonly) {
      return(name);
    } else {
      strcpy(usrname, name);
      return name;
    }
  } 
  else 
    if (( password_entry = fast_getpwuid(getuid())) != NULL) 
      {
	if (returnonly) 
	  {
	    return(password_entry->pw_name);
	  }
	else 
	  {
	    strcpy(usrname, password_entry->pw_name);
	    return name;
	  }
      } 
    else 
      {
	return NULL;
      }
}

#endif


#ifndef STRTOK

char *strtok(source, keys)
char *source, *keys;
{
	/** This function returns a pointer to the next word in source
	    with the string considered broken up at the characters 
	    contained in 'keys'.  Source should be a character pointer
	    when this routine is first called, then NULL subsequently.
	    When strtok has exhausted the source string, it will 
	    return NULL as the next word. 

	    WARNING: This routine will DESTROY the string pointed to
	    by 'source' when first invoked.  If you want to keep the
	    string, make a copy before using this routine!!
	 **/

	register int  last_ch;
	static   char *sourceptr;
		 char *return_value;

	if (source != NULL)
	  sourceptr = source;
	
	if (*sourceptr == '\0') 
	  return(NULL);		/* we hit end-of-string last time!? */

	sourceptr += strspn(sourceptr, keys);	/* skip leading crap */
	
	if (*sourceptr == '\0') 
	  return(NULL);		/* we've hit end-of-string */

	last_ch = strcspn(sourceptr, keys);	/* end of good stuff */

	return_value = sourceptr;		/* and get the ret   */

	sourceptr += last_ch;			/* ...value 	     */

	if (*sourceptr != '\0')		/* don't forget if we're at END! */
	  sourceptr++;			   /* and skipping for next time */

	return_value[last_ch] = '\0';		/* ..ending right    */
	
	return((char *) return_value);		/* and we're outta here! */
}

#endif


#ifndef STRPBRK

char *strpbrk(source, keys)
char *source, *keys;
{
	/** Returns a pointer to the first character of source that is any
	    of the specified keys, or NULL if none of the keys are present
	    in the source string. 
	**/

	register char *s, *k;

	for (s = source; *s != '\0'; s++) {
	  for (k = keys; *k; k++)
	    if (*k == *s)
	      return(s);
	}
	
	return(NULL);
}

#endif

#ifndef STRSPN

strspn(source, keys)
char *source, *keys;
{
	/** This function returns the length of the substring of
	    'source' (starting at zero) that consists ENTIRELY of
	    characters from 'keys'.  This is used to skip over a
	    defined set of characters with parsing, usually. 
	**/

	register int loc = 0, key_index = 0;

	while (source[loc] != '\0') {
	  key_index = 0;
	  while (keys[key_index] != source[loc])
	    if (keys[key_index++] == '\0')
	      return(loc);
	  loc++;
	}

	return(loc);
}

#endif

#ifndef STRCSPN

strcspn(source, keys)
char *source, *keys;
{
	/** This function returns the length of the substring of
	    'source' (starting at zero) that consists entirely of
	    characters NOT from 'keys'.  This is used to skip to a
	    defined set of characters with parsing, usually. 
	    NOTE that this is the opposite of strspn() above
	**/

	register int loc = 0, key_index = 0;

	while (source[loc] != '\0') {
	  key_index = 0;
	  while (keys[key_index] != '\0')
	    if (keys[key_index++] == source[loc])
	      return(loc);
	  loc++;
	}

	return(loc);
}

#endif

#ifndef TEMPNAM
/* and a tempnam for temporary files */
static int cnt = 0;

char *tempnam( dir, pfx)
 char *dir, *pfx;
{
	char space[SLEN];
	char *newspace;

	if (dir == NULL) {
		dir = "/usr/tmp";
	} else if (*dir == '\0') {
		dir = "/usr/tmp";
	}
	
	if (pfx == NULL) {
		pfx = "";
	}

	sprintf(space, "%s%s%d.%d", dir, pfx, getpid(), cnt);
	cnt++;
	
	newspace = malloc(strlen(space) + 1);
	if (newspace != NULL) {
		strcpy(newspace, space);
	}
	return newspace;
}

#endif


#ifndef GETOPT

/*LINTLIBRARY*/
#define NULL	0
#define EOF	(-1)
#define ERR(s, c)	if(opterr){\
	extern size_t strlen();\
        extern int write();\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, (unsigned)strlen(s));\
	(void) write(2, errbuf, 2);}

extern int strcmp();

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

int
getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == NULL) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=index(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			cp = catgets(elm_msg_cat, ErrorSet, ErrorGetoptReq,
				": option requires an argument -- ");
			ERR(cp, c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

#endif

#ifndef RENAME
int rename(tmpfname, fname)
const char *tmpfname, *fname;
{
	int status;

	(void) unlink(fname);
	if ((status = link(tmpfname, fname)) != 0)
		return(status);
	(void) unlink(tmpfname);
	return(0);
}
#endif
