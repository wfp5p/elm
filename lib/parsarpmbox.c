/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
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
 * $Log: parsarpmbox.c,v $
 * Revision 1.4  1996/05/09  15:51:09  wfp5p
 * Alpha 10
 *
 * Revision 1.3  1995/09/29  17:41:25  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:18:56  wfp5p
 * Alpha 7
 *
 * Revision 1.1  1995/07/18  18:59:50  wfp5p
 * Alpha 6
 *
 * 
 *
 ******************************************************************************/


/*
 * parse_arpa_mailbox() parses RFC-822 "mailbox" specifications into address
 * and fullname components.  A "mailbox" is the formal name of the RFC-822
 * lexical element that corresponds to what we normally might call an
 * "address".  (RFC-822 uses the term "address" to describe something else.)
 *
 * A "mailbox" can be in one of two formats:
 *
 *   addr-spec
 *       such as:  joe@acme.com (Joe User)
 *
 * or:
 *
 *   [phrase] "<" [route] addr-spec ">"
 *       such as:  Joe User <joe@acme.com>
 *
 * We invent the names "bare addr-spec" to describe the first form and
 * "angle addr-spec" to describe the second.
 *
 * Synopsis:
 *
 *   int parse_arpa_mailbox(buf, ret_addr, len_addr,
 *		ret_name, len_name, next_field);
 *   char *buf, *ret_addr, *ret_name, **next_field;
 *   int len_addr, len_name;
 *
 * This routine takes a comma-delimited list of mailbox specifications
 * pointed to by "buf", and breaks the next mailbox specification in the
 * list into the address and fullname components.  It is NONdestructive
 * to the buffer.
 *
 * The return code will be 0 for success, -1 for failure.  All bets are
 * off if the mailbox specification is poorly formed (i.e. syntax errors).
 * We might catch the problem and return -1.  Or we might indicate success
 * and return nonsense values.  Other error conditions are discussed
 * below.
 *
 * If "ret_addr" is not NULL, then it points to a buffer where the
 * extracted address is stored and the "len_addr" indicates the size of
 * the buffer.  If we cannot locate a non-empty address or if it is too
 * large to fit into the buffer, then an error is returned.
 *
 * If "ret_name" is not NULL, then it points to a buffer where the
 * extracted fullname is stored and the "len_name" indicates the size of
 * the buffer.  If we cannot locate a non-empty fullname or if it is too
 * large to fit into the buffer, then an empty string is stored in the
 * buffer.  We need to discard the fullname rather than truncating it
 * because truncation could result in an illegal value (e.g. unbalanced
 * quotes).  As an added little glitch, if the fullname value is fully
 * enclosed in double-quotes (and with no interior double-quotes), then
 * the quotes will be stripped.
 *
 * If the "next_field" pointer is not NULL, it will be set to point to
 * the beginning of the next mailbox specification in the list.  It will
 * point to the '\0' string terminator when the list is complete.  This
 * update occurs even if an error code is returned, thus address parsing
 * may continue with the next mailbox in the list.  If the "buf" contains
 * a single address, the "next_field" result should be checked upon return
 * to ensure it points to the '\0' string terminator.
 */

#include "elm_defs.h"


static int fullname_is_quoted(const char *fn_str, int fn_len);
static int parse_bare_addrspec(register const char *buf, char *ret_addr,
			       int len_addr, char *ret_name,
			       int len_name, char **next_field);
static int parse_angle_addrspec(register const char *buf, char *ret_addr,
				int len_addr, char *ret_name, int len_name,
				char **next_field);


