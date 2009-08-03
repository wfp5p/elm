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

echo "Extracting src/Makefile (with variable substitutions)"
cat >Makefile <<'!NO!SUBS!'
# $Id: Makefile.SH,v 1.9 1996/03/14 17:27:46 wfp5p Exp $
#
#  Makefile for the ELM mail program.
#
#	Copyright (c) 1988-1992 USENET Community Trust
#	Copyright (c) 1986,1987 Dave Taylor
#
# Bug reports, patches, comments, suggestions should be sent to:
#
#	Syd Weinstein - elm@DSI.COM
#			dsinc!elm
#
# $Log: Makefile.SH,v $
# Revision 1.9  1996/03/14  17:27:46  wfp5p
# Alpha 9
#
# Revision 1.8  1995/09/29  17:41:52  wfp5p
# Alpha 8 (Chip's big changes)
#
# Revision 1.7  1995/09/11  15:19:01  wfp5p
# Alpha 7
#
# Revision 1.6  1995/07/18  18:59:53  wfp5p
# Alpha 6
#
# Revision 1.5  1995/06/30  14:56:24  wfp5p
# Alpha 5
#
# Revision 1.4  1995/06/22  14:48:46  wfp5p
# Performance enhancements from Paul Close
#
# Revision 1.3  1995/05/10  13:34:48  wfp5p
# Added mailing list stuff by Paul Close <pdc@sgi.com>
#
# Revision 1.2  1995/04/26  18:49:54  wfp5p
# Changed Configure to allow a different install path
#
# Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
# Initial import of elm 2.4 PL0 as base for elm 2.5.
#
# Revision 5.1  1992/10/03  22:58:40  syd
# Initial checkin as of 2.4 Release at PL0
#

SUBDIR		= src

ALL = $(BINDIR)/elm

INSTS = $(DESTBIN)/elm

SRCS = addr_util.c alias.c aliaslib.c args.c a_edit.c a_screen.c \
	a_sendmsg.c a_sort.c a_quit.c bouncebk.c builtin.c calendar.c \
	chstatus.c curses.c date.c delete.c edit.c editmsg.c elm.c \
	encode.c fbrowser.c file_ops.c file_util.c fileio.c \
	find_alias.c forms.c hdrconfg.c help.c in_utils.c init.c \
	leavembox.c lock.c limit.c mime.c newmbox.c options.c out_utils.c \
	pattern.c quit.c read_rc.c reply.c returnadd.c save.c save_opts.c \
	savecopy.c screen.c showmsg.c showmsg_c.c signals.c sndattach.c \
	sndhdrs.c sndmsg.c sndpart_io.c sndpart_lib.c softkeys.c sort.c \
	string2.c strings.c syscall.c utils.c wordwrap.c

OBJS = addr_util.o alias.o aliaslib.o args.o a_edit.o a_screen.o \
	a_sendmsg.o a_sort.o a_quit.o bouncebk.o builtin.o calendar.o \
	chstatus.o curses.o date.o delete.o edit.o editmsg.o elm.o \
	encode.o fbrowser.o file_ops.o file_util.o fileio.o \
	find_alias.o forms.o hdrconfg.o help.o in_utils.o init.o \
	leavembox.o lock.o limit.o mime.o newmbox.o options.o out_utils.o \
	pattern.o quit.o read_rc.o reply.o returnadd.o save.o save_opts.o \
	savecopy.o screen.o showmsg.o showmsg_c.o signals.o sndattach.o \
	sndhdrs.o sndmsg.o sndpart_io.o sndpart_lib.o softkeys.o sort.o \
	string2.o strings.o syscall.o utils.o wordwrap.o
!NO!SUBS!

cat ../Make.defs >>Makefile

cat >>Makefile <<'!NO!SUBS!'

$(BINDIR)/elm:	$& $(OBJS) ../lib/libutil.a
	$(CC) $(LFLAGS) -o $@ $(OBJS) ../lib/libutil.a $(LIBS)

$(DESTBIN)/elm:	$(BINDIR)/elm
	$(INST_MAILER)

rmt-install:	rmt-defined
	-$(MV) $(DESTBIN)/elm $(DESTBIN)/elm.old
	-$(RM) -f $(DESTBIN)/elm.old
	$(CP) $(REMOTE)$(DESTBIN)/elm $(DESTBIN)/elm
	$(TEST) $(MAILERMODE) -lt 999 || $(CHGRP) $(MAILGRP) $(DESTBIN)/elm
	$(CHMOD) $(MAILERMODE) $(DESTBIN)/elm

LINT.OUT:
	$(LINT) $(LINTFLAGS) $(SRCS) >$@

!NO!SUBS!

cat $source_dir/src/depend.out >>Makefile

