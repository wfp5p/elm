#include "elm_defs.h"
#include "elm_globals.h"
#include "s_fbrowser.h"
#include "port_stat.h"
#include "port_dirent.h"
#include <time.h>
#ifdef PWDINSYS
#  include <sys/pwd.h>
#else
#  include <pwd.h>
#endif

#define FB_CHUNK	64			/* malloc increment	*/
#define FB_DUMMYMODE	(user_level == 0)	/* more handholding	*/

#define IS_DOT(s)	((s)[0] == '.' && (s)[1] == '\0')
#define IS_DOTDOT(s)	((s)[0] == '.' && (s)[1] == '.' && (s)[2] == '\0')

/* WARNING - side effects */
#define PutRight(line, str) PutLine((line), COLS-(strlen(str)+2), (str))

#define S_(sel, str)	catgets(elm_msg_cat, FbrowserSet, (sel), (str))


/*
 * Locations within the screen display.
 */
#define FBLINE_TITLE	0		/* page title			*/
#define FBLINE_CURR	1		/* current dir/pat display	*/
#define FBLINE_EHDR	3		/* entry column header		*/
#define FBLINE_LTOP	(FB_DUMMYMODE ? 5 : 3)
					/* top line of listing window	*/
					/* ...DUMMYMODE adds titles	*/
#define FBLINE_LBOT	(LINES - (FB_DUMMYMODE ? 6 : 5))
					/* bot line of listing window	*/
					/* ...DUMMYMODE adds more help	*/
#define FBLINE_INSTRUCT	(LINES-3)	/* instructions display		*/
#define FBLINE_INPUT	(LINES-2)	/* user data entry line		*/

/*
 * Display redraw requests.
 */
#define FBDRAW_NONE	(0)	/* nothing to redisplay			*/
#define FBDRAW_HEADER	(1<<0)	/* redisplay the screen header		*/
#define FBDRAW_LIST	(1<<1)	/* redisplay entire list of files	*/
#define FBDRAW_SELECT	(1<<2)	/* redisplay just the selection change	*/
#define FBDRAW_FOOTER	(1<<3)	/* redisplay the screen footer		*/
#define FBDRAW_ERROR	(1<<4)	/* redisplay the error line		*/
#define FBDRAW_ALL	(~0)	/* redisplay everything			*/

/*
 * Entry sorting order.
 */
#define FBSORT_TYPE_NAME	(1<<0)
#define FBSORT_TYPE_SIZE	(1<<1)
#define FBSORT_TYPE_MTIME	(1<<2)
#define FBSORT_TYPE_MASK	((1<<3)-1)
#define FBSORT_OPT_REVERSE	(1<<3)

/*
 * Entry selection/sizing/placment values.
 */
#define FB_lines_per_page	((FBLINE_LBOT-FBLINE_LTOP) + 1)
#define FB_pagenum(sel)		((sel) / FB_lines_per_page)
#define FB_line(sel)		((sel) % FB_lines_per_page)
#define FB_first_sel_of_page	(FB_pagenum(curr_sel)*FB_lines_per_page)
#define FB_last_sel_of_page	((FB_pagenum(curr_sel)+1)*FB_lines_per_page - 1)
#define FB_first_sel_of_list	(0)
#define FB_last_sel_of_list	(curr_dlist->num_entries - 1)
#define FB_num_pages		(FB_last_sel_of_list/FB_lines_per_page + 1)


/*
 * Full information on a directory being browsed.
 */
typedef struct fb_dirlist {
    struct fb_entryinfo { /* dynamically allocated list of entry recs	*/
	char *name;		/*  entry name	*/
	unsigned short mode;	/*  st_mode	*/
	unsigned short uid;	/*  st_uid	*/
	long size;		/*  st_size	*/
	time_t mtime;		/*  st_mtime	*/
    } *entry;
    int num_entries;	/* number of elements used in entry list	*/
    int alloc_entries;	/* allocated size of the entry list		*/
} FB_DIR;


/*
 * Local data.
 */
static int fb_show_dotfiles = FALSE;		/* display .dot files?	*/
static int fb_sortby = FBSORT_TYPE_NAME;	/* entry sort order	*/
static char fb_save_dir[SLEN];			/* previous dir browsed	*/
static char fb_acursor_on[]  = "->";		/* arrow cursor on	*/
static char fb_acursor_off[] = "  ";		/* arrow cursor blanked	*/

/*
 * Local procedures.
 */
static void fb_submenu_options P_((FB_DIR **, const char *, const char *));
static void fb_disp_enthdr P_((int));
static void fb_disp_entry P_((int, const FB_DIR *, int, int));
static void fb_disp_instr P_((const char *, const char *, const char *, int));
static FB_DIR *fb_start_dir P_((const char *, const char *, int));
static void fb_finish_dir P_((FB_DIR *));
static char *fb_getsorttype P_((void));
static int (*fb_getsortproc P_((void))) P_((const malloc_t, const malloc_t));
static int fbcmp_name_ascending P_((const malloc_t, const malloc_t));
static int fbcmp_name_descending P_((const malloc_t, const malloc_t));
static int fbcmp_size_ascending P_((const malloc_t, const malloc_t));
static int fbcmp_size_descending P_((const malloc_t, const malloc_t));
static int fbcmp_mtime_ascending P_((const malloc_t, const malloc_t));
static int fbcmp_mtime_descending P_((const malloc_t, const malloc_t));
static int fb_mbox_check P_((char *, int));
static int safe_copy P_((char *, const char *, int));
static int safe_mkpath P_((char *, const char *, const char *, int));


