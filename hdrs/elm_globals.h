
/* $Id: elm_globals.h,v 1.5 1997/10/20 20:24:23 wfp5p Exp $ */

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
 * $Log: elm_globals.h,v $
 * Revision 1.5  1997/10/20  20:24:23  wfp5p
 * Incomingfolders no longer set Magic mode on for all remaining folders.
 *
 * Revision 1.4  1996/08/08  19:49:18  wfp5p
 * Alpha 11
 *
 * Revision 1.3  1996/05/09  15:50:56  wfp5p
 * Alpha 10
 *
 * Revision 1.2  1996/03/14  17:27:22  wfp5p
 * Alpha 9
 *
 * Revision 1.1  1995/09/29  17:40:48  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.10  1995/09/27  17:57:19  wfp5p
 * 2.5 Alpha 7 changes
 *
 * Revision 1.9  1995/09/11  15:18:47  wfp5p
 * Alpha 7
 *
 * Revision 1.8  1995/07/18  18:59:47  wfp5p
 * Alpha 6
 *
 * Revision 1.7  1995/06/30  14:56:19  wfp5p
 * Alpha 5
 *
 * Revision 1.6  1995/06/21  15:26:36  wfp5p
 * editflush and confirmtagsave are new in the elmrc (Keith Neufeld)
 *
 * Revision 1.5  1995/06/13  16:04:23  wfp5p
 * No newline on the last line -- choked some compilers.
 *
 * Revision 1.4  1995/06/12  20:32:30  wfp5p
 * Fixed up a couple of multiple declares
 *
 * Revision 1.3  1995/05/10  13:34:29  wfp5p
 * Added mailing list stuff by Paul Close <pdc@sgi.com>
 * And NetBSD stuff.
 *
 * Revision 1.2  1995/04/20  21:01:10  wfp5p
 * Removed filter
 *
 * Revision 1.1.1.1  1995/04/19  20:38:30  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/


/*
 * All source files in the Elm package should include this right
 * after "elm_defs.h".  This header should not be used by the
 * library or utility files.
 */

#include "elm_curses.h"


/****************************************************************************/


#define BACKSPACE	'\b'	/* backspace character	*/
#define TAB		'\t'	/* tab character	*/
#define RETURN		'\r'	/* carriage return char	*/
#define LINE_FEED	'\n'	/* line feed character	*/
#define FORMFEED	'\f'	/* form feed (^L) char	*/
#define ESCAPE		'\033'	/* the escape		*/

#define NO		0
#define YES		1
#define NO_TITE		2		/* ti/te or in flag	   */
#define MAYBE		2		/* a definite define, eh?  */
#define FORM		3		/*      <nevermind>        */
#define PREFORMATTED	4		/* forwarded form...       */

#define SAME_PAGE	1		/* redraw current only     */
#define NEW_PAGE	2		/* redraw message list     */
#define ILLEGAL_PAGE	0		/* error in page list, punt */

/* special cookies that may appear in the user's message text */
#define MSSG_START_ENCODE	"[encode]"
#define MSSG_END_ENCODE		"[clear]"
#define MSSG_DONT_SAVE		"[no save]"
#define MSSG_DONT_SAVE2		"[nosave]"
#define MSSG_INCLUDE		"include"
#define MSSG_ATTACH		"attach"

/* message sorting selections */
#define REVERSE		-		/* for reverse sorting           */
#define SENT_DATE	1		/* the date message was sent     */
#define RECEIVED_DATE	2		/* the date message was received */
#define SENDER		3		/* the name/address of sender    */
#define SIZE		4		/* the # of lines of the message */
#define SUBJECT		5		/* the subject of the message    */
#define STATUS		6		/* the status (deleted, etc)     */
#define MAILBOX_ORDER	7		/* the order it is in the file   */

/* alias sorting selections */
#define ALIAS_SORT	1		/* the name of the alias         */
#define NAME_SORT	2		/* the actual name for the alias */
#define TEXT_SORT	3		/* the order of aliases.text     */
#define LAST_ALIAS_SORT	TEXT_SORT

/* options to the copy_message() procedure */
#define CM_REMOVE_HEADER	(1<<0)	/* skip header of message	     */
#define CM_REMOTE		(1<<1)	/* append remote from hostname to    */
					/*   first line			     */
