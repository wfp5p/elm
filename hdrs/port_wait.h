#if (defined(ANSI_C) && !defined(XENIX)) || defined(BSD)
#   define HAVE_SYS_WAIT_H
#endif
#if defined(BSD) && !defined(ANSI_C)
#   define USE_UNION_WAIT
#endif

#ifdef HAVE_SYS_WAIT_H
#   include <sys/wait.h>
#endif

#ifdef USE_UNION_WAIT
    typedef union wait waitstatus_t;
#else
    typedef int waitstatus_t;
#endif

#ifndef WIFEXITED
#   ifdef USE_UNION_WAIT
#       ifndef W_STOPPED
#           ifdef _WSTOPPED
#               define W_STOPPED _WSTOPPED
#           else
#               define W_STOPPED 0177
#           endif
#       endif
#       define WIFEXITED(w)	((w).w_termsig == 0)
#       define WEXITSTATUS(w)	((w).w_retcode)
#       define WIFSIGNALED(w)	((w).w_stopval != W_STOPPED && (w).w_termsig != 0)
#       define WTERMSIG(w)	((w).w_termsig)
#       define WCOREDUMP(w)	((w).w_coredump)
#       define WIFSTOPPED(w)	((w).w_stopval == W_STOPPED)
#       define WSTOPSIG(w)	((w).w_stopsig)
#   else /*!USE_UNION_WAIT*/
#       define WIFEXITED(w)	(((w) & 0xFF) == 0)
#       define WEXITSTATUS(w)	(((w) >> 8) & 0xFF)
#       define WIFSIGNALED(w)	(((w) != 0) && ((w) & 0xFF00) == 0)
#       define WTERMSIG(w)	((w) & 0x7F)
#       define WCOREDUMP(w)	(((w) & 0x80) != 0)
#       define WIFSTOPPED(w)	(((w) & 0xFF) == 0x7F)
#       define WSTOPSIG(w)	(((w) >> 8) & 0xFF)
#   endif /*!USE_UNION_WAIT*/
#endif /*!WIFEXITED*/

