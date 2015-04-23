

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.6 $   $State: Exp $
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
 * $Log: addr_util.c,v $
 * Revision 1.6  1999/03/24  14:03:57  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.5  1996/05/09  15:51:14  wfp5p
 * Alpha 10
 *
 * Revision 1.4  1996/03/14  17:27:49  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:41:56  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/05/24  15:34:36  wfp5p
 * Change to deal with parenthesized comments in when eliminating members from
 * an alias. (from Keith Neufeld <neufeld@pvi.org>)
 *
 * Allow a shell escape from the alias screen (just like from
 * the index screen).  It does not put the shell escape onto the alias
 * screen menu. (from Keith Neufeld <neufeld@pvi.org>)
 *
 * Allow the use of "T" from the builtin pager. (from Keith Neufeld
 * <neufeld@pvi.org>)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This file contains addressing utilities

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"


void translate_return(char *addr, char *ret_addr)
{
	/** Return ret_addr to be the same as addr, but with the login
            of the person sending the message replaced by '%s' for
            future processing...
	    Fixed to make "%xx" "%%xx" (dumb 'C' system!)
	**/

	int loc, loc2, iindex = 0;
	char *remaining_addr;

/*
 *	check for RFC-822 source route: format @site:usr@site
 *	if found, skip to after the first : and then retry.
 *	source routes can be stacked
 */
	remaining_addr = addr;
	while (*remaining_addr == '@') {
	  loc = qchloc(remaining_addr, ':');
	  if (loc == -1)
	    break;

	  remaining_addr += loc + 1;
	}

	loc2 = qchloc(remaining_addr,'@');
	loc = qchloc(remaining_addr, '%');
	if ((loc < loc2) && (loc != -1))
	  loc2 = loc;

	if (loc2 != -1) {	/* ARPA address. */
	  /* algorithm is to get to '@' sign and move backwards until
	     we've hit the beginning of the word or another metachar.
	  */
	  for (loc = loc2 - 1; loc > -1 && remaining_addr[loc] != '!'; loc--)
	     ;
	}
	else {			/* usenet address */
	  /* simple algorithm - find last '!' */

	  loc2 = strlen(remaining_addr);	/* need it anyway! */

	  for (loc = loc2; loc > -1 && remaining_addr[loc] != '!'; loc--)
	      ;
	}

	/** now copy up to 'loc' into destination... **/

	while (iindex <= loc) {
	  ret_addr[iindex] = remaining_addr[iindex];
	  iindex++;
	}

	/** now append the '%s'... **/

	ret_addr[iindex++] = '%';
	ret_addr[iindex++] = 's';

	/*
	 *  and, finally, if anything left, add that
	 * however, just pick up the address part, we do
	 * not want any comments.  Thus stop copying at
	 * the first blank character.
	 */

	if ((loc = qchloc(remaining_addr,' ')) == -1)
	  loc = strlen(addr);
	while (loc2 < loc) {
	  ret_addr[iindex++] = remaining_addr[loc2++];
	  if (remaining_addr[loc2-1] == '%')	/* tweak for "printf" */
	    ret_addr[iindex++] = '%';
	}

	ret_addr[iindex] = '\0';
}

