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


case "$d_usenls" in
define)	mcsrc="mcprt.c mcprtlib.c msgcat.c"
	mcobj="mcprt.o mcprtlib.o msgcat.o"
	;;
*)	mcsrc=
	mcobj=
	;;
esac

./ELM-VERSION-GEN
VERSION=$(sed -e 's/^ELM_VERSION = //' < ELM-VERSION-FILE)


echo "Extracting Make.defs (with variable substitutions)"
cat >Make.defs <<!GROK!THIS!

##############################################################################
#
# Make.defs - common definitions
#
# DO NOT EDIT THE RESULTING Makefile !!!
# Any changes made will be lost if this makefile is rebuilt.
#
# The Elm makefiles support the following standard actions:
#
#	make all - This is the default.  Build the targets.
#
#	make install - Install targets into system directories.
#
#	make clean - Delete targets, objects, and intermediate files.
#
#	make uninstall - Delete installed targets from system directories
#
#	make lint - Run "lint".
#
#	make setup - Ensure directories and .SH files up to date.
#
#	make tags - Create "tags" file for the sources.
#
# The customized portion of the Makefile must provide the following defs:
#
#	SUBDIR - This directory relative to SRCDIR, e.g. "lib".
#
#	ALL - List of targets to build for "make all"  These also
#		are deleted by the "make clean" rule.
#
#	INSTS - List of targets to install for "make install".
#
#	SRCS - List of source files that comprise ALL targets.  These
#		are all scanned when building automatic dependancies.
#
#	OBJS - List of intermediate object files for ALL targets.
#
# The customized portion also must provide the following rules:
#
#	Rules to build each item in the ALL list.
#
#	Rules to install each item in the INSTS list.
#
#	A "rmt-install" rule that does remote installation.
#
#	A "LINT.OUT" rule that runs lint on the SRCS files.
#

SHELL		= /bin/sh

SRCDIR		= $source_dir
$VPATH		= \$(SRCDIR)/\$(SUBDIR)

BINDIR		= ../bin
INCLDIR		= \$(SRCDIR)/hdrs

CC		= $cc
OPTIMIZE	= $optimize
#DEBUG		= -g -DDEBUG
CCFLAGS		= $ccflags $xencf
INCLS		= -I.. -I\$(INCLDIR)
CFLAGS		= \$(CCFLAGS) \$(OPTIMIZE) \$(DEBUG) \$(INCLS) -DVERSION='"$VERSION"'
LFLAGS		= $ldflags $xenlf
LIBS		= $libs $cryptlib $termlib $dbm
LINTFLAGS	= \$(INCLS)

AR		= ar
CHGRP		= $chgrp
CHMOD		= $chmod
CP		= $cp
CTAGS		= ctags
ECHO		= $echo
LINT		= $lint
LN		= $ln
MV		= $mv
RANLIB		= $ranlib
RM		= $rm
TEST		= $test
TOUCH		= $touch

DESTBIN		= $installbin
DESTLIB		= $installlib
MAILGRP		= $mailgrp
MAILERMODE	= $mailermode

MCSRC		= $mcsrc
MCOBJ		= $mcobj

!GROK!THIS!

cat >>Make.defs <<'!NO!SUBS!'

INSTALL_EXE = \
	$(RM) -f $@.old ; \
	$(TEST) ! -f $@ || $(MV) $@ $@.old ; \
	$(RM) -f $@.old ; \
	$(CP) $? $@ ; \
	$(CHMOD) a+x $@

INSTALL_LINK = \
	$(RM) -f $@.old ; \
	$(TEST) ! -f $@ || $(MV) $@ $@.old ; \
	$(RM) -f $@.old ; \
	$(LN) $? $@

INST_MAILER = \
	$(RM) -f $@.old ; \
	$(TEST) ! -f $@ || $(MV) $@ $@.old ; \
	$(RM) -f $@.old ; \
	$(CP) $? $@ ; \
	$(TEST) $(MAILERMODE) -lt 999 || $(CHGRP) $(MAILGRP) $@ ; \
	$(CHMOD) $(MAILERMODE) $@

#
# Common rules
#

all:		Makefile $& $(ALL)

clean:		; $(RM) -f $(ALL) *.o a.out core core.* tags LINT.OUT

install:	all $(INSTS)

uninstall:	; $(RM) -f $(INSTS)

lint:		LINT.OUT

setup:		Makefile

depend:		$(SRCDIR)/$(SUBDIR)/depend.out

rmt-defined:
	@if $(TEST) "X$(REMOTE)" = "X" ; then \
	    $(ECHO) "You need to define 'REMOTE' as the remote file system" ; \
	    $(ECHO) "for this particular command.  The easiest way to do" ; \
	    $(ECHO) "this is to type:";\
	    $(ECHO) "        make REMOTE=<remote file system> rmt-install" ; \
	    false ; \
	fi

tags: $(SRCDIR)/$(SUBDIR)/*.[ch] $(SRCDIR)/lib/*.[ch] $(SRCDIR)/hdrs/*.[ch]
	@set -x ; \
		cd $(SRCDIR)/$(SUBDIR) ; \
		rm -f tags ; \
		if [ "X$(SUBDIR)" = "Xlib" ] ;  \
			then $(CTAGS) ../lib/*.[ch] ../hdrs/*.[ch] ; \
			else $(CTAGS) *.[ch] ../lib/*.[ch] ../hdrs/*.[ch] ; \
		fi

#
# "depend.out" is a Makefile dependancy, but we omit it to avoid rebuilding
# everything whenever a source file changes
#
Makefile: ../config.sh $(SRCDIR)/$(SUBDIR)/Makefile.SH ../Make.defs
	sh $(SRCDIR)/$(SUBDIR)/Makefile.SH
	@echo "Makefile has been rebuilt - please restart make"
	@false

$(SRCDIR)/$(SUBDIR)/depend.out: $(SRCS)
	cd $(SRCDIR)/$(SUBDIR) ; sh ../dev/makedeps.sh $(SRCS) >$@

#
# end of Make.defs
#
##############################################################################

!NO!SUBS!

