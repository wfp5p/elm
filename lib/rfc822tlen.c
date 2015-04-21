

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: rfc822tlen.c,v $
 * Revision 1.3  1995/09/29  17:41:35  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:18:58  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

/*
 * rfc822_toklen(str) - Returns length of RFC-822 token that starts at "str".
 *
 * We understand the following tokens:
 *
 *	linear-white-space
 *	"quoted string"
 *	[dom.ain.lit.eral]
 *	(comment)
 *	\c (quoted character)
 *	control characters
 *	special characters (other chars with semantic meaning in addresses)
 *	atom (strings of alphanumerics and non-special/non-control chars)
 *
 * This routine is a profiling hot spot.  To speed things up, a lookup
 * table is used to classify the character types.  The table is initialized
 * the first time this routine is called.
 *
 * At this time, this routine does not do any error handling, and will
 * process defective tokens (e.g. no closing paren or quote).  Grep
 * for ERROR to see the places where error handling should be added if
 * it ever is necessary.
 */

#define charlen(s)	((s)[0] == '\\' && (s)[1] != '\0' ? 2 : 1)

/*
 * Assuming headers only contain 7-bit US-ASCII, which
 * should be true for the structured address fields.
 */
static char chtab[0200];
static int first_time = 1;

#define CH_EOS		0	/* \0 - we should not see this!		*/
#define CH_ATOM		1	/* char that can be part of an atom	*/
#define CH_SPACE	2	/* linear white space character		*/
#define CH_COMMENT	3	/* ( char - comment			*/
#define CH_QSTR		4	/* " char - quoted string		*/
#define CH_QCHAR	5	/* \ char - quoted character		*/
#define CH_DOMLIT	6	/* [ char - domain literal		*/
#define CH_SPECIAL	7	/* some other char with special meaning	*/
#define CH_CTL		8	/* a non-printing control character	*/

#define chtype(c)	(chtab[(c) & 0177])

int rfc822_toklen(const char *str)
{
	const char *str0;
	int depth;

	if (first_time) {
		int i = 0;

		/* most chars in the range 001 - 037 are control chars */
		while (i < 040)
			chtab[i++] = CH_CTL;

		/* most char in the range 040 - 0177 are "atom" chars */
		while (i < 0200)
			chtab[i++] = CH_ATOM;

		chtab[0] = CH_EOS;

		/* mark whitespace chars */
		chtab[' '] = CH_SPACE;
		chtab['\t'] = CH_SPACE;
		chtab['\r'] = CH_SPACE;
		chtab['\n'] = CH_SPACE;

		/* mark special chars that require further lexical processing */
		chtab['"'] = CH_QSTR;
		chtab['('] = CH_COMMENT;
		chtab['['] = CH_DOMLIT;
		chtab['\\'] = CH_QCHAR;

		/* mark remaining chars that are special in address fields */
		chtab[')'] = CH_SPECIAL;
		chtab['<'] = CH_SPECIAL;
		chtab['>'] = CH_SPECIAL;
		chtab['@'] = CH_SPECIAL;
		chtab[','] = CH_SPECIAL;
		chtab[';'] = CH_SPECIAL;
		chtab[':'] = CH_SPECIAL;
		chtab['.'] = CH_SPECIAL;
		chtab[']'] = CH_SPECIAL;

		first_time = 0;
	}

	str0 = str;
	switch (chtype(*str)) {

	case CH_ATOM:
		do {
		    ++str;
		} while (chtype(*str) == CH_ATOM);
		return (str-str0);

	case CH_SPACE:
		do {
			++str;
		} while (chtype(*str) == CH_SPACE);
		return (str-str0);

	case CH_COMMENT:
		++str;
		depth = 0;
		while (*str != '\0' && (*str != ')' || depth > 0)) {
			switch (*str) {
			case '(':
				++str;
				++depth;
				break;
			case ')':
				++str;
				--depth;
				break;
			default:
				str += charlen(str);
				break;
			}
		}
		if (*str == ')')
			++str;
		else
			; /* ERROR - unterminated paren */
		return (str-str0);

	case CH_QSTR:
		++str;
		while (*str != '\0' && *str != '"')
			str += charlen(str);
		if (*str == '"')
			++str;
		else
			; /* ERROR - unterminated quote */
		return (str-str0);

	case CH_QCHAR:
		if (str[1] != '\0')
			return 2;
		return 1; /* ERROR - string ends with backslash */

	case CH_DOMLIT:
		++str;
		while (*str != '\0' && *str != ']')
			str += charlen(str);
		if (*str == ']')
			++str;
		else
			; /* ERROR - unterminated domain literal */
		return (str-str0);

	case CH_EOS:
	        if (*str != '\0') return 1; /* 0x80 and not really end of string */
		return 0; /* ERROR - we should not see this */

	/* case CH_SPECIAL: */
	/* case CH_CTL: */
	default:
		return 1;

	}
	/*NOTREACHED*/
}


#ifdef _TEST

main()
{
	char buf[1024], *bp;
	int len;
	for (;;) {
		fputs("\nstr> ", stdout);
		fflush(stdout);
		if (gets(buf) == NULL) {
			putchar('\n');
			break;
		}
		bp = buf;
		while (*bp != '\0') {
			len = rfc822_toklen(bp);
			printf("len %4d  |%.*s|\n", len, len, bp);
			bp += len;
		}
	}
	exit(0);
}
#endif

