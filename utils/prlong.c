

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $
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
 * $Log: prlong.c,v $
 * Revision 1.4  1996/03/14  17:30:11  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:47  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/04/20  21:02:09  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:41  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/


/*
 * Format large amounts of output, folding across multiple lines.
 *
 * Input is read in a line at a time, and the input records are joined
 * together into output records up to a maximum line width.  A field
 * seperator is placed between every input record, and a configurable
 * leader is printed at the front of each line.  The default field
 * seperator is a single space.  The default output leader is an empty
 * string for the first output line, and a single tab for all subsequent
 * lines.
 *
 * Usage:
 *
 *	prlong [-w wid] [-1 first_leader] [-l leader] [-f sep] < data
 *
 * Options:
 *
 *	-w wid		Maximum output line width.
 *	-1 leader	Leader for the first line of output.
 *	-l leader	Leader for all subsequent lines of output.
 *	-f sep		Field seperator.
 *
 * Example:
 *
 *    $ elmalias -en friends | /usr/lib/elm/prlong -w40
 *    tom@sleepy.acme.com (Tom Smith) 
 *	    dick@dopey.acme.com (Dick Jones) 
 *	    harry@grumpy.acme.com
 *    $ elmalias -en friends | /usr/lib/elm/prlong -w40 -1 "To: " -f ", "
 *    To: tom@sleepy.acme.com (Tom Smith), 
 *	    dick@dopey.acme.com (Dick Jones), 
 *	    harry@grumpy.acme.com
 */


#include "elm_defs.h"

#define MAXWID		78	/* default maximum line width		*/
#define ONE_LDR		""	/* default leader for first line	*/
#define DFLT_LDR	"\t"	/* default leader for other lines	*/
#define FLDSEP		" "	/* default field seperator		*/

char inbuf[1024];		/* space to hold an input record	*/
char outbuf[4096];		/* space to accumulate output record	*/

int calc_col();			/* calculate output column position	*/

void usage_error(char *prog)
{
    fprintf(stderr,
	"usage: %s [-w wid] [-1 first_leader] [-l leader] [-f sep]\n", prog);
    exit(1);
}

int main(int argc, char *argv[])
{
    char *one_ldr;		/* output leader to use on first line	*/
    char *dflt_ldr;		/* output leader for subsequent lines	*/
    char *fld_sep;		/* selected field seperator		*/
    char *curr_sep;		/* text to output before next field	*/
    int maxwid;			/* maximum output line width		*/
    int outb_col;		/* current column pos in output rec	*/
    int outb_len;		/* current length of output record	*/
    int i;
    extern int optind;
    extern char *optarg;

    /*
     * Initialize defaults.
     */
    maxwid = MAXWID;
    one_ldr = ONE_LDR;
    dflt_ldr = DFLT_LDR;
    fld_sep = FLDSEP;

    /*
     * Crack command line.
     */
    while ((i = getopt(argc, argv, "w:1:l:f:")) != EOF) {
	switch (i) {
	    case 'w':	maxwid = atoi(optarg);	break;
	    case '1':	one_ldr = optarg;	break;
	    case 'l':	dflt_ldr = optarg;	break;
	    case 'f':	fld_sep = optarg;	break;
	    default:	usage_error(argv[0]);
	}
    }
    if (optind != argc)
	usage_error(argv[0]);

    /*
     * Initialize output buffer.
     */
    (void) strfcpy(outbuf, one_ldr, sizeof(outbuf));
    outb_col = calc_col(0, one_ldr);
    outb_len = strlen(one_ldr);
    curr_sep = "";

    /*
     * Process the input a line at a time.
     */
    while (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {

	/*
	 * Trim trailing space.  Skip blank lines.
	 */
	for (i = strlen(inbuf) - 1 ; i >= 0 && isspace(inbuf[i]) ; --i)
		;
	inbuf[i+1] = '\0';
	if (inbuf[0] == '\0')
		continue;

	/*
	 * If this text exceeds the line length then print the stored
	 * info and reset the line.
	 */
	if (calc_col(calc_col(outb_col, curr_sep), inbuf) >= maxwid) {
	    printf("%s%s\n", outbuf, curr_sep);
	    curr_sep = dflt_ldr;
	    outb_col = 0;
	    outb_len = 0;
	    outbuf[0] = '\0';
	}

	/*
	 * Append the current field seperator to the stored info.
	 */
	(void) strfcpy(outbuf+outb_len, curr_sep, sizeof(outbuf)-outb_len);
	outb_col = calc_col(outb_col, outbuf+outb_len);
	outb_len += strlen(outbuf+outb_len);

	/*
	 * Append the text to the stored info.
	 */
	(void) strfcpy(outbuf+outb_len, inbuf, sizeof(outbuf)-outb_len);
	outb_col = calc_col(outb_col, outbuf+outb_len);
	outb_len += strlen(outbuf+outb_len);

	/*
	 * Enable the field seperator.
	 */
	curr_sep = fld_sep;

    }

    if (*outbuf != '\0')
	puts(outbuf);
    exit(0);
}

int calc_col(register int col, register char *s)
{
    while (*s != '\0') {
	switch (*s) {
	    case '\b':	--col;			break;
	    case '\r':	col = 0;		break;
	    case '\t':	col = ((col + 8) & ~7);	break;
	    default:	++col;			break;
	}
	++s;
    }
    return col;
}

