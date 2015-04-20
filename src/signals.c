

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.5 $   $State: Exp $
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
 * $Log: signals.c,v $
 * Revision 1.5  1999/03/24  14:04:06  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.4  1996/03/14  17:29:53  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:29  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/07/18  19:00:10  wfp5p
 * Alpha 6
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This set of routines traps various signals and informs the
    user of the error, leaving the program in a nice, graceful
    manner.

**/

#include "elm_defs.h"
#include <setjmp.h>
#include "elm_globals.h"
#include "s_elm.h"


#define INIT_SIG(NUM, NAME, ACTION, DESCR) \
    (signame[NUM] = (NAME), sigdescr[NUM] = (DESCR), signal((NUM), (ACTION)))

#ifdef __QNX__
# define NSIG _SIGMAX
#endif


static char *signame[NSIG];
static char *sigdescr[NSIG];

/*
 * it makes me very very nervous that we are doing stuff
 * like printfs in signal handlers
 */

static SIGHAND_TYPE bailout_handler P_((int));
static SIGHAND_TYPE sigalrm_catcher P_((int));
static SIGHAND_TYPE sigpipe_catcher P_((int));
#ifdef SIGTSTP
static SIGHAND_TYPE sigtstp_catcher P_((int));
static SIGHAND_TYPE sigcont_catcher P_((int));
#endif
#ifdef SIGWINCH
static SIGHAND_TYPE sigwinch_catcher P_((int));
#endif

#ifdef SIGTSTP
static char *mssg_sigtstp, *mssg_sigcont;
#endif


void initialize_signals(void)
{
    int i;

    /* signals to ignore */
    INIT_SIG(SIGINT, "SIGINT", SIG_IGN,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripInt,
	"Interrupt"));

    /* signals that invoke error exit */
    INIT_SIG(SIGHUP,  "SIGHUP",  bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripHup,
	"Hangup"));
    INIT_SIG(SIGQUIT, "SIGQUIT", bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripQuit,
	"Quit"));
    INIT_SIG(SIGTERM, "SIGTERM", bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripTerm,
	"Terminate"));

    /* signals that invoke a mailbox cleanup */
    INIT_SIG(SIGUSR1, "SIGUSR1", bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripUsr1,
	"First User-Defined"));
    INIT_SIG(SIGUSR2, "SIGUSR2", bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripUsr2,
	"Second User-Defined"));

    /* signals that invoke emergency exit */
    INIT_SIG(SIGILL,  "SIGILL",  bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripIll,
	"Illegal Instruction"));
    INIT_SIG(SIGFPE,  "SIGFPE",  bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripFpe,
	"Floating Point Exception"));
#ifdef SIGBUS
    INIT_SIG(SIGBUS,  "SIGBUS",  bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripBus,
	"Bus Error"));
#endif
    INIT_SIG(SIGSEGV, "SIGSEGV", bailout_handler,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripSegv,
	"Segment Violation"));

#if !defined(POSIX_SIGNALS) && defined(SIGVEC) && defined(SV_INTERRUPT)
    INIT_SIG(SIGALRM, "SIGALRM", SIG_DFL,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripAlrm, "Alarm Clock"));
    {
	struct sigvec alarm_vec;
	bzero((char *) &alarm_vec, sizeof(alarm_vec));
	alarm_vec.sv_handler = alarm_signal;
	alarm_vec.sv_flags = SV_INTERRUPT;
	sigvec(SIGALRM, &alarm_vec, (struct sigvec *)0);
    }
#else
    INIT_SIG(SIGALRM, "SIGALRM", sigalrm_catcher,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripAlrm, "Alarm Clock"));
#endif

    INIT_SIG(SIGPIPE, "SIGPIPE", sigpipe_catcher,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripPipe, "Write to Pipe"));

#ifdef SIGTSTP
    INIT_SIG(SIGTSTP, "SIGTSTP", sigtstp_catcher,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripTstp,
	"Stop from Terminal"));
    INIT_SIG(SIGCONT, "SIGCONT", sigcont_catcher,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripCont,
	"Continue Stopped Process"));
#endif
#ifdef SIGWINCH
    INIT_SIG(SIGWINCH,"SIGWINCH",sigwinch_catcher,
	catgets(elm_msg_cat, ElmSet, ElmSigDescripWinch,
	"Window Size Change"));
#endif

    for (i = 0 ; i < NSIG ; ++i) {
	char buf[64];
	if (signame[i] == NULL) {
	    sprintf(buf, "SIGNAL %d", i);
	    signame[i] = safe_strdup(buf);
	    sprintf(buf, "Signal Number %d", i);
	    sigdescr[i] = safe_strdup(buf);
	}
    }

