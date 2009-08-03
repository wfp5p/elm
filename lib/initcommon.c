#include <limits.h>
#include "elm_defs.h"
#include "s_error.h"

#ifdef PWDINSYS
#  include <sys/pwd.h>
#else
#  include <pwd.h>
#endif

#ifndef ANSI_C
extern struct passwd *getpwnam(), *getpwuid();
extern char *getenv();
#endif


/*
 * Initialize global data commonly used throughout the Elm package.
 * Basically ... this means set the user and host information.
 *
 * WARNING!!!  The following items may be overridden by the elmrc file:
 *
 *	- user_fullname
 *	- host_name
 *	- host_domain
 *	- host_fullname
 *
 */

void initialize_common()
{
    char buf[SLEN], *cp;
    struct passwd *pw;

#ifdef I_LOCALE
    setlocale(LC_ALL, "");
#endif
    elm_msg_cat = catopen("elm2.5", 0);

    /*
     * Full_username will get overridden by fullname in elmrc, if defined.
     *
     * For those sites that have various user names with the same user
     * ID, use the passwd entry corresponding to the user name as long 
     * as it matches the user ID.  Otherwise fall back on the entry 
     * associated with the user ID alone.
     */

    /*
     * Obtain user information.
     *
     * These contortions ensures that if there are multiple passwd entries
     * associated with this UID, we will pick up the one associated with
     * the user name.
     */
    if ((cp = getenv("LOGNAME")) == NULL)
	cp = getenv("USER");
    if (cp && (pw = getpwnam(cp)) != NULL && pw->pw_uid == geteuid()) {
	;  /* ok! */
    } else if ((pw = getpwuid(geteuid())) == NULL) {
	fputs(catgets(elm_msg_cat, ErrorSet, ErrorInitCommonNoPasswordEntry,
		    "You have no password entry!\n"), stderr);
	exit(1);
    }
    user_name = safe_strdup(pw->pw_name);
    user_home = safe_strdup(pw->pw_dir);
    if ((cp = get_full_name(user_name)) != NULL)
	strncpy(user_fullname, cp,SLEN);
    else
	strncpy(user_fullname, user_name,SLEN);

    /*
     * Get the host and domain names.
     */
    get_hostname(host_name, sizeof(host_name));
    get_hostdomain(host_domain, sizeof(host_domain));
    (void) strcat(strcpy(host_fullname, host_name), host_domain);

    /*
     * Determine the default mail file name.
     */
    if ((cp = getenv("MAIL")) == NULL)
	cp = strcat(strcpy(buf, mailhome), user_name);
   
#ifdef PATH_MAX /* 7! die here instead! */
    if ( strlen(cp) > PATH_MAX ) cp[PATH_MAX]='\0';
#endif    
  
    incoming_folder = safe_strdup(cp);

}


#ifdef _TEST
main()
{
    initialize_common();
#define SHOWSTR(name, val)	printf("%s = \"%s\"\n", (name), (val))
    SHOWSTR("user_name",	user_name);
    SHOWSTR("user_home",	user_home);
    SHOWSTR("user_fullname",	user_fullname);
    SHOWSTR("host_name",	host_name);
    SHOWSTR("host_domain",	host_domain);
    SHOWSTR("host_fullname",	host_fullname);
    SHOWSTR("incoming_folder",	incoming_folder);
    exit(0);
}
#endif