#define CM_UPDATE_STATUS	(1<<2)	/* Update Status: Header	     */

#define CM_REMAIL		(1<<4)	/* Add Sender: and Orig-To: headers  */
#define CM_DECODE		(1<<5)	/* prompt for key if message is	     */
					/*   encrypted			     */
#define CM_PREFIX		(1<<6)	/* Add prefix (">", etc.) to lines   */
#define CM_FORWARDING		(1<<7)	/* text is a message that we are     */
					/*   forwarding			     */
#define CM_ATTRIBUTION		(1<<8)	/* add attribution strings that	     */
					/*   format this as an included mssg */

/* options to the leave() procedure */
#define LEAVE_NORMAL		000	/* normal program termination	*/
#define LEAVE_ERROR		001	/* exit due to program error	*/
#define LEAVE_KEEP_EDITTMP	010	/* ...preserve composition file	*/
#define LEAVE_KEEP_TEMPFOLDER	020	/* ...preserve temp folder	*/
#define LEAVE_KEEP_LOCK		040	/* ...preserve folder locks	*/
#define LEAVE_EMERGENCY		071	/* emergency exit		*/
#define LEAVE_EXIT_STATUS(mode)	((mode) & 07)

/* options to the lock() procedure */
#define LOCK_OUTGOING	0
#define LOCK_INCOMING	1

/* options to the meta_match() procedure */
#define MATCH_TAG	0		/* tag matching entries		*/
#define MATCH_DELETE	1		/* delete matching entries	*/
#define MATCH_UNDELETE	2		/* undelete matching entries	*/

/* options to the system_call() procedure */
#define SY_USER_SHELL	(1<<0)		/* use user shell instead of /bin/sh */
#define SY_ENV_SHELL	(1<<1)		/* put SHELL=[shell] into environ    */
#define SY_ENAB_SIGHUP	(1<<2)		/* pgm to exec can handle signals    */
#define SY_ENAB_SIGINT	(1<<3)		/*  ...and it can handle SIGINT too  */
#define SY_DUMPSTATE	(1<<4)		/* create folder state dump file     */
#define SY_COOKED	(1<<5)		/* run with tty in cooked mode	     */


/****************************************************************************/


/* map character (e.g. 'A') to its control equivalent (e.g. '^A') */
#define ctrl(c)	        (((c) + '@') & 0x7f)

/* calculate next tabstop from position (col) */
/* leftmost col is zero, assumes 8-position tabstops */
#define tabstop(col)	(((col) & ~07) + 010)

#define whitespace(c)	((c) == ' ' || (c) == '\t')

#define no_ret(s)	{ register int xyz; /* varname is for lint */	      \
		          for (xyz=strlen(s)-1; xyz >= 0 &&		      \
				(s[xyz] == '\r' || s[xyz] == '\n'); )	      \
			     s[xyz--] = '\0';                                 \
			}

#define onoff(n)		((n) == 0 ? "OFF" : "ON")
#define ison(n, mask)		(((n) & (mask)) != 0)
#define isoff(n, mask)		(((n) & (mask)) == 0)
#define setit(n, mask)		(n) |= (mask)
#define clearit(n, mask)	(n) &= ~(mask)


/****************************************************************************/


/*
 * "opmode" global - current operation mode settings.
 */

#define OPMODE_READ		010	/* running in folder read mode	*/
#define OPMODE_READ_NONEMPTY	011	/*  ...only read if non-empty	*/
#define OPMODE_SEND		020	/* running in send-only mode	*/
#define OPMODE_SEND_BATCH	021	/*  ...performing batch send	*/

#define OPMODE_IS_READMODE(op)		(((op) & OPMODE_READ) != 0)
#define OPMODE_IS_SENDMODE(op)		(((op) & OPMODE_SEND) != 0)
#define OPMODE_IS_INTERACTIVE(op)	((op) != OPMODE_SEND_BATCH)

/*
 * FOLDER "flags" settings.
 */

#define FOLDER_IS_SPOOL		(1<<0)