int fbrowser_analyze_spec(const char *spec, char *ret_dir, char *ret_pat)
{
    struct stat sbuf;
    char *s;

    (void) strcpy(ret_dir, spec);
    trim_trailing_spaces(ret_dir);
    trim_trailing_slashes(ret_dir);
    if (ret_dir[0] == '\0')
	return FALSE;

    /* if the entry exists then it should be a directory to browse */
    if (stat(ret_dir, &sbuf) == 0) {
	if (!S_ISDIR(sbuf.st_mode)) {
	    /*
	     * This is a file (in which case we do not want to browse)
	     * or it is something that we totally cannot grok.
	     */
	    return FALSE;
	}
	(void) strcpy(ret_pat, "*");
	return TRUE;
    }

    if ((s = strrchr(ret_dir, '/')) != NULL) {
	/* maybe this is a "directory/pattern" specification */
	(void) strcpy(ret_pat, s+1);
	if (s == ret_dir)
	    ret_dir[1] = '\0';	/* dir is "/" */
	else
	    *s = '\0';
    } else {
	/* maybe this is a "pattern" specification */
	(void) strcpy(ret_pat, ret_dir);
	(void) strcpy(ret_dir, ".");
    }

    /* verify the "directory" part really is one */
    if (stat(ret_dir, &sbuf) != 0 || !S_ISDIR(sbuf.st_mode))
	return FALSE;

    /* verify the "pattern" part really is one */
    if (strpbrk(ret_pat, "*?") == NULL)
	return FALSE;

    return TRUE;
}

int fbrowser(char *ret_buf, int ret_bufsiz, const char *start_dir,
	     const char *start_pat, int options, const char *prompt)
