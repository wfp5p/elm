/*
 * Declaration of routines in the Elm library.
 * This header normally included through "elm_defs.h".
 */

/* add_site.c */

void add_site P_((char *, const char *, char *));


/* addrmchusr.c */

int addr_matches_user P_((char *, const char *));


/* aliasdb.c */

#ifdef ANSI_C
struct dbz;
#endif
int read_one_alias P_((struct dbz *, struct alias_disk_rec *));
struct alias_rec *fetch_alias P_((struct dbz *, char *));
char *next_addr_in_list P_((char **));


/* atonum.c */

int atonum P_((const char *));


/* basename.c */

char *basename P_((const char *));


/* can_access.c */

int can_access P_((const char *, int));


/* can_open.c */

int can_open P_((const char *, const char *));


/* chloc.c */

int chloc P_((const char *, int));
int qchloc P_((const char *, int));


/* date_util.c */

int cvt_dayname_to_daynum P_((const char *, int *));
int cvt_monthname_to_monthnum P_((const char *, int *));
int cvt_yearstr_to_yearnum P_((const char *, int *));
int cvt_mmddyy_to_dayofyear P_((int, int, int, int *));
int cvt_timezone_to_offset P_((char *, int *));
int cvt_numtz_to_mins P_((const char *));
int cvt_timestr_to_hhmmss P_((const char *, int *, int *, int *));
long make_gmttime P_((int, int, int, int, int, int));


/* elm_access.c */

int elm_access P_((const char *, int));

/* elmrc.c */
void getelmrcName(char *filename, int len);
void setelmrcName(char *name);

/* expand.c */

int expand P_((char *));
char *expand_define P_((const char *));


/* figadrssee.c */

void figure_out_addressee P_((const char *, const char *, char *));


/* gcos_name.c */

char *gcos_name P_((char *, const char *));


/* getarpdate.c */

char *get_arpa_date P_((void));


/* getfullnam.c */

char *get_full_name P_((const char *));


/* gethostname.c */

void get_hostname P_((char *, int));
void get_hostdomain P_((char *, int));


/* getword.c */

int get_word P_((const char *, int, char *, int));


/* header_cmp.c */

char *header_cmp P_((const char *, const char *, const char *));
int header_ncmp P_((const char *, const char *, int, const char *, int));


/* in_list.c */

int in_list P_((char *, char *));


/* initcommon.c */

void initialize_common P_((void));


/* ldstate.c */

/* environment parameter that points to folder state file */
#define FOLDER_STATE_ENV	"ELMSTATE"

struct folder_state {
    char *folder_name;	/* full pathname to current folder	*/
    int num_mssgs;	/* number of messages in the folder	*/
    long *idx_list;	/* index of seek offsets for messages	*/
    long *clen_list;	/* list of content lengths for mssgs	*/
    int num_sel;	/* number of messages selected		*/
    int *sel_list;	/* list of selected message numbers	*/

};

int load_folder_state_file P_((struct folder_state *));


/* len_next.c */

int len_next_part P_((const char *));


/* mail_gets.c */

int mail_gets P_((char *, int, FILE *));


/* mailfile.c */

	/* ##### defined in mailfile.h  ##### */


/* mcprt.c */

	/* ##### defined in mcprt.h ##### */


/* mcprtlib.c */

	/* ##### defined in mcprtlib.c ###### */


/* mk_aliases.c */

int check_alias P_((char *));
void despace_address P_((char *));
int do_newalias P_((char *, char *, int, int));


/* mk_lockname.c */

char *mk_lockname P_((const char *));


/* mlist.c some here, some in parseaddrs.h */
extern void mlist_parse_header_rec(struct header_rec *entry);
extern int mlist_match_user(struct header_rec *entry);
extern int mlist_match_address(struct header_rec *entry, char *string);


/* move_left.c */

void move_left P_((char *, int));


/* msgcat.c */

	/* ##### defined in msgcat.h ##### */


/* ndbz.c */

	/* ##### defined in ndbz.h ##### */



/* okay_addr.c */

int okay_address P_((char *, char *));


/* opt_utils.c */

    /* strtok() declared in elm_defs.h */
    /* strpbrk() declared in elm_defs.h */
    /* strspn() declared in elm_defs.h */
    /* strcspn() declared in elm_defs.h */
#ifndef TEMPNAM
char *tempnam P_((char *, char *));
#endif

/* parsarpdat.c */

int parse_arpa_date P_((const char *, struct header_rec *));


/* parsarpmbox.c */

int parse_arpa_mailbox P_((const char *, char *, int, char *, int, char **));


/* parsarpwho.c */

int parse_arpa_who P_((const char *, char *));


/* patmatch.c */

#define PM_NOCASE	(1<<0)		/* letter case insignificant	*/
#define PM_WSFOLD	(1<<1)		/* fold white space		*/
#define PM_FANCHOR	(1<<2)		/* anchor pat at front (like ^)	*/
#define PM_BANCHOR	(1<<3)		/* anchor pat at back (like $)	*/

int patmatch P_((const char *, const char *, int));


/* posixsig.c */