typedef struct mail_folder {
	int flags;		/* special handling flags for folder	*/
	char filename[SLEN];	/* pathname to the current folder	*/
	char tempname[SLEN];	/* pathname to workfile copy of the	*/
				/*   folder if IS_SPOOL, otherwise	*/
				/*   an empty string			*/
	FILE *fp;		/* file stream for folder		*/
	long size;		/* size of folder in bytes		*/
	int num_mssgs;		/* number of messages in folder		*/
	int curr_mssg;		/* selected message (first = 1)		*/
	struct header_rec **headers; /* list of message headers info	*/
	int max_headers;	/* allocated size of "headers" list	*/
} FOLDER;


/*
 * global information initialized at startup
 */

EXTERN char version_buff[NLEN];		/* version buffer		*/
EXTERN int userid;			/* uid for current user		*/
EXTERN int groupid;			/* groupid for current user	*/
EXTERN int mailgroupid;			/* gid for mail spool files	*/
EXTERN int original_umask;		/* umask at startup		*/

/*
 * options set by the command line
 */

EXTERN int opmode;			/* selected operating mode	*/
EXTERN int sendmail_verbose;		/* enable sendmail debug option	*/
EXTERN int TreatAsSpooled;		/* lock even non-spool folders (-M flag) */
EXTERN char *batch_subject;		/* subject for batch transmission */
EXTERN char *included_file;		/* message to load in send mode	*/

/*
 * the current mail folder
 *
 * FOO - Eventually I would like to see this deleted, and Elm have
 * the ability to process multiple folders doing various things.
 */

EXTERN FOLDER curr_folder;

/*
 * aliases
 */

EXTERN struct alias_rec **aliases;	/* list of defined aliases	*/
EXTERN int max_aliases INIT(0);		/* allocated size of the list	*/
EXTERN int num_aliases INIT(0);		/* number aliases used in list	*/
EXTERN int curr_alias;			/* index of selected alias	*/


/*
 * butt ugly state stuff, most of which should not be global in this fashion
 */

EXTERN int inalias INIT(FALSE);		/* TRUE if in the alias menu */
EXTERN int header_page INIT(0);		/* current header page */
EXTERN int last_current INIT(-1);	/* previous current message */
EXTERN int last_header_page INIT(-1);	/* last header page */
EXTERN int headers_per_page INIT(0);	/* number of headers/page */
EXTERN int redraw INIT(FALSE);		/* need to rewrite the entire screen? */
EXTERN int nucurr INIT(FALSE);		/* change list or just curr pointer */
EXTERN int nufoot INIT(FALSE);		/* clear lines 16 thru bottom */
EXTERN int selected INIT(FALSE);	/* used for select stuff */


/*
 * configuration file settings
 *
 * In addition to these settings, the following globals from "elm_defs.h"
 * also may be set by the configuration file:
 *
 *	- char host_name[SLEN];
 *	- char host_domain[SLEN];
 *	- char host_fullname[SLEN];
 *	- char full_user_name[SLEN];
 *	- struct addr_rec *alternative_addresses;
 */

