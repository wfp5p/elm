
/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
 *
 * This file and all associated files and documentation:
 *                      Copyright (c) 1988-1998 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: sndpart_io.c,v $
 * Revision 1.3  1999/03/24  14:04:07  wfp5p
 * elm 2.5PL0
 *
 *
 ******************************************************************************/




#include "elm_defs.h"
#include "elm_globals.h"
#include "mime.h"
#include "sndparts.h"
#include "s_elm.h"
#include <assert.h>

static int multipart_seqnum;

static SEND_BODYPART *process_attach_line P_((char *));

static int do_emit_mssgtext_header P_((FILE *, SEND_BODYPART *));
static int do_emit_mssgtext_body P_((FILE *, SEND_BODYPART *));
static int do_emit_mimepart_header P_((FILE *, SEND_BODYPART *));
static int do_emit_mimepart_body P_((FILE *, SEND_BODYPART *));
static int do_emit_multipart_header P_((FILE *, SEND_BODYPART *));
static int do_emit_multipart_sep P_((FILE *, SEND_BODYPART *, int));


/*
 * This is the first of two routines that create a new (SEND_BODYPART).
 * This one is used to process generic parts, such as attachments/inclusions.
 */
SEND_BODYPART *newpart_mimepart(const char *fname)
{
    unsigned char buf[512], *s;
    int len, n;
    FILE *fp;
    SEND_BODYPART *part;

    if ((part = bodypart_new(fname, BP_IS_MIMEPART)) == NULL)
	return (SEND_BODYPART *) NULL;
    part->emit_hdr_proc = do_emit_mimepart_header;
    part->emit_body_proc = do_emit_mimepart_body;
    if ((fp = file_open(fname, "r")) == NULL) {
	bodypart_destroy(part);
	return (SEND_BODYPART *) NULL;
    }

    len = 0;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {

	for (s = buf ; --n >= 0 ; ++s) {

	    if (*s == '\n') {
		if (len > part->max_linelen)
		    part->max_linelen = len;
		len = 0;
	    } else {
		++len;
	    }

	    if (*s & 0x80) {
		++part->binhi_chars;
	    } else if ((*s & 0xE0) || isspace(*s)) {
		/* either in the range 0x20-0x7F or is whitespace */
		++part->ascii_chars;
	    } else
		++part->binlo_chars;

	    ++part->total_chars;

	}

    }

    if (file_close(fp, fname) < 0) {
	bodypart_destroy(part);
	return (SEND_BODYPART *) NULL;
    }

    /*
     * We have not set a content type (which IS required) or encoding.
     * The calling procedure probably should do so immediately.
     */

    bodypart_integrity_check(part);
    return part;
}



/*
 * This is the second of two routines that create a new (SEND_BODYPART).
 * This one is used to process the text of the main message.
 */
SEND_BODYPART *newpart_mssgtext(const char *fname)
{
    char buf[SLEN];
    unsigned char *s;
    int len;
    long pos;
    FILE *fp;
    SEND_BODYPART *part, *subpart;

    if ((part = bodypart_new(fname, BP_IS_MSSGTEXT)) == NULL)
	return (SEND_BODYPART *) NULL;
    part->emit_hdr_proc = do_emit_mssgtext_header;
    part->emit_body_proc = do_emit_mssgtext_body;
    if ((fp = file_open(fname, "r")) == NULL) {
	bodypart_destroy(part);
	return (SEND_BODYPART *) NULL;
    }

    while (pos = ftell(fp), fgets(buf, sizeof(buf), fp) != NULL) {

	if (buf[0] == '[') {

	    if (strbegConst(buf+1, MSSG_ATTACH)) {
		if ((subpart = process_attach_line(buf)) == NULL) {
		    (void) fclose(fp);
		    bodypart_destroy(part);
		    return (SEND_BODYPART *) NULL;
		}
		multipart_insertpart(part->subparts,
			    MULTIPART_TAIL(part->subparts),
			    subpart, MP_ID_ATTACHMENT);
		continue;
	    }

	    if (strbegConst(buf+1, MSSG_INCLUDE)) {
		if ((subpart = process_attach_line(buf)) == NULL) {
		    (void) fclose(fp);
		    bodypart_destroy(part);
		    return (SEND_BODYPART *) NULL;
		}
		multipart_insertpart(part->subparts,
			    MULTIPART_TAIL(part->subparts), subpart, pos);
		continue;
	    }

	}

	for (s = (unsigned char *)buf ; *s != '\0' ; ++s) {

	    if (*s & 0x80) {
		++part->binhi_chars;
	    } else if ((*s & 0xE0) || isspace(*s)) {
		/* either in the range 0x20-0x7F or is whitespace */
		++part->ascii_chars;
	    } else {
		++part->binlo_chars;
	    }
	    ++part->total_chars;

	}
	len = (s - (unsigned char *)buf) - 1; /* length less '\n' term */
	if (len > part->max_linelen)
		part->max_linelen = len;

    }

    if (file_close(fp, fname) < 0) {
	bodypart_destroy(part);
	return (SEND_BODYPART *) NULL;
    }

    if (part->binhi_chars == 0) {
	bodypart_set_content(part, BP_CONT_TYPE,
		    "text/plain; charset=us-ascii");
    } else {
	sprintf(buf, "text/plain; charset=%s", charset);
	bodypart_set_content(part, BP_CONT_TYPE, buf);
    }

    if (part->max_linelen > RFC821_MAXLEN)
	bodypart_set_content(part, BP_CONT_ENCODING, "quoted-printable");
    else if (part->binhi_chars == 0)
	bodypart_set_content(part, BP_CONT_ENCODING, "7bit");
    else
	bodypart_set_content(part, BP_CONT_ENCODING, "8bit");

    bodypart_integrity_check(part);
    return part;
}