/* char *ret_buf;			/\* storage space for final selection	*\/ */
/* int ret_bufsiz;			/\* size of storage space		*\/ */
/* const char *start_dir;		/\* starting directory to browse		*\/ */
/* const char *start_pat;		/\* starting match pattern		*\/ */
/* int options;			/\* options (see FB_XXX in elm_defs.h)	*\/ */
/* const char *prompt;		/\* message for screen header		*\/ */
{
    char curr_dir[SLEN];	/* current directory being browsed	*/
    char curr_pat[SLEN];	/* current match pattern for entries	*/
    FB_DIR *curr_dlist;		/* entries in the current directory	*/
    FB_DIR *new_dlist;		/* set non-NULL to switch directories	*/
    int inp_line, inp_col;	/* cursor position for user input	*/
    int do_redraw;		/* what parts of screen need redrawing	*/
    int bad_cmd;		/* TRUE if last command was bad		*/
    int cmd;			/* command from user			*/
    int curr_sel;		/* currently selected entry		*/
    int prev_sel;		/* selection at previous iteration	*/
    int rc;			/* final return status			*/
    char tmp_buf[SLEN], *np, *s;
    int n, i;

    if (!safe_copy(curr_dir, start_dir, sizeof(curr_dir)))
	return FALSE;
    if (!safe_copy(curr_pat, start_pat, sizeof(curr_pat)))
	return FALSE;
    if ((new_dlist = fb_start_dir(curr_dir, curr_pat, FALSE)) == NULL) {
	error2(S_(FbrowserCannotBrowse, "Cannot browse \"%s/%s\"."),
		    curr_dir, curr_pat);
	return FALSE;
    }
    rc = FALSE;			/* initialize to failure		*/
    curr_dlist = NULL;		/* prevent fb_finish_dir() below	*/
    bad_cmd = FALSE;		/* nothing to bitch about yet		*/

    for (;;) {

	/* complain if last entry was bad */
	if (bad_cmd) {
	    Beep();
	    bad_cmd = FALSE;
	}

	/* see if we need to switch to a new directory */
	if (new_dlist != NULL) {
	    if (curr_dlist != NULL)
		fb_finish_dir(curr_dlist);
	    curr_dlist = new_dlist;
	    new_dlist = NULL;
	    prev_sel = curr_sel = FB_first_sel_of_list;
	    do_redraw = FBDRAW_ALL;
	}

	/* see if the selection moved */
	if (prev_sel != curr_sel) {
	    if (curr_sel < FB_first_sel_of_list) {
		/* adjust for movement beyond start of list */
		if ((curr_sel = FB_first_sel_of_list) == prev_sel) {
		    bad_cmd = TRUE;
		    continue;
		}
	    }
	    if (curr_sel > FB_last_sel_of_list) {
		/* adjust for movement beyond end of list */
		if ((curr_sel = FB_last_sel_of_list) == prev_sel) {
		    bad_cmd = TRUE;
		    continue;
		}
	    }
	    if (FB_pagenum(curr_sel) != FB_pagenum(prev_sel)) {
		/* page changed */
		do_redraw = FBDRAW_ALL;
	    } else {
		/* moved to a selection on this page */
		do_redraw = FBDRAW_SELECT;
	    }
	}

	/* do screen updates */
	if (do_redraw != FBDRAW_NONE) {

	    if (do_redraw == FBDRAW_ALL)
		ClearScreen();

	    /* redraw the title and selection header lines */
	    if (do_redraw & FBDRAW_HEADER) {
		if (do_redraw != FBDRAW_ALL)
		    ClearLine(FBLINE_TITLE);
		CenterLine(FBLINE_TITLE, prompt);
		sprintf(tmp_buf, S_(FbrowserHeaderPageOf, "[page %d/%d]"),
			    FB_pagenum(curr_sel)+1, FB_num_pages);
		PutLine(FBLINE_TITLE, COLS-(strlen(tmp_buf)+2), tmp_buf);
		if (do_redraw != FBDRAW_ALL)
		    ClearLine(FBLINE_CURR);
		PutLine(FBLINE_CURR, 0,
			    S_(FbrowserHeaderDirectory, "Directory: %s"),
			    curr_dir);
		sprintf(tmp_buf, S_(FbrowserHeaderPattern, "Pattern: %s"),
			    curr_pat);
		PutRight(FBLINE_CURR, tmp_buf);
	    }

	    /* display titles on entry list */
	    if (do_redraw == FBDRAW_ALL && FB_DUMMYMODE)
		fb_disp_enthdr(FBLINE_EHDR);

	    /* redraw the entire list of entries */
	    if (do_redraw & FBDRAW_LIST) {
		n = FB_first_sel_of_page;
		for (i = 0 ; i < FB_lines_per_page ; ++i) {
		    if (n <= FB_last_sel_of_list) {
			fb_disp_entry(FBLINE_LTOP+i, curr_dlist,
				    n, (n == curr_sel));
		    } else if (do_redraw != FBDRAW_ALL) {
			ClearLine(FBLINE_LTOP+i);
		    }
		    ++n;
		}
		do_redraw &= ~FBDRAW_SELECT;
	    }

	    /* redraw just the entries with selection changes */
	    if (do_redraw & FBDRAW_SELECT) {
		if (prev_sel >= FB_first_sel_of_page
			&& prev_sel <= FB_last_sel_of_page) {
		    if (arrow_cursor) {
			PutLine(FBLINE_LTOP+FB_line(prev_sel), 0,
				    fb_acursor_off);
		    } else {
			fb_disp_entry(FBLINE_LTOP+FB_line(prev_sel),
				    curr_dlist, prev_sel, FALSE);
		    }
		}
		if (arrow_cursor) {
		    PutLine(FBLINE_LTOP+FB_line(curr_sel), 0,
				fb_acursor_on);
		} else {
		    fb_disp_entry(FBLINE_LTOP+FB_line(curr_sel),
				curr_dlist, curr_sel, TRUE);
		}
	    }

	    /* redraw the instructions and prompt footer lines */
	    if (do_redraw & FBDRAW_FOOTER) {
		fb_disp_instr(
			    S_(FbrowserInstNorm, /*(*/
"Use \"jk+-\" to move, \"/=~.\" to enter name, ENTER to select, or q)uit."),
			    S_(FbrowserInstrDummy1,
"Move the highlighted selection:  j/k = down/up, +/- = down/up page"),
			    S_(FbrowserInstrDummy2, /*(*/
"Press ENTER to make selection, ? for help, or q)uit."),
			    (do_redraw != FBDRAW_ALL));
		PutLine(FBLINE_INPUT, 0, S_(FbrowserMainPrompt, "Command: "));
		GetCursorPos(&inp_line, &inp_col);
	    }

	    if (do_redraw & FBDRAW_ERROR)
		show_last_error();

	    do_redraw = FBDRAW_NONE;

	}

	prev_sel = curr_sel;

	/* prompt for command */
	MoveCursor(inp_line, inp_col);
	CleartoEOLN();
	if ((cmd = GetKey(0)) == KEY_REDRAW) {
	    do_redraw = FBDRAW_ALL;
	    continue;
	}
	if (clear_error())
	    MoveCursor(inp_line, inp_col);

	switch (cmd) {

	case 'q':			/* quit menu */
	    goto done;

	case '?':			/* help */
	    display_helpfile("fbrowser");
	    do_redraw = FBDRAW_ALL;
	    break;

	case ctrl('L'):			/* redraw display */
	    do_redraw = FBDRAW_ALL;
	    break;

	case ctrl('R'):			/* recall previous dir */
	    if (fb_save_dir[0] == '\0') {
		error(S_(FbrowserNoDirectorySaved, "No directory saved."));
		break;
	    }
	    if (strcmp(fb_save_dir, curr_dir) == 0) {
		error1(S_(FbrowserAlreadyIn,
			    "Already in \"%s\"."), fb_save_dir);
		break;
	    }
	    new_dlist = fb_start_dir(fb_save_dir, curr_pat, FALSE);
	    if (new_dlist != NULL) {
		(void) strcpy(tmp_buf, fb_save_dir);
		(void) strcpy(fb_save_dir, curr_dir);
		(void) strcpy(curr_dir, tmp_buf);
	    }
	    break;

	case 'j':			/* down entry */
	case KEY_DOWN:
	    ++curr_sel;
	    break;

	case 'k':			/* up entry */
	case KEY_UP:
	    --curr_sel;
	    break;

	case '+':			/* down page */
	case KEY_NPAGE:
        case KEY_RIGHT:
	    curr_sel += FB_lines_per_page;
	    break;

	case '-':			/* up page */
	case KEY_PPAGE:
        case KEY_LEFT:
	    curr_sel -= FB_lines_per_page;
	    break;

	case '1':			/* first entry */
	case KEY_HOME:
	    curr_sel = FB_first_sel_of_list;
	    break;

	case '*':			/* last entry */
	case KEY_END:
	    curr_sel = FB_last_sel_of_list;
	    break;

#ifdef ALLOW_SUBSHELL
	case '!':			/* subshell */
	    fb_disp_instr((char *)NULL, (char *)NULL, (char *)NULL, TRUE);
	    ClearLine(FBLINE_INPUT);
	    do_redraw = (subshell() ? FBDRAW_ALL : FBDRAW_FOOTER);
	    break;
#endif

#ifdef notdef /*FOO*/
	/*
	 * The "limit" command can be done with
	 * the new read-my-mind dir/file entry.
	 */
	case 'l':			/* change pattern (limit) */
	case 'p':
	    do_redraw = FBDRAW_FOOTER;
	    fb_disp_instr(
			S_(FbrowserLimitInstrNorm,
"Enter new pattern for file listing."),
			S_(FbrowserLimitInstrDummy1,
"Enter new pattern for file listing.  Only matching files will be displayed."),
			S_(FbrowserLimitInstrDummy2,
"In patterns, \"?\" means any one char, and \"*\" means any number of chars."),
			TRUE);
	    PutLine(FBLINE_INPUT, 0, S_(FbrowserLimitPrompt, "New Pattern: "));
	    strcpy(tmp_buf, "*");
	    if (enter_string(tmp_buf, sizeof(tmp_buf), -1, -1, ESTR_REPLACE) < 0
			|| tmp_buf[0] == '\0'
			|| strcmp(tmp_buf, curr_pat) == 0) {
		error(S_(FbrowserNotChanged, "Not changed."));
		break;
	    }
	    if ((new_dlist = fb_start_dir(curr_dir, tmp_buf, TRUE)) != NULL)
		(void) strcpy(curr_pat, tmp_buf);
	    break;
#endif

#ifdef notdef /*FOO*/
	/*
	 * The "chdir" command can be done with
	 * the new read-my-mind dir/file entry.
	 */
	case 'c':			/* change dir */
	    do_redraw = FBDRAW_FOOTER;
	    fb_disp_instr(
			S_(FbrowserDirInstrNorm,
"Enter directory to browse.  \"~\" and \"=\" ok, CTRL/D to abort"),
			S_(FbrowserDirInstrDummy1,
"Enter pathname of directory to browse, or CTRL/D to abort entry."),
			S_(FbrowserDirInstrDummy2,
"You may say \"~\" for your home dir and \"=\" for your folders dir."),
			TRUE);
	    PutLine(FBLINE_INPUT, 0, S_(FbrowserDirPrompt, "New Directory: "));
	    if (enter_string(tmp_buf, sizeof(tmp_buf), -1, -1, ESTR_ENTER) < 0
			|| tmp_buf[0] == '\0') {
		error(S_(FbrowserNotChanged, "Not changed."));
		break;
	    }
	    (void) strcat(tmp_buf, "/");	/* expand_filename silliness */
	    if (!expand_filename(tmp_buf))
		break;
	    trim_trailing_slashes(tmp_buf);	/* more silliness */
	    if ((new_dlist = fb_start_dir(tmp_buf, curr_pat, FALSE)) != NULL) {
		(void) strcpy(fb_save_dir, curr_dir);
		(void) strcpy(curr_dir, tmp_buf);
	    }
	    break;
#endif

	case 'o':			/* change options submenu */
	    fb_submenu_options(&new_dlist, curr_dir, curr_pat);
	    do_redraw = FBDRAW_ALL;
	    break;

	case '/':			/* enter value from "/" */
	    (void) strcpy(tmp_buf, "/");
	    goto enter_value;

	case '=':			/* enter value from "~/Mail/" */
	    (void) strcpy(tmp_buf, "=/");
	    if (!expand_filename(tmp_buf))
		break;
	    (void) strcat(trim_trailing_slashes(tmp_buf), "/");
	    goto enter_value;

	case '~':			/* enter value from "$HOME/" */
	    (void) strcpy(tmp_buf, "~/");
	    if (!expand_filename(tmp_buf))
		break;
	    (void) strcat(trim_trailing_slashes(tmp_buf), "/");
	    goto enter_value;

	case '.':			/* enter value from "." */
	    (void) strcpy(tmp_buf, curr_dir);
	    (void) strcat(tmp_buf, "/");

enter_value:
	    do_redraw = FBDRAW_FOOTER;
#define FbFOO 1
	    fb_disp_instr(
			S_(FbFOO,
"Enter file name (pattern ok) or directory.  CTRL/D to abort entry."),
			S_(FbFOO,
"Enter either the name of a file to select or a directory to browse."),
			S_(FbFOO,
"Press CTRL/D to abort entry and stay in current directory."),
			TRUE);
	    PutLine(FBLINE_INPUT, 0, S_(FbFOO, "Select: "));
	    if (enter_string(tmp_buf, sizeof(tmp_buf), -1, -1, ESTR_UPDATE) < 0
			|| tmp_buf[0] == '\0') {
		error(S_(FbrowserNotChanged, "Not changed."));
		break;
	    }
	    trim_trailing_slashes(tmp_buf);

	    {
		struct stat sbuf;
		if (stat(tmp_buf, &sbuf) == 0 && S_ISREG(sbuf.st_mode)) {
		    if (!safe_copy(ret_buf, tmp_buf, ret_bufsiz))
			break;
		    if (!fb_mbox_check(ret_buf, options))
			break;
		    rc = TRUE;
		    goto done;
		}
	    }

	    {
		char tmp_dir[SLEN], tmp_pat[SLEN];
		if (!fbrowser_analyze_spec(tmp_buf, tmp_dir, tmp_pat)) {
		    error(S_(FbFOO, "Not a valid directory or pattern."));
		    break;
		}
		if ((new_dlist = fb_start_dir(tmp_dir, tmp_pat,
				streq(tmp_dir, curr_dir))) == NULL)
		    break;
		(void) strcpy(curr_dir, tmp_dir);
		(void) strcpy(curr_pat, tmp_pat);
	    }

	    (void) strcpy(fb_save_dir, curr_dir);
	    break;

	case '\r':			/* accept entry */
	case '\n':
	case ' ':
	    if (!S_ISDIR(curr_dlist->entry[curr_sel].mode)) {
		if (!safe_mkpath(ret_buf, curr_dir,
			    curr_dlist->entry[curr_sel].name, ret_bufsiz))
		    break;
		if (!fb_mbox_check(ret_buf, options))
		    break;
		rc = TRUE;
		goto done;
	    }
	    (void) strcpy(tmp_buf, curr_dir);
	    np = curr_dlist->entry[curr_sel].name;
	    if (IS_DOT(np)) {
		/* stay where we are */
		np = NULL;
	    } else if (IS_DOTDOT(np) && (s = strrchr(tmp_buf, '/')) != NULL
			    && strcmp(s, "/..") != 0) {
		/* trim "/.." from end */
		if (s == tmp_buf)
		    tmp_buf[1] = '\0';
		else
		    *s = '\0';
		np = NULL;
	    }
	    if (np != NULL) {
		/* append name to end of path */
		if (!safe_mkpath(tmp_buf, (char *)NULL, np, sizeof(tmp_buf)))
		    break;
	    }
	    if ((new_dlist = fb_start_dir(tmp_buf, curr_pat, FALSE)) != NULL) {
		(void) strcpy(fb_save_dir, curr_dir);
		(void) strcpy(curr_dir, tmp_buf);
	    }
	    break;

	default:
	    bad_cmd = TRUE;
	    break;

	}

    }

done:
    (void) strcpy(fb_save_dir, curr_dir);
    fb_finish_dir(curr_dlist);
    for (i = FBLINE_LBOT+1 ; i < LINES ; ++i)
	ClearLine(i);	/* clear out entire bottom but the error line */
    return rc;
}


