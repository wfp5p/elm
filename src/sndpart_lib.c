#include "elm_defs.h"
#include "elm_globals.h"
#include "mime.h"
#include "sndparts.h"
#include "s_elm.h"
#include <assert.h>


/*******************/
/* Part is parts!! */
/*******************/

/*
 * The values in this array must correspond to
 * the BP_CONT_xxxx definitions.
 */
char *Mime_header_names[BP_NUM_CONT_HEADERS] = {
	"Content-Type:",		/* BP_CONT_TYPE		*/
	"Content-Transfer-Encoding:",	/* BP_CONT_ENCODING	*/
	"Content-Description:",		/* BP_CONT_DESCRIPTION	*/
	"Content-Disposition:",		/* BP_CONT_DISPOSITION	*/
	"Content-MD5:"			/* BP_CONT_MD5		*/
};


static char *scan_mimetypes P_((const char *, const char *, char *, int));


int encoding_is_reasonable(const char *value)
{
    switch (mime_encoding_type(value)) {
    case ENCODING_NONE:
    case ENCODING_7BIT:
    case ENCODING_8BIT:
    case ENCODING_QUOTED:
    case ENCODING_BASE64:
    case ENCODING_UUENCODE:
	return TRUE;
    case ENCODING_BINARY:
    case ENCODING_EXPERIMENTAL:
	show_error("Content encoding value \"%s\" is not reasonable.", value);
	return FALSE;
    case ENCODING_ILLEGAL:
    default:
	show_error("Content encoding value \"%s\" is illegal.", value);
	return FALSE;
    }
    /*NOTREACHED*/
}



/*****************************************************************************
 *
 * Body part management routines.
 *
 ****************************************************************************/


#ifndef NDEBUG
void bodypart_integrity_check(const SEND_BODYPART *part)
{
    assert(part != NULL);

    switch (part->part_type) {

    case BP_IS_DUMMY:
	assert(part->link_count == 0);
	assert(part->fname != NULL);
	assert(part->subparts == NULL);
	assert(part->boundary == NULL);
	break;

    case BP_IS_MSSGTEXT:
	assert(part->link_count == 0);
	assert(part->fname != NULL);
	assert(part->subparts != NULL);
	break;

    case BP_IS_MIMEPART:
	assert(part->link_count >= 0);
	assert(part->fname != NULL);
	assert(part->subparts == NULL);
	assert(part->boundary == NULL);
	break;

    case BP_IS_MULTIPART:
	assert(part->link_count >= 0);
	assert(part->fname == NULL);
	assert(part->subparts != NULL);
	break;

    default:
	assert((part->part_type, FALSE));
	break;

    }

}
#endif /* !NDEBUG */



/*
 * bodypart_new - Create a new (SEND_BODYPART).
 *
 * fname - Pathname of the file that contains this part.
 * part_type - The BP_IS_xxxx type of this part.
 *
 * This routine merely returns an initialized (SEND_BODYPART) for the
 * part.  The only verification is that the filename exists.  All
 * of the MIME headers are initialized to NULL, and none of the
 * part statistics are filled in.
 */
SEND_BODYPART *bodypart_new(const char *fname, int part_type)
{
    SEND_BODYPART *part;
    int i;

    assert(fname != NULL);
    if (part_type != BP_IS_DUMMY && file_access(fname, R_OK) < 0)
	return (SEND_BODYPART *) NULL;

    part = (SEND_BODYPART *) safe_malloc(sizeof(SEND_BODYPART));

    part->part_type = part_type;
    part->link_count = 0;
    part->fname = safe_strdup(fname);
    part->emit_hdr_proc = NULL;
    part->emit_body_proc = NULL;

    for (i = 0 ; i < BP_NUM_CONT_HEADERS ; ++i)
	part->content_header[i] = NULL;

    part->total_chars = 0L;
    part->ascii_chars = 0L;
    part->binhi_chars = 0L;
    part->binlo_chars = 0L;
    part->max_linelen = 0;

    if (part_type == BP_IS_MSSGTEXT || part_type == BP_IS_MULTIPART)
	part->subparts = multipart_new((SEND_BODYPART *) NULL, 0L);
    else
	part->subparts = NULL;
    part->boundary = NULL;

    bodypart_integrity_check(part);
    return part;
}


/*
 * bodypart_destroy - Destroy a (SEND_BODYPART).
 *
 * This routine releases the space created by bodypart_new().
 */
void bodypart_destroy(SEND_BODYPART *part)
{
    int i;
    bodypart_integrity_check(part);
    assert(part->link_count == 0);
    if (part->fname != NULL)
	free((malloc_t)part->fname);
    for (i = 0 ; i < BP_NUM_CONT_HEADERS ; ++i) {
	if (part->content_header[i] != NULL)
	    free((malloc_t)part->content_header[i]);
    }
    if (part->subparts != NULL)
	multipart_destroy(part->subparts);
    if (part->boundary != NULL)
	free((malloc_t)part->boundary);
    free((malloc_t)part);
}


