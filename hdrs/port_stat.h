#include <sys/stat.h>

#ifndef S_ISREG
# define S_ISREG(m) 	(((m)&S_IFMT) == S_IFREG)
# define S_ISBLK(m) 	(((m)&S_IFMT) == S_IFBLK)
# define S_ISDIR(m) 	(((m)&S_IFMT) == S_IFDIR)
# define S_ISCHR(m) 	(((m)&S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISLNK
# ifdef S_IFLNK
#  define S_ISLNK(m)	(((m)&S_IFMT) == S_IFLNK)
# else
#  define S_ISLNK(m)	(FALSE)
# endif
#endif

#ifdef notdef

	/*
	 * The following are not used by the Elm package so we
	 * won't bother chewing up the macro space with them.
	 */

#ifndef S_ISNAM
# ifdef S_IFNAM
#  define S_ISNAM(m)	(((m)&S_IFMT) == S_IFNAM)
# else
#  define S_ISNAM(m)	(FALSE)
# endif
#endif

#ifndef S_ISSOCK
# ifdef S_IFSOCK
#  define S_ISSOCK(m)	(((m)&S_IFMT) == S_IFSOCK)
# else
#  define S_ISSOCK(m)	(FALSE)
# endif
#endif

#ifndef S_ISFIFO
# ifdef S_IFIFO		/* BSD4.4 considers S_IFSOCK to be a FIFO too */
#    define S_ISFIFO(m)	(((m)&S_IFMT) == S_IFIFO || S_ISSOCK(m))
#  else
#    define S_ISFIFO(m)	(FALSE)
# endif
#endif

#endif /*notdef*/