/*
 * Process "[attach/include filename content-type content-encoding]" line.
 */
static SEND_BODYPART *process_attach_line(char *cmdline)
{
    char *fld, *cmd, *fname, *cont_type, *cont_encoding;
    char buf[SLEN];
    const char *actual_type;
    SEND_BODYPART *part;
    int fldno, len;

    len = strlen(trim_trailing_spaces(cmdline));
    if (cmdline[0] != '[' || cmdline[len-1] != ']') {
	error("malformed \"[attach/include ...]\" line");
	return (SEND_BODYPART *) NULL;
    }
    cmdline[len-1] = '\0';
    ++cmdline;

    for (fldno = 0 ; (fld = strtok(cmdline, " \t")) != NULL ; ++fldno) {
	cmdline = NULL;
	switch (fldno) {
	case 0:		/* attach/include */
	    cmd = fld;
	    break;
	case 1:		/* filename */
	    fname = fld;
	    break;
	case 2:		/* content-type */
	    cont_type = fld;
	    break;
	case 3:		/* content-encoding */
	    cont_encoding = fld;
	    break;
	default:	/* too many fields */
	    break;
	}
    }

    switch (fldno) {
    case 0:
	error("malformed \"[attach/include ...]\" line");
	return (SEND_BODYPART *) NULL;
    case 1:
	error1("filename missing from \"[%s ...]\" line", cmd);
	return (SEND_BODYPART *) NULL;
    case 2:
	cont_type = NULL;
	/*FALLTHRU */
    case 3:
	cont_encoding = NULL;
	/*FALLTHRU*/
    case 4:
	break;
    default:
	error1("too many fields in \"[%s ...]\" line", cmd);
	return (SEND_BODYPART *) NULL;
    }

    if (cont_encoding != NULL && !encoding_is_reasonable(cont_encoding))
	return (SEND_BODYPART *) NULL;

    if ((part = newpart_mimepart(fname)) == NULL)
	return (SEND_BODYPART *) NULL;

    if (cont_type == NULL)
	bodypart_guess_content(part, BP_CONT_TYPE);
    else
	bodypart_set_content(part, BP_CONT_TYPE, cont_type);

    if (cont_encoding == NULL)
	bodypart_guess_content(part, BP_CONT_ENCODING);
    else
/*	bodypart_set_content(part, BP_CONT_ENCODING, cont_type);*/
     bodypart_set_content(part, BP_CONT_ENCODING, cont_encoding);

    actual_type = bodypart_get_content(part, BP_CONT_TYPE);
    if (actual_type == NULL || strncasecmp(actual_type, "text", 4) != 0) {
	bodypart_guess_content(part, BP_CONT_DESCRIPTION);
	bodypart_guess_content(part, BP_CONT_MD5);
    }

    sprintf(buf, "%s; filename=\"%s\"",
		(*cmd == 'i' ? "inline" : "attachment"), basename(fname));
    bodypart_set_content(part, BP_CONT_DISPOSITION, buf);

    bodypart_integrity_check(part);
    return part;
}


/*****************************************************************************
 *
 * Message part headers and body generation.
 *
 ****************************************************************************/


/* write out both part header and body with separating newline */
int emitpart(FILE *fp, SEND_BODYPART *part)
{
    bodypart_integrity_check(part);
    assert(part->emit_hdr_proc != NULL);
    assert(part->emit_body_proc != NULL);
    if ((*part->emit_hdr_proc)(fp, part) < 0)
	return -1;
    putc('\n', fp);
    if ((*part->emit_body_proc)(fp, part) < 0)
	return -1;
    return 0;
}


