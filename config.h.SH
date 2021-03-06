case $CONFIG in
'')
    if test ! -f config.sh; then
	ln ../config.sh . || \
	ln ../../config.sh . || \
	ln ../../../config.sh . || \
	(echo "Can't find config.sh."; exit 1)
	echo "Using config.sh from above..."
    fi
    . ./config.sh
    ;;
esac
echo "Extracting config.h (with variable substitutions)"
sed <<!GROK!THIS! >config.h -e 's!^#undef!/\*#undef!'
/* config.h
 * This file was produced by running the config.h.SH script, which
 * gets its values from config.sh, which is generally produced by
 * running Configure.
 *
 * Feel free to modify any of this as the need arises.  Note, however,
 * that running config.h.SH again will wipe out any changes you've made.
 * For a more permanent change edit config.sh and rerun config.h.SH.
 */


/* CPPSTDIN:
 *	This symbol contains the first part of the string which will invoke
 *	the C preprocessor on the standard input and produce to standard
 *	output.	 Typical value of "cc -E" or "/lib/cpp".
 */
/* CPPMINUS:
 *	This symbol contains the second part of the string which will invoke
 *	the C preprocessor on the standard input and produce to standard
 *	output.  This symbol will have the value "-" if CPPSTDIN needs a minus
 *	to specify standard input, otherwise the value is "".
 */
#define CPPSTDIN "$cppstdin"
#define CPPMINUS "$cppminus"


/* USE_FLOCK_LOCKING
 *	This symbol, if defined, indicates that the flock mailbox locking should be used.
 */
/* USE_DOTLOCK_LOCKING
 *	This symbol, if defined, indicates that the .lock mailbox locking should be used.
 */
/* USE_FCNTL_LOCKING
 *	This symbol, if defined, indicates that SYSV style fcntl file locking should be used.
 */
/* LOCK_DIR
 *	This symbol is the name of the lock directory for access (not mailbox) locks.
 *	It will be /usr/spool/locks or /usr/spool/uucp
 */
#$d_flock	USE_FLOCK_LOCKING		/**/

#$d_dotlock	USE_DOTLOCK_LOCKING		/**/

#$d_fcntlock	USE_FCNTL_LOCKING	/**/

#define		LOCK_DIR	"$lock_dir"	/**/

/* ISPELL:
 *	This symbol, if defined, indicates that the GNU ispell
 *	spelling checker that is available to Elm.
 */
/* ISPELL_PATH:
 *	This symbol contains the path to the GNU ispell
 *	spelling checker that is available to Elm.
 */
/* ISPELL_OPTIONS:
 *	This symbol contains the options to the GNU ispell
 *	spelling checker that is available to Elm.
 */
#$d_ispell	ISPELL	/**/
#define		ISPELL_PATH	"$ispell_path" /**/
#define		ISPELL_OPTIONS	"$ispell_options" /**/

/* I_LOCALE:
 *	This symbol, if defined, indicates that the file locale.h
 *	should be included
 */
/* MSGCAT:
 *	This symbol, if defined, indicates that the MSGCAT NLS libraries
 *	are available.
 */
#$d_locale	I_LOCALE	/**/
#$d_nl_types	I_NL_TYPES	/**/
#$d_msgcat	MSGCAT		/**/

/* ALLOW_MAILBOX_EDITING:
 *	This symbol, if defined, indicates that the E)dit mailbox
 *	function is to be allowed.
 */
#$d_mboxedit	ALLOW_MAILBOX_EDITING	/**/

/* MIME:
 *	This symbol, if defined, indicates that the MIME mail
 *	extension utilities are available
 */
#$d_mime	MIME	/**/

/* AUTO_BACKGROUND:
 *	This symbol, if defined, indicates that newmail should go to
 *	the background automatically.
 */
#$d_newauto AUTO_BACKGROUND /**/

/* NO_XHEADER:
 *	This symbol, if defined, will not automatically add "X-Mailer:"
 *	headers.
 */
#$d_noxheader	NO_XHEADER	/**/

/* REMOVE_AT_LAST:
 *	This symbol, if defined, tells the C code to remove the lock
 *	file on lock failure.
 */
