case $CONFIG in
 '')	for dir in . .. ../.. ../../.. ; do
 		if test -f $dir/config.sh ; then
 			. $dir/config.sh
 			break
		fi
 	done
 	case $CONFIG in
 	'') (echo "Can't find config.sh."; exit 1) ;;
 	esac
 	;;
  esac
  case "$0" in
 */*)	cd `echo "$0" | sed -e 's!/[^/]*$!!' -e "s!^$source_dir!$build_dir!"` ;;
  esac
  
 echo "Extracting sysdefs.h (with variable substitutions)"
 sed <<!GROK!THIS! >../sysdefs.h -e 's!^#undef!/\*#undef!'

/* $Id: sysdefs.SH,v 1.6 1996/03/14 17:27:34 wfp5p Exp $ */
/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.6 $   $State: Exp $
 *
 * 			Copyright (c) 1988-1992 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Bill Pemberton, Elm Coordinator
 *	elm@DSI.COM	
 *
 *******************************************************************************
 * $Log: sysdefs.SH,v $
# Revision 1.6  1996/03/14  17:27:34  wfp5p
# Alpha 9
#
# Revision 1.5  1995/09/29  17:41:00  wfp5p
# Alpha 8 (Chip's big changes)
#
# Revision 1.4  1995/06/30  14:56:22  wfp5p
# Alpha 5
#
# Revision 1.3  1995/06/01  13:07:37  wfp5p
# Changes over to enhanced mode in for the print command.  From Chip Rosenthal
# <chip@unicom.com>
#
# Revision 1.2  1995/05/10  13:34:32  wfp5p
# Added mailing list stuff by Paul Close <pdc@sgi.com>
# And NetBSD stuff.
#
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**  System level, configurable, defines for the ELM mail system.  **/

/* max headers to weed out */
#define MAX_IN_WEEDLIST 150

/* max hops in return addr to E)veryone  */
#define MAX_HOPS 35

#define DEFAULT_BATCH_SUBJECT "$defbatsub"

/* if mydomain file is missing */
#define DEFAULT_DOMAIN "$mydomain"

/* FOO - interim while the intent for MIME support is sorted out */
#define MIME_SEND
#ifdef MIME
# define MIME_RECV
#endif

/* system data files - fully rooted pathnames */

#define system_data_file	"$lib/aliases"
#define system_text_file	"$lib/aliases.text"
#define system_hostdom_file	"$lib/domain"
#define system_rc_file		"$lib/elm.rc"
#define system_rcinfo_file	"$lib/elmrc-info"
#define system_mimetypes_file	"$lib/mime-types"
#define system_mlist_file	"$lib/mlists"
#define system_help_dir		"$lib"


/* user data files - relative to user home directory */

#define elmrcfile	".elm/elmrc"
#define old_elmrcfile	".elm/elmrc.old"
#define mailheaders	".elm/elmheaders"
#define mlistfile	".elm/mlists"

#define ALIAS_TEXT	".elm/aliases.text"
#define ALIAS_DATA	".elm/aliases"

#define dead_letter	"Canceled.mail"
#define unedited_mail	"emergency.mbox"

/* where to put the output of the elm -d command... */
#define DEBUGFILE	"ELM:debug.info"
#define OLDEBUG		"ELM:debug.last"


/* temporary files */

#define	default_temp   	"$tmpdir/"
#define temp_file	"snd."
#define temp_form_file	"form."
#define temp_mbox	"mbox."
#define temp_print      "print."
#define temp_edit	"elm-edit"
#define temp_uuname	"uuname."
#define temp_state	"est."


/* commands and options */

#define emacs_editor	"$emacs"
#define vi_editor	"$vi"

#define default_editor	"$defeditor $editoropts"

#define default_shell	"$prefshell"
#define default_pager	"$pager"
#define default_printout "$linepr"

#define sendmail	"$sendmail"
#define smflags		"-oi -oem"	/* ignore dots and mail back errors */
#define smflagsv	"-oi -oem -v"	/* ...plus verbose voyuer mode */
#define smflagmt	" -om"		/* metoo copy to sender */

#define execmail	"$execmail"
#define emflags		""
#define emflagsv	"-d"		/* verbose voyuer mode */
#define emflagmt	"-m"		/* metoo copy to sender */

#define submitmail	"$submit"
#define submitflags	"-mlrnvxto,cc*"
#define submitflags_s	"-mlrnv"

#define mailer		"$mailer"

#define newalias	"newalias 1>&2 > /dev/null"
#define readmsg		"readmsg"

#define remove_cmd	"$rm -f"	/* how to remove a file */
#define cat		"$cat"		/* how to display files */
#define sed_cmd		"$sed"		/* how to access sed */
#define move_cmd	"$mv"		/* how to access sed */
#define uuname		"$uuname"	/* how to get a uuname  */


/* other system definitions */

#define mailhome	"$maildir/"

#define default_charset		"$defcharset"
#define default_display_charset	"$defdispcharset"

#define MSG_SEPARATOR	"\001\001\001\001\n"	/* mmdf message seperator */

		

!GROK!THIS!
