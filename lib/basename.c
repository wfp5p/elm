#include "elm_defs.h"

char *basename(pname)
const char *pname;
{
    const char *s;
    return (char *) ((s = strrchr(pname, '/')) != NULL ? s+1 : pname);
}

#ifdef _TEST
main()
{
	char buf[512];
	puts("Enter pathnames...\n");
	while (gets(buf) != NULL) {
		puts(basename(buf));
		putchar('\n');
	}
	putchar('\n');
	exit(1);
}
#endif