#define FBOLINE_TITLE		(FBLINE_TITLE)
#define FBOLINE_OPT_DOTF	(FBOLINE_TITLE+2)
#define FBOLINE_OPT_SORT	(FBOLINE_TITLE+3)
#define FBOLINE_INPUT		(FBLINE_INPUT)
#define FBOLINE_INSTRUCT	(FBLINE_INSTRUCT)

#define FBOCOL_OPTVAL		25
#define FBOCOL_OPTHELP		(FBOCOL_OPTVAL+20)

#define FBOSEL_NONE		0
#define FBOSEL_DOTF		1
#define FBOSEL_SORT		2


static void fb_submenu_options(FB_DIR **dl_p, const char *curr_dir, const char *curr_pat)
{
    int orig_show_dotfiles = fb_show_dotfiles;
    int orig_sortby = fb_sortby;
    int done = FALSE;
    int do_redraw = TRUE;
    int prev_sel = FBOSEL_NONE;
    int curr_sel = FBOSEL_NONE;
    int inp_line, inp_col, cmd, m;
    static char title_fmt[] = "%-22s : ";
    static char value_fmt[] = "%-20s";

    while (!done) {

	/* screen title */
	if (do_redraw) {
	    ClearScreen();
	    CenterLine(FBOLINE_TITLE, S_(FbrowserOptionsTitle,
			"File Selection Browser -- Options"));
	}

	/* fb_show_dotfiles setting */
	if (do_redraw) {
	    PutLine(FBOLINE_OPT_DOTF, 0, title_fmt,
			S_(FbrowserOptionsTitleDotf, /*(*/
			"D)ot files displayed?"));
	}
	if (do_redraw || curr_sel == FBOSEL_DOTF) {
	    PutLine(FBOLINE_OPT_DOTF, FBOCOL_OPTVAL, value_fmt,
			(fb_show_dotfiles
			? S_(FbrowserOptionsOn, "ON")
			: S_(FbrowserOptionsOff, "OFF")));
	}
	if (curr_sel != FBOSEL_DOTF && prev_sel == FBOSEL_DOTF && !do_redraw) {
	    MoveCursor(FBOLINE_OPT_DOTF, FBOCOL_OPTHELP);
	    CleartoEOLN();
	}
	if (curr_sel == FBOSEL_DOTF && (do_redraw || prev_sel != curr_sel)) {
	    PutRight(FBOLINE_OPT_DOTF, S_(FbrowserOptionsSpaceToToggle,
			"(SPACE to toggle)"));
	}

	/* fb_sortby setting */
	if (do_redraw) {
	    PutLine(FBOLINE_OPT_SORT, 0, title_fmt,
			S_(FbrowserOptionsTitleSort, /*(*/
			"S)orting criteria"));
	}
	if (do_redraw || curr_sel == FBOSEL_SORT) {
	    PutLine(FBOLINE_OPT_SORT, FBOCOL_OPTVAL, value_fmt,
			fb_getsorttype());
	}
	if (curr_sel != FBOSEL_SORT && prev_sel == FBOSEL_SORT && !do_redraw) {
	    MoveCursor(FBOLINE_OPT_DOTF, FBOCOL_OPTHELP);
	    CleartoEOLN();
	}
	if (curr_sel == FBOSEL_SORT && (do_redraw || prev_sel != curr_sel)) {
	    PutRight(FBOLINE_OPT_SORT, S_(FbrowserOptionsNextOrReverse, /*(*/
			"(SPACE for next, or r)everse)"));
	}

	/* instructions */
	if (do_redraw || curr_sel != prev_sel) {
	    if (!do_redraw)
		ClearLine(FBLINE_INSTRUCT);
	    switch (curr_sel) {
	    case FBOSEL_DOTF:
		CenterLine(FBOLINE_INSTRUCT, S_(FbrowserOptionsInstructDotf,
"Do you want filenames beginning with \".\" dot to be displayed?"));
		break;
	    case FBOSEL_SORT:
		CenterLine(FBOLINE_INSTRUCT, S_(FbrowserOptionsInstructSort,
"How should the filename listing be sorted?"));
		break;
	    case FBOSEL_NONE:
	    default:
		CenterLine(FBOLINE_INSTRUCT, S_(FbrowserOptionsInstructMain,
/*(*/ "Select option letter, or q)uit to return to File Selection Browser."));
		break;
	    }
	}

	/* command prompt */
	if (do_redraw || curr_sel != prev_sel) {
	    if (curr_sel == FBOSEL_NONE) {
		PutLine(FBLINE_INPUT, 0, S_(FbrowserOptionsPromptCommand,
			    "Command: "));
		GetCursorPos(&inp_line, &inp_col);
		WriteChar('q');
		CleartoEOLN();
	    } else if (prev_sel == FBOSEL_NONE) {
		ClearLine(FBLINE_INPUT);
	    }
	}

	if (do_redraw)
	    show_last_error();

	switch (curr_sel) {
	case FBOSEL_DOTF:
	    MoveCursor(FBOLINE_OPT_DOTF, FBOCOL_OPTVAL);
	    break;
	case FBOSEL_SORT:
	    MoveCursor(FBOLINE_OPT_SORT, FBOCOL_OPTVAL);
	    break;
	case FBOSEL_NONE:
	    MoveCursor(inp_line, inp_col);
	    break;
	default:
	    error1(S_(FbrowserOptionsBogusSel,
			"Wierd!!  curr_sell was %d??  It's fixed now."),
			curr_sel);
	    curr_sel = FBOSEL_NONE;
	    continue;
	}

	do_redraw = FALSE;
	prev_sel = curr_sel;

	cmd = ReadCh();
	clear_error();

	switch (cmd) {
	case ctrl('D'):			/* abort without change */
	    fb_show_dotfiles = orig_show_dotfiles;
	    fb_sortby = orig_sortby;
	    set_error(S_(FbrowserOptionsNotChanged, "Options not changed."));
	    return;
	case ctrl('L'):			/* redraw */
	    do_redraw = TRUE;
	    continue;
	case ctrl('R'):			/* restore */
	    fb_show_dotfiles = orig_show_dotfiles;
	    fb_sortby = orig_sortby;
	    do_redraw = TRUE;
	    curr_sel = prev_sel = FBOSEL_NONE;
	    set_error(S_(FbrowserOptionsChangesUndone,
			"All option changes have been undone."));
	    continue;
	case '?':			/* help */
	    display_helpfile("fbrowser");
	    do_redraw = TRUE;
	    continue;
	}

	switch (curr_sel) {

	case FBOSEL_DOTF:
	    switch (cmd) {
	    case ' ':
		fb_show_dotfiles = !fb_show_dotfiles;
		break;
	    default:
		curr_sel = FBOSEL_NONE;
		break;
	    }
	    break;

	case FBOSEL_SORT:
	    switch (cmd) {
	    case ' ':
		if ((m = ((fb_sortby<<1) & FBSORT_TYPE_MASK)) == 0)
		    m = 1;
		fb_sortby = (fb_sortby & ~FBSORT_TYPE_MASK) | m;
		break;
	    case 'r':
		fb_sortby ^= FBSORT_OPT_REVERSE;
		break;
	    default:
		curr_sel = FBOSEL_NONE;
		break;
	    }
	    break;

	case FBOSEL_NONE:
	default:
	    switch (cmd) {
	    case 'd':
		curr_sel = FBOSEL_DOTF;
		break;
	    case 's':
		curr_sel = FBOSEL_SORT;
		break;
	    case 'q':
	    case ' ':
	    case '\r':
	    case '\n':
		done = TRUE;
		break;
	    default:
		Beep();
		break;
	    }
	    break;

	}

    }

    /* see if anything changed */
    if (orig_show_dotfiles != fb_show_dotfiles || orig_sortby != fb_sortby)
	*dl_p = fb_start_dir(curr_dir, curr_pat, TRUE);
    else
	set_error(S_(FbrowserOptionsNotChanged, "Options not changed."));
}

