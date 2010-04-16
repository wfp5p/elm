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

echo "Extracting lib/Makefile (with variable substitutions)"
cat >Makefile <<'!NO!SUBS!'
# $Id: Makefile.SH,v 1.7 1996/03/14 17:27:35 wfp5p Exp $
#
#  Makefile for the ELM mail program.
#
#	Copyright (c) 1986,1987 Dave Taylor
#	Copyright (c) 1988-1992 USENET Community Trust
#
# Bug reports, patches, comments, suggestions should be sent to:
#
#	Syd Weinstein - elm@DSI.COM
#			dsinc!elm
#
# $Log: Makefile.SH,v $
# Revision 1.7  1996/03/14  17:27:35  wfp5p
# Alpha 9
#
# Revision 1.6  1995/09/29  17:41:01  wfp5p
# Alpha 8 (Chip's big changes)
#
# Revision 1.5  1995/09/11  15:18:51  wfp5p
# Alpha 7
#
# Revision 1.4  1995/07/18  18:59:49  wfp5p
# Alpha 6
#
# Revision 1.3  1995/06/30  14:56:23  wfp5p
# Alpha 5
#
# Revision 1.2  1995/06/22  14:48:34  wfp5p
# Performance enhancements from Paul Close
#
# Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
# Initial import of elm 2.4 PL0 as base for elm 2.5.
#
# Revision 5.15  1993/08/03  19:28:39  syd
# Elm tries to replace the system toupper() and tolower() on current
# BSD systems, which is unnecessary.  Even worse, the replacements
# collide during linking with routines in isctype.o.  This patch adds
# a Configure test to determine whether replacements are really needed
# (BROKE_CTYPE definition).  The <ctype.h> header file is now included
# globally through hdrs/defs.h and the BROKE_CTYPE patchup is handled
# there.  Inclusion of <ctype.h> was removed from *all* the individual
# files, and the toupper() and tolower() routines in lib/opt_utils.c
# were dropped.
# From: chip@chinacat.unicom.com (Chip Rosenthal)
#
# Revision 5.14  1993/08/03  19:17:33  syd
# Implement new timezone handling.  New file lib/get_tz.c with new timezone
# routines.  Added new TZMINS_USE_xxxxxx and TZNAME_USE_xxxxxx configuration
# definitions.  Obsoleted TZNAME, ALTCHECK, and TZ_MINUTESWEST configuration
# definitions.  Updated Configure.  Modified lib/getarpdate.c and
# lib/strftime.c to use new timezone routines.
#
# From: chip@chinacat.unicom.com (Chip Rosenthal)
#
# Revision 5.13  1993/08/02  18:14:35  syd
# add missing references for mcprt.c/mcprtlib.c to defs.h
# From: Syd
#
# Revision 5.12  1993/06/10  03:02:46  syd
# break_down_tolist() tried to blindly split address lists at "," which
# caused bogus results with addreses that had a comma inside a comment
# or quoted text, such as "user@domain (Last, First)".  This patch steps
# through the address in quanta of RFC-822 tokens when searching for a
# delimiting comma.  It also adds "rfc822_toklen()" to the library to
# get that length.
# From: chip@chinacat.unicom.com (Chip Rosenthal)
#
# Revision 5.11  1993/04/12  01:51:42  syd
# Added safe_malloc(), safe_realloc(), and safe_strdup().  They
# will be used in the new elmalias utility.
# From: chip@chinacat.unicom.com (Chip Rosenthal)
#
# Revision 5.10  1993/02/03  16:21:03  syd
# add strtokq
# From: Syd
#
# Revision 5.9  1993/01/27  18:52:03  syd
# add strftime as optional component
#
# Revision 5.8  1993/01/19  05:07:05  syd
# Trim erroreous extra log entry
# From: Syd
#
# Revision 5.7  1993/01/19  04:47:12  syd
# Significant changes to provide consistent Date and From_ header
# cracking.  Overhauled date utilities and moved into library.  Moved
# real_from() into library.  Modified frm, newmail, and readmsg utilities
# to use library version of real_from().  Moved get_word() from Elm
# source into library.  Added new library routines atonum() and strfcpy().
# Fixed trailing backslash bug in len_next().
# From: chip@chinacat.unicom.com (Chip Rosenthal)
#
# Revision 5.6  1992/12/24  19:33:59  syd
# tune dependency list further
# From: Syd
#
# Revision 5.5  1992/12/24  19:30:46  syd
# Make all object depend on config.h, so a reconfig for a new
# arch in shared sources (or copied sources) forces a recompile
# of all modules.
# From: Syd
#
# Revision 5.4  1992/12/20  05:15:58  syd
# Add a c)hange alias, -u and -t options to listalias to list only user
# and only system aliases respectively.
# From: "Robert L. Howard" <robert.howard@matd.gatech.edu>
#
# Revision 5.3  1992/11/07  20:07:23  syd
# Add header_cmp routine
# From: Syd
#
# Revision 5.2  1992/10/27  01:43:40  syd
# Move posix_signal to lib directory
# From: tom@osf.org
#
# Revision 5.1  1992/10/03  22:41:36  syd
# Initial checkin as of 2.4 Release at PL0
#

