#include "elm_defs.h"
#include "elm_globals.h"
#include "mime.h"
#include "sndhdrs.h"
#include "s_elm.h"
#include <assert.h>


static int add_mailheaders P_((FILE *));
static int expand_backquote P_((FILE *, const char *));


SEND_HEADER *sndhdr_new(void)
{
    SEND_HEADER *shdr;
    char *p;

    shdr = (SEND_HEADER *) safe_malloc(sizeof(SEND_HEADER));

    shdr->subject[0] = '\0';

    shdr->to[0] = '\0';
    shdr->cc[0] = '\0';
    shdr->bcc[0] = '\0';

    shdr->expanded_to[0] = '\0';
    shdr->expanded_cc[0] = '\0';
    shdr->expanded_bcc[0] = '\0';

    if ((p = getenv("REPLYTO")) != NULL) {
	(void) strfcpy(shdr->reply_to, p, sizeof(shdr->reply_to));
	(void) build_address(strip_commas(shdr->reply_to),
		    shdr->expanded_reply_to);
    } else {
	shdr->reply_to[0] = '\0';
	shdr->expanded_reply_to[0] = '\0';
    }

    shdr->in_reply_to[0] = '\0';
    shdr->precedence[0] = '\0';
    shdr->priority[0] = '\0';
    shdr->action[0] = '\0';
    shdr->user_defined_header[0] = '\0';

    return shdr;
}


void sndhdr_destroy(SEND_HEADER *shdr)
{
	free((malloc_t)shdr);
}


static void write_header_line(FILE *fp, const char *hdrname, const char *hdrvalue)
{
    fputs(hdrname, fp);
    putc(' ', fp);
    fputs(hdrvalue, fp);
    putc('\n', fp);
}

int sndhdr_output(FILE *fp, const SEND_HEADER *shdr, int is_form, int is_copy)
{
    /** Write mail headers to stream "fp".
	Added the ability to have backquoted stuff in the users
	    .elmheaders file!
	If copy is TRUE, then treat this as the saved copy of outbound
	    mail.
    **/

    /** Subject moved to top of headers for mail because the
	pure System V.3 mailer, in its infinite wisdom, now
	assumes that anything the user sends is part of the
	message body unless either:
	    1. the "-s" flag is used (although it doesn't seem
	       to be supported on all implementations?? )
	    2. the first line is "Subject:".  If so, then it'll
	       read until a blank line and assume all are meant
	       to be headers.
	So the gory solution here is to move the Subject: line
	up to the top.  I assume it won't break anyone elses program
	or anything anyway (besides, RFC-822 specifies that the *order*
	of headers is irrelevant).  Gahhhhh....
    **/

    write_header_line(fp, "Subject:", shdr->subject);
    write_header_line(fp, "To:",
		format_long(shdr->expanded_to, sizeof("To: ")-1));
    write_header_line(fp, "Date:", get_arpa_date());
    if (*shdr->cc) {
	write_header_line(fp, "Cc:",
		    format_long(shdr->expanded_cc, sizeof("Cc: ")-1));
    }
    if (is_copy && *shdr->bcc) {
	write_header_line(fp, "Bcc:",
		    format_long(shdr->expanded_bcc, sizeof("Bcc: ")-1));
    }
    if (*shdr->action)
	write_header_line(fp, "Action:", shdr->action);
    if (*shdr->priority)
	write_header_line(fp, "Priority:", shdr->priority);
    if (*shdr->precedence)
	write_header_line(fp, "Precedence:", shdr->precedence);
    if (*shdr->expanded_reply_to)
	write_header_line(fp, "Reply-To:", shdr->expanded_reply_to);
    if (*shdr->in_reply_to)
	write_header_line(fp, "In-Reply-To:", shdr->in_reply_to);
    if (*shdr->user_defined_header) {
	fputs(shdr->user_defined_header, fp);
	putc('\n', fp);
    }
    if (add_mailheaders(fp) < 0)
	return -1;
#ifndef NO_XHEADER
    fprintf(fp, "X-Mailer: ELM [version %s]\n", version_buff);
#endif /* !NO_XHEADER */

    if (is_form)
	fputs("Content-Type: mailform\n", fp);

    return 0;
}


void generate_in_reply_to(SEND_HEADER *shdr, int msg)
{
    char from_buf[SLEN];
    struct header_rec *hdr = curr_folder.headers[msg];

    assert(msg >= 0 && msg < curr_folder.num_mssgs);

    if (strchr(hdr->from, '!') != NULL)
	tail_of(hdr->from, from_buf, (char *)NULL);
    else
	strcpy(from_buf, hdr->from);

   sprintf(shdr->in_reply_to, "%s",
	    (*hdr->messageid ? hdr->messageid : "<no.id>"));

}


/*
 * Add contents of ~/.elm/elmheaders file to output stream.
 * Allows `backquote` expansion to do fortune(6) type things. (*shudder*)
 */
static int add_mailheaders(FILE *fp_mssg)
{
    char buf[SLEN], *s, *beg_cmd, *end_cmd;
    int rc;
    FILE *fp_mhdrs;

    sprintf(buf, "%s/%s", user_home, mailheaders);
    if ((fp_mhdrs = fopen(buf, "r")) == NULL)
	return 0;

    rc = -1;
    while (fgets(buf, sizeof(buf), fp_mhdrs) != NULL) {

	trim_trailing_spaces(buf);
	if (buf[0] == '#' || buf[0] == '\0')
	    continue;

	for (s = buf ; s != NULL && *s != '\0' ; ++s) {
	    if (*s != '`') {
		putc(*s, fp_mssg);
		continue;
	    }
	    beg_cmd = s+1;
	    if ((end_cmd = strchr(beg_cmd, '`')) != NULL)
		*end_cmd++ = '\0';
	    if (expand_backquote(fp_mssg, beg_cmd) < 0)
		goto done;
	    s = end_cmd;
	}

	putc('\n', fp_mssg);

    }

    rc = 0;

done:
    if (fp_mhdrs != NULL)
	(void) fclose(fp_mhdrs);
    return rc;
}


/*
 * Execute command dump output to stream.  Blank lines are discarded.
 * Multiple lines are tab indented to allow header continuations.
 */
static int expand_backquote(FILE *fp_mssg, const char *cmd)
{
    FILE *fp_expan;
    char fname[SLEN], buf[SLEN];
    int rc, first_line;

    rc = -1;
    sprintf(fname, "%s%s%d", temp_dir, temp_print, getpid());
    sprintf(buf, "%s > %s", cmd, fname);

    if (system_call(buf, 0) != 0 || (fp_expan = fopen(fname, "r")) == NULL) {
	show_error(catgets(elm_msg_cat, ElmSet, ElmBackquoteCmdFailed,
		"Backquoted command \"%s\" in elmheaders failed."), cmd);
	goto done;
    }

    first_line = TRUE;
    while (fgets(buf, sizeof(buf), fp_expan) != NULL) {
	trim_trailing_spaces(buf);
	if (buf[0] != '\0') {
	    if (!first_line)
		fputs("\n\t", fp_mssg);
	    first_line = FALSE;
	    fputs(buf, fp_mssg);
	}
    }

    (void) fclose(fp_expan);
    rc = 0;

done:
    (void) unlink(fname);
    return rc;
}