int build_address(char *to, char *full_to)
{
	/** loop on all words in 'to' line...append to full_to as
	    we go along, until done or length > len.  Modified to
	    know that stuff in parens are comments...Returns non-zero
	    if it changed the information as it copied it across...
	**/

	int	i, j, k, l,
			in_parens = 0, a_in_parens,
			expanded_information = 0,
			eliminated = 0;
	int too_long = FALSE;
	int to_len;
	char word[SLEN], next_word[SLEN], *ptr;
	char elim_list[SLEN], word_a[SLEN], next_word_a[SLEN];
	char *gecos;

	full_to[0] = '\0';
	to_len = 0;

	elim_list[0] = '\0';

	i = get_word(to, 0, word, sizeof(word));

	/** Look for addresses to be eliminated from aliases **/
	while (i > 0) {

	  j = get_word(to, i, next_word, sizeof(next_word));

	  if(word[0] == '(')
	    in_parens++;

	  if (in_parens) {
	    if(word[strlen(word)-1] == ')')
	      in_parens--;
	  }

	  else if (word[0] == '-') {
	    for (k = 0; word[k] != '\0'; word[k] = word[k+1], k++);
	    if (elim_list[0] != '\0')
	      strcat(elim_list, " ");
	    /*  expand alias and eliminate all members */
	    if ((ptr = get_alias_address(word, TRUE, &too_long)) != NULL) {
	      /*  ignores overflow, like every bloody other place in elm */
	      strcat(elim_list, ptr);
	      too_long = FALSE;
	    } else
	      strcat(elim_list, word);
	  }

	  if ((i = j) > 0)
	    strcpy(word, next_word);
	}

	if (elim_list[0] != '\0')
	  eliminated++;

	i = get_word(to, 0, word, sizeof(word));

	while (i > 0) {

	  j = get_word(to, i, next_word, sizeof(next_word));

	  if(word[0] == '(')
	    in_parens++;

	  if (in_parens) {
	    if(word[strlen(word)-1] == ')')
	      in_parens--;
	    strcpy(full_to+to_len, " ");
	    strcpy(full_to+to_len+1, word);
	    to_len += strlen(word)+1;
	  }

	  else if (word[0] == '-') {
		; /* huh??? I don't understand this  (*FOO*) */
	  }

	  else if (qstrpbrk(word,"!@:") != NULL) {
	    if (to_len > 0) {
		(void) strcpy(full_to+to_len, ", ");
		to_len += 2;
	    }
	    (void) strcpy(full_to+to_len, word);
	    to_len += strlen(word);
	  }
	  else if ((ptr = get_alias_address(word, TRUE, &too_long)) != NULL) {

	    /** check aliases for addresses to be eliminated **/
	    if (eliminated) {
	      k = get_word(strip_commas(ptr), 0, word_a, sizeof(word_a));

	      while (k > 0) {
		l = get_word(ptr, k, next_word_a, sizeof(next_word_a));
		if (in_list(elim_list, word_a) == 0) {

		if (to_len > 0) {
		    (void) strcpy(full_to+to_len, ", ");
		    to_len += 2;
		}
		(void) strcpy(full_to+to_len, word_a);
		to_len += strlen(word_a);

		  /** copy possible () comment **/
		  if (next_word_a[0] == '(') {
		    a_in_parens = 0;

		    while (l > 0) {
		      if (to_len > 0) {
			(void) strcpy(full_to+to_len, " ");
			++to_len;
		      }
		      (void) strcpy(full_to+to_len, next_word_a);
		      to_len += strlen(next_word_a);
		      if (next_word_a[0] == '(')
			++a_in_parens;
		      if (next_word_a[strlen(next_word_a)-1] == ')')
			--a_in_parens;
		      l = get_word(ptr, l, next_word_a, sizeof(next_word_a));
		      if (! a_in_parens)
			break;
		    }
		  }
		}
		else {
		  /** skip possible () comment **/
                  if (next_word_a[0] == '(') {
                    a_in_parens = 0;

                    while (l > 0) {
                      if (next_word_a[0] == '(')
                        ++a_in_parens;
                      if (next_word_a[strlen(next_word_a)-1] == ')')
                        --a_in_parens;
                      l = get_word(ptr, l, next_word_a, sizeof(next_word_a));
                      if (! a_in_parens)
                        break;
                    }
                  }
		}

		if ((k = l) > 0)
		  strcpy(word_a, next_word_a);
	      }
	    } else {
	      if (to_len > 0) {
		  (void) strcpy(full_to+to_len, ", ");
		  to_len += 2;
	      }
	      (void) strcpy(full_to+to_len, ptr);
	      to_len += strlen(ptr);
	    }
	    expanded_information++;
	  }
	  else if (too_long) {
	 /*
	  *   We don't do any real work here.  But we need some
	  *   sort of test in this line of tests to make sure
	  *   that none of the other else's are tried if the
	  *   alias expansion failed because it was too long.
	  */
	      dprint(2,(debugfile,"Overflowed alias expansion for %s\n", word));
	  }
	  else if (word[0] != '\0') {
	    if (to_len > 0) {
	      (void) strcpy(full_to+to_len, ", ");
	      to_len += 2;
	    }
	    if (valid_name(word)) {
	      (void) strcpy(full_to+to_len, word);
	      to_len += strlen(word);

	      if (next_word[0] != '(')
		if ((gecos = get_full_name(word)) != NULL && *gecos != '\0') {
		  sprintf(full_to+to_len, " (%s)", gecos);
		  to_len += strlen(gecos)+3;
		}
	    } else {
	      (void) strcpy(full_to+to_len, word);
	      to_len += strlen(word);
	    }
	  }

	  if((i = j) > 0)
	    strcpy(word, next_word);
	}

	return( expanded_information > 0 ? 1 : 0 );
}