/* MAX_ATTEMPTS:
 *	This symbol defines to the C code the number of times to try
 *	locking the mail file.
 */
#$d_remlock REMOVE_AT_LAST	/**/
#define MAX_ATTEMPTS	$maxattempts

/* SAVE_GROUP_MAILBOX_ID:
 *	This symbol, if defined, indicates that Elm needs to restore the
 *	group id of the file, as it is running setgid.
 */
#$d_savegrpmboxid SAVE_GROUP_MAILBOX_ID	/**/

/* SIGVEC:
 *	This symbol, if defined, indicates that BSD reliable signals routine
 *	sigvec is available.
 */
/* SIGVECTOR:
 *	This symbol, if defined, indicates that the sigvec() routine is called
 *	sigvector() instead, and that sigspace() is provided instead of
 *	sigstack().  This is probably only true for HP-UX.
 */
/* SIGSET:
 *	This symbol, if defined, indicates that BSD reliable signal routine
 *	sigset is available.
 */
/* POSIX_SIGNALS:
 *	This symbol, if defined, indicated that POSIX sigaction
 *	routine is available.
 */
/* HASSIGHOLD:
 *	This symbol, if defined, indicates that sighold routine is
 *	available.
 */
/* HASSIGBLOCK
 *	This symbol, if defined, indicates that the sigblock routine is
 *	available.
 */
/* HASSIGPROCMASK:
 *	This symbol, if defined, indicates that POSIX sigprocmask
 *	routine is available.
 */
#$d_sigvec	SIGVEC		/**/

#$d_sigvectr	SIGVECTOR	/**/

#$d_sigset	SIGSET	/**/

#$d_sigaction	POSIX_SIGNALS	/**/

#$d_sighold	HASSIGHOLD	/**/

#$d_sigblock	HASSIGBLOCK	/**/

#$d_sigprocmask	HASSIGPROCMASK	/**/

/* ALLOW_STATUS_CHANGING:
 *	This symbol, if defined, indicates that the S)tatus Change
 *	function is to be allowed.
 */
#$d_statuschg	ALLOW_STATUS_CHANGING	/**/

/* STRSPN:
 *	This symbol, if defined, indicates that the strspn() routine exists.
 */
/* STRCSPN:
 *	This symbol, if defined, indicates that the strcspn() routine exists.
 */
/* STRPBRK:
 *	This symbol, if defined, indicates that the strpbrk() routine exists.
 */
#$d_strspn	STRSPN		/**/

#$d_strcspn	STRCSPN		/**/

#$d_strpbrk	STRPBRK		/**/

/* STRFTIME:
 *	This symbol, if defined, indicates that strftime() exists.
 */
#$d_strftime	STRFTIME	/**/

/* STRINGS:
 *	This symbol, if defined, indicates that the file strings.h
 *	should be included not string.h
 */
/* PWDINSYS:
 *	This symbol, if defined, indicates that the file pwd.h
 *	is in the sys sub directory
 */
#$d_strings	STRINGS		/**/
#$d_pwdinsys	PWDINSYS	/**/

/* STRSTR:
 *	This symbol, if defined, indicates that strstr() exists.
 */
#$d_strstr	STRSTR	/**/

/* STRTOK:
 *	This symbol, if defined, indicates that strtok() exists.
 */
#$d_strtok	STRTOK	/**/

/* ALLOW_SUBSHELL:
 *	This symbol, if defined, indicates that the '!' subshell
 *	function is to be allowed at various places.
 */
#$d_subshell	ALLOW_SUBSHELL	/**/

/* TEMPNAM:
 *	This symbol, if defined, indicates that the tempnam() routine exists.
 */
#$d_tempnam	TEMPNAM		/**/

/* TERMIOS:
 *	This symbol, if defined, indicates that the program should include
 *	termios.h rather than sgtty.h or termio.h.  There are also differences
 *	in the ioctl() calls that depend on the value of this symbol.
 */
/* TERMIO:
 *	This symbol, if defined, indicates that the program should include
 *	termio.h rather than sgtty.h.  There are also differences in the
 *	ioctl() calls that depend on the value of this symbol.
 */
#$d_termios	TERMIOS		/**/

#$d_termio	TERMIO		/**/

