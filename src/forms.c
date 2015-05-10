

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
 * $Log: forms.c,v $
 * Revision 1.3  1996/03/14  17:29:36  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:12  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:36  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This set of files supports the 'forms' options (AT&T Mail Forms) to
    the mail system.  The specs are drawn from a document from AT&T entitled
    "Standard for Exchanging Forms on AT&T Mail", version 1.9.

**/

/** Some notes on the format of a FORM;

	First off, in AT&T Mail parlance, this program only supports SIMPLE
	forms, currently.  This means that while each form must have three
 	sections;

		[options-section]
		***
		[form-image]
		***
		[rules-section]

	this program will ignore the first and third sections completely.  The
	program will assume that the user merely enteres the form-image section,
	and will append and prepend the triple asterisk sequences that *MUST*
	be part of the message.  The messages are also expected to have a
	specific header - "Content-Type: mailform" - which will be added on all
	outbound mail and checked on inbound...
**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

static void prompt_for_entries(char *buffer, FILE *fd, int entries);
static void prompt_for_sized_entry(char *prompt, char *buffer, int field_size);

int check_form_file(const char *filename)
{
	/** This routine returns the number of fields in the specified file,
	    or -1 if an error is encountered. **/

	FILE *form;
	char buffer[SLEN];
	int field_count = 0;

	if ((form = fopen(filename, "r")) == NULL) {
	  show_error(catgets(elm_msg_cat, ElmSet, ElmErrorOpeningCheckFields,
		  "Error %s trying to open %s to check fields!"),
		  strerror(errno), filename);
	  return(-1);
	}

	while (mail_gets(buffer, SLEN, form)) {
	  field_count += occurances_of(':', buffer);
	}

	fclose(form);

	return(field_count);
}

int format_form(char *filename)
{
	/** This routine accepts a validated file that is the middle
	    section of a form message and prepends and appends the appropriate
	    instructions.  It's pretty simple.
	    This returns the number of forms in the file, or -1 on errors
	**/

	FILE *form, *newform;
	char  newfname[SLEN], buffer[SLEN];
	int form_count = 0;
	int  len_buf, err;

	dprint(4, (debugfile, "Formatting form file '%s'\n", filename));

	/** first off, let's open the files... **/

	if ((form = fopen(filename, "r")) == NULL) {
	  err = errno;
	  show_error(catgets(elm_msg_cat, ElmSet, ElmCantReadMessageToValidate,
		"Can't read the message to validate the form!"));
	  dprint(1, (debugfile,
              "** Error encountered opening file \"%s\" - %s (check_form) **\n",
	      filename, strerror(err)));
	  return(-1);
	}

	sprintf(newfname, "%s%s%d", temp_dir, temp_form_file, getpid());

	if ((newform = file_open(newfname, "w")) == NULL) {
	  err = errno;
	  show_error(catgets(elm_msg_cat, ElmSet, ElmCouldntOpenNewformOutput,
		"Couldn't open newform file for form output!"));
	  dprint(1, (debugfile,
              "** Error encountered opening file \"%s\" - %s (check_form) **\n",
	      newfname, strerror(err)));
	  return(-1);
	}

	/** the required header... **/

	/* these are actually the defaults, but let's be sure, okay? */

	fprintf(newform, "WIDTH=78\nTYPE=SIMPLE\nOUTPUT=TEXT\n***\n");

	/** and let's have some fun transfering the stuff across... **/

	while ((len_buf = mail_gets(buffer, SLEN, form))) {
	  fwrite(buffer, 1, len_buf, newform);
	  form_count += occurances_of(':', buffer);
	}

	fprintf(newform, "***\n");	/* that closing bit! */

	fclose(form);
	fclose(newform);

	if (form_count > 0) {
	  if (unlink(filename) != 0) {
	    show_error(catgets(elm_msg_cat, ElmSet, ElmErrorUnlinkingFile,
		"Error %s unlinking file %s."),
		strerror(errno), filename);
	    return(-1);
	  }
	  if (link(newfname, filename)) {
	    show_error(catgets(elm_msg_cat, ElmSet, ElmErrorLinkingFile,
		"Error %s linking %s to %s."),
		    strerror(errno), newfname, filename);
	    return(-1);
	  }
	}

	if (unlink(newfname)) {
	  show_error(catgets(elm_msg_cat, ElmSet, ElmErrorUnlinkingFile,
	        "Error %s unlinking file %s."),
		strerror(errno), newfname);
	  return(-1);
	}

	return(form_count);
}