/*
 * bodypart_set_content() - Set a particular MIME header.
 *
 * part - The (SEND_BODYPART) to modify.
 * sel - The BP_CONT_xxxx header to set.
 * value - The header value.  A NULL or empty string de-assigns the header.
 */
void bodypart_set_content(SEND_BODYPART *part, int sel, const char *value)
{
    assert(sel >= 0 && sel < BP_NUM_CONT_HEADERS);
    bodypart_integrity_check(part);

    if (part->content_header[sel] != NULL)
	free((malloc_t)part->content_header[sel]);
    part->content_header[sel] = (value && *value ? safe_strdup(value) : NULL);
}


/*
 * bodypart_get_content() - Retrieve a particular MIME header.
 *
 * part - The (SEND_BODYPART) to examine.
 * sel - The BP_CONT_xxxx selector of the header.
 *
 * Returns the header value, or an empty string if the header is not set.
 */
const char *bodypart_get_content(SEND_BODYPART *part, int sel)
{
    const char *value;

    assert(sel >= 0 && sel < BP_NUM_CONT_HEADERS);
    bodypart_integrity_check(part);

    value = part->content_header[sel];
    return (value ? value : "");
}



/*
 * bodypart_guess_content() - Set a particular MIME header.
 *
 * part - The (SEND_BODYPART) to modify.
 * sel - The BP_CONT_xxxx header to set.
 *
 * This routine applies some simple heuristics to guess a
 * reasonable value for the header.
 */
void bodypart_guess_content(SEND_BODYPART *part, int sel)
{
    char buf[SLEN], *value, *fname_tmp, *bp;
    int len;
    FILE *fp;
    float p;

    assert(sel >= 0 && sel < BP_NUM_CONT_HEADERS);
    bodypart_integrity_check(part);

    switch (sel) {

    case BP_CONT_TYPE:
	/* FOO - scan a user mimetypes file? */
	value = scan_mimetypes(system_mimetypes_file, part->fname,
		    buf, sizeof(buf));
	if (value == NULL) {
	    value = "application/octet-stream";
	    /* following heuristic assumes reasonable text won't be this long */
	    if (part->max_linelen < 200) {
	        p = (float)((part->binlo_chars+part->binhi_chars)/part->total_chars);
		if (part->ascii_chars == part->total_chars) {
		    value = "text/plain; charset=us-ascii";
		} else if (part->binlo_chars == 0) {
		    sprintf(buf, "text/plain; charset=%s", charset);
		    value = buf;
		} else if ((1.5*p)+0.75 <= 1.0) {
		    if (part->binhi_chars == 0)
		        value = "text/plain; charset=us-ascii";
		    else {
		        sprintf(buf, "text/plain; charset=%s", charset);
			value = buf;
		    }
		}
	    }
	}
	break;

    case BP_CONT_ENCODING:
       {
 	    /* pick based upon rough estimates of the encoded sizes */
 	    long qp_size = part->ascii_chars
 		+ (part->total_chars-part->ascii_chars) * (sizeof("=00")-1);
	    long b64_size = (part->total_chars * 4) / 3;
 	    value = (qp_size < b64_size ? "quoted-printable" : "base64");
       }
 	if (part->max_linelen <= RFC821_MAXLEN) {
            if (part->ascii_chars == part->total_chars)
		value = "7bit";
	    else if (part->binlo_chars == 0)
		value = "8bit";
	}
	break;

    case BP_CONT_DESCRIPTION:
	value = NULL;
	if ((fname_tmp = tempnam(temp_dir, "fil.")) != NULL) {
	    MIME_FILE_CMD(buf, part->fname, fname_tmp);
	    if (system_call(buf, 0) == 0) {
		if ((fp = fopen(fname_tmp, "r")) != NULL) {
		    if (fgets(buf, sizeof(buf), fp) != NULL) {
			bp = trim_trailing_spaces(buf);
			len = strlen(part->fname);
			if (strncmp(buf, part->fname, len) == 0) {
			    for (bp += len ; *bp == ':' || isspace(*bp) ; ++bp)
				;
			}
			value = bp;
		    }
		    (void) fclose(fp);
		}
	    }
	    (void) unlink(fname_tmp);
	    (void) free((malloc_t)fname_tmp);
	}
	break;

    case BP_CONT_DISPOSITION:
	sprintf(buf, "attachment; filename=\"%s\"", basename(part->fname));
	value = buf;
	break;

    case BP_CONT_MD5:
	value = NULL; /* FOO - not implemented yet */
	break;

    default:
	value = NULL; /* can't happen */
	break;

    }

    bodypart_set_content(part, sel, value);
}