EXTERN char temp_dir[SLEN];		/* name of temp directory */
EXTERN char raw_temp_dir[SLEN];		/* unexpanded name of temp directory */
EXTERN char folders[SLEN];		/* folder home directory */
EXTERN char raw_folders[SLEN];		/* unexpanded folder home directory */
EXTERN char recvd_mail[SLEN];		/* folder for storing received mail */
EXTERN char raw_recvdmail[SLEN];	/* unexpanded recvd_mail name */
EXTERN char editor[SLEN];		/* default editor for mail */
EXTERN char raw_editor[SLEN];		/* unexpanded default editor for mail */
EXTERN char alternative_editor[SLEN];	/* the 'other' editor */
EXTERN char printout[SLEN];		/* how to print messages */
EXTERN char raw_printout[SLEN];		/* unexpanded how to print messages */
EXTERN char printhdrs[SLEN];		/* headers to select and weed on printing */
EXTERN char sent_mail[SLEN];		/* name of file to save copies to */
EXTERN char raw_sentmail[SLEN];		/* unexpanded name of file to save to */
EXTERN char calendar_file[SLEN];	/* name of file for clndr */
EXTERN char raw_calendar_file[SLEN];	/* unexpanded name of file for clndr */
EXTERN char attribution[SLEN];		/* attribution string for replies */
EXTERN char fwdattribution[SLEN];	/* attribution string for forwarded mssgs */
EXTERN char prefixchars[SLEN] INIT("> "); /* prefix char(s) for msgs */
EXTERN char shell[SLEN];		/* default system shell */
EXTERN char raw_shell[SLEN];		/* unexpanded default system shell */
EXTERN char pager[SLEN];		/* what pager to use... */
EXTERN char raw_pager[SLEN];		/* unexpanded what pager to use... */
EXTERN char local_signature[SLEN];	/* local msg signature file */
EXTERN char raw_local_signature[SLEN];	/* unexpanded local msg sig file */
EXTERN char remote_signature[SLEN];	/* remote msg signature file */
EXTERN char raw_remote_signature[SLEN];	/* unexpanded remote msg sig file */
EXTERN char e_editor[SLEN];		/* "~e" editor... */
EXTERN char v_editor[SLEN];		/* "~v" editor... */
EXTERN char config_options[SLEN];	/* which options are in o)ptions */
EXTERN char allowed_precedences[SLEN];	/* list of precedences user may specify */
EXTERN char to_chars[SLEN];		/* chars to indicate who mail is to */
EXTERN char charset[SLEN];		/* 8-bit charset for outbound mssgs */
EXTERN char display_charset[SLEN];	/* charset display can handle */
EXTERN char charset_compatlist[SLEN];	/* charsets upward compat to US-ASCII */

EXTERN unsigned char escape_char INIT('~');	/* '~' or something else... */

EXTERN char *weedlist[MAX_IN_WEEDLIST];	/* header weeding list */
EXTERN int weedcount;			/* number of headers to check */

EXTERN char *magiclist[MAX_IN_WEEDLIST];/* list of folders to use magic mode */
EXTERN int magiccount;			/* number of magic folders */

EXTERN int allow_forms INIT(FALSE);	/* are AT&T Mail forms okay? */
EXTERN int allow_setuid INIT(FALSE);	/* allow execution where uid!=euid */
EXTERN int always_delete INIT(FALSE);	/* always delete marked msgs? */
EXTERN int always_keep INIT(TRUE);	/* always keep unread msgs? */
EXTERN int always_store INIT(FALSE);	/* always store read mail? */
EXTERN int arrow_cursor INIT(FALSE);	/* use "->" regardless? */
EXTERN int ask_delete INIT(TRUE);	/* confirm delete on resync? */
EXTERN int ask_keep INIT(TRUE);		/* ask to keep unread msgs? */
EXTERN int ask_reply_copy INIT(TRUE);	/* ask to copy mssg into reply? */
EXTERN int ask_store INIT(TRUE);	/* ask to store read mail? */
EXTERN int auto_cc INIT(FALSE);		/* mail copy to yourself? */
EXTERN int bounceback INIT(FALSE);	/* bounce copy off remote? */
EXTERN int builtin_lines INIT(-3);	/* int use builtin pager? */
EXTERN int confirm_append INIT(FALSE);	/* confirm append to folder? */
EXTERN int confirm_create INIT(FALSE);	/* confirm create new folder? */
EXTERN int confirm_files INIT(FALSE);	/* confirm files for append? */
EXTERN int confirm_folders INIT(FALSE);	/* confirm folders for create? */
EXTERN int confirm_tag_save INIT(TRUE);	/* confirm saving tagged messages if the cursor on untagged message? */
EXTERN int edit_flush INIT(TRUE);	/* flush input after extern edit? */
EXTERN int filter INIT(TRUE);		/* weed out header lines? */
EXTERN int force_name INIT(FALSE);	/* save by name forced? */
EXTERN int hp_softkeys INIT(FALSE);	/* are there softkeys? */
EXTERN int keep_empty_files INIT(FALSE);/* keep empty files? */
EXTERN int metoo INIT(FALSE);		/* copy me on mail to alias? */
EXTERN int mini_menu INIT(TRUE);	/* display menu? */
EXTERN int move_when_paged INIT(FALSE);	/* move when '+' or '-' used? */
EXTERN int names_only INIT(TRUE);	/* display names but no addrs? */
EXTERN int noheader INIT(TRUE);		/* copy + header to file? */
EXTERN int point_to_new INIT(TRUE);	/* start pointing at new msgs? */
EXTERN int prompt_after_pager INIT(TRUE);/* prompt after pager exits */
EXTERN int prompt_for_cc INIT(TRUE);	/* prompt user for 'cc' value? */
EXTERN int readmsginc INIT(1);		/* msg cnt incr during new mbox read */
EXTERN int reply_copy INIT(TRUE);	/* copy message into reply? */
EXTERN int resolve_mode INIT(TRUE);	/* resolve before moving mode? */
EXTERN int save_by_alias INIT(TRUE);	/* save mail by alias of login name? */
EXTERN int save_by_name INIT(TRUE);	/* save mail by login name? */
EXTERN int show_mlists INIT(FALSE);	/* show mailing list info? */
EXTERN int show_reply INIT(FALSE);	/* show 'r' for replied mail */
EXTERN int sig_dashes INIT(TRUE);	/* put dashes above signature? */
EXTERN int sleepmsg INIT(2);		/* secs pause for transient messages */
EXTERN int title_messages INIT(TRUE);	/* title message display? */
EXTERN int use_tite INIT(TRUE);		/* use termcap/terminfo ti/te? */
EXTERN int user_level INIT(0);		/* numeric user level */