#ifdef SIGTSTP
    mssg_sigtstp = catgets(elm_msg_cat, ElmSet, ElmStoppedUseFGToReturn,
	"\n\nStopped.  Use \"fg\" to return to ELM\n\n");
    mssg_sigcont = catgets(elm_msg_cat, ElmSet, ElmBackInElmRedraw,
	"\nBack in ELM. (You might need to explicitly request a redraw.)\n\n");
#endif

}


static void bailout_handler(int sig)
{
    /*
     * This routine does lots of things that are dangerous to perform
     * in a signal hander.  Since, however, we are about to exit we
     * probably can get away with it.
     */

    dprint(1, (debugfile, "\n\n** Received %s **\n\n", signame[sig]));
    ShutdownTerm();
    show_error("Received %s signal!", sigdescr[sig]);

    switch (sig) {

    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
	leave(LEAVE_ERROR | LEAVE_KEEP_EDITTMP);

    case SIGUSR1:
	/* leave WITHOUT moving read messages to =received */
	while (leave_mbox(/*resync*/TRUE, /*quit*/TRUE, /*prompt*/FALSE) == -1)
	    newmbox(curr_folder.filename, TRUE); /* new mail has arrived */
	leave(LEAVE_NORMAL);

    case SIGUSR2:
	/* leave WITH moving read messages to =received */
	while (leave_mbox(/*resync*/FALSE, /*quit*/TRUE, /*prompt*/FALSE) == -1)
		newmbox(curr_folder.filename, TRUE); /* new mail has arrived */
	leave(LEAVE_NORMAL);

    case SIGILL:
    case SIGFPE:
#ifdef SIGBUS
    case SIGBUS:
#endif
    case SIGSEGV:
    default:
	leave(LEAVE_EMERGENCY);

    }

    fprintf(stderr, "\r\n\nSignal handling failed!!!  Bailing out!\r\n");
    exit(1);
}


static void sigalrm_catcher(int sig)
{
    signal(SIGALRM, sigalrm_catcher);
    if (GetKey_active) {
#ifdef HASSIGHOLD
	sigrelse(SIGALRM);
#endif
	LONGJMP(GetKey_jmpbuf, sig);
    }
}


static void sigpipe_catcher(int sig)
{
    extern int pipe_abort;

    /*DANGEROUS*/
    dprint(2, (debugfile, "\n\n** Received %s **\n\n", signame[sig]));

    signal(SIGPIPE, sigpipe_catcher);
    pipe_abort = TRUE;
}


#ifdef SIGTSTP /*{*/

/* state passed between SIGTSTP and SIGCONT handlers */
static int save_term_status;

static void sigtstp_catcher(int sig)
{
    /*DANGEROUS*/
    dprint(1, (debugfile, "\n\n** Received %s **\n\n", signame[sig]));

    signal(SIGTSTP, SIG_DFL);

    save_term_status = Term.status;

    /* We probably shouldn't be doing stdio in a signal handler. */
    Raw(OFF);
    EnableFkeys(OFF);

    /*
     * write() is ok in signal handler.  This might get jumbled about
     * with any buffered stdio data, but if the guy did a ^Z in the
     * middle of display output things are going to be jumbled about anyway.
     */
    write(STDERR_FILENO, mssg_sigtstp, strlen(mssg_sigtstp));

    kill(0, SIGSTOP);
}


static void sigcont_catcher(int sig)
{
    /*DANGEROUS*/
    dprint(1, (debugfile, "\n\n** Received %s **\n\n", signame[sig]));

    /*
     * This flag is set when either a SIGWINCH or SIGCONT is
     * received.  When a calling routine detects this flag is
     * set, it should call ResizeScreen() and then fully redraw
     * the display.
     */
    caught_screen_change_sig = TRUE;

    signal(SIGCONT, sigcont_catcher);
    signal(SIGTSTP, sigtstp_catcher);

    write(STDERR_FILENO, mssg_sigcont, strlen(mssg_sigcont));

    /* restore tty state */
    if (save_term_status & TERM_IS_RAW)
	Raw(ON);
    if (save_term_status & TERM_IS_FKEY)
	EnableFkeys(ON);

    if (GetKey_active) {
#ifdef HASSIGHOLD
	sigrelse(SIGTSTP);
	sigrelse(SIGCONT);
#endif
	LONGJMP(GetKey_jmpbuf, sig);
    }
}

#endif /*}SIGTSTP*/


#ifdef SIGWINCH /*{*/

static void sigwinch_catcher(int sig)
{
    /*
     * This flag is set when either a SIGWINCH or SIGCONT is
     * received.  When a calling routine detects this flag is
     * set, it should call ResizeScreen() and then fully redraw
     * the display.
     */
    caught_screen_change_sig = TRUE;

    signal(SIGWINCH, sigwinch_catcher);

    if (GetKey_active) {
#ifdef HASSIGHOLD
	    sigrelse(SIGWINCH);
#endif
	    LONGJMP(GetKey_jmpbuf, sig);
    }
}

#endif /*}SIGWINCH*/
