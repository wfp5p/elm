

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
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
 * $Log: aliaslib.c,v $
 * Revision 1.4  1996/05/09  15:51:15  wfp5p
 * Alpha 10
 *
 * Revision 1.3  1996/03/14  17:27:51  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:58  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** Library of functions dealing with the alias system...

 **/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

/*
 * Expand "name" as an alias and return a pointer to static data containing
 * the expansion.  If "name" is not an alias, then NULL is returned.
 */
char *get_alias_address(char *name, int mailing, int *too_longp)
{
	static char buffer[VERY_LONG_STRING];
	char *bufptr;
	int bufsize, are_in_aliases = TRUE;

	if (!inalias) {
	    main_state();
	    are_in_aliases = FALSE;
	}
/*
 *	Reopens files iff changed since last read
 */
	open_alias_files(are_in_aliases);
/*
 *	If name is an alias then return its expansion
 */
	bufptr = buffer;
	bufsize = sizeof(buffer);
	if (do_get_alias(name,&bufptr,&bufsize,mailing,FALSE,0,too_longp)) {
	/*
	 *  Skip comma/space from add_name_to_list()
	 */
	    bufptr = buffer+2;
	}
	else {
	/*
	 *  Nope...not an alias (or it was too long to expand)
	 */
	    dprint(2, (debugfile,
		"Could not expand alias in get_alias_address()%s\n",
		*too_longp ? "\t...alias buffer overflowed." : ""));
	    bufptr = NULL;
	}

	if (! are_in_aliases)
	    main_state();

	return(bufptr);
}


/*
 * Determine if "name" is an alias, and if so expand it and store the result in
 * "*bufptr".  TRUE returned if any expansion occurs, else FALSE is returned.
 */
int do_get_alias(char *name, char **bufptr, int *bufsizep, int mailing,
		 int sysalias, int depth, int *too_longp)
{
	struct alias_rec *match;
	char abuf[VERY_LONG_STRING];
	int loc;

	/* update the recursion depth counter */
	++depth;

	dprint(6, (debugfile, "%*s->attempting alias expansion on \"%s\"\n",
		(depth*2), "", name));

	/* strip out (comments) and leading/trailing whitespace */
	remove_possible_trailing_spaces( name = strip_parens(name) );
	for ( ; isspace(*name)  ; ++name ) ;

	/* throw back empty addresses */
	if ( *name == '\0' )
	  return FALSE;

/* The next two blocks could be merged somewhat */
	/* check for a user alias, unless in the midst of sys alias expansion */
	if ( !sysalias ) {
	  if ( (loc = find_alias(name, USER)) >= 0 ) {
	    match = aliases[loc];
	    strcpy(abuf, match->address);
	    if ( match->type & PERSON ) {
	      if (strlen(match->name) > 0) {
                sprintf(abuf+strlen(abuf), " (%s)", match->name);
	      }
	    }
	    goto do_expand;
	  }
	}

	/* check for a system alias */
	  if ( (loc = find_alias(name, SYSTEM)) >= 0 ) {
	    match = aliases[loc];
	    strcpy(abuf, match->address);
	    if ( match->type & PERSON ) {
	      if (strlen(match->name) > 0) {
                sprintf(abuf+strlen(abuf), " (%s)", match->name);
	      }
	    }
	    sysalias = TRUE;
	    goto do_expand;
	  }

	/* nope...this name wasn't an alias */
	return FALSE;

do_expand:

	/* at this point, alias is expanded into "abuf" - now what to do... */

	dprint(7, (debugfile, "%*s  ->expanded alias to \"%s\"\n",
	    (depth*2), "", abuf));

	/* check for an exact match */
	loc = strlen(name);
	if ( strncmp(name, abuf, loc) == 0 &&
	     (isspace(abuf[loc]) || abuf[loc] == '\0') ) {
	  if (add_name_to_list(abuf, bufptr, bufsizep)) {
	      return TRUE;
	  }
	  else {
	      *too_longp = TRUE;
	      return FALSE;
	  }
	}

	/* see if we are stuck in a loop */
	if ( depth > 12 ) {
	  dprint(2, (debugfile,
	      "alias expansion loop detected at \"%s\" - bailing out\n", name));
	    error1(catgets(elm_msg_cat, ElmSet, ElmErrorExpanding,
		"Error expanding \"%s\" - probable alias definition loop."),
		name);
	    return FALSE;
	}

	/* see if the alias equivalence is a group name */
	if ( mailing && match->type & GROUP )
	  return do_expand_group(abuf,bufptr,bufsizep,sysalias,depth,too_longp);

	/* see if the alias equivalence is an email address */
	if ( qstrpbrk(abuf,"!@:") != NULL ) {
	    if (add_name_to_list(abuf, bufptr, bufsizep)) {
	        return TRUE;
	    }
	    else {
	        *too_longp = TRUE;
	        return FALSE;
	    }
	}

	/* see if the alias equivalence is itself an alias */
	if ( mailing &&
	     do_get_alias(abuf,bufptr,bufsizep,TRUE,sysalias,depth,too_longp) )
	  return TRUE;

	/* the alias equivalence must just be a local address */
	if (add_name_to_list(abuf, bufptr, bufsizep)) {
	    return TRUE;
	}
	else {
	    *too_longp = TRUE;
	    return FALSE;
	}
}