static void fb_disp_enthdr(int line)
{
    int w;
    time_t tval;
    char buf[SLEN];

    ClearLine(line);
    MoveCursor(line, strlen(fb_acursor_off));
    if (!FB_DUMMYMODE) {
	PutLine(-1, -1, "%-10.10s",
		    S_(FbrowserEnthdrPermission, "permission"));
    }
    PutLine(-1, -1, " %-4.4s", S_(FbrowserEnthdrType, "type"));
    if (!FB_DUMMYMODE)
	PutLine(-1, -1, " %-8.8s", S_(FbrowserEnthdrOwner, "owner"));
    PutLine(-1, -1, " %8.8s", S_(FbrowserEnthdrSize, "size"));
    time(&tval);
    strftime(buf, sizeof(buf), S_(FbrowserEntryDateFmt, "%y-%b-%d"),
		localtime(&tval)); w = strlen(buf);
    sprintf(buf, "%-*.*s", w, w, S_(FbrowserEnthdrDate, "date"));
    PutLine(-1, -1, "  %s", buf);
    PutLine(-1, -1, "  %s", S_(FbrowserEnthdrFilename, "filename"));
}

static void fb_disp_entry(int line, const FB_DIR *dl, int n, int selected)
{
    int w, ech;
    unsigned mode;
    struct passwd *pw;
    char out_buf[SLEN], *etype, *bp, *np;

    static char *perms_list[] =
	{ "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx", NULL };

    /* these are used a lot - make static to avoid re-lookup */
    static char *fbstr_entry_file;
    static char *fbstr_entry_dir;
    static char *fbstr_entry_date_fmt;
    static char *fbstr_entry_current_dir;
    static char *fbstr_entry_parrent_dir;

    if (fbstr_entry_file == NULL) {
	fbstr_entry_file = S_(FbrowserEntryFile,
	  "file");
	fbstr_entry_dir = S_(FbrowserEntryDir,
	  "dir");
	fbstr_entry_date_fmt = S_(FbrowserEntryDateFmt,
	  "%y-%b-%d");
	fbstr_entry_current_dir = S_(FbrowserEntryCurrentDir,
	  ".  [current directory]");
	fbstr_entry_parrent_dir = S_(FbrowserEntryParentDir,
	  ".. [parent directory]");
    }

    (void) strcpy(out_buf,
	(arrow_cursor && selected ? fb_acursor_on : fb_acursor_off));
    bp = out_buf + strlen(out_buf);

    mode = dl->entry[n].mode;
    switch (mode & S_IFMT) {
    case S_IFREG:
	ech = '-';
	etype = fbstr_entry_file;
	break;
    case S_IFDIR:
	ech = 'd';
	etype = fbstr_entry_dir;
	break;
    default:
	ech = '?';
	etype = "";
	break;
    }

    if (!FB_DUMMYMODE) {
	*bp++ = ech;
	(void) strcpy(bp, perms_list[(mode>>6) & 07]);
	bp += 3;
	(void) strcpy(bp, perms_list[(mode>>3) & 07]);
	bp += 3;
	(void) strcpy(bp, perms_list[ mode     & 07]);
	bp += 3;
    }

    *bp++ = ' ';
    (void) sprintf(bp, "%-4s", etype);
    bp += 4;

    if (!FB_DUMMYMODE) {
	*bp++ = ' ';
	if ((pw = getpwuid(dl->entry[n].uid)) != NULL)
	    (void) sprintf(bp, "%-8.8s", pw->pw_name);
	else
	    (void) sprintf(bp, "%-8d", dl->entry[n].uid);
	bp += 8;
    }

    *bp++ = ' ';
    sprintf(bp, "%8lu", (unsigned long)dl->entry[n].size);
    bp += 8;

    *bp++ = ' ';
    *bp++ = ' ';
    bp += strftime(bp, sizeof(out_buf)-(bp-out_buf),
		fbstr_entry_date_fmt, localtime(&dl->entry[n].mtime));

    *bp++ = ' ';
    *bp++ = ' ';
    w = COLS - ((bp-out_buf)+2);
    np = dl->entry[n].name;
    if (IS_DOT(np))
	np = fbstr_entry_current_dir;
    else if (IS_DOTDOT(np))
	np = fbstr_entry_parrent_dir;
    (void) sprintf(bp, "%-*.*s", w, w, np);

    MoveCursor(line, 0);
    if (selected && !arrow_cursor)
	StartStandout();
    PutLine(-1, -1, out_buf);
    if (selected && !arrow_cursor)
	EndStandout();
}