void forwarded(char *buffer, struct header_rec *entry)
{
	/** Change 'from' and date fields to reflect the ORIGINATOR of
	    the message by iteratively parsing the >From fields...
	    Modified to deal with headers that include the time zone
	    of the originating machine... **/

	char machine[SLEN], buff[SLEN], holding_from[SLEN];
	int len;

	machine[0] = holding_from[0] = '\0';

	sscanf(buffer, "%*s %s", holding_from);

	/* after skipping over From and address, process rest as date field */

	while (!isspace(*buffer)) buffer++;	/* skip From */
	while (isspace(*buffer)) buffer++;

	while (*buffer) {
	  len = len_next_part(buffer);
	  if (len > 1) {
	    buffer += len;
	  } else {
	    if (isspace(*buffer))
	      break;
	    buffer++;
	  }
	}
	while (isspace(*buffer)) buffer++;

	parse_arpa_date(buffer, entry);

	/* the following fix is to deal with ">From xyz ... forwarded by xyz"
	   which occasionally shows up within AT&T.  Thanks to Bill Carpenter
	   for the fix! */

	if (strcmp(machine, holding_from) == 0)
	  machine[0] = '\0';

	if (machine[0] == '\0')
	  strcpy(buff, holding_from[0] ? holding_from : "anonymous");
	else
	  sprintf(buff,"%s!%s", machine, holding_from);

	strfcpy(entry->from, buff, STRING);
}


int fix_arpa_address(char *address)
{
	/** Given a pure ARPA address, try to make it reasonable.

	    This means that if you have something of the form a@b@b make
            it a@b.  If you have something like a%b%c%b@x make it a%b@x...
	**/

	int host_count = 0, i;
	char     hosts[MAX_HOPS][LONG_STRING];	/* array of machine names */
	char     *host, *addrptr;

	/*  break down into a list of machine names, checking as we go along */

	addrptr = (char *) address;

	while ((host = get_token(addrptr, "%@", 2)) != NULL) {
	  for (i = 0; i < host_count && ! streq(hosts[i], host); i++)
	      ;

	  if (i == host_count) {
	    strcpy(hosts[host_count++], host);
	    if (host_count == MAX_HOPS) {
	       dprint(2, (debugfile,
           "Can't build return address - hit MAX_HOPS in fix_arpa_address\n"));
	       show_error(catgets(elm_msg_cat, ElmSet, ElmCantBuildRetAddr,
			"Can't build return address - hit MAX_HOPS limit!"));
	       return(1);
	    }
	  }
	  else
	    host_count = i + 1;
	  addrptr = NULL;
	}

	/** rebuild the address.. **/

	address[0] = '\0';

	for (i = 0; i < host_count; i++)
	  sprintf(address, "%s%s%s", address,
	          address[0] == '\0'? "" :
	 	    (i == host_count - 1 ? "@" : "%"),
	          hosts[i]);

	return(0);
}