int mail_filled_in_form(char *address, char *subject)
{
	/** This is the interesting routine.  This one will read the
	    message and prompt the user, line by line, for each of
	    the fields...returns non-zero if it succeeds
	**/

	FILE  	     *fd;
	int lines = 0, count, len_buf, max_lines;
	char         buffer[SLEN];

	dprint(4, (debugfile,
		"replying to form with;\n\taddress=%s and\n\t subject=%s\n",
		 address, subject));

        if (fseek(curr_folder.fp, curr_folder.headers[curr_folder.curr_mssg-1]->offset, 0) == -1) {
	  dprint(1, (debugfile,
		   "Error: seek %ld resulted in errno %s (%s)\n",
		   curr_folder.headers[curr_folder.curr_mssg-1]->offset, strerror(errno),
		   "mail_filled_in_form"));
	  show_error(catgets(elm_msg_cat, ElmSet, ElmSeekFailedFile,
		"ELM [seek] couldn't read %d bytes into file (%s)."),
	         curr_folder.headers[curr_folder.curr_mssg-1]->offset, strerror(errno));
	  return(0);
        }

	/* now we can fly along and get to the message body... */

	max_lines = curr_folder.headers[curr_folder.curr_mssg-1]->lines;
	while ((len_buf = mail_gets(buffer, SLEN, curr_folder.fp))) {
	  if (len_buf == 1)	/* <return> only */
	    break;
	  else if (lines >= max_lines) {
	    show_error(catgets(elm_msg_cat, ElmSet, ElmNoFormInMessage,
		"No form in this message!?"));
	    return(0);
	  }

	  if (buffer[len_buf - 1] == '\n')
	    lines++;
	}

	if (len_buf == 0) {
	  show_error(catgets(elm_msg_cat, ElmSet, ElmNoFormInMessage,
	      "No form in this message!?"));
	  return(0);
	}

	dprint(6, (debugfile, "- past header of form message -\n"));

	/* at this point we're at the beginning of the body of the message */

	/* now we can skip to the FORM-IMAGE section by reading through a
	   line with a triple asterisk... */

	while ((len_buf = mail_gets(buffer, SLEN, curr_folder.fp))) {
	  if (strcmp(buffer, "***\n") == 0)
	    break;	/* we GOT it!  It's a miracle! */

	  if (buffer[len_buf - 1] == '\n')
	    lines++;

	  if (lines >= max_lines) {
	    show_error(catgets(elm_msg_cat, ElmSet, ElmBadForm,
		"Badly constructed form.  Can't reply!"));
	    return(0);
	  }
	}

	if (len_buf == 0) {
	  show_error(catgets(elm_msg_cat, ElmSet, ElmBadForm,
		"Badly constructed form.  Can't reply!"));
	  return(0);
	}

	dprint(6, (debugfile, "- skipped the non-forms-image stuff -\n"));

	/* one last thing - let's open the tempfile for output... */

	sprintf(buffer, "%s%s%d", temp_dir, temp_form_file, getpid());

	dprint(2, (debugfile,"-- forms sending using file %s --\n", buffer));

	if ((fd = file_open(buffer,"w")) == NULL) {
	  show_error(catgets(elm_msg_cat, ElmSet, ElmCantOpenAsOutputFile,
		"Can't open \"%s\" as output file! (%s)."),
		buffer, strerror(errno));
	  dprint(1, (debugfile,
		  "** Error %s encountered trying to open temp file %s;\n",
		  strerror(errno), buffer));
	  return(0);
	}

	/* NOW we're ready to read the form image in and start prompting... */

	Raw(OFF);
	ClearScreen();

	while ((len_buf = mail_gets(buffer, SLEN, curr_folder.fp))) {
	  dprint(9, (debugfile, "- read %s", buffer));
	  if (strcmp(buffer, "***\n") == 0) /* end of form! */
	    break;

	  if (buffer[len_buf - 1] == '\n')
	    lines++;

	  if (lines > max_lines)
	    break; /* end of message */

	  switch ((count = occurances_of(':', buffer))) {
	    case 0 : fwrite(buffer, 1, len_buf, stdout);	/* output line */
		     fwrite(buffer, 1, len_buf, fd);
		     break;
            case 1 : if (buffer[0] == ':') {
	               printf(catgets(elm_msg_cat, ElmSet, ElmEnterAsManyLines,
"(Enter as many lines as needed, ending with a '.' by itself on a line)\n"));
                       while (fgets(buffer, SLEN, stdin) != NULL) {
		         no_ret(buffer);
	                 if (strcmp(buffer, ".") == 0)
	                   break;
	                 else
			   fprintf(fd,"%s\n", buffer);
		       }
	             }
	             else
		       prompt_for_entries(buffer, fd, count);
	             break;
            default: prompt_for_entries(buffer, fd, count);
	  }
	}

	Raw(ON);
	fclose(fd);

	/** let's just mail this off now... **/

	(void) send_message(address, (char *)NULL, subject, SM_FORMRESP);

	return(1);
}

