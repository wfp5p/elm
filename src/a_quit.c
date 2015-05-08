

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
 * $Log: a_quit.c,v $
 * Revision 1.4  1996/03/14  17:27:47  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:41:54  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/07/18  18:59:53  wfp5p
 * Alpha 6
 *
 * Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** a_quit: leave the aliases menu and return to the main menu.
  
**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_aliases.h"


int delete_aliases(int newaliases, int prompt)
{
/*
 *	Update aliases by processing deletions.  Prompting is
 *	done as directed by user input and elmrc options.
 *
 *	If "prompt" is false, then no prompting is done.
 *	Otherwise prompting is dependent upon the variable
 *	ask_delete, as set by an elmrc option.  This behavior
 *	makes the 'q' command prompt just like '$', while
 *	retaining the 'Q' command for a quick exit that never
 *	prompts.
 *
 *	Return	1		Aliases were deleted
 * 		newalias	Aliases were not deleted
 *
 *	The return allows the caller to determine if the "newalias"
 *	routine should be run.
 */

	char buffer[SLEN];
	char **list;

	int to_delete = 0, to_keep = 0, i,
		     marked_deleted = 0,
		     ask_questions;
	char answer;

	dprint(1, (debugfile, "\n\n-- leaving aliases --\n\n"));

	if (num_aliases == 0)
	  return(newaliases);	/* nothing changed */

	ask_questions = ((!prompt) ? FALSE : ask_delete);

	/* Determine if deleted messages are really to be deleted */

	/* we need to know how many there are to delete */
	for (i=0; i<num_aliases; i++)
	  if (ison(aliases[i]->status, DELETED))
	    marked_deleted++;

	dprint(6, (debugfile, "Number of aliases marked deleted = %d\n",
	            marked_deleted));

        if(marked_deleted) {
	  if(ask_questions) {
	    if (marked_deleted == 1)
	      strcpy(buffer, catgets(elm_msg_cat, AliasesSet, AliasesDelete,
			"Delete 1 alias?"));
	    else
	      sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesDeletePlural,
			"Delete %d aliases?"), marked_deleted);
	                    
	    answer = enter_yn(buffer, always_delete, LINES-3, FALSE);
	  } else {
	    answer = always_delete;
	  }

	  if(answer) {
	    list = (char **) malloc(marked_deleted*sizeof(*list));
	    for (i = 0; i < num_aliases; i++) {
	      if (ison(aliases[i]->status, DELETED)) {
		list[to_delete] = aliases[i]->alias;
	        dprint(8,(debugfile, "Alias number %d, %s is deletion %d\n",
	               i, list[to_delete], to_delete));
	        to_delete++;
	      }
	      else {
	        to_keep++;
	      }
	    }
	   /*
	    * Formulate message as to number of keeps and deletes.
	    * This is only complex so that the message is good English.
	    */
	    if (to_keep > 0) {
	      curr_alias = 1;		/* Reset current alias */
	      if (to_keep == 1)
		sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesKeepDelete,
			  "[Keeping 1 alias and deleting %d.]"), to_delete);
	      else
		sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesKeepDeletePlural,
			  "[Keeping %d aliases and deleting %d.]"), to_keep, to_delete);
	    }
	    else {
	      curr_alias = 0;		/* No aliases left */
	      sprintf(buffer, catgets(elm_msg_cat, AliasesSet, AliasesDeleteAll,
			  "[Deleting all aliases.]"));
	    }

	    dprint(2, (debugfile, "Action: %s\n", buffer));
	    show_error(buffer);

	    delete_from_alias_text(list, to_delete);

	    free((char *) list);
	  }
	}
	dprint(3, (debugfile, "Aliases deleted: %d\n", to_delete));

	/* If all aliases are to be kept we don't need to do anything
	 * (like run newalias for the deleted messages).
	 */

	if(to_delete == 0) {
	  dprint(3, (debugfile, "Aliases kept as is!\n"));
	  return(newaliases);
	}

	return(1);
}

void exit_alias(void)
{

	int i;

	/* Clear the deletes from all aliases.  */

	for(i = 0; i < num_aliases; i++)
	  if (ison(aliases[i]->status, DELETED))
	    clearit(aliases[i]->status, DELETED);

	dprint(6, (debugfile, "\nexit_alias:  Done clearing deletes.\n"));

}