static void fb_disp_instr(const char *instr_normal, const char *instr_dummy1,
			  const char *instr_dummy2, int do_erase)
{
    if (FB_DUMMYMODE) {
	if (do_erase)
	    ClearLine(FBLINE_INSTRUCT-1);
	if (instr_dummy1 != NULL && *instr_dummy1)
	    CenterLine(FBLINE_INSTRUCT-1, instr_dummy1);
	if (do_erase)
	    ClearLine(FBLINE_INSTRUCT);
	if (instr_dummy2 != NULL && *instr_dummy2)
	    CenterLine(FBLINE_INSTRUCT, instr_dummy2);
    } else {
	if (do_erase)
	    ClearLine(FBLINE_INSTRUCT);
	if (instr_normal != NULL && *instr_normal)
	    CenterLine(FBLINE_INSTRUCT, instr_normal);
    }
}


static FB_DIR *fb_start_dir(const char *sel_dir, const char *sel_pat,
			    int is_rescan)
{
    FB_DIR *dl;
    char pname[SLEN], *np;
    int nread, nsel, plen;
    DIR *dirp;
    struct DIRENT *dirent;
    struct stat sbuf;

    if (!safe_mkpath(pname, sel_dir, "", sizeof(pname)-32)) {
	/* don't even bother if the path buffer is close to overflowing */
	return (FB_DIR *)NULL;
    }
    plen = strlen(pname);
    np = pname+plen;

    if ((dirp = opendir(sel_dir)) == NULL) {
	error2(S_(FbrowserEnterdirCannotRead,
		    "Cannot read \"%s\".  [%s]"), sel_dir, strerror(errno));
	return (FB_DIR *)NULL;
    }

    dl = (FB_DIR *) safe_malloc(sizeof(FB_DIR));
    dl->entry = (struct fb_entryinfo *)
		safe_malloc(FB_CHUNK*sizeof(struct fb_entryinfo));
    dl->num_entries = 0;
    dl->alloc_entries = FB_CHUNK;

    nread = nsel = 0;
    error1(S_(FbrowserEnterdirScanning, "Scanning %s ..."), sel_dir);
    FlushOutput();

    while ((dirent = readdir(dirp)) != NULL) {

	++nread;
	/* FOO - this causes files to disappear mysteriously from the
	 * listing.  Maybe a warning of some sort? */
	(void) strfcpy(np, dirent->d_name, sizeof(pname)-plen);
	if (stat(pname, &sbuf) < 0)
	    continue;

	/* see if we want to accept this entry */
	switch (sbuf.st_mode & S_IFMT) {
	case S_IFDIR:
	    /*
	     * Always accept "." and "..".
	     * Accept any other dirs subject to dot_entries qualification.
	     */
	    if (np[0] == '.' && !fb_show_dotfiles
			&& !IS_DOT(np) && !IS_DOTDOT(np))
		continue;
	    break;
	case S_IFREG:
	    /*
	     * Accept this file it matches the pattern, subject to
	     * dot_entries qualification.
	     */
	    if (np[0] == '.' && !fb_show_dotfiles && sel_pat[0] != '.')
		continue;
	    if (!patmatch(sel_pat, np, PM_FANCHOR|PM_BANCHOR))
		continue;
	    break;
	default:
	    /*
	     * Reject anything other than regular files and directories.
	     */
	    continue;
	}

	/* add this entry to the list */
	if (dl->num_entries >= dl->alloc_entries) {
	    dl->alloc_entries += FB_CHUNK;
	    dl->entry = (struct fb_entryinfo *)
			safe_realloc( (malloc_t)dl->entry,
			dl->alloc_entries*sizeof(struct fb_entryinfo));
	}
	dl->entry[dl->num_entries].name = safe_strdup(np);
	dl->entry[dl->num_entries].mode = sbuf.st_mode;
	dl->entry[dl->num_entries].uid = sbuf.st_uid;
	dl->entry[dl->num_entries].size = sbuf.st_size;
	dl->entry[dl->num_entries].mtime = sbuf.st_mtime;
	++dl->num_entries;
	++nsel;

    }

    closedir(dirp);

    /* sort the list - leaving "." and ".." at the top */
    qsort((malloc_t)(dl->entry+2), dl->num_entries-2,
		sizeof(struct fb_entryinfo),
#ifdef ANSI_C
	    /*
	     * Some ANSI compilers get bent out of shape that the
	     * sort procedures use (malloc_t) instead of (void *).
	     * This cast avoids a spurious complaint.
	     */
		(int(*)(const void *, const void *))
#endif
		fb_getsortproc());

    np = (is_rescan
		? S_(FbrowserEnterdirRescan, "Rescan")
		: S_(FbrowserEnterdirScan, "Scan"));
    if (nsel == nread) {
	sprintf(pname, S_(FbrowserEnterdirSelectedAll,
		    "%s complete - selected %d entries."), np, nsel);
    } else {
	sprintf(pname, S_(FbrowserEnterdirSelectedEntries,
		    "%s complete - selected %d of %d entries."),
		    np, nsel, nread);
    }
    set_error(pname);
    return dl;
}