EXTERN int sortby INIT(REVERSE SENT_DATE);	/* how to sort folders */
EXTERN int alias_sortby INIT(NAME_SORT);/* how to sort aliases */

EXTERN int clear_pages INIT(FALSE);	/* clear screen w/ builtin pgr? */

EXTERN long timeout INIT(600L);		/* seconds for main level timeout */


/*
 * pre-formatted NLS strings
 */

EXTERN char *def_ans_yes;		/* yes answer - single char, lwr case */
EXTERN char *def_ans_no;		/* no answer - single char, lwr case */
EXTERN char *nls_deleted;		/* [deleted] */
EXTERN char *nls_form;			/* Form */
EXTERN char *nls_message;		/* Message */
EXTERN char *nls_to;			/* To */
EXTERN char *nls_from;			/* From */
EXTERN char *nls_page;			/* Page */
EXTERN char *nls_item;			/* message or alias */
EXTERN char *nls_items;			/* messages or aliases */
EXTERN char *nls_Item;			/* Message or Alias */
EXTERN char *nls_Items;			/* Messages or Aliases */
EXTERN char *nls_Prompt;		/* Command or Alias */


/*
 * old declarations
 */

char *bounce_off_remote();
char *expand_system();
char *format_long();
char *get_alias_address();
char *get_date();
char *get_token();
char *level_name();
char *strip_commas();


/*
 * The files I've been through sorting out the public and private routines
 * are marked internally as either "PUBLIC" or "static".
 */

#define PUBLIC


/* fbrowser.c */

#define FB_EXIST	(1<<0)	/* file must exist			*/
#define FB_READ		(1<<1)	/* file must exist and be readable	*/
#define FB_WRITE	(1<<2)	/* if file exists, must be writable	*/
#define FB_MBOX		(1<<3)	/* if file exists, must be mbox format	*/

int fbrowser P_((char *, int, const char *, const char *, int, const char *));
int fbrowser_analyze_spec P_((const char *, char *, char *));


/* file_ops.c */

FILE *file_open P_((const char *, const char *));
int file_close P_((FILE *, const char *));
int file_access P_((const char *, int));
int file_seek P_((FILE *, long, int, const char *));
int file_copy P_((FILE *, FILE *, const char *, const char *));
int file_rename P_((const char *, const char *));


/* sendmsg.c */

/* options to send_message() */
#define SM_ORIGINAL	0	/* this is an original message		  */
#define SM_REPLY	1	/* this is a response to the current mssg */
#define SM_FORWARD	2	/* the current mssg is being forwarded	  */
#define SM_FWDUNQUOTE	3	/* forward - without quoting or editing	  */
#define SM_FORMRESP	4	/* this is a response to a fill-out form  */

int send_message P_((const char *, const char *, const char *, int));
void display_to P_((char *));
int get_to P_((char *, char *, int));
char *build_mailer_command P_((char *, const char *, char *, char *, char *));

