

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
 * $Log: getword.c,v $
 * Revision 1.3  1996/03/14  17:27:37  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:12  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

int get_word(const char *buffer, int start, char *word, int wordlen)
{
    /*
     * Extracts the next white-space delimited word from the "buffer"
     * starting at "start" characters into the buffer and skipping any
     * leading white-space there.  Handles backslash-quoted characters,
     * (parenthesized comments) and "quoted strings" as an atomic unit.
     * The resulting word, up to "wordlen" bytes long, is saved in "word".
     * Returns the buffer index where extraction terminated, e.g. the next
     * word can be extracted by starting at start+<return-val>.  If no words
     * are found in the buffer then -1 is returned.
     */

    int len;
    const char *p;

    for (p = buffer+start ; isspace(*p) ; ++p)
	;

    if (*p == '\0')
	return (-1);		/* nothing IN buffer! */

    if (*p == '(') { /*(*/	/* parenthesized comment */
	len = rfc822_toklen(p);
	if (len < wordlen) {
	    (void) strncpy(word, p, len);
	    word[len] = '\0';
	} else {
	    (void) strfcpy(word, p, wordlen);
	}
	return (p+len - buffer);
    }

    while (*p != '\0') {
	len = len_next_part(p);
	if (len == 1 && isspace(*p))
	    break;
	while (--len >= 0) {
	    if (--wordlen > 0)
		*word++ = *p;
	    ++p;
	}
    }

    *word = '\0';
    return (p - buffer);
}


#ifdef _TEST
main()
{
	char buf[1024], word[1024], *bufp;
	int start, len;

	while (gets(buf) != NULL) {

		puts("parsing with front of buffer anchored");
		start = 0;
		while ((len = get_word(buf, start, word, sizeof(word))) > 0) {
			printf("start=%d len=%d word=\"%s\"\n",
				    start, len, word);
			start = len;
		}
		putchar('\n');

		puts("parsing with front of buffer updated");
		bufp = buf;
		while ((len = get_word(bufp, 0, word, sizeof(word))) > 0) {
			printf("start=%d len=%d word=\"%s\"\n",
				    0, len, word);
			bufp += len;
		}
		putchar('\n');

	}

	exit(0);
}
#endif