/* USE_EMBEDDED_ADDRESSES:
 *	This symbol, if defined, indicates that replyto: and from:
 *	headers can be trusted.
 */
#$d_useembed USE_EMBEDDED_ADDRESSES	 /**/

/* UTIMBUF:
 *	This symbol is defined if this system defines struct utimbuf.
 */
#$d_utimbuf UTIMBUF		/**/

/* VFORK:
 *	This symbol, if defined, indicates that vfork() exists.
 */
#$d_vfork	VFORK	/**/

/* DEFEDITOR:
 *	This symbol contains the name of the default editor.
 */
/* EDITOROPTS:
 *	This symbol contains the command line options for the default editor.
 */
#define DEFEDITOR "$defeditor"		/**/

#define EDITOROPTS "$editoropts"		/**/

/* HOSTNAME:
 *	This symbol contains name of the host the program is going to run on.
 *	The domain is not kept with hostname, but must be gotten from MYDOMAIN.
 *	The dot comes with MYDOMAIN, and need not be supplied by the program.
 *	If gethostname() or uname() exist, HOSTNAME may be ignored.
 */
/* MYDOMAIN:
 *	This symbol contains the domain of the host the program is going to
 *	run on.  The domain must be appended to HOSTNAME to form a complete
 *	host name.  The dot comes with MYDOMAIN, and need not be supplied by
 *	the program.  If the host name is derived from PHOSTNAME, the domain
 *	may or may not already be there, and the program should check.
 */
#define HOSTNAME "$hostname"		/**/
#define MYDOMAIN "$mydomain"		/**/

/* I_MEMORY:
 *	This symbol, if defined, indicates that the file memory.h
 *	should be included instead of declaring the memory routines.
 */
#$i_memory	I_MEMORY	/**/

/* I_STDARG:
 *	This symbol, if defined, indicates that the file stdarg.h
 *	should be included instead of varargs
 */
#$i_stdarg	I_STDARG	/**/

/* I_STDLIB:
 *	This symbol, if defined, indicates that the file stdlib.h
 *	should be included instead of declaring the stdlib routines.
 */
#$i_stdlib	I_STDLIB	/**/

/* I_TIME:
 *	This symbol is defined if the program should include <time.h>.
 */
/* I_SYSTIME:
 *	This symbol is defined if the program should include <sys/time.h>.
 */
/* I_SYSTIMEKERNEL:
 *	This symbol is defined if the program should include <sys/time.h>
 *	with KERNEL defined.
 */
#$i_time		I_TIME	 	/**/
#$i_systime	I_SYSTIME 	/**/
#$d_systimekernel	SYSTIMEKERNEL 	/**/

/* I_UNISTD:
 *	This symbol, if defined, indicates that the file unistd.h
 *	should be included instead of declaring the unistd routines.
 */
#$i_unistd	I_UNISTD	/**/

/* I_UTIME:
 *	This symbol, if defined, indicates that the file utime.h
 *	should be included instead of declaring our own utimbuf.
 */
/* I_USYSTIME:
 *	This symbol, if defined, indicates that the file sys/utime.h
 *	should be included instead of declaring our own utimbuf.
 */
#$i_utime	I_UTIME	/**/
#$i_sysutime	I_SYSUTIME	/**/


/* SIG_TYPE:
 *	This symbol contains the type name of the signal handler functions.
 */
#define	SIGHAND_TYPE	$sigtype

/* TZNAME_USE_xxxxxx:
 *	Specify how to get timezone name.  Possible values are:
 *		 TZNAME_USE_TM_NAME	use (struct tm *)->tm_name
 *		 TZNAME_USE_TM_ZONE	use (struct tm *)->tm_zone
 *		 TZNAME_USE_TZNAME	use "tzname[]" external
 *		 TZNAME_USE_TIMEZONE	use timezone() function
 */
#define TZNAME_USE_$tzname_handling

/* XENIX:
 *	This symbol, if defined, indicates this is a Xenix system,
 *	for knocking  out the far keyword in selected places.
 */
/* BSD:
 *	This symbol, if defined, indicates this is a BSD type system,
 */
#$d_xenix	XENIX	/**/
#$d_bsd	BSD	/**/

!GROK!THIS!
