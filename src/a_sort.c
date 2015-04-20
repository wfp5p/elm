

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
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
 * $Log: a_sort.c,v $
 * Revision 1.3  1996/03/14  17:27:49  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:56  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** Sort alias table by the field specified in the global
    variable "alias_sortby"...

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_aliases.h"

static void alias_old_current(long iindex);
static int compare_aliases(const void *a, const void *b);

int sort_aliases(int entries, int visible, int are_in_aliases)
{
	/** Sort the header_table definitions... If 'visible', then
	    put the status lines etc **/

	long last_index = -1;

	dprint(2, (debugfile, "\n** sorting aliases by %s **\n\n",
		alias_sort_name(TRUE)));

	/* Don't get last_index if no entries or no current. */
	/* There would be no current if we are sorting for the first time. */
	if (entries > 0 && curr_alias > 0)
	  last_index = aliases[curr_alias-1]->length;

	if ((entries > 30) && visible && are_in_aliases) {
	    show_error(catgets(elm_msg_cat, AliasesSet, AliasesSort,
		    "Sorting aliases by %s..."), alias_sort_name(TRUE));
	}

	if (entries > 1)
	  qsort((char *) aliases, (unsigned) entries,
	        sizeof (struct alias_rec *), compare_aliases);

	if (last_index > -1)
	  alias_old_current(last_index);

	if (are_in_aliases) {
	    clear_error();
	}
}

static int compare_aliases(const void *a, const void *b)
{
	/** compare two aliases according to the sortby value.

	    Both are simple strcmp()'s on the alias or last_name
	    components of the alias.
	 **/

	register struct alias_rec *first, *second;
	register int ret;
	register long diff;

	struct alias_rec **p1 = (struct alias_rec **) a;
	struct alias_rec **p2 = (struct alias_rec **) b;

	first = *p1;
	second = *p2;

	/* If (only) one of the compares is a duplicate we want it
	 * to go to the end of the list regardless of the sorting
	 * method.
	 */
	if ((first->type ^ second->type) & DUPLICATE) {
	    if (first->type & DUPLICATE)
	        ret = 1;
	    else			/* It must be second... */
	        ret = -1;
	    return ret;
	}

	switch (abs(alias_sortby)) {
	case ALIAS_SORT:
		ret = strcmp(first->alias, second->alias);
		break;

	case NAME_SORT:
		ret = strcmp(first->last_name, second->last_name);
	     /*
	      * If equal on last name then compare on first name
	      * which is the first part of 'name'.
	      */
		if (ret == 0) {
		    ret = strcmp(first->name, second->name);
		}
		break;

	case TEXT_SORT:
		diff = (first->length - second->length);
 		if ( diff < 0 )	ret = -1;
 		else if ( diff > 0 ) ret = 1;
 		else ret = 0;
		break;

	default:
		/* never get this! */
		ret = 0;
		break;
	}

	if (alias_sortby < 0)
	  ret = -ret;

	return ret;
}

char *alias_sort_name(int longname)
{

    if (alias_sortby < 0) {
	switch (-alias_sortby) {
	case ALIAS_SORT: return (longname
		? catgets(elm_msg_cat, AliasesSet, AliasesRevAliasName,
			"Reverse Alias Name")
		: catgets(elm_msg_cat, AliasesSet, AliasesRevAliasAbr,
			"Reverse-Alias"));
	case NAME_SORT: return (longname
		? catgets(elm_msg_cat, AliasesSet, AliasesRevFullName,
			"Reverse Full (Real) Name")
		: catgets(elm_msg_cat, AliasesSet, AliasesRevFullAbr,
			"Reverse-Name"));
	case TEXT_SORT: return (longname
		? catgets(elm_msg_cat, AliasesSet, AliasesRevTextFile,
			"Reverse Text File")
		: catgets(elm_msg_cat, AliasesSet, AliasesRevTextAbr,
			"Reverse-Text"));
	}
    } else {
	switch (alias_sortby) {
	case ALIAS_SORT: return (longname
		? catgets(elm_msg_cat, AliasesSet, AliasesAliasName,
			"Alias Name")
		: catgets(elm_msg_cat, AliasesSet, AliasesAliasAbr,
			"Alias"));
	case NAME_SORT: return (longname
		? catgets(elm_msg_cat, AliasesSet, AliasesFullName,
			"Full (Real) Name")
		: catgets(elm_msg_cat, AliasesSet, AliasesFullAbr,
			"Name"));
	case TEXT_SORT: return (longname
		? catgets(elm_msg_cat, AliasesSet, AliasesTextFile,
			"Text File")
		: catgets(elm_msg_cat, AliasesSet, AliasesTextAbr,
			"Text"));
	}
    }

    return("*UNKNOWN-SORT-PARAMETER*");
}

static void alias_old_current(long iindex)
{
	/** Set current to the message that has "index" as it's
	    index number.  This is to track the current message
	    when we resync... **/

	register int i;

	dprint(4, (debugfile, "alias-old-current(%d)\n", iindex));

	for (i = 0; i < num_aliases; i++)
	  if (aliases[i]->length == iindex) {
	    curr_alias = i+1;
	    dprint(4, (debugfile, "\tset curr_alias to %d!\n", curr_alias));
	    return;
	  }

	dprint(4, (debugfile,
		"\tcouldn't find current index.  curr_alias left as %d\n",
		curr_alias));
	return;		/* can't be found.  Leave it alone, then */
}