#ifdef POSIX_SIGNALS
SIGHAND_TYPE (*posix_signal P_((int, SIGHAND_TYPE (*)(int)))) P_((int));
#endif


/* qstrings.c */

char *qstrpbrk P_((const char *, const char *));
int qstrspn P_((const char *, const char *));
int qstrcspn P_((const char *, const char *));


/* realfrom.c */

int real_from P_((const char *, struct header_rec *));


/* remfirstwd.c */

void remove_first_word P_((char *));
void remove_header_keyword P_((char *));


/* reverse.c */

void reverse P_((char *));


/* rfc822tlen.c */

int rfc822_toklen P_((const char *));


/* safemalloc.c */

/*
 * The "safe_malloc_fail_handler" vector points to a routine that is
 * invoked if one of the safe_malloc() routines fails.  At startup, this
 * will point to the default handler that prints a diagnostic message
 * and aborts.  The vector may be changed to install a different error
 * handler.
 */
extern void (*safe_malloc_fail_handler) P_((const char *, unsigned));

void dflt_safe_malloc_fail_handler P_((const char *, unsigned));
malloc_t safe_malloc P_((unsigned));
malloc_t safe_realloc P_((malloc_t, unsigned));
char *safe_strdup P_((const char *));


/* shiftlower.c */

char *shift_lower P_((char *));


/* strfcpy.c */

char *strfcpy P_((char *, const char *, int));
void  strfcat P_((char *, const char *, int));

/* striparens.c */

char *strip_parens P_((const char *));
char *get_parens P_((const char *));


/* strstr.c */

    /* strstr() declared in elm_defs.h */


/* strtokq.c */

char *strtokq P_((char *, const char *, int));


/* tail_of.c */

int tail_of P_((char *, char *, char *));


/* trim.c */

char *trim_quotes P_((char *));
char *trim_trailing_slashes P_((char *));
char *trim_trailing_spaces P_((char *));


/* validname.c */

int valid_name P_((const char *));

/* src/addr_util.c */
int translate_return(char *addr, char *ret_addr);
int build_address(char *to, char *full_to);
int forwarded(char *buffer, struct header_rec *entry);
int fix_arpa_address(char *address);

/* src/aliaslib.c */
char *get_alias_address(char *name, int mailing, int *too_longp);


/* src/alias.c */
int open_alias_files(int are_in_aliases);
int find_alias(char *word, int alias_type);
int sort_aliases(int entries, int visible, int are_in_aliases);
int main_state(void);
void alias(void);
void install_aliases(void);

/* src/a_edit.c */
int edit_aliases_text(void);
void alias(void);
void alias(void);
int delete_from_alias_text(char **name, int num_to_delete);

/* src/args.c */
char *parse_arguments(int argc, char *argv[], char *to_whom);

/* src/a_quit.c */
int delete_aliases(int newaliases, int prompt);
int exit_alias(void);

/* src/a_screen.c */
int alias_screen(int modified);
int alias_title(int modified);
int show_alias_menu(void);
char *alias_type(int type);
int on_page(int message);
int build_alias_line(char *buffer, struct alias_rec *entry,
		     int message_number, int highlight);


/* src/a_sendmsg.c */
int a_sendmsg(void);

/* src/a_sort.c */
char *alias_sort_name(int longname);

/* src/bouncebk.c */
char *bounce_off_remote(register char *to);
int uucp_hops(register char *to);

/* src/builtin.c */
int start_builtin(int lines_in_message);
int display_line(char *input_line, int input_size);

/* src/data.c */
char *elm_date_str(char *buf, struct header_rec *entry);
int make_menu_date(struct header_rec *entry);

/* src/delete.c */
int show_msg_tag(int msg);
int show_msg_status(int msg);
int delete_msg(int real_del, int update_screen);
int undelete_msg(int update_screen);
int tag_message(int update_screen);
int show_new_status(int msg);

/* src/edit.c */
int edit_aliases_text(void);

/* src/elm.c */
int motion(int ch);
int check_range(void);

/* src/fileio.c */
int save_file_stats(const char *fname);
int restore_file_stats(const char *fname);

/* src/file_util.c */
long fsize(FILE *fd);

/* src/help.c */
int display_helpfile(char *topic);

/* src/limit.c */
int compute_visible(int message);

/* src/quit.c */
void quit_abandon(int do_prompt);

/* src/reply.c */
int get_return_address(char *address, char *single_address);

/* src/returnadd.c */
int get_return(char *buffer, int msgnum);

/* src/screen.c */
int show_headers(void);
int show_current(void);
char *show_status(int status);

/* src/strings.c */
char *strip_commas(char *string);
char *get_token(char *source, char *keys, int depth);

/* src/string2.c */
int remove_possible_trailing_spaces(char *string);

/* src/syscall.c */
int subshell(void);
int system_call(char *string, int options);
int remove_folder_state_file(void);
int create_folder_state_file(void);

/* src/utils.c */
int get_page(int msg_pointer);
void leave(int mode);

/* src/wordwrap.c */
int wrapped_enter(char *string, char *tail, int x, int y, FILE *edit_fd,
		  int *append_current);

