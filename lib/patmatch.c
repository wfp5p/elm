#include "elm_defs.h"

/*
 * patmatch(pat, str, opts) - Very simple pattern matching.
 *
 * Returns TRUE if the pattern "pat" completely matches
 * any portion of the string "str".
 *
 * Patterns may include the following special characters:
 *
 *   - A '?' matches any single character.
 *   - A '*' matches zero or more characters.
 *   - A '^' at the front of the pattern anchors the front.
 *   - A '$' at the end of the pattern anchors the tail.
 *   - A '\' suppresses the special meaning of the above characters.
 *
 * The following options modify the match conditions:
 *
 *   PM_NOCASE  Matching of alphabetic chars normally is case-significant.
 *		When this option is specified, alphabetic characters are
 *		compared in a case-INsignificant fashion.
 *
 *   PM_WSFOLD	When this option is specified, any sequence of one or more
 *		whitespace characters in the pattern match a sequence of
 *		any one or more whitespace characters in the target string.
 *
 *   PM_FANCHOR	When this option is specified, the pattern is treated as
 *		if a "^" anchor appeared at the front.
 *
 *   PM_BANCHOR	When this option is specified, the pattern is treated as
 *		if a "$" anchor appeared at the back.
 *
 * That's it.  No [character classes], or other overly fancy stuff.
 */

static int trymatch(const char *str, const char *pat, const char *pstop,
		    int opts)
{

    while (pat < pstop) {

	/* handle "*" wildcard */
	if (*pat == '*') {
	    do {
		if (++pat >= pstop)
		    return TRUE;
	    } while (*pat == '*');
	    for ( ; *str != '\0' ; ++str) {
		if (trymatch(str, pat, pstop, opts))
		    return TRUE;
	    }
	    return FALSE;
	}

	/* end of string matches nothing but "*" */
	if (*str == '\0')
	    return FALSE;

	switch (*pat) {

	case '?':	/* "?" matches anything */
	    ++str;
	    ++pat;
	    break;

	case ' ':	/* whitespace */
	case '\t':
	case '\n':
	case '\r':
	    if (opts & PM_WSFOLD) {
		if (!isspace(*str))
		    return FALSE;
		do {
		    ++pat;
		} while (pat < pstop && isspace(*pat));
		do {
		    ++str;
		} while (*str != '\0' && isspace(*str));
	    } else {
		if (*pat != *str)
		    return FALSE;
		++pat;
		++str;
	    }
	    break;

	case '\\':	/* literal character */
	    if (++pat >= pstop || *pat != *str)
		return FALSE;
	    ++pat;
	    ++str;
	    break;

	default:		/* character match */
	    if (opts & PM_NOCASE) {
		if (tolower(*pat) != tolower(*str))
		    return FALSE;
	    } else {
		if (*pat != *str)
		    return FALSE;
	    }
	    ++str;
	    ++pat;
	    break;

	}

    }

    return (*str == '\0' || !(opts & PM_BANCHOR));
}

int patmatch(const char *pat, const char *str, int opts)
{
    int len;
    const char *pstop;

    if (pat[0] == '^') {
	opts |= PM_FANCHOR;
	++pat;
    }

    len = strlen(pat);
    if (len > 1 && pat[len-1] == '$' && pat[len-2] != '\\') {
	opts |= PM_BANCHOR;
	--len;
    }
    pstop = pat+len;

    for ( ; *str != '\0' ; ++str) {
	if (trymatch(str, pat, pstop, opts))
	    return TRUE;
	if (opts & PM_FANCHOR)
	    return FALSE;
    }
    return FALSE;
}



#ifdef _TEST

main()
{
	char pat[256], str[256], ans[256];
	int opts;

	(void) strcpy(pat, "*");
	(void) strcpy(str, "hello world");
	opts = 0;

	for (;;) {

		printf("options [%d] : ", opts);
		fflush(stdout);
		if (gets(ans) == NULL)
		    break;
		if (ans[0] != '\0')
		    opts = atoi(ans);

		printf("pattern [%s] : ", pat);
		fflush(stdout);
		if (gets(ans) == NULL)
			break;
		if (ans[0] != '\0')
			(void) strcpy(pat, ans);

		printf("string  [%s] : ", str);
		fflush(stdout);
		if (gets(ans) == NULL)
			break;
		if (ans[0] != '\0')
			(void) strcpy(str, ans);

		fputs("options =", stdout);
		if (opts & PM_NOCASE)	fputs(" NOCASE", stdout);
		if (opts & PM_WSFOLD)	fputs(" WSFOLD", stdout);
		if (opts & PM_FANCHOR)	fputs(" FANCHOR", stdout);
		if (opts & PM_BANCHOR)	fputs(" BANCHOR", stdout);
		putchar('\n');
		printf("patmatch returns %s\n\n",
			(patmatch(pat, str, opts) ? "TRUE" : "FALSE"));


	}

}

#endif /* _TEST */

