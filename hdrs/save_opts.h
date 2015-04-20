
/* @(#)$Id: save_opts.h,v 1.11 1996/08/08 19:49:19 wfp5p Exp $ */

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.11 $   $State: Exp $
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
 * $Log: save_opts.h,v $
 * Revision 1.11  1996/08/08  19:49:19  wfp5p
 * Alpha 11
 *
 * Revision 1.10  1996/05/09  15:51:05  wfp5p
 * Alpha 10
 *
 * Revision 1.9  1996/03/14  17:27:32  wfp5p
 * Alpha 9
 *
 * Revision 1.8  1995/09/29  17:40:58  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.7  1995/09/11  15:18:50  wfp5p
 * Alpha 7
 *
 * Revision 1.6  1995/07/18  18:59:48  wfp5p
 * Alpha 6
 *
 * Revision 1.5  1995/06/30  14:56:21  wfp5p
 * Alpha 5
 *
 * Revision 1.4  1995/06/21  15:26:37  wfp5p
 * editflush and confirmtagsave are new in the elmrc (Keith Neufeld)
 *
 * Revision 1.3  1995/05/10  13:34:31  wfp5p
 * Added mailing list stuff by Paul Close <pdc@sgi.com>
 * And NetBSD stuff.
 *
 * Revision 1.2  1995/04/20  21:01:13  wfp5p
 * Removed filter
 *
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/*
 *	Defines for the storage of options portion of the Elm system.
 */

#define DT_SYN		0	/* synonym entry (old name) */
#define DT_STR		1	/* string */
#define DT_NUM		2	/* number */
#define DT_BOL		3	/* ON/OFF (boolean) */
#define DT_CHR		4	/* character */
#define DT_WEE		5	/* weed list */
#define DT_ALT		6	/* alternate addresses list */
#define DT_SRT		7	/* sort-by code */
#define DT_MLT		8	/* multiple destinations for data */
#define DT_ASR		9	/* sort-by code */
#define DT_NOP		10	/* obsolete - ignored for back-compatibility */
#define DT_INC          11      /* incoming folder list */
#define DT_MASK		037	/* mask for data type */
#define FL_LOCAL	(1<<5)	/* flag if changed */
#define FL_NOSPC	(1<<6)	/* flag if preserve blanks as "_" */
#define FL_SYS		(1<<7)	/* flag if only valid in system RC */
#define FL_OR 		(1<<8)	/* flag if boolean value may have been set */
#define FL_AND		(1<<9)	/* flag if boolean value may have been unset */

typedef struct { 
	char name[NLEN];	/* name of configuration setting*/
        int flags;		/* DT_STR, DT_NUM, DT_BOL, etc	*/
	malloc_t val;		/* pointer to setting value	*/
} save_info_recs;

#define SAVE_INFO(x, type)	((type)(save_info[x].val))
#define SAVE_INFO_STR(x)	SAVE_INFO((x), char *)
#define SAVE_INFO_NUM(x)	SAVE_INFO((x), int *)
#define SAVE_INFO_BOL(x)	SAVE_INFO((x), int *)
#define SAVE_INFO_CHR(x)	SAVE_INFO((x), char *)
#define SAVE_INFO_WEE(x)	SAVE_INFO((x), char **)
#define SAVE_INFO_ALT(x)	SAVE_INFO((x), struct addr_rec **)
#define SAVE_INFO_SRT(x)	SAVE_INFO((x), int *)
#define SAVE_INFO_ASR(x)	SAVE_INFO((x), int *)
#define SAVE_INFO_SYN(x)	SAVE_INFO((x), char *)
#define SAVE_INFO_MLT(x)	SAVE_INFO((x), char **)

#ifdef SO_INTERN /*{*/

/* "lists" for DT_MLT.  These and DT_SYN could be eliminated if support
   of the old parameter names was dropped.
*/
char *ALWAYSLEAVE[]={"alwayskeep","!alwaysstore",NULL};
char *ASK[]={"askdelete","askkeep","askstore",NULL};
char *AUTOCOPY[]={"!askreplycopy","replycopy",NULL};
char *SIGS[]={"remotesignature","localsignature",NULL};

/*
 * This list *MUST* be sorted in alphabetical order by configuration
 * option name.  That's because the option-writing routine does a
 * binary search to locate options in the list.
 */