/*
 * Expand the comma-delimited group of names in "group", storing the result
 * in "*bufptr".  Returns TRUE if expansion occurs OK, else FALSE in the
 * event of errors. */
/* char *group;	group list to expand */
/* char **bufptr;	place to store result of expansion */
/* int *bufsizep;	available space in the buffer */
/* int sysalias;	TRUE to suppress checks of the user's aliases */
/* int depth;	nesting depth */
/* int *too_longp;	error code if expansion overflows buffer */
int do_expand_group(char *group, char **bufptr, int *bufsizep,
		    int sysalias, int depth, int *too_longp)
{
	char *name, *gecos;
	char expanded_address[LONG_STRING];

	/* go through each comma-delimited name in the group */
	while ( group != NULL ) {

	  /* extract the next name from the list */
	  for ( name = group ; isspace(*name) ; ++name ) ;
	  if ( (group = strchr(name,',')) != NULL )
	      *group++ = '\0';
	  remove_possible_trailing_spaces(name);
	  if ( *name == '\0' )
	    continue;

	  /* see if this name is really an alias */
	  if (do_get_alias(name,bufptr,bufsizep,TRUE,sysalias,depth,too_longp))
	    continue;

	  if ( *too_longp )
	      return FALSE;

	  /* verify it is a valid address */
	  if ( valid_name(name) ) {
	    gecos = get_full_name(name);

	    if (gecos && (strlen(gecos) > 0)) {
	        sprintf(expanded_address, "%s (%s)", name, gecos);
	        name = expanded_address;
	    }
	  }

	  /* add it to the list */
	  if ( !add_name_to_list(name, bufptr, bufsizep) ) {
	    *too_longp = TRUE;
	    return FALSE;
	  }

	}

	return TRUE;
}


/*
 * Append "<comma><space>name" to the list, checking to ensure the buffer
 * does not overflow.  Upon return, *bufptr and *bufsizep will be updated to
 * reflect the stuff added to the buffer.  If a buffer overflow would occur,
 * an error message is printed and FALSE is returned, else TRUE is returned.
 */
/* register char *name;	/\* name to append to buffer			*\/ */
/* register char **bufptr;	/\* pointer to pointer to end of buffer		*\/ */
/* register int *bufsizep;	/\* pointer to space remaining in buffer		*\/ */

int add_name_to_list(register char *name, register char **bufptr,
		     register int *bufsizep)
{
	if ( *bufsizep < 0 )
	    return FALSE;

	*bufsizep -= strlen(name)+2;
	if ( *bufsizep <= 0 ) {
	    *bufsizep = -1;
	    dprint(2, (debugfile,
		"Alias expansion is too long in add_name_to_list()\n"));
	    error(catgets(elm_msg_cat, ElmSet, ElmAliasExpTooLong,
		"Alias expansion is too long."));
	    return FALSE;
	}

	*(*bufptr)++ = ',';
	*(*bufptr)++ = ' ';
	while ( *name != '\0' )
	  *(*bufptr)++ = *name++ ;
	**bufptr = '\0';

	return TRUE;
}
