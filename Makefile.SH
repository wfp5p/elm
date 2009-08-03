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
	source_dir=.
	VPATH='#VPATH'
else
	VPATH='VPATH'
fi

echo "Extracting Makefile (with variable substitutions)"
cat >Makefile <<!GROK!THIS!
#
# $Id: Makefile.SH,v 1.8 1996/03/14 17:27:09 wfp5p Exp $
#
#  Makefile for the entire ELM mail system
#
#         (C) Copyright 1986,1987, by Dave Taylor
#         (C) Copyright 1988-1995, USENET Community Trust
#
#  $Log: Makefile.SH,v $
# Revision 1.8  1996/03/14  17:27:09  wfp5p
# Alpha 9
#
# Revision 1.7  1995/09/29  17:40:28  wfp5p
# Alpha 8 (Chip's big changes)
#
# Revision 1.6  1995/09/11  15:18:35  wfp5p
# Alpha 7
#
# Revision 1.5  1995/07/18  18:59:31  wfp5p
# Alpha 6
#
# Revision 1.4  1995/06/30  14:55:57  wfp5p
# Alpha 5
#
# Revision 1.3  1995/05/10  13:33:48  wfp5p
# *** empty log message ***
#
# Revision 1.2  1995/04/20  21:00:39  wfp5p
# Removed filter
# Added the showreply feature and emacs key bindings
#
# Revision 1.1.1.1  1995/04/19  20:38:25  wfp5p
# Initial import of elm 2.4 PL0 as base for elm 2.5.
#
#

#	Variables established by Configure
SRCDIR		=	$source_dir
$VPATH		=	\$(SRCDIR)
CAT		=	$cat
GREP		=	$grep
ECHO		=	$echo
PR		=	$pr
SHAR		=	$shar -s 60000
SLEEP		=	$sleep
TAR		=	$tar
TAPE		=	$rmttape

!GROK!THIS!
cat >>Makefile <<'!NO!SUBS!'
#	Other variables
SHELL			=	/bin/sh
SOURCE_FILES		=	`$(GREP) -v 'doc/' MANIFEST`
DISTRIBUTED_FILES	=	`$(CAT) MANIFEST`

default_target:	all

REQS = Make.defs Makefile config.h sysdefs.h

setup all install rmt-install uninstall lint clean tags depend : $(REQS) .CHECK
	cd lib && $(MAKE) $(MFLAGS) $(MJ) $@
	cd src && $(MAKE) $(MFLAGS) $(MJ) $@
	cd utils && $(MAKE) $(MFLAGS) $(MJ) $@
	cd doc && $(MAKE) $(MFLAGS) $(MJ) $@

#
# Ensure required subdirectories and their Makefiles exist.
#
.CHECK :
	@test -d bin || ( set -x ; mkdir bin )
	-@for d in lib src utils doc ; do \
		test -d $$d || ( set -x ; mkdir $$d ) ; \
		test -f $$d/Makefile || ( set -x ; cd $$d ; sh $(SRCDIR)/$$d/Makefile.SH ) ; \
	done

Make.defs:	$(SRCDIR)/Make.defs.SH config.sh
		sh $(SRCDIR)/Make.defs.SH

Makefile:	$(SRCDIR)/Makefile.SH Make.defs config.sh
		sh $(SRCDIR)/Makefile.SH
		@echo "Makefile has been rebuilt - please restart make"
		@false

config.h:	$(SRCDIR)/config.h.SH config.sh
		sh $(SRCDIR)/config.h.SH

sysdefs.h:	$(SRCDIR)/hdrs/sysdefs.SH config.sh
		sh $(SRCDIR)/hdrs/sysdefs.SH

#	Targets that apply to all elm at once
listing:
		@$(ECHO) "Making source listing in file LISTING."
		@$(PR) -n -f $(SOURCE_FILES) > LISTING

shar:
		$(SHAR) $(DISTRIBUTED_FILES)

tar:
		$(TAR) cvf $(TAPE) $(DISTRIBUTED_FILES)

#	Targets that are really in subordinate make files
documentation:
	cd doc; $(MAKE) $(MFLAGS) $(MJ) $@

elm:
	cd lib; $(MAKE) $(MFLAGS) $(MJ) all
	cd src; $(MAKE) $(MFLAGS) $(MJ) all


utils: .FORCE
	cd utils; $(MAKE) $(MFLAGS) $(MJ) all

.FORCE:

!NO!SUBS!
