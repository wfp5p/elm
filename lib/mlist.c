
/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $
 *
 * This file and all associated files and documentation:
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: mlist.c,v $
 * Revision 1.3  1996/03/14  17:27:41  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:20  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1  1995/09/11  15:18:55  wfp5p
 * Alpha 7
 *
 * Revision 1.5  1995/06/21  15:27:09  wfp5p
 * editflush and confirmtagsave are new in the elmrc (Keith Neufeld)
 * The mlist code has a little bug fix.
 *
 * Revision 1.4  1995/06/15  13:09:32  wfp5p
 * Changed so the local mlist files adds to the global one instead of
 * overriding it. (Paul Close <pdc@sgi.com>)
 *
 * Revision 1.3  1995/06/14  19:58:26  wfp5p
 * Changes for alpha 3 -- K&R-ized it.
 *
 *
 ******************************************************************************/


#include "elm_defs.h"

struct addrs patterns;
struct addrs mlnames;

static void mlist_read();

void
mlist_init()
{
    char buffer[SLEN];

    patterns.len = patterns.max = 0; patterns.str = NULL;
    mlnames.len = mlnames.max = 0; mlnames.str = NULL;

    /*
     * Load home mlistfile first, as we take the first match we can find,
     * which means that the user file will override the system file.
     */
    sprintf(buffer, "%s/%s", user_home, mlistfile); 
    mlist_read(buffer);
    sprintf(buffer, "%s", system_mlist_file);   
    mlist_read(buffer);
}

static void
mlist_read(filename)
char *filename;
{
    FILE *ml;
    char buffer[SLEN];
    char addr[STRING], name[STRING];
    char *next;
    int lineno = 0;
    int errors = 0;

    ml = fopen(filename, "r");
    if (ml == NULL)
	return;

    while (fgets(buffer, sizeof(buffer), ml) != NULL) {
	lineno++;
	if (*buffer == '\n' || *buffer == '#')
	    continue;

	/* mlist addr */
	*name = '\0';
	if (parse_arpa_mailbox(buffer, addr, sizeof(addr), name, sizeof(name),
			       &next) < 0 || *addr == '\0') {
	    fprintf(stderr, "Error parsing %s, line %d\n", filename, lineno);
	    errors++;
	    continue;
	}
	mlist_push(&patterns, addr);

	if (*name != '\0')
	    mlist_push(&mlnames, name);
	else {
		/* no name?  use the address */
		mlist_push(&mlnames, patterns.str[patterns.len - 1]);
	}
	if (*next != '\0') {
	    fprintf(stderr, "Warning, extra text in %s, line %d: %s",
		filename, lineno, next);
	    errors++;
	}
    }
    if (errors)
	sleep(2);

    fclose(ml);
}

void
parseaddrs(p, array, append)
    char *p; 
    struct addrs *array;
    int append;
{
    char addr[STRING];
    char *buf, *next;

    if (!append)
	freeaddrs(array);

    if (p == NULL)
	return;

    buf = p;
    while (*buf != '\0') {
	if (parse_arpa_mailbox(buf, addr, sizeof(addr), NULL, 0, &next) >= 0 &&
	      *addr != '\0') {
	    /*printf("comment stripped string = %s\n", addr);*/
	    mlist_push(array, addr);
	}
	buf = next;
    }
}


void
freeaddrs(array)
    struct addrs *array;
{
    int i;
    for (i=0; i < array->len; i++) {
	if (array->str[i] != NULL)
	    free(array->str[i]);
    }
    if (array->str)
	free(array->str);
    array->len = array->max = 0;
    array->str = NULL;
}

void
mlist_push(arr, str)
    struct addrs *arr;
    char *str;
{
    int index = arr->len;
    int newlen = index+1;

    if (str == NULL)
      return;

    if (newlen > arr->max) {
	int newmax = (newlen>2*arr->max)? newlen+1: 2*arr->max;
	/* printf("growing to %d elements\n", newmax); */
	if (arr->str == (char **)0)
	  arr->str = (char **)malloc(newmax*sizeof(char*));
	else
	  arr->str = (char **)realloc(arr->str, newmax*sizeof(char*));
	arr->max = newmax;
    }
    if (newlen > arr->len)
	arr->len = newlen;
    arr->str[index] = safe_strdup(str);
}

void
mlist_parse_header_rec(entry)
struct header_rec *entry;
{
    if (entry->ml_to.str == NULL) {
	if (entry->cc_index >= 0) {
	    char savechar = entry->allto[entry->cc_index];
	    entry->allto[entry->cc_index] = '\0';
	    parseaddrs(entry->allto, &entry->ml_to, FALSE);
	    entry->allto[entry->cc_index] = savechar;
	    entry->ml_cc_index = entry->ml_to.len;
	    parseaddrs(entry->allto+entry->cc_index, &entry->ml_to, TRUE);
	}
	else {
	    parseaddrs(entry->allto, &entry->ml_to, FALSE);
	    entry->ml_cc_index = entry->ml_to.len;
	}
    }
}

int
mlist_match_user(entry)
struct header_rec *entry;
{
    int match;
    for (match = 0; match < entry->ml_to.len; match++) {
	if (!okay_address(entry->ml_to.str[match], NULL)) {
	    return match;
	}
    }
    return -1;
}

int
mlist_match_address(entry, string)
struct header_rec *entry;
char *string;
{
    int i, match;
    char addr[STRING];

    if (parse_arpa_mailbox(string, addr, sizeof(addr), NULL, 0, NULL) < 0 ||
	  *addr == '\0')
	return -1;

    match = -1;

    for (i = 0; i < entry->ml_to.len; i++) {
	if (istrcmp(entry->ml_to.str[i], addr) == 0) {
	    match = i;
	    break;
	}
    }

    return match;
}

/*
 * case-insensitive searcher
 */
int
addrmatch(addr, pattern)
  struct addrs *addr;
  struct addrs *pattern;
{
    int i, j;
    static int initialized = 0;
    static char lower[256];

    if (!initialized) {
	for (i=0; i < 256; i++)
	    lower[i] = tolower(i);
	initialized = 1;
    }

    for (i=0; i < pattern->len; i++) {
	int pattlen;
	if (pattern->str[i] == NULL)
	    continue;
	pattlen = strlen(pattern->str[i]);
	for(j=0; j < addr->len; j++) {
	    register char *s1, *s2;
	    char *addrsearch = addr->str[j];
	    while (addrsearch != NULL) {
		s1 = pattern->str[i];
		s2 = addrsearch;
		while (*s1 && lower[*s1] == lower[*s2])
		    s1++, s2++;
		if (*s1 == '\0' &&
		      (*s2 == '\0' || *s2 == '@' || *s2 == '%' || *s2 == '!'))
		    return i;	/* found one */
		if ((addrsearch=strpbrk(addrsearch, "%@!:")) == NULL)
		    break;
		addrsearch++;
	    }
	}
    }
    return -1;
}
