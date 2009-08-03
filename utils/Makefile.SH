case $CONFIG in
'')	for dir in . .. ../.. ../../.. ; do
		if test -f $dir/config.sh ; then
			. $dir/config.sh
			break
		fi
	done
	case $CONFIG in
	'') (echo "Can't find config.sh."; exit 1) ;;
	esac
	;;
esac
case "$0" in
*/*)	cd `echo "$0" | sed -e 's!/[^/]*$!!' -e "s!^$source_dir!$build_dir!"` ;;
esac

if [ "X$source_dir" = "X$build_dir" ] ; then
	source_dir=..
	VPATH='#VPATH'
else
	VPATH='VPATH'
fi

echo "Extracting utils/Makefile (with variable substitutions)"
cat >Makefile <<'!NO!SUBS!'
# @(#)$Id: Makefile.SH,v 1.8 1996/03/14 17:30:05 wfp5p Exp $
#
#  Makefile for the Elm system utilities
#
#         (C) Copyright 1986,1987, by Dave Taylor
#         (C) Copyright 1988-1992, USENET Community Trust
#
# Bug reports, patches, comments, suggestions should be sent to:
#
#	Syd Weinstein - elm@DSI.COM
#			dsinc!elm
#
# $Log: Makefile.SH,v $
# Revision 1.8  1996/03/14  17:30:05  wfp5p
# Alpha 9
#
# Revision 1.7  1995/09/29  17:42:39  wfp5p
# Alpha 8 (Chip's big changes)
#
# Revision 1.6  1995/09/11  15:19:34  wfp5p
# Alpha 7
#
# Revision 1.5  1995/07/18  19:00:11  wfp5p
# Alpha 6
#
# Revision 1.4  1995/06/30  14:56:29  wfp5p
# Alpha 5
#
# Revision 1.3  1995/06/12  20:33:42  wfp5p
# Alpha 2 clean up
#
# Revision 1.2  1995/04/26  18:49:56  wfp5p
# Changed Configure to allow a different install path
#
# Revision 1.1.1.1  1995/04/19  20:38:40  wfp5p
# Initial import of elm 2.4 PL0 as base for elm 2.5.
#
# Revision 5.6  1994/03/10  17:20:23  syd
# Remove autoreply
# From: Syd
#
# Revision 5.5  1993/05/31  19:17:28  syd
# In the Makefile listalias was given only a+x permission
# though it should have been a+rx.
# From: Jukka Ukkonen <ukkonen@csc.fi>
#
# Revision 5.4  1993/04/16  05:07:10  syd
# add elmalias
#
# Revision 5.3  1993/04/12  02:14:15  syd
# Fix comments
#
# Revision 5.2  1993/04/12  02:11:05  syd
# Added new utilities "elmalias" and "prlong".  Modified "checkalias"
# and "listalias" to use them.
# From: chip@chinacat.unicom.com (Chip Rosenthal)
#
# Revision 5.1  1992/10/04  00:46:45  syd
# Initial checkin as of 2.4 Release at PL0
#

SUBDIR		= utils

#	mmencode installation is optional
INSTALL_MMENCODE = $(DESTBIN)/mmencode
REMOTE_MMENCODE = $(REMOTE)/$(DESTBIN)/mmencode

ALL	=	$(BINDIR)/answer	\
		$(BINDIR)/checkalias	\
		$(BINDIR)/elmalias	\
		$(BINDIR)/fastmail	\
		$(BINDIR)/frm		\
		$(BINDIR)/listalias	\
		$(BINDIR)/mmencode	\
		$(BINDIR)/messages	\
		$(BINDIR)/newalias	\
		$(BINDIR)/newmail	\
		$(BINDIR)/printmail	\
		$(BINDIR)/prlong	\
		$(BINDIR)/readmsg

INSTS	=	$(DESTBIN)/answer	\
		$(DESTBIN)/checkalias	\
		$(DESTBIN)/elmalias	\
		$(DESTBIN)/fastmail	\
		$(DESTBIN)/frm		\
		$(DESTBIN)/listalias	\
		$(DESTBIN)/messages	\
		$(INSTALL_MMENCODE)	\
		$(DESTBIN)/newalias	\
		$(DESTBIN)/newmail	\
		$(DESTBIN)/nfrm		\
		$(DESTBIN)/printmail	\
		$(DESTBIN)/prlong	\
		$(DESTBIN)/readmsg	\
		$(DESTBIN)/wnewmail

SRCS	=	answer.c		\
		elmalias.c		\
		fastmail.c		\
		from.c			\
		mmencode.c		\
		newalias.c		\
		newmail.c		\
		readmsg.c		\
		prlong.c

OBJS	=	answer.o		\
		elmalias.o		\
		fastmail.o		\
		from.o			\
		mmencode.o		\
		newalias.o		\
		newmail.o		\
		readmsg.o		\
		prlong.o

REMOTE_LIST	=	$(REMOTE)$(DESTBIN)/answer	\
			$(REMOTE)$(DESTBIN)/checkalias	\
			$(REMOTE)$(DESTBIN)/elmalias	\
			$(REMOTE)$(DESTBIN)/fastmail	\
			$(REMOTE)$(DESTBIN)/frm		\
			$(REMOTE)$(DESTBIN)/listalias	\
			$(REMOTE)$(DESTBIN)/messages	\
			$(REMOTE_MMENCODE)		\
			$(REMOTE)$(DESTBIN)/newalias	\
			$(REMOTE)$(DESTBIN)/newmail	\
			$(REMOTE)$(DESTBIN)/printmail	\
			$(REMOTE)$(DESTBIN)/prlong	\
			$(REMOTE)$(DESTBIN)/readmsg

LINT_LIST	=	answer_lint	\
			elmalias_lint	\
			fastmail_lint	\
			frm_lint	\
			mmencode_lint	\
			newalias_lint	\
			newmail_lint	\
			readmsg_lint	\
			prlong_lint

!NO!SUBS!

cat ../Make.defs >>Makefile

cat >>Makefile <<'!NO!SUBS!'

#	Lists of source and object files for each C program
ANSWER_SRC	=	answer.c
ANSWER_OBJ	=	answer.o
ELMALIAS_SRC	=	elmalias.c
ELMALIAS_OBJ	=	elmalias.o
FASTMAIL_SRC	=	fastmail.c
FASTMAIL_OBJ	=	fastmail.o
FRM_SRC		=	from.c
FRM_OBJ		=	from.o
MMENCODE_SRC	=	mmencode.c
MMENCODE_OBJ	=	mmencode.o
NEWALIAS_SRC	=	newalias.c
NEWALIAS_OBJ	=	newalias.o
NEWMAIL_SRC	=	newmail.c
NEWMAIL_OBJ	=	newmail.o
PRLONG_SRC	=	prlong.c
PRLONG_OBJ	=	prlong.o
READMSG_SRC	=	readmsg.c
READMSG_OBJ	=	readmsg.o


checkalias:           ../config.sh $(SRCDIR)/$(SUBDIR)/checkalias.SH
	sh $(SRCDIR)/$(SUBDIR)/checkalias.SH

listalias:            ../config.sh $(SRCDIR)/$(SUBDIR)/listalias.SH
	sh $(SRCDIR)/$(SUBDIR)/listalias.SH

messages:             ../config.sh $(SRCDIR)/$(SUBDIR)/messages.SH
	sh $(SRCDIR)/$(SUBDIR)/messages.SH

printmail:            ../config.sh $(SRCDIR)/$(SUBDIR)/printmail.SH
	sh $(SRCDIR)/$(SUBDIR)/printmail.SH


$(BINDIR)/answer:	$(ANSWER_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(ANSWER_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/elmalias:	$(ELMALIAS_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(ELMALIAS_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/fastmail:	$(FASTMAIL_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(FASTMAIL_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/frm:		$(FRM_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(FRM_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/mmencode:	$(MMENCODE_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(MMENCODE_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/newalias:	$(NEWALIAS_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(NEWALIAS_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/newmail:	$(NEWMAIL_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(NEWMAIL_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/readmsg:	$(READMSG_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(READMSG_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/prlong:	$(PRLONG_OBJ) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(PRLONG_OBJ) ../lib/libutil.a $(LIBS)

$(BINDIR)/checkalias:	checkalias	; $(CP) $? $@ ; $(CHMOD) u+w,a+rx $@
$(BINDIR)/listalias:	listalias	; $(CP) $? $@ ; $(CHMOD) u+w,a+rx $@
$(BINDIR)/messages:	messages	; $(CP) $? $@ ; $(CHMOD) u+w,a+rx $@
$(BINDIR)/printmail:	printmail	; $(CP) $? $@ ; $(CHMOD) u+w,a+rx $@

$(DESTBIN)/answer:	$(BINDIR)/answer	; $(INSTALL_EXE)
$(DESTBIN)/checkalias:	$(BINDIR)/checkalias	; $(INSTALL_EXE)
$(DESTBIN)/elmalias:	$(BINDIR)/elmalias	; $(INSTALL_EXE)
$(DESTBIN)/fastmail:	$(BINDIR)/fastmail	; $(INSTALL_EXE)
$(DESTBIN)/frm:		$(BINDIR)/frm		; $(INSTALL_EXE)
$(DESTBIN)/nfrm:	$(DESTBIN)/frm		; $(INSTALL_LINK)
$(DESTBIN)/listalias:	$(BINDIR)/listalias	; $(INSTALL_EXE)
$(DESTBIN)/messages:	$(BINDIR)/messages	; $(INSTALL_EXE)
$(DESTBIN)/mmencode:	$(BINDIR)/mmencode	; $(INSTALL_EXE)
$(DESTBIN)/newalias:	$(BINDIR)/newalias	; $(INSTALL_EXE)
$(DESTBIN)/newmail:	$(BINDIR)/newmail	; $(INSTALL_EXE)
$(DESTBIN)/printmail:	$(BINDIR)/printmail	; $(INSTALL_EXE)
$(DESTBIN)/readmsg:	$(BINDIR)/readmsg	; $(INSTALL_EXE)
$(DESTBIN)/wnewmail:	$(DESTBIN)/newmail	; $(INSTALL_LINK)
$(DESTBIN)/prlong:	$(BINDIR)/prlong	; $(INSTALL_EXE)

#	This is the only target that gets installed even if not out-of-date
#	with respect the files from which it is installed.
rmt-install:		rmt-defined
			-$(MV) $(DESTBIN)/answer $(DESTBIN)/answer.old
			-$(MV) $(DESTBIN)/checkalias $(DESTBIN)/checkalias.old
			-$(MV) $(DESTBIN)/elmalias $(DESTBIN)/elmalias.old
			-$(MV) $(DESTBIN)/fastmail $(DESTBIN)/fastmail.old
			-$(MV) $(DESTBIN)/frm $(DESTBIN)/frm.old
			-$(MV) $(DESTBIN)/nfrm $(DESTBIN)/nfrm.old
			-$(MV) $(DESTBIN)/listalias $(DESTBIN)/listalias.old
			-$(MV) $(DESTBIN)/messages $(DESTBIN)/messages.old
			-test "X$(INSTALL_MMENCODE)" = X || \
			    -$(MV) $(INSTALL_MMENCODE) $(INSTALL_MMENCODE).old
			-$(MV) $(DESTBIN)/newalias $(DESTBIN)/newalias.old
			-$(MV) $(DESTBIN)/newmail $(DESTBIN)/newmail.old
			-$(MV) $(DESTBIN)/printmail $(DESTBIN)/printmail.old
			-$(MV) $(DESTBIN)/prlong $(DESTBIN)/prlong.old
			-$(MV) $(DESTBIN)/readmsg $(DESTBIN)/readmsg.old
			-$(MV) $(DESTBIN)/wnewmail $(DESTBIN)/wnewmail.old
			-$(RM) $(DESTBIN)/answer.old
			-$(RM) $(DESTBIN)/checkalias.old
			-$(RM) $(DESTBIN)/elmalias.old
			-$(RM) $(DESTBIN)/fastmail.old
			-$(RM) $(DESTBIN)/frm.old
			-$(RM) $(DESTBIN)/nfrm.old
			-$(RM) $(DESTBIN)/listalias.old
			-$(RM) $(DESTBIN)/messages.old
			-test "X$(INSTALL_MMENCODE)" = X || \
			    -$(RM) $(INSTALL_MMENCODE)
			-$(RM) $(DESTBIN)/newalias.old
			-$(RM) $(DESTBIN)/newmail.old
			-$(RM) $(DESTBIN)/printmail.old
			-$(RM) $(DESTBIN)/prlong.old
			-$(RM) $(DESTBIN)/readmsg.old
			-$(RM) $(DESTBIN)/wnewmail.old
			$(CP) $(REMOTE_LIST) $(DESTBIN)
			$(LN) $(DESTBIN)/frm $(DESTBIN)/nfrm
			$(LN) $(DESTBIN)/newmail $(DESTBIN)/wnewmail
			$(CHMOD) a+rx $(INSTALL_LIST)

LINT.OUT :
	@( set -x ; \
		$(LINT) $(LINTFLAGS) $(ANSWER_SRC)	; \
		$(LINT) $(LINTFLAGS) $(ELMALIAS_SRC)	; \
		$(LINT) $(LINTFLAGS) $(FASTMAIL_SRC)	; \
		$(LINT) $(LINTFLAGS) $(FRM_SRC)		; \
		$(LINT) $(LINTFLAGS) $(MMENCODE_SRC)	; \
		$(LINT) $(LINTFLAGS) $(NEWALIAS_SRC)	; \
		$(LINT) $(LINTFLAGS) $(NEWMAIL_SRC)	; \
		$(LINT) $(LINTFLAGS) $(READMSG_SRC)	; \
		$(LINT) $(LINTFLAGS) $(PRLONG_SRC)	\
	) >$@

!NO!SUBS!

