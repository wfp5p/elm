

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
 * $Log: fastmail.c,v $
 * Revision 1.4  1996/03/14  17:30:07  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/04/20  21:02:06  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:40  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This program is specifically written for group mailing lists and
    such batch type mail processing.  It does NOT use aliases at all,
    it does NOT read the /etc/password file to find the From: name
    of the user and does NOT expand any addresses.  It is meant 
    purely as a front-end for either /bin/mail or /usr/lib/sendmail
    (according to what is available on the current system).

         **** This program should be used with CAUTION *****

**/

/** The calling sequence for this program is:

	fastmail {args}  [ filename | - ] full-email-address 

   where args could be any (or all) of;

	   -b bcc-list		(Blind carbon copies to)
	   -c cc-list		(carbon copies to)
	   -C comment-line      (Comments:)
	   -d			(debug on)
	   -f from 		(from name)
	   -F from-addr		(the actual address to be put in the From: line)
	   -i msg-id            (In-Reply-To: msgid)
	   -r reply-to-address 	(Reply-To:)
	   -R references        (References:)
	   -s subject 		(subject of message)
**/

#define INTERN
#include "elm_defs.h"
#include "patchlevel.h"
#include "s_fastmail.h"

int addword P_((char *, int, const char *, int *));

extern char *get_arpa_date();


