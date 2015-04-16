#include "elm_defs.h"

/* return local host name (with any domain elided) */
void get_hostname(char *retval, int retsiz)
{
    char *s;

    retval[0] = '\0';

    /* warning - some systems return FQDN */
    if (retval[0] == '\0')
	(void) gethostname(retval, retsiz-1);

#if (defined(XENIX) || defined(M_UNIX))
    if (retval[0] == '\0') {
	FILE *fp;
	if ((fp = fopen("/etc/systemid", "r")) != NULL) {
	    if (fgets(retval, retsiz-1, fp) != NULL) {
		if ((s = strchr(retval, '\n')) != NULL)
		    *s = '\0';
	    }
	    (void) fclose(fp);
	}
    }
#endif

    if (retval[0] == '\0')
	(void) strfcpy(retval, HOSTNAME, retsiz);

    if ((s = strchr(retval, '.')) != NULL)
	*s = '\0';
}


/* return local domain (with leading dot) */
void get_hostdomain(char *retval, int retsiz)
{
    char buf[SLEN], *s;
    FILE *fp;

    buf[0] = '\0';

    if ((fp = fopen(system_hostdom_file, "r")) != NULL) {
	if (fgets(buf, sizeof(buf), fp) != NULL) {
	    if ((s = strchr(buf, '\n')) != NULL)
		*s = '\0';
	}
	(void) fclose(fp);
    }

    if (buf[0] == '\0')
	(void) getdomainname(buf, sizeof(buf));

    if (buf[0] == '\0') {
	char fqdn[SLEN];
	(void) gethostname(fqdn, sizeof(fqdn));
	if ((s = strchr(fqdn, '.')) != NULL)
	    (void) strfcpy(buf, s, sizeof(buf));
    }

    if (buf[0] == '\0')
	(void) strfcpy(buf, DEFAULT_DOMAIN, sizeof(buf));

    if (buf[0] != '.') {
	*retval++ = '.';
	--retsiz;
    }
    (void) strfcpy(retval, buf, retsiz);
}


#ifdef _TEST
main()
{
    char buf[SLEN];
    get_hostname(buf, sizeof(buf));
    printf("get_hostname() returns \"%s\"\n", buf);
    get_hostdomain(buf, sizeof(buf));
    printf("get_hostdomain() returns \"%s\"\n", buf);
    exit(0);
}
#endif