/* write out the header for this part */
int emitpart_hdr(FILE *fp, SEND_BODYPART *part)
{
    bodypart_integrity_check(part);
    assert(part->emit_hdr_proc != NULL);
    return (*part->emit_hdr_proc)(fp, part);
}


/* write out the body for this part */
int emitpart_body(FILE *fp, SEND_BODYPART *part)
{
    bodypart_integrity_check(part);
    assert(part->emit_body_proc != NULL);
    return (*part->emit_body_proc)(fp, part);
}


static int do_emit_mssgtext_header(FILE *fp, SEND_BODYPART *part)
{
    bodypart_integrity_check(part);
    assert(part->part_type == BP_IS_MSSGTEXT);
    fputs("MIME-Version: 1.0\n", fp);
    return (MULTIPART_IS_EMPTY(part->subparts)
		? do_emit_mimepart_header(fp, part)
		: do_emit_multipart_header(fp, part));
}

static int do_emit_mssgtext_body(FILE *fp_dest, SEND_BODYPART *part)
{
    char buf[SLEN];
    SEND_MULTIPART *mp;
    int rc, part_active, len;
    int have_encode_key, curr_encode_running, new_encode_running;
    long fpos;
    FILE *fp_src;

    bodypart_integrity_check(part);
    assert(part->part_type == BP_IS_MSSGTEXT);

    rc = -1;
    have_encode_key = FALSE;
    curr_encode_running = new_encode_running = FALSE;

    /*
     * This clever hack initializes properly on multipart/mixed
     * and suppresses multipart boundaries on text/plain.
     */
    part_active = MULTIPART_IS_EMPTY(part->subparts);

    if ((fp_src = file_open(part->fname, "r")) == NULL)
	goto done;

    while (fpos = ftell(fp_src), (len =
		mail_gets(buf, sizeof(buf), fp_src)) > 0) {

	if (buf[0] == '[') {

	    /* [nosave] and [no save] are directives to savecopy */
	    if (strbegConst(buf, MSSG_DONT_SAVE))
		continue;
	    if (strbegConst(buf, MSSG_DONT_SAVE2))
		continue;

	    /* [attach ...] handled after body complete */
	    if (strbegConst(buf+1, MSSG_ATTACH))
		continue;

	    /* do [include] */
	    if (strbegConst(buf+1, MSSG_INCLUDE)) {
		assert(!MULTIPART_IS_EMPTY(part->subparts));
		if ((mp = multipart_find(part->subparts, fpos)) == NULL) {
		    error1(catgets(elm_msg_cat, ElmSet,
			    ElmCannotRefindInclusion,
			    "Cannot re-find inclusion located at offset %lu!"),
			    (unsigned long)fpos);
		    goto done;
		}
		if (do_emit_multipart_sep(fp_dest, part, FALSE) < 0)
		    goto done;
		if (emitpart(fp_dest, MULTIPART_PART(mp)) < 0)
		    goto done;
		part_active = FALSE;
		continue;
	    }

	}

	/* see if we need to emit the multipart separator */
	if (!part_active) {
	    if (do_emit_multipart_sep(fp_dest, part, FALSE) < 0)
		goto done;
	    if (do_emit_mimepart_header(fp_dest, part) < 0)
		goto done;
	    putc('\n', fp_dest);
	    part_active = TRUE;
	}

	/* urer vf bhe fhcre frperg qrpbqre evat */
	if (curr_encode_running)
	    encode(buf);
	curr_encode_running = new_encode_running;

#ifdef NEED_LONE_PERIOD_ESCAPE
	/* some transports (e.g. smail2.5) take lone dot as end of message */
	if (buf[0] == '.' && buf[1] == '\n' && buf[2] == '\0')
	    (void) strcpy(buf, ". \n");
#endif

	/* emit the line of output */
	if (fwrite(buf, 1, len, fp_dest) != len) {
	    error1("Error writing message body to temp file.  [%s]",
		strerror(errno));
	    goto done;
	}

    }

    /* emit the attachments */
    if (!MULTIPART_IS_EMPTY(part->subparts)) {
	mp = NULL;
	while ((mp = multipart_next(part->subparts, mp)) != NULL) {
	    if (MULTIPART_ID(mp) == MP_ID_ATTACHMENT) {
		if (do_emit_multipart_sep(fp_dest, part, FALSE) < 0)
		    goto done;
		if (emitpart(fp_dest, MULTIPART_PART(mp)) < 0)
		    goto done;
	    }
	}
	if (do_emit_multipart_sep(fp_dest, part, TRUE) < 0)
	    goto done;
    }

    if (file_close(fp_src, part->fname) < 0)
	goto done;
    fp_src = NULL;
    rc = 0;

done:
    if (fp_src != NULL)
	(void) fclose(fp_src);
    return rc;
}