SUBDIR = lib

ALL = libutil.a

INSTS =

SRCS = add_site.c addrmchusr.c aliasdb.c atonum.c basename.c can_access.c \
	can_open.c chloc.c date_util.c elm_access.c elmrc.c expand.c \
	fast_getpw.c figadrssee.c gcos_name.c get_tz.c getarpdate.c \
	getfullnam.c gethostname.c getword.c header_cmp.c in_list.c \
	initcommon.c ldstate.c len_next.c $(MCSRC) mail_gets.c mailfile.c \
	mk_aliases.c mk_lockname.c mlist.c move_left.c ndbz.c okay_addr.c \
	opt_utils.c parsarpdat.c parsarpmbox.c parsarpwho.c patmatch.c \
	posixsig.c qstrings.c realfrom.c remfirstwd.c reverse.c \
	rfc822tlen.c safemalloc.c shiftlower.c strfcpy.c strftime.c \
	strincmp.c striparens.c strstr.c strtokq.c tail_of.c trim.c \
	validname.c

OBJS = add_site.o addrmchusr.o aliasdb.o atonum.o basename.o can_access.o \
	can_open.o chloc.o date_util.o elm_access.o elmrc.o expand.o \
	fast_getpw.o figadrssee.o gcos_name.o get_tz.o getarpdate.o \
	getfullnam.o gethostname.o getword.o header_cmp.o in_list.o \
	initcommon.o ldstate.o len_next.o mail_gets.o mailfile.o \
	$(MCOBJ) mk_aliases.o mk_lockname.o mlist.o move_left.o ndbz.o \
	okay_addr.o opt_utils.o parsarpdat.o parsarpmbox.o parsarpwho.o \
	patmatch.o posixsig.o qstrings.o realfrom.o remfirstwd.o \
	reverse.o rfc822tlen.o safemalloc.o shiftlower.o strfcpy.o \
	strftime.o strincmp.o striparens.o strstr.o strtokq.o tail_of.o \
	trim.o validname.o
!NO!SUBS!

cat ../Make.defs >>Makefile

cat >>Makefile <<'!NO!SUBS!'

libutil.a: $(OBJS)
	$(AR) r $@ $?
	$(RANLIB) $@

#
# Some of the library modules have test code enabled by the _TEST definition.
# The following target can be used to build test cases.  For instance, to
# test the "foobar.c" file, run "MOD=foobar make test".
#
test: $(SRCDIR)/$(SUBDIR)/$(MOD).c
	$(CC) -D_TEST -DINTERN $(CFLAGS) $? libutil.a

#
# Catch "make test" without $(MOD) defined.
#
$(SRCDIR)/$(SUBDIR)/.c:
	@echo "Error - usage: MOD=module_to_test $(MAKE) $(MFLAGS) test"
	@false

LINT.OUT:
	$(LINT) $(LINTFLAGS) $(SRCS) >$@

!NO!SUBS!

cat $source_dir/lib/depend.out >>Makefile