int parse_arpa_mailbox(const char *buf, char *ret_addr, int len_addr,
		       char *ret_name, int len_name, char **next_field)
{
    register const char *s;
    int rc;

    /*
     * Take a quick look through the buffer to determine the format.
     */
    for (s = buf ; *s != '\0' && *s != '<' && *s != ',' ; s += rfc822_toklen(s))
	;

    /*
     * Handle as the appropriate format.
     */
    if (*s == '<') {
	rc = parse_angle_addrspec(buf,
	    ret_addr, len_addr, ret_name, len_name, next_field);
    } else {
	rc = parse_bare_addrspec(buf,
	    ret_addr, len_addr, ret_name, len_name, next_field);
    }

    /*
     * In the case of an error, advance to next mailbox field.
     */
    if (rc < 0 && next_field != NULL) {
	for (s = buf ; *s != '\0' && *s != ',' ; s += rfc822_toklen(s))
	    ;
	if (*s == ',')
	    ++s;
	*next_field = (char *) s;
    }

    dprint(5, (debugfile, "parse_arpa_mailbox - addr=\"%s\" name=\"%s\"\n",
	(ret_addr != NULL ? ret_addr : "(null)"),
	(ret_name != NULL ? ret_name : "(null)")));
    return rc;
}


/*
 * Return TRUE if the fullname string is enclosed in double-quotes.
 * AND it is safe to strip the quotes.
 */
static int fullname_is_quoted(const char *fn_str, int fn_len)
{
    if (fn_len < 2 || fn_str[0] != '"' || fn_str[fn_len-1] != '"')
	return FALSE;
    for (fn_len -= 2 ; fn_len > 0 ; --fn_len) {
	switch (*++fn_str) {
	    case '"': return FALSE;
	    case '(': return FALSE;
	    case ')': return FALSE;
	}
    }
    return TRUE;
}


/*
 * Parse a mailbox spec in the format:  addr-spec
 */
static int parse_bare_addrspec(register const char *buf, char *ret_addr,
			       int len_addr, char *ret_name,
			       int len_name, char **next_field)
{
    const char *n_ptr;		/* pointer to (user name) into "buf"	*/
    int n_len;			/* length of text pointed to by "n_ptr"	*/
    register char *a_ptr;	/* pointer into "ret_addr"		*/
    int a_size;			/* space remaining in "ret_addr"	*/
    register int tlen;		/* length of current token		*/
    int got_addr;		/* indicates an address was found	*/

    /*
     * Initialize pointer into address storage, and reserve space for
     * the '\0' terminator.
     */
    a_ptr = ret_addr;
    a_size = (len_addr - 1);

    /*
     * We will set "n_ptr" to the right-most occurance of (parens)
     * encountered when scanning the buffer.  We go back later and
     * extract this into the "ret_name" buffer.
     */
    n_ptr = NULL;
    n_len = 0; /* to keep "gcc -Wall" from whining */

    /*
     * We will set this TRUE when we discover there really is an addr here.
     */
    got_addr = FALSE;

    /*
     * Discard leading space.
     */
    while (isspace(*buf))
	++buf;

    /*
     * Scan through the field, copying out the address elements.
     */
    while (*buf != '\0' && *buf != ',') {
	tlen = rfc822_toklen(buf);
	if (isspace(*buf)) {
	    /*
	     * Discard whitespace.
	     */
	    ; /* nop */
	} else if (*buf == '(') { /*)*/
	    /*
	     * Save info so we can go back later and extract
	     * the right-most comment with (parens) stripped.
	     */
	    n_ptr = buf+1;
	    n_len = tlen-2;
	} else {
	    /*
	     * This is a portion of the address.
	     */
	    if (a_ptr != NULL) {
		if (tlen >= a_size)
		    return -1;
		(void) strncpy(a_ptr, buf, tlen);
		a_ptr += tlen;
		a_size -= tlen;
	    }
	    got_addr = TRUE;
	}
	buf += tlen;
    }

    /*
     * Make sure we extracted a valid address and terminate the string.
     */
    if (!got_addr)
	return -1;
    if (a_ptr != NULL)
	*a_ptr = '\0';

    /*
     * If there is a fullname comment then save it off, else set the result
     * to an empty string.  Also return an empty string if the buffer isn't
     * big enough.  That's because if we only stored off a portion, we
     * could end up with something like unbalanced quotes.
     */
    if (ret_name != NULL) {
	if (n_ptr == NULL) {
	    *ret_name = '\0';
	} else {
	    if (*n_ptr == '"' && fullname_is_quoted(n_ptr, n_len)) {
		++n_ptr;
		n_len -= 2;
	    }
	    if (n_len > 0 && n_len < len_name)
		(void) strfcpy(ret_name, n_ptr, n_len+1);
	    else
		*ret_name = '\0';
	}
    }

    /*
     * We should be at the end of the mailbox field.
     */
    if (*buf != '\0' && *buf != ',')
	return -1;

    /*
     * Save off pointer to next mailbox field.
     */
    if (next_field != NULL)
	*next_field = (char *) buf + (*buf == ',' ? 1 : 0);

    return 0;
}