static void prompt_for_entries(char *buffer, FILE *fd, int entries)
{
	/** deals with lines that have multiple colons on them.  It must first
	    figure out how many spaces to allocate for each field then prompts
	    the user, line by line, for the entries...
	**/

	char mybuffer[SLEN], prompt[SLEN], spaces[SLEN];
	int  field_size, i, j, offset = 0, extra_tabs = 0;

	dprint(7, (debugfile,
		"prompt-for-multiple [%d] -entries \"%s\"\n", entries,
		buffer));

	strcpy(prompt, catgets(elm_msg_cat, ElmSet, ElmFormNoPrompt,
		"No Prompt Available:"));

	while (entries--) {
	  j=0;
	  i = chloc((char *) buffer + offset, ':') + 1;
	  while (j < i - 1) {
	    prompt[j] = buffer[j+offset];
	    j++;
	  }
	  prompt[j] = '\0';

	  field_size = 0;

	  while (whitespace(buffer[i+offset])) {
	    if (buffer[i+offset] == TAB) {
	      field_size += 8 - (i % 8);
	      extra_tabs += (8 - (i % 8)) - 1;
	    }
	    else
	      field_size += 1;
	    i++;
	  }

	  offset += i;

	  if (field_size == 0) 	/* probably last prompt in line... */
	    field_size = 78 - (offset + extra_tabs);

	  prompt_for_sized_entry(prompt, mybuffer, field_size);

	  spaces[0] = ' ';	/* always at least ONE trailing space... */
	  spaces[1] = '\0';

	  /*  field_size-1 for the space spaces[] starts with  */
	  for (j = strlen(mybuffer); j < field_size-1; j++)
	    strcat(spaces, " ");

	  fprintf(fd, "%s: %s%s", prompt, mybuffer, spaces);
	  fflush(fd);
	}

	fprintf(fd, "\n");
}

static void prompt_for_sized_entry(char *prompt, char *buffer, int field_size)
{
	/* This routine prompts for an entry of the size specified. */

	int i;

	dprint(7, (debugfile, "prompt-for-sized-entry \"%s\" %d chars\n",
		prompt, field_size));

	printf("%s: ", prompt);

	for (i=0;i<field_size; i++)
	  putchar('_');
	for (i=0;i<field_size; i++)
	  putchar(BACKSPACE);
	fflush(stdout);

	fgets(buffer, SLEN, stdin);
	no_ret(buffer);

	if (strlen(buffer) > field_size) buffer[field_size] = '\0';
}