static void fb_finish_dir(FB_DIR *dl)
{
    int i;
    for (i = 0 ; i < dl->num_entries ; ++i)
	free((malloc_t)dl->entry[i].name);
    free((malloc_t)dl->entry);
    free((malloc_t)dl);
}


static int (*fb_getsortproc(void))(const malloc_t, const malloc_t)
{
    switch (fb_sortby & FBSORT_TYPE_MASK) {
    case FBSORT_TYPE_NAME:
	return (!(fb_sortby & FBSORT_OPT_REVERSE)
		    ? fbcmp_name_ascending : fbcmp_name_descending);
    case FBSORT_TYPE_SIZE:
	return (!(fb_sortby & FBSORT_OPT_REVERSE)
		    ? fbcmp_size_ascending : fbcmp_size_descending);
    case FBSORT_TYPE_MTIME:
	return (!(fb_sortby & FBSORT_OPT_REVERSE)
		    ? fbcmp_mtime_ascending : fbcmp_mtime_descending);
    }
    return fbcmp_name_ascending;
}

static char *fb_getsorttype(void)
{
    static char sbuf[32];
    char *s;

    if (fb_sortby & FBSORT_OPT_REVERSE) {
	strcpy(sbuf, S_(FbrowserSorttypeReverse, "Reverse"));
	s = sbuf + strlen(sbuf);
	*s++ = ' ';
    } else {
	s = sbuf;
    }
    switch (fb_sortby & FBSORT_TYPE_MASK) {
    	case FBSORT_TYPE_NAME:
	    (void) strcpy(s, S_(FbrowserSorttypeName, "Name"));
	    break;
    	case FBSORT_TYPE_SIZE:
	    (void) strcpy(s, S_(FbrowserSorttypeSize, "Size"));
	    break;
    	case FBSORT_TYPE_MTIME:
	    (void) strcpy(s, S_(FbrowserSorttypeDate, "Date"));
	    break;
    	default:
	    (void) strcpy(s, S_(FbrowserSorttypeUnknown, "?Unknown?"));
	    break;
    }
    return sbuf;
}