main(argc, argv)
int argc;
char *argv[];
{
    extern char *optarg;
    extern int optind;
    FILE *ifp, *ofp;
    int fp_handle;
    char *progname, temp_fname[SLEN], cmdbuf[LONG_STRING], buf[LONG_STRING];
    char *subject, *cc_list, *bcc_list, to_list[LONG_STRING];
    char *from_addr, *from_fullname;
    char *replyto, *comments, *inreplyto, *references;
    char *p;
    int  add_from, i, len;

    initialize_common();
    progname = basename(argv[0]);
    sprintf(temp_fname, "%s/fastmail.%d", default_temp, getpid());
    subject = NULL;
    cc_list = NULL;
    bcc_list = NULL;
    to_list[0] = '\0';
    from_addr = NULL;
    from_fullname = NULL;
    replyto = getenv("REPLYTO");
    comments = NULL;
    inreplyto = NULL;
    references = NULL;

    add_from = FALSE;


    while ((i = getopt(argc, argv, "b:c:C:df:F:i:r:R:s:")) != EOF) {
	switch (i) {
	case 'b' : bcc_list = optarg;				break;
	case 'c' : cc_list = optarg;				break;
	case 'C' : comments = optarg;				break;
#ifdef DEBUG
	case 'd' : debug++;					break;	
#else
	case 'd' :
	    fprintf(stderr, catgets(elm_msg_cat, FastmailSet, FastmailNoDebug,
			"%s: cannot use \"-d\" (not compiled with DEBUG)\n"),
			progname);
	    exit(1);
#endif
	case 'f' : from_fullname = optarg; add_from = TRUE;	break;
	case 'F' : from_addr = optarg;	 add_from = TRUE;	break;
	case 'i' : inreplyto = optarg;				break;
	case 'r' : replyto = optarg;				break;
	case 'R' : references = optarg;				break;
	case 's' : subject = optarg;				break;
	default  : goto usage_error;
	}
    }

    if (argc-optind < 2) {
usage_error:
	fprintf(stderr, catgets(elm_msg_cat, FastmailSet, FastmailUsage,
"Usage: %s [args ...] filename address ...\n\
\n\
\t\"filename\" may be \"-\" to send stdin\n\
\t\"args ...\" can be:\n\
\n\
\t-b bcc-list\n\
\t-c cc-list\n\
\t-C comments\n\
\t-f from-name\n\
\t-F from-addr\n\
\t-i msg-id\n\
\t-r reply-to\n\
\t-R references\n\
\t-s subject\n\
\t-d  (to enable debugging)\n\
\n"), progname);
	exit(1);
    }

    p = argv[optind++];
    if (streq(p, "-")) {
	ifp = stdin;
    } else if ((ifp = fopen(p, "r")) == NULL) {
	fprintf(stderr, catgets(elm_msg_cat, FastmailSet,
		    FastmailCannotOpenInput,
		    "%s: cannot open input file \"%s\" [%s]\n"),
		    progname, p, strerror(errno));
	exit(1);
    }

    for (len = 0 ; optind < argc ; ++optind) {
	i = strlen(argv[optind]);
	if (len+2+i >= sizeof(to_list)) {
	    fprintf(stderr, catgets(elm_msg_cat, FastmailSet,
			FastmailTooManyToRecip,
			"%s: too many \"To\" recipients\n"), progname);
	    exit(1);
	}
	if (len > 0) {
	    to_list[len++] = ',';
	    to_list[len++] = ' ';
	}
	strcpy(to_list+len, argv[optind]);
	len += i;
    }

    if (add_from) {
	if (!from_addr) {
	    int l1 = strlen(user_name), l2 = strlen(host_fullname);
	    from_addr = (char *) safe_malloc(l1+1+l2+1);
	    sprintf(from_addr, "%s@%s", user_name, host_fullname);
	}
	if (!from_fullname)
	    from_fullname = user_fullname;
    }

    (void) strcpy(cmdbuf, mailer);
    len = strlen(cmdbuf);
    if ((addword(cmdbuf, sizeof(cmdbuf), to_list, &len) < 0)
	|| (cc_list && addword(cmdbuf, sizeof(cmdbuf), cc_list, &len) < 0)
	|| (bcc_list && addword(cmdbuf, sizeof(cmdbuf), bcc_list, &len) < 0)
	|| (addword(cmdbuf, sizeof(cmdbuf), "<", &len) < 0)
	|| (addword(cmdbuf, sizeof(cmdbuf), temp_fname, &len) < 0)
    ) {
	fprintf(stderr, catgets(elm_msg_cat, FastmailSet, FastmailTooManyRecip,
		    "%s: too many recipients specified\n"), progname);
	exit(1);
    }

    
    
    if ( ((fp_handle = open(temp_fname, O_RDWR|O_CREAT|O_EXCL,0600)) != -1) &&
	 ((ofp = fdopen(fp_handle, "w")) == NULL) ) {
	fprintf(stderr, catgets(elm_msg_cat, FastmailSet,
		    FastmailCannotCreateTemp,
		    "%s: cannot create temp file %s [%s]\n"),
		    progname, temp_fname, strerror(errno));
	exit(1);
    }

    /*
     * Warning - the following comment is somewhat obsolete.
     * It does explain, however, why we do some seemingly
     * wierd things looking at the mailer before handling the subject.
     */

	/** Subject must appear even if "null" and must be first
	    at top of headers for mail because the
	    pure System V.3 mailer, in its infinite wisdom, now
	    assumes that anything the user sends is part of the 
	    message body unless either:
		1. the "-s" flag is used (although it doesn't seem
		   to be supported on all implementations?)
		2. the first line is "Subject:".  If so, then it'll
		   read until a blank line and assume all are meant
		   to be headers.
	    So the gory solution here is to move the Subject: line
	    up to the top.  I assume it won't break anyone elses program
	    or anything anyway (besides, RFC-822 specifies that the *order*
	    of headers is irrelevant).  Gahhhhh....

	    If we have been configured for a smart mailer then we don't want
	    to add a from line.  If the user has specified one then we have
	    to honor their wishes.  If they've just given a 'from name' then
	    we'll just put in the username and hope the mailer can add the
	    correct domain in.
	**/

    if (!subject) {
	i = strlen(mailer);
	if (i > 5 && streq(mailer+i-5, "/mail"))
	    subject = "";
	else
	if (i > 6 && streq(mailer+i-6, "/mailx"))
	    subject = "";
    }

    /*
     * Emit the message header.
     */
    if (subject)
	fprintf(ofp, "Subject: %s\n", subject);
    if (from_addr) {
	if (from_fullname)
	    fprintf(ofp, "From: %s (%s)\n", from_addr, from_fullname);
	else
	    fprintf(ofp, "From: %s\n", from_addr);
    }
    fprintf(ofp, "Date: %s\n", get_arpa_date());
    if (replyto)
	fprintf(ofp, "Reply-To: %s\n", replyto);
    fprintf(ofp, "To: %s\n", to_list);
    if (cc_list)
	fprintf(ofp, "Cc: %s\n", cc_list);
    if (references)
	fprintf(ofp, "References: %s\n", references);
    if (inreplyto)
	fprintf(ofp, "In-Reply-To: %s\n", inreplyto);
    if (comments)
	fprintf(ofp, "Comments: %s\n", comments);
#ifndef NO_XHEADER
    fprintf(ofp, "X-Mailer: fastmail [version %s PL%s]\n",
		VERSION, PATCHLEVEL);
#endif
    putc('\n', ofp);

    /*
     * Emit the message body.
     */
    while (fgets(buf, sizeof(buf), ifp) != NULL)
	fputs(buf, ofp);

    fclose(ifp);
    fclose(ofp);

    dprint(1, (debugfile, "executing: %s\n", cmdbuf));
   
    i = system(cmdbuf);
    (void) unlink(temp_fname);

    if (i != 0) {
	fprintf(stderr, catgets(elm_msg_cat, FastmailSet, FastmailErrorSending,
		"%s: error sending message [%s return status %d]\n"),
		progname, mailer, i);
	exit(1);
    }
    exit(i);
}


int addword(dest, destsiz, word, len_p)
char *dest;
int destsiz;
const char *word;
int *len_p;
{
    int dlen, wlen;

    dlen = *len_p;
    dest += dlen;
    wlen = strlen(word);
    if (dlen+1+wlen+1 > destsiz)
	return -1;

    if (dlen > 0) {
	*dest++ = ' ';
	++dlen;
    }

    for ( ; *word != '\0' ; ++word) {
	if (*word != ',') 
        {
	    *dest++ = *word;
	    ++dlen;
	}
        else
       {
	  *dest++ = ' ';
 	  ++dlen;
       }
       
    }
    *dest = '\0';

    *len_p = dlen;
    return 0;
}

