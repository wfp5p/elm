#include "elm_defs.h"


/*
 * Remove all trailing slashes from a string -- unless it is just a string
 * of one or more slashes, in which case the result will be "/".
 */
char *trim_trailing_slashes(str)
char *str;
{
    char *s;
    int i;

    for (i = strlen(str), s = str+i ; i > 0 && *--s == '/' ; --i)
	    ;
    s[1] = '\0';
    return str;
}


/*
 * Remove all trailing spaces from a string.
 */
char *trim_trailing_spaces(str)
char *str;
{
    char *s;
    int i;

    for (i = strlen(str), s = str+i ; i >= 0 && (--s, isspace(*s)) ; --i)
	    ;
    s[1] = '\0';
    return str;
}


/*
 * Trim any double quotes that enclose a string.
 */
char *trim_quotes(str)
char *str;
{
    int len;
    if (str[0] != '"' || str[0] == '\0')
	return str;
    len = strlen(str);
    if (str[len-1] != '"')
	return str;
    str[len-1] = '\0';
    return str+1;
}

#ifdef _TEST
main()
{
    static char fmt[] = "%22s = |%s|\n";
    char ibuf[512], obuf[512];

    while (fputs("input string : ", stdout), gets(ibuf) != NULL) {
/* I don't want to depend upon ANSI token pasting in this macro */
#define TRY(NAME, PROC) printf(fmt, NAME, PROC(strcpy(obuf, ibuf)))
	TRY("trim_trailing_spaces", trim_trailing_spaces);
	TRY("trim_trailing_slashes", trim_trailing_slashes);
	TRY("trim_quotes", trim_quotes);
	putchar('\n');
    }
    putchar('\n');
    exit(0);
}
#endif

