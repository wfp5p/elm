#include "elm_defs.h"
#ifdef PWDINSYS
#  include <sys/pwd.h>
#else
#  include <pwd.h>
#endif


/*
 * The pw_cache[] list is a cache of the PWCACHESIZ most recently used
 * password entries.  The most-recently used entry is at index 0.
 * The least-recently used entry (still in the cache) is at the end.
 *
 * WARNING - The (struct passwd) entries in the list do NOT have all
 * the fields filled in -- just the ones that are known to be used.
 */
#define PWCACHESIZ 16
static struct passwd *pw_cache[PWCACHESIZ];
static int pw_used = 0;

static struct passwd *pwc_setlru P_((int));
static struct passwd *pwc_add P_((struct passwd *));


struct passwd *fast_getpwuid(uid)
int uid;
{
    int i;
    struct passwd *pw;

    for (i = 0 ; i < pw_used ; ++i) {
	if (pw_cache[i]->pw_uid == uid)
	    return pwc_setlru(i);
    }
    if ((pw = getpwuid(uid)) != NULL)
	return pwc_add(pw);
    return (struct passwd *) NULL;
}


static struct passwd *pwc_setlru(i)
int i;
{
    struct passwd *pw;

    for (pw = pw_cache[i] ; i > 0 ; --i)
	pw_cache[i] = pw_cache[i-1];
    pw_cache[0] = pw;
    return pw;
}


static struct passwd *pwc_add(pw)
struct passwd *pw;
{
    int i;

    /* if cache is full toss out last entry */
    if (pw_used >= PWCACHESIZ) {
	--pw_used;
	free((malloc_t)pw_cache[pw_used]->pw_name);
	free((malloc_t)pw_cache[pw_used]);
    }

    /* shift everything down one */
    for (i = pw_used ; i >= 0 ; --i)
	pw_cache[i+1] = pw_cache[i];
    ++pw_used;

    pw_cache[0] = (struct passwd *)safe_malloc(sizeof(struct passwd));
    (void) bzero((char *)pw_cache[0], sizeof(struct passwd));
    pw_cache[0]->pw_name = safe_strdup(pw->pw_name);
    pw_cache[0]->pw_uid = pw->pw_uid;
    pw_cache[0]->pw_gid = pw->pw_gid;

    return pw_cache[0];
}

#ifdef _TEST

main()
{
    char buf[512];
    int uid, i;
    struct passwd *pw;

    for (;;) {

	printf("\ncache size = %d\n", pw_used);
	for (i = 0 ; i < pw_used ; ++i) {
	    printf("%4d  %8.8s  %6d  %6d\n",
			i+1, pw_cache[i]->pw_name,
			pw_cache[i]->pw_uid, pw_cache[i]->pw_gid);
	}

	if (printf("uid> "), fflush(stdout), gets(buf) == NULL)
	    break;

	uid = atoi(buf);
	if ((pw = fast_getpwuid(uid)) != NULL)
	    printf("\nuid %d = %s\n", uid, pw->pw_name);
	else
	    printf("\nuid %d = ?unknown?\n", uid);

    }

    putchar('\n');
    exit(0);
}

#endif

