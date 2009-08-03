#ifdef TERMIOS
# include <termios.h>
#else
# ifdef TERMIO
#  include <termio.h>
# else
#  include <sgtty.h>
#  ifndef TIOCFLUSH
#   include <sys/ioctl.h>
#  endif
#  ifndef FREAD
#   include <sys/file.h>
#  endif
# endif
#endif

#ifdef PTEM
# include <sys/stream.h>
# include <sys/ptem.h>
#endif

#ifndef TIOCGWINSZ
# include <sys/ioctl.h>
#endif

#ifndef TERMIOS
# ifdef TERMIO
#  define termios termio
#  define tcgetattr(fd, tty_p)		ioctl((fd), TCGETA, (tty_p))
#  define tcsetattr(fd, opt, tty_p)	ioctl((fd), (opt), (tty_p))
#  define  TCSANOW	TCSETA
#  define  TCSADRAIN	TCSETAW
#  define  TCSAFLUSH	TCSETAF
#  define tcflush(fd, mode)		ioctl((fd), TCFLSH, (mode))
#  define  TCIFLUSH	0
#  define  TCOFLUSH	1
#  define  TCIOFLUSH	2
# else
#  define termios sgttyb
#  define tcgetattr(fd, tty_p)		ioctl((fd), TIOCGETP, (tty_p))
#  define tcsetattr(fd, opt, tty_p)	ioctl((fd), (opt), (tty_p))
#  define  TCSANOW	TIOCSETN
#  define  TCSADRAIN	TIOCSETN
#  define  TCSAFLUSH	TIOCSETP
#  define ttflush(fd, mode)		ioctl((fd), TIOCFLUSH, (mode))
#  define  TCIFLUSH	FREAD
#  define  TCOFLUSH	FWRITE
#  define  TCIOFLUSH	(FREAD|FWRITE)
# endif /*TERMIO*/
#endif /*TERMIOS*/