static int fbcmp_name_ascending(const malloc_t p1, const malloc_t p2)
{
    const struct fb_entryinfo *e1, *e2;
    e1 = (struct fb_entryinfo *) p1;
    e2 = (struct fb_entryinfo *) p2;
    return strcmp(e1->name, e2->name);
}

static int fbcmp_name_descending(const malloc_t p1, const malloc_t p2)
{
    const struct fb_entryinfo *e1, *e2;
    e1 = (struct fb_entryinfo *) p1;
    e2 = (struct fb_entryinfo *) p2;
    return -strcmp(e1->name, e2->name);
}

static int fbcmp_size_ascending(const malloc_t p1, const malloc_t p2)
{
    const struct fb_entryinfo *e1, *e2;
    e1 = (struct fb_entryinfo *) p1;
    e2 = (struct fb_entryinfo *) p2;
    return (int) (e1->size - e2->size);
}

static int fbcmp_size_descending(const malloc_t p1, const malloc_t p2)
{
    const struct fb_entryinfo *e1, *e2;
    e1 = (struct fb_entryinfo *) p1;
    e2 = (struct fb_entryinfo *) p2;
    return (int) (e2->size - e1->size);
}

static int fbcmp_mtime_ascending(const malloc_t p1, const malloc_t p2)
{
    const struct fb_entryinfo *e1, *e2;
    e1 = (struct fb_entryinfo *) p1;
    e2 = (struct fb_entryinfo *) p2;
    return (int) (e1->mtime - e2->mtime);
}

static int fbcmp_mtime_descending(const malloc_t p1, const malloc_t p2)
{
    const struct fb_entryinfo *e1, *e2;
    e1 = (struct fb_entryinfo *) p1;
    e2 = (struct fb_entryinfo *) p2;
    return (int) (e2->mtime - e1->mtime);
}


static int fb_mbox_check(char *fname, int options)
{
    char buf[SLEN];
    int ok;
    FILE *fp;
    struct stat sbuf;

    if (options & FB_READ)
	options |= FB_EXIST;	/* readable implies it exists */

    if (stat(fname, &sbuf) < 0) {
	if (options & FB_EXIST) {
	    error2(S_(FbrowserMboxCannotGetStatus,
			"Cannot get \"%s\" status.  [%s]"),
			fname, strerror(errno));
	    return FALSE;
	}
	return TRUE;
    }

    if (!S_ISREG(sbuf.st_mode)) {
	error1(S_(FbrowserMboxNotRegularFile,
		    "\"%s\" is not a regular file."), fname);
	return FALSE;
    }

    if (options & FB_READ) {
	if (sbuf.st_uid == geteuid())
	    ok = !!(sbuf.st_mode & 0400);
	else if (sbuf.st_gid == getgid())
	    ok = !!(sbuf.st_mode & 0040);
	else
	    ok = !!(sbuf.st_mode & 0004);
	if (!ok) {
	    error1(S_(FbrowserMboxNoPermissionRead,
			"No permission to read \"%s\"."), fname);
	    return FALSE;
	}
    }

    if (options & FB_WRITE) {
	if (sbuf.st_uid == geteuid())
	    ok = !!(sbuf.st_mode & 0200);
	else if (sbuf.st_gid == getgid())
	    ok = !!(sbuf.st_mode & 0020);
	else
	    ok = !!(sbuf.st_mode & 0002);
	if (!ok) {
	    error1(S_(FbrowserMboxNoPermissionWrite,
			"No permission to write \"%s\"."), fname);
	    return FALSE;
	}
    }

    if (options & FB_MBOX) {
	if ((fp = file_open(fname, "r")) == NULL)
	    return FALSE;
	while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] == '\n')
	    ;
	if (file_close(fp, fname) < 0)
	    return FALSE;
	ok = strbegConst(buf, "From ");
	if (!ok) {
	    error1(S_(FbrowserNotValidMailbox,
			"\"%s\" is not a valid mailbox."), fname);
	    return FALSE;
	}
    }

    return TRUE;
}


static int safe_copy(char *dst, const char *src, int dstsiz)
{
    if (dstsiz > 0) {
	/* strncpy() is supposed to zero-fill the target, Xenix is broke */
	dst[dstsiz-1] = '\0';
	(void) strncpy(dst, src, dstsiz);
	if (dst[dstsiz-1] == '\0')
	    return TRUE;
    }
    error(S_(FbrowserSafeCopyOverflow,
      "Internal Error - buffer not large enough to hold return result."));
    return FALSE;
}


static int safe_mkpath(char *pbuf, const char *dname, const char *fname, int pbufsiz)
{
    int len;
    if (dname != NULL && !safe_copy(pbuf, dname, pbufsiz))
	return FALSE;
    len = strlen(pbuf);
    if (pbuf[0] != '/' || pbuf[1] != '\0') {
	if (!safe_copy(pbuf+len, "/", pbufsiz-len))
	    return FALSE;
	++len;
    }
    return safe_copy(pbuf+len, fname, pbufsiz-len);
}