static char *scan_mimetypes(const char *fname_mimetypes, const char *fname_part, char *retbuf, int retbufsiz)
{
    char *s;
    int len_fname, len_ext;
    FILE *fp;

    if ((fp = fopen(fname_mimetypes, "r")) == NULL)
	return (char *) NULL;

    len_fname = strlen(fname_part);

    while (fgets(retbuf, retbufsiz, fp) != NULL) {

	/* trim newline, skip comments and blank lines */
	(void) trim_trailing_spaces(retbuf);
	if (retbuf[0] == '\0' || retbuf[0] == '#')
	    continue;

	/* locate end of extension field */
	if (isspace(retbuf[0]))
	    continue;	/* blah - illegal line */
	for (s = retbuf ; *s != '\0' && !isspace(*s) ; ++s)
	    ;

	/* locate start of content type field */
	for (*s++ = '\0' ; isspace(*s) ; ++s)
	    ;
	if (*s == '\0')
	    continue; /* blah - illegal line */

	if ((len_ext = strlen(retbuf)) < len_fname
		    && fname_part[len_fname-len_ext-1] == '.'
		    && streq(fname_part+len_fname-len_ext, retbuf)) {
	    (void) fclose(fp);
	    return s;
	}

    }

    (void) fclose(fp);
    return (char *)NULL;
}


/*****************************************************************************
 *
 * Multipart management routines.
 *
 ****************************************************************************/


#ifndef NDEBUG
void multipart_integrity_check(const SEND_MULTIPART *multi)
{
    const SEND_MULTIPART *m1;

    assert(multi != NULL);
    assert(multi->part == NULL);
    assert(multi->prev->next == multi);
    assert(multi->next->prev == multi);

    for (m1 = multi->next ; m1 != multi ; m1 = m1->next) {
	assert(m1->prev->next == m1);
	assert(m1->next->prev == m1);
	assert(m1->part != NULL);
	assert(m1->part->link_count > 0);
    }
}
#endif /* !NDEBUG */


SEND_MULTIPART *multipart_new(SEND_BODYPART *part, long id)
{
    SEND_MULTIPART *multi;

    multi = (SEND_MULTIPART *) safe_malloc(sizeof(SEND_MULTIPART));
    multi->id = id;
    multi->part = part;
    multi->prev = multi->next = multi;
    return multi;
}

void multipart_destroy(SEND_MULTIPART *multi)
{
    SEND_MULTIPART *mnext;

    multipart_integrity_check(multi);
    mnext = multi->next;

    while (multi = mnext, multi->part != NULL) {
	mnext = multi->next;
	assert(multi->part->link_count >= 0);
	if (--multi->part->link_count == 0)
	    bodypart_destroy(multi->part);
	free((malloc_t)multi);
    }

    free((malloc_t)multi);
}


SEND_MULTIPART *multipart_insertpart(SEND_MULTIPART __attribute__((unused)) *multi,
				     SEND_MULTIPART *mp_curr,
				     SEND_BODYPART *part, long id)
{
    SEND_MULTIPART *mp_new;

    multipart_integrity_check(multi);
    mp_new = multipart_new(part, id);

    /* insert "mp_new" before "mp_curr */
    mp_new->next = mp_curr;
    mp_new->prev = mp_curr->prev;
    mp_curr->prev->next = mp_new;
    mp_curr->prev = mp_new;

    ++part->link_count;
    multipart_integrity_check(multi);
    return mp_new;
}


SEND_MULTIPART *multipart_appendpart(SEND_MULTIPART __attribute__((unused)) *multi,
				     SEND_MULTIPART *mp_curr,
				     SEND_BODYPART *part, long id)
{
    SEND_MULTIPART *mp_new;

    multipart_integrity_check(multi);
    mp_new = multipart_new(part, id);

    /* append "mp_new" after "mp_curr" */
    mp_new->prev = mp_curr;
    mp_new->next = mp_curr->next;
    mp_curr->next->prev = mp_new;
    mp_curr->next = mp_new;

    ++part->link_count;
    multipart_integrity_check(multi);
    return mp_new;
}


SEND_BODYPART *multipart_deletepart(SEND_MULTIPART __attribute__((unused)) *multi,
				    SEND_MULTIPART *mp_curr)
{
    SEND_BODYPART *part;

    multipart_integrity_check(multi);
    mp_curr->prev->next = mp_curr->next;
    mp_curr->next->prev = mp_curr->prev;
    part = mp_curr->part;
    free((malloc_t)mp_curr);
    --part->link_count;
    multipart_integrity_check(multi);
    return part;
}



SEND_MULTIPART *multipart_next(SEND_MULTIPART *multi, SEND_MULTIPART *mp_curr)
{
    mp_curr = (mp_curr == NULL ? multi->next : mp_curr->next);
    return (mp_curr->part == NULL ? (SEND_MULTIPART *) NULL : mp_curr);
}


SEND_MULTIPART *multipart_find(SEND_MULTIPART *multi, long id)
{
    while (multi = multi->next, multi->part != NULL && multi->id != id)
	;
    return multi;
}