/*
 * Parse a mailbox spec in the format:  [phrase] "<" [route] addr-spec ">"
 */
static int parse_angle_addrspec(register const char *buf, char *ret_addr,
				int len_addr, char *ret_name, int len_name,
				char **next_field)
{
    const char *beg_field, *end_field;
    register int tlen;
    int w;

    /*
     * Discard leading space.
     */
    while (isspace(*buf))
	++buf;

    /*
     * Locate the front and back of the fullname portion.
     * "end_field" actually points one beyond the end of the field.
     */
    beg_field = end_field = buf;
    while (*buf != '<' && *buf != '\0' && *buf != ',') {
	tlen = rfc822_toklen(buf);

	/*
	 * By updating "end_field" only on non-space tokens we ensure
	 * that when we copy beg_field->end_field trailing whitespace
	 * will be elided.
	 */
	if (!isspace(*buf)) {
	    buf += tlen;
	    end_field = buf;
	} else {
	    buf += tlen;
	}
    }
    if (*buf != '<')
	return -1;

    /*
     * If there is a fullname field then save it off, else set the result
     * to an empty string.  Also return an empty string if the buffer isn't
     * big enough.  That's because if we only stored off a portion, we
     * could end up with something like unbalanced quotes.
     */
    if (ret_name != NULL) {
	w = end_field - beg_field;
	if (w <= 0 || w >= len_name)
	    *ret_name = '\0';
	else {
	    if (*beg_field == '"' && fullname_is_quoted(beg_field, w)) {
		++beg_field;
		w -= 2;
	    }
	    (void) strfcpy(ret_name, beg_field, w+1);
	}
    }

    /*
     * Locate the front and back of the address field.
     * "end_field" actually points one beyond the end of the field.
     */
    beg_field = ++buf;
    while (*buf != '>' && *buf != '\0')
	buf += rfc822_toklen(buf);
    if (*buf != '>')
	return -1;
    end_field = buf;

    /*
     * Calculate the length of the address and save off the result.
     */
    if ((w = end_field - beg_field) <= 0)
	return -1;
    if (ret_addr != NULL) {
	if (w > len_addr)
	    return -1;
	(void) strfcpy(ret_addr, beg_field, w+1);
    }

    /*
     * There shouldn't be anything but comments and whitespace left.
     */
    ++buf;
    while (isspace(*buf) || *buf == '(') /*)*/
	buf += rfc822_toklen(buf);
    if (*buf != '\0' && *buf != ',')
	return -1;

    /*
     * Save off pointer to next mailbox field.
     */
    if (next_field != NULL)
	*next_field = (char *) buf + (*buf == ',' ? 1 : 0);

    return 0;
}


#ifdef _TEST
int debug = 0;
FILE *debugfile = stderr;
main()
{
    char buf[256], abuf[128], nbuf[128], *cf, *nf;
    int rc;

    fputs("Enter address list, one per line.  EOF to terminate\n", stderr);

    while (gets(buf) != NULL) {
	cf = buf;
	while (*cf != '\0') {
	    rc = parse_arpa_mailbox(cf, abuf, sizeof(abuf),
		nbuf, sizeof(nbuf), &nf);
	    if (rc < 0)
		printf("illegal mailbox: %.*s\n", (nf-cf), buf);
	    else
		printf("addr=|%s| name=|%s|\n", abuf, nbuf);
	    cf = nf;
	}
    }

    exit(0);
}
#endif /*_TEST*/