static int do_emit_mimepart_header(FILE *fp, SEND_BODYPART *part)
{
    int i;
    assert(part->content_header[BP_CONT_TYPE]);
    assert(*part->content_header[BP_CONT_TYPE]);
    for (i = 0 ; i < BP_NUM_CONT_HEADERS ; ++i) {
	if (part->content_header[i] && *part->content_header[i]) {
	    fputs(Mime_header_names[i], fp);
	    putc(' ', fp);
	    fputs(part->content_header[i], fp);
	    putc('\n', fp);
	}
    }
   return 1;
}


static int do_emit_mimepart_body(FILE *fp_dest, SEND_BODYPART *part)
{
    char *fname_tmp, *fname_sel, cmd_buf[SLEN], *s;
    int rc, i;
    FILE *fp_src;

    rc = -1;
    fp_src = NULL;

    switch (mime_encoding_type(part->content_header[BP_CONT_ENCODING])) {

    case ENCODING_QUOTED:
	if ((fname_tmp = tempnam(temp_dir, "emm.")) == NULL) {
	    error("Cannot make temp file name.");
	    return -1;
	}
	MIME_ENCODE_CMD_QUOTED(cmd_buf, part->fname, fname_tmp);
	break;

    case ENCODING_BASE64:
	if ((fname_tmp = tempnam(temp_dir, "emm.")) == NULL) {
	    error("Cannot make temp file name.");
	    return -1;
	}
	MIME_ENCODE_CMD_BASE64(cmd_buf, part->fname, fname_tmp);
	break;

    case ENCODING_UUENCODE:
	if ((fname_tmp = tempnam(temp_dir, "emm.")) == NULL) {
	    error("Cannot make temp file name.");
	    return -1;
	}
	MIME_ENCODE_CMD_UUENCODE(cmd_buf, part->fname, fname_tmp);
	break;

    default:
	/* encoding not needed */
	fname_tmp = NULL;
	break;
    }

    if (fname_tmp == NULL) {
	fname_sel = part->fname;
    } else if ((i =  system_call(cmd_buf, 0)) == 0) {
	fname_sel = fname_tmp;
    } else {
	for (s = cmd_buf ; *s != '\0' && !isspace(*s) ; ++s)
	    ;
	*s = '\0';
	s = (cmd_buf[0] != '\0' ? basename(cmd_buf) : "encoder");
	error2("Cannot encode \"%s\".  (\"%s\" exit status %d)",
		    basename(part->fname), s, i);
	goto done;
    }

    if ((fp_src = file_open(fname_sel, "r")) == NULL)
	goto done;
    if (file_copy(fp_src, fp_dest, fname_sel, "message body") < 0)
	goto done;
    if (file_close(fp_src, fname_sel) < 0)
	goto done;
    else 
        fp_src = NULL;

    rc = 0;

done:
    if (fp_src != NULL)
	(void) fclose(fp_src);
    if (fname_tmp != NULL) {
	(void) unlink(fname_tmp);
	free((malloc_t)fname_tmp);
    }
    return rc;
}



static int do_emit_multipart_header(FILE *fp, SEND_BODYPART *part)
{
    char buf[SLEN];
    if (part->boundary == NULL) {
	MIME_MAKE_BOUNDARY(buf, "multipart-mixed", ++multipart_seqnum);
	part->boundary = safe_strdup(buf);
    }
    fprintf(fp, "Content-Type: multipart/mixed; boundary=\"%s\"\n",
		part->boundary);
   return 1;
}

/* ***** not used
static int do_emit_multipart_body(fp_dest, part)
FILE *fp_dest;
SEND_BODYPART *part;
{
    SEND_MULTIPART *mp;

    assert(part->boundary != NULL);
    assert(!MULTIPART_IS_EMPTY(part->subparts));

    mp = NULL;
    while ((mp = multipart_next(part->subparts, mp)) != NULL) {
	if (do_emit_multipart_sep(fp_dest, part, FALSE) < 0)
	    return -1;
	if (emitpart(fp_dest, MULTIPART_PART(mp)) < 0)
	    return -1;
    }

    return do_emit_multipart_sep(fp_dest, part, TRUE);
}
*/

static int do_emit_multipart_sep(FILE *fp, SEND_BODYPART *part, int finished)
{
    assert(!MULTIPART_IS_EMPTY(part->subparts));
    assert(part->boundary != NULL);
    fputs("\n--", fp);
    fputs(part->boundary, fp);
    if (finished)
	fputs("--", fp);
    putc('\n', fp);
    return 0;
}

