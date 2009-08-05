

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: strfcpy.c,v $
 * Revision 1.4  1995/09/29  17:41:39  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/09/11  15:18:59  wfp5p
 * Alpha 7
 *
 * Revision 1.2  1995/05/10  13:34:41  wfp5p
 * Added mailing list stuff by Paul Close <pdc@sgi.com>
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"


/*
 * This is like strncpy() except the result is guaranteed to be '\0' terminated.
 */
char *strfcpy(dest, src, len)
register char *dest;
register const char *src;
register int len;
{
	(void) strncpy(dest, src, len);
	dest[len-1] = '\0';
	return dest;
}


 /*
  * differs from strncat in the following ways:
  *   Takes 'len' as the max size of dest, not the bytes to copy.
  *   Doesn't copy whitespace from front and end of src.
  *   The result is guaranteed to be '\0' terminated.
  *   A comma is appended to dest.
  */
void  strfcat(dest, src, len)
char *dest;
const char *src;
int len;
 {
     len -= 3;
     while (*dest++)
 	len--;
     if (len <= 0)
 	return;
     dest--;
     while (*src == ' ' || *src == '\t')
 	src++;
     while (--len > 0 && *src)
 	*dest++ = *src++;
     dest--;
     while (*dest == ' ' || *dest == '\t' || *dest == '\n' || *dest == ',')
 	dest--;
     *++dest = ',';
     *++dest = ' ';
     *++dest = '\0';
 }

#ifdef _TEST
main()
{
	char src[1024], dest[1024];
	int len;

	for (;;) {
		printf("string > ");
		fflush(stdout);
		if (gets(src) == NULL)
			break;
		printf("maxlen > ");
		fflush(stdout);
		if (gets(dest) == NULL)
			break;
		len = atoi(dest);
		(void) strfcpy(dest, src, len);
		printf("dest=\"%s\" maxlen=%d len=%d\n",
			dest, len, strlen(dest));
		putchar('\n');
	}
	putchar('\n');
	exit(0);
}
#endif