save_info_recs save_info_data[] = {

{"aliassortby",		DT_ASR,		(malloc_t)&alias_sortby},
{"allow_setuid",	DT_BOL|FL_SYS,	(malloc_t)&allow_setuid},
{"alteditor",		DT_STR,		(malloc_t)alternative_editor},
{"alternatives",	DT_ALT,		(malloc_t)&alternative_addresses},
{"alwaysdelete",	DT_BOL,		(malloc_t)&always_delete},
{"alwayskeep",		DT_BOL,		(malloc_t)&always_keep},
{"alwaysleave",		DT_MLT,		(malloc_t)ALWAYSLEAVE},
{"alwaysstore",		DT_BOL,		(malloc_t)&always_store},
{"arrow",		DT_BOL|FL_OR,	(malloc_t)&arrow_cursor},
{"ask",			DT_MLT,		(malloc_t)ASK},
{"askcc",		DT_BOL,		(malloc_t)&prompt_for_cc},
{"askdelete",		DT_BOL,		(malloc_t)&ask_delete},
{"askkeep",		DT_BOL,		(malloc_t)&ask_keep},
{"askreplycopy",	DT_BOL,		(malloc_t)&ask_reply_copy},
{"askstore",		DT_BOL,		(malloc_t)&ask_store},     
{"attribution",		DT_STR,		(malloc_t)attribution},
{"auto_cc",		DT_SYN,		(malloc_t)"copy"},
{"autocopy",		DT_MLT,		(malloc_t)AUTOCOPY},
{"bounce",		DT_SYN,		(malloc_t)"bounceback"},
{"bounceback",		DT_NUM,		(malloc_t)&bounceback},
{"builtinlines",	DT_NUM,		(malloc_t)&builtin_lines},
{"calendar",		DT_STR,		(malloc_t)raw_calendar_file},
{"cc",			DT_SYN,		(malloc_t)"askcc"},
{"charset",		DT_STR,		(malloc_t)charset},
{"compatcharsets",	DT_STR,		(malloc_t)charset_compatlist},
{"configoptions",	DT_STR,		(malloc_t)config_options},
{"confirmappend",	DT_BOL,		(malloc_t)&confirm_append},
{"confirmcreate",	DT_BOL,		(malloc_t)&confirm_create},
{"confirmfiles",	DT_BOL,		(malloc_t)&confirm_files},
{"confirmfolders",	DT_BOL,		(malloc_t)&confirm_folders},
{"confirmtagsave",	DT_BOL,		(malloc_t)&confirm_tag_save},
{"copy",		DT_BOL,		(malloc_t)&auto_cc},
{"delete",		DT_SYN,		(malloc_t)"alwaysdelete"},
{"displaycharset",	DT_STR,		(malloc_t)display_charset},
{"easyeditor",		DT_STR,		(malloc_t)e_editor},
{"editflush",		DT_BOL,		(malloc_t)&edit_flush},
{"editor",		DT_STR,		(malloc_t)raw_editor},
{"escape",		DT_CHR,		(malloc_t)&escape_char},
{"folders",		DT_SYN,		(malloc_t)"maildir"},
{"forcename",		DT_BOL,		(malloc_t)&force_name},
{"form",		DT_SYN,		(malloc_t)"forms"},
{"forms",		DT_BOL,		(malloc_t)&allow_forms},
{"fullname",		DT_STR,		(malloc_t)user_fullname},
{"fwdattribution",	DT_STR,		(malloc_t)fwdattribution},
{"hostdomain",		DT_STR|FL_SYS,	(malloc_t)host_domain},
{"hostfullname",	DT_STR|FL_SYS,	(malloc_t)host_fullname},
{"hostname",		DT_STR|FL_SYS,	(malloc_t)host_name},
{"hpkeypad",		DT_NOP,		(malloc_t)NULL},
{"hpsoftkeys",		DT_SYN,		(malloc_t)"softkeys"},
{"incomingfolders",     DT_INC,         (malloc_t)magiclist},
{"keep",		DT_SYN,		(malloc_t)"keepempty"},
{"keepempty",		DT_BOL,		(malloc_t)&keep_empty_files},
{"keypad",		DT_NOP,		(malloc_t)NULL},
{"localsignature",	DT_STR,		(malloc_t)raw_local_signature},
{"mailbox",		DT_SYN,		(malloc_t)"receivedmail"},
{"maildir",		DT_STR,		(malloc_t)raw_folders},
{"mailedit",		DT_SYN,		(malloc_t)"editor"},
{"menu",		DT_BOL|FL_AND,	(malloc_t)&mini_menu},
{"menus",		DT_SYN,		(malloc_t)"menu"},
{"metoo",		DT_BOL,		(malloc_t)&metoo},
{"movepage",		DT_BOL,		(malloc_t)&move_when_paged},
{"movewhenpaged",	DT_SYN,		(malloc_t)"movepage"},
{"name",		DT_SYN,		(malloc_t)"fullname"},
{"names",		DT_BOL,		(malloc_t)&names_only},
{"noheader",		DT_BOL,		(malloc_t)&noheader},
{"page",		DT_SYN,		(malloc_t)"pager"},
{"pager",		DT_STR,		(malloc_t)raw_pager},
{"pointnew",		DT_BOL,		(malloc_t)&point_to_new},
{"pointtonew",		DT_SYN,		(malloc_t)"pointnew"},
{"precedences",		DT_STR,		(malloc_t)allowed_precedences},
{"prefix",		DT_STR|FL_NOSPC, (malloc_t)prefixchars},
{"print",		DT_STR,		(malloc_t)raw_printout},
{"printhdrs",		DT_STR,		(malloc_t)printhdrs},
{"printmail",		DT_SYN,		(malloc_t)"print"},
{"promptafter",		DT_BOL,		(malloc_t)&prompt_after_pager},
{"question",		DT_SYN,		(malloc_t)"ask"},
{"readmsginc",		DT_NUM,		(malloc_t)&readmsginc},
{"receivedmail",	DT_STR,		(malloc_t)raw_recvdmail},
{"remotesignature",	DT_STR,		(malloc_t)raw_remote_signature},
{"replycopy",		DT_BOL,		(malloc_t)&reply_copy},
{"resolve",		DT_BOL,		(malloc_t)&resolve_mode},
{"savebyalias",		DT_BOL,		(malloc_t)&save_by_alias},
{"savebyname",		DT_BOL,		(malloc_t)&save_by_name},
{"savemail",		DT_SYN,		(malloc_t)"sentmail"},
{"savename",		DT_SYN,		(malloc_t)"savebyname"},
{"saveto",		DT_SYN,		(malloc_t)"sentmail"},
{"sentmail",		DT_STR,		(malloc_t)raw_sentmail},
{"shell",		DT_STR,		(malloc_t)raw_shell},
{"showmlists",		DT_BOL,		(malloc_t)&show_mlists},   
{"showreply",		DT_BOL,		(malloc_t)&show_reply},
{"sigdashes",		DT_BOL,		(malloc_t)&sig_dashes},
{"signature",		DT_MLT,		(malloc_t)SIGS},
{"sleepmsg",		DT_NUM,		(malloc_t)&sleepmsg},
{"softkeys",		DT_BOL|FL_OR,	(malloc_t)&hp_softkeys},
{"sort",		DT_SYN,		(malloc_t)"sortby"},
{"sortby",		DT_SRT,		(malloc_t)&sortby},
{"store",		DT_SYN,		(malloc_t)"alwaysstore"},
{"textencoding",	DT_NOP,		(malloc_t)NULL},
{"timeout",		DT_NUM,		(malloc_t)&timeout},
{"titles",		DT_BOL,		(malloc_t)&title_messages},
{"tmpdir",		DT_STR,		(malloc_t)raw_temp_dir},
{"tochars",		DT_STR|FL_NOSPC, (malloc_t)to_chars},
{"userlevel",		DT_NUM,		(malloc_t)&user_level},
{"username",		DT_SYN,		(malloc_t)"fullname"},
{"usetite",		DT_BOL|FL_AND,	(malloc_t)&use_tite},
{"visualeditor",	DT_STR,		(malloc_t)v_editor},
{"weed",		DT_BOL,		(malloc_t)&filter},
{"weedout",		DT_WEE,		(malloc_t)weedlist},

};

#define SO_EXTERN
#define SO_INIT(X) = X

#else /*}!SO_INTERN{*/

#define SO_EXTERN extern
#define SO_INIT(X)

#endif /*}!SO_INTERN*/

SO_EXTERN save_info_recs *save_info
	SO_INIT(save_info_data);
SO_EXTERN int NUMBER_OF_SAVEABLE_OPTIONS 
	SO_INIT(sizeof(save_info_data)/sizeof(save_info_recs));

