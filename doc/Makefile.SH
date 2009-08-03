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


#   Eliminate complaints about duplicate makefile entries if mansrc and
#   catmansrc are both "none".
if [ $mansrc = "none" ]; then mansrc="man-none"; fi
if [ $catmansrc = "none" ]; then catmansrc="catman-none"; fi


echo "Extracting doc/Makefile (with variable substitutions)"
cat >Makefile <<!GROK!THIS!
#
# $Id: Makefile.SH,v 1.13 1999/03/24 14:03:38 wfp5p Exp $
#
#  Makefile for the documentation of the ELM mail system
#
#         (C) Copyright 1988-1992, USENET Community Trust
#
#  $Log: Makefile.SH,v $
# Revision 1.13  1999/03/24  14:03:38  wfp5p
# elm 2.5PL0
# 
# Revision 1.12  1996/05/09  15:50:40  wfp5p
# Alpha 10
#
# Revision 1.11  1996/03/14  17:27:17  wfp5p
# Alpha 9
#
# Revision 1.10  1995/09/29  17:40:35  wfp5p
# Alpha 8 (Chip's big changes)
#
# Revision 1.9  1995/09/11  15:18:41  wfp5p
# Alpha 7
#
# Revision 1.8  1995/07/18  18:59:41  wfp5p
# Alpha 6
#
# Revision 1.7  1995/06/30  14:56:13  wfp5p
# Alpha 5
#
# Revision 1.6  1995/06/09  22:06:31  wfp5p
# mlist wasn't getting installed correctly
#
# Revision 1.5  1995/06/08  15:44:48  wfp5p
# Changes to configure for AIX
#
# Revision 1.4  1995/05/10  13:34:14  wfp5p
# Added mailing list stuff by Paul Close <pdc@sgi.com>
#
# Revision 1.2  1995/04/20  21:01:00  wfp5p
# Removed filter
# Added the showreply feature and emacs key bindings
#
# Revision 1.1.1.1  1995/04/19  20:38:28  wfp5p
# Initial import of elm 2.4 PL0 as base for elm 2.5.
#
#
# Variables
#	Variables established by Configure
SRCDIR		=	$source_dir
$VPATH		=	\$(SRCDIR)/doc
CHMOD		=	$chmod
CP		=	$cp
DEST		=	$installbin
ECHO		=	$echo
FORMATTER	=	$roff
FORMATTER_OPTS	=	$roffopts
LIB		=	$installlib
LN		=	$ln
MAN		=	$mansrc
MANEXT		=	$manext
CATMAN		=	$catmansrc
CATMANEXT	=	$catmanext
RM		= 	$rm -f
TBL       	=	$tbl
MANROFF		=	$manroff
MANROFF_OPTS	=	$manroffopts
MV		=	$mv
TEST		=	$test
SUFFIX		=	$suffix
PACKED		=	$packed
PACKER		=	$packer

!GROK!THIS!

cat >>Makefile <<'!NO!SUBS!'

#	Other variables
SHELL		=	/bin/sh

# List of installed man pages (except for wnemail.1 - handled differently)
MAN_LIST	=	$(MAN)/answer$(MANEXT)		\
			$(MAN)/checkalias$(MANEXT)	\
			$(MAN)/elm$(MANEXT)		\
			$(MAN)/elmalias$(MANEXT)	\
			$(MAN)/fastmail$(MANEXT)	\
			$(MAN)/frm$(MANEXT)		\
			$(MAN)/listalias$(MANEXT)	\
			$(MAN)/messages$(MANEXT)	\
			$(MAN)/mmencode$(MANEXT)	\
			$(MAN)/newalias$(MANEXT)	\
			$(MAN)/newmail$(MANEXT)	\
			$(MAN)/printmail$(MANEXT)	\
			$(MAN)/readmsg$(MANEXT)

# List of installed catman pages (except for wnemail.1 - handled differently)
CATMAN_LIST	=	$(CATMAN)/answer$(CATMANEXT)$(SUFFIX)	\
			$(CATMAN)/checkalias$(CATMANEXT)$(SUFFIX)\
			$(CATMAN)/elm$(CATMANEXT)$(SUFFIX)	\
			$(CATMAN)/elmalias$(CATMANEXT)$(SUFFIX)\
			$(CATMAN)/fastmail$(CATMANEXT)$(SUFFIX)	\
			$(CATMAN)/frm$(CATMANEXT)$(SUFFIX)	\
			$(CATMAN)/listalias$(CATMANEXT)$(SUFFIX) \
			$(CATMAN)/messages$(CATMANEXT)$(SUFFIX)	\
			$(CATMAN)/mmencode$(CATMANEXT)$(SUFFIX)	\
			$(CATMAN)/newalias$(CATMANEXT)$(SUFFIX)	\
			$(CATMAN)/newmail$(CATMANEXT)$(SUFFIX)	\
			$(CATMAN)/printmail$(CATMANEXT)$(SUFFIX) \
			$(CATMAN)/readmsg$(CATMANEXT)$(SUFFIX)

# List of formatted pages for catman
FORMATTED_PAGES_LIST =	catman/answer$(CATMANEXT)$(SUFFIX)	\
			catman/checkalias$(CATMANEXT)$(SUFFIX)	\
			catman/elm$(CATMANEXT)$(SUFFIX)		\
			catman/elmalias$(CATMANEXT)$(SUFFIX)	\
			catman/fastmail$(CATMANEXT)$(SUFFIX)	\
			catman/frm$(CATMANEXT)$(SUFFIX)		\
			catman/listalias$(CATMANEXT)$(SUFFIX)	\
			catman/messages$(CATMANEXT)$(SUFFIX)	\
			catman/mmencode$(CATMANEXT)$(SUFFIX)	\
			catman/newalias$(CATMANEXT)$(SUFFIX)	\
			catman/newmail$(CATMANEXT)$(SUFFIX)	\
			catman/printmail$(CATMANEXT)$(SUFFIX)	\
			catman/readmsg$(CATMANEXT)$(SUFFIX)

# List of remotely installed man pages (except for wnemail.1 - handled
# differently)
REMOTE_MAN_LIST	=	$(REMOTE)$(MAN)/answer$(MANEXT)		\
			$(REMOTE)$(MAN)/checkalias$(MANEXT)	\
			$(REMOTE)$(MAN)/elm$(MANEXT)		\
			$(REMOTE)$(MAN)/elmalias$(MANEXT)	\
			$(REMOTE)$(MAN)/fastmail$(MANEXT)	\
			$(REMOTE)$(MAN)/frm$(MANEXT)		\
			$(REMOTE)$(MAN)/listalias$(MANEXT)	\
			$(REMOTE)$(MAN)/messages$(MANEXT)	\
			$(REMOTE)$(MAN)/mmencode$(MANEXT)	\
			$(REMOTE)$(MAN)/newalias$(MANEXT)	\
			$(REMOTE)$(MAN)/newmail$(MANEXT)	\
			$(REMOTE)$(MAN)/printmail$(MANEXT)	\
			$(REMOTE)$(MAN)/readmsg$(MANEXT)

# List of files installed in LIB directory
LIB_LIST	=	$(LIB)/elmrc-info	\
			$(LIB)/help-main	\
			$(LIB)/help-options	\
			$(LIB)/help-alias	\
			$(LIB)/help-pager	\
			$(LIB)/help-status	\
			$(LIB)/help-fbrowser	\
			$(LIB)/help-attach	\
			$(LIB)/help-selfolder	\
			$(LIB)/mime-types	\
			$(LIB)/mlists.eg

# List of files installed in remote LIB directory
REMOTE_LIB_LIST	=	$(REMOTE)$(LIB)/elmrc-info	\
			$(REMOTE)$(LIB)/help-main	\
			$(REMOTE)$(LIB)/help-options	\
			$(REMOTE)$(LIB)/help-alias	\
			$(REMOTE)$(LIB)/help-pager	\
			$(REMOTE)$(LIB)/help-status	\
			$(REMOTE)$(LIB)/help-fbrowser	\
			$(REMOTE)$(LIB)/help-attach	\
			$(REMOTE)$(LIB)/help-selfolder	\
			$(REMOVE)$(LIB)/mime-types	\
			$(REMOVE)$(LIB)/mlists.eg

# List of formatted guides
FORMATTED_LIST	=	Alias.fmtd	\
			Config.fmtd 	\
			Cover.fmtd	\
			Form.fmtd	\
			Ref.fmtd	\
			Users.fmtd

# Targets
all:  Makefile
	@$(TEST) '$(CATMAN)' = "catman-none" || \
		$(MAKE) $(MFLAGS) formatted_pages

#	The following are no-ops
lint:
clean:	

Makefile:     $(SRCDIR)/doc/Makefile.SH ../config.sh
		sh $(SRCDIR)/doc/Makefile.SH
		@echo "Makefile has been rebuilt - please restart make"
		@false

formatted_pages: catman $(FORMATTED_PAGES_LIST)

catman:
	mkdir catman

install: $(LIB_LIST)
	@$(TEST) '$(MAN)' = "man-none" || $(MAKE) $(MFLAGS) install_man
	@$(TEST) '$(CATMAN)' = "catman-none" || $(MAKE) $(MFLAGS) install_catman
	
install_man: $(MAN_LIST) $(MAN)/wnewmail$(MANEXT)

install_catman: $(CATMAN_LIST) $(CATMAN)/wnewmail$(CATMANEXT)$(SUFFIX)

uninstall:
	$(RM) $(LIB_LIST) 
	@$(TEST) '$(MAN)' = "man-none" \
		|| $(RM) $(MAN_LIST) $(MAN)/wnewmail$(MANEXT)
	@$(TEST) '$(CATMAN)' != "catman-none" \
		|| $(RM) $(CATMAN_LIST) $(CATMAN)/wnewmail$(CATMANEXT)$(SUFFIX)

#	Dependencies and rules for installing man pages and lib files
#	taken from a remote system
#	This is the only target that gets installed even if not out-of-date
#	with respect the files from which it is installed.
rmt-install:	rmt-defined
		$(RM) $(MAN)/newmail$(MANEXT) $(MAN)/wnewmail$(MANEXT)
		$(CP) $(REMOTE_MAN_LIST) $(MAN)
		$(CHMOD) u+rw,go+r $(MAN_LIST)
		$(LN) $(MAN)/newmail$(MANEXT) $(MAN)/wnewmail$(MANEXT)
		$(CP) $(REMOTE_LIB_LIST) $(LIB)
		$(CHMOD) u+rw,go+r $(LIB_LIST)

rmt-defined:
	@(if [ "$(REMOTE)" = "" ];\
	  then\
	    $(ECHO) "You need to define 'REMOTE' as the remote file system";\
	    $(ECHO) "for this particular command. The easiest way to do this";\
	    $(ECHO) "to type:";\
	    $(ECHO) "        make REMOTE=<remote file system> rmt-install";\
	    exit 1;\
	  fi);

BEEP = ( echo X | tr X '\07' | tr -d '\012' ) 2>/dev/null
I_CHECK = if $(TEST) ! -f $@ ; then : ; else \
	echo "" ; \
	echo "WARNING - installation is about to overwrite" ; \
	echo "     $@" ; \
	echo "I will pause 10 seconds.  Abort NOW to prevent overwrite." ; \
	echo "" ; \
	$(BEEP) ; sleep 2 ; \
	$(BEEP) ; sleep 2 ; \
	$(BEEP) ; sleep 2 ; \
	$(BEEP) ; sleep 2 ; \
	$(BEEP) ; sleep 2 ; \
	echo "Continuing with install..." ; \
	echo ; \
	fi

I_DELETE = $(TEST) ! -f $@ || $(RM) $@
INST_444 = $(I_DELETE) ; $(CP) $? $@ ; $(CHMOD) 444 $@
INST_644 = $(I_DELETE) ; $(CP) $? $@ ; $(CHMOD) 644 $@
INST_LINK = $(I_DELETE) ; $(LN) $? $@

#	Dependencies and rules for installing man pages and lib files

$(MAN)/answer$(MANEXT):		answer.1		; $(INST_444)
$(MAN)/checkalias$(MANEXT):	chkalias.1		; $(INST_444)
$(MAN)/elm$(MANEXT):		elm.1			; $(INST_444)
$(MAN)/elmalias$(MANEXT):	elmalias.1		; $(INST_444)
$(MAN)/fastmail$(MANEXT):	fastmail.1		; $(INST_444)
$(MAN)/frm$(MANEXT):		frm.1			; $(INST_444)
$(MAN)/listalias$(MANEXT):	listalias.1		; $(INST_444)
$(MAN)/messages$(MANEXT):	messages.1		; $(INST_444)
$(MAN)/mmencode$(MANEXT):	mmencode.1		; $(INST_444)
$(MAN)/newalias$(MANEXT):	newalias.1		; $(INST_444)
$(MAN)/newmail$(MANEXT):	newmail.1		; $(INST_444)
$(MAN)/printmail$(MANEXT):	printmail.1		; $(INST_444)
$(MAN)/readmsg$(MANEXT):	readmsg.1		; $(INST_444)
$(MAN)/wnewmail$(MANEXT):	$(MAN)/newmail$(MANEXT)	; $(INST_LINK)

$(LIB)/help-main:		help-main		; $(INST_444)
$(LIB)/help-options:		help-options		; $(INST_444)
$(LIB)/help-alias:		help-alias		; $(INST_444)
$(LIB)/help-pager:		help-pager		; $(INST_444)
$(LIB)/help-status:		help-status		; $(INST_444)
$(LIB)/help-fbrowser:		help-fbrowser		; $(INST_444)
$(LIB)/help-attach:		help-attach		; $(INST_444)
$(LIB)/help-selfolder:		help-selfolder		; $(INST_444)

$(LIB)/mlists.eg:		mlists.eg		; $(INST_444)
$(LIB)/elmrc-info:		elmrc-info		; $(INST_444)

# This may have custom modifications.  Be careful about overwriting.
$(LIB)/mime-types:		mime-types
	@ $(I_CHECK)
	$(INST_444)

#	Dependencies and rules for installing catman

CES = $(CATMANEXT)$(SUFFIX)
$(CATMAN)/answer$(CES):		catman/answer$(CES)	; $(INST_644)
$(CATMAN)/checkalias$(CES):	catman/checkalias$(CES)	; $(INST_644)
$(CATMAN)/elm$(CES):		catman/elm$(CES)	; $(INST_644)
$(CATMAN)/elmalias$(CES):	catman/elmalias$(CES)	; $(INST_644)
$(CATMAN)/fastmail$(CES):	catman/fastmail$(CES)	; $(INST_644)
$(CATMAN)/frm$(CES):		catman/frm$(CES)	; $(INST_644)
$(CATMAN)/listalias$(CES):	catman/listalias$(CES)	; $(INST_644)
$(CATMAN)/messages$(CES):	catman/messages$(CES)	; $(INST_644)
$(CATMAN)/mmencode$(CES):	catman/mmencode$(CES)	; $(INST_644)
$(CATMAN)/newalias$(CES):	catman/newalias$(CES)	; $(INST_644)
$(CATMAN)/newmail$(CES):	catman/newmail$(CES)	; $(INST_644)
$(CATMAN)/printmail$(CES):	catman/printmail$(CES)	; $(INST_644)
$(CATMAN)/readmsg$(CES):	catman/readmsg$(CES)	; $(INST_644)
$(CATMAN)/wnewmail$(CES):	$(CATMAN)/newmail$(CES)	; $(INST_LINK)


#	Dependencies and rules for making catman pages

DO_ROFF = $(MANROFF) $(MANROFF_OPTS) -man $? \
		> catman/`basename $@ $(SUFFIX)`
DO_PACK = if $(TEST) "X$(PACKED)" != "Xy" ; then : ; else \
		$(RM) $@ ; $(PACKER) catman/`basename $@ $(SUFFIX)` ; fi

catman/answer$(CATMANEXT)$(SUFFIX):		answer.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/checkalias$(CATMANEXT)$(SUFFIX):		chkalias.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/elm$(CATMANEXT)$(SUFFIX):		elm.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/elmalias$(CATMANEXT)$(SUFFIX):		elmalias.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/fastmail$(CATMANEXT)$(SUFFIX):		fastmail.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/frm$(CATMANEXT)$(SUFFIX):		frm.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/listalias$(CATMANEXT)$(SUFFIX):		listalias.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/messages$(CATMANEXT)$(SUFFIX):		messages.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/mmencode$(CATMANEXT)$(SUFFIX):		mmencode.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/newalias$(CATMANEXT)$(SUFFIX):		newalias.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/newmail$(CATMANEXT)$(SUFFIX):		newmail.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/printmail$(CATMANEXT)$(SUFFIX):		printmail.1
	$(DO_ROFF)
	@ $(DO_PACK)

catman/readmsg$(CATMANEXT)$(SUFFIX):		readmsg.1
	$(DO_ROFF)
	@ $(DO_PACK)


#	Dependencies and rules for formatting guides
documentation:	$(FORMATTED_LIST)

Alias.fmtd:	Alias.guide
		$(TBL) $? | $(FORMATTER) $(FORMATTER_OPTS) tmac.n - > $@

Config.fmtd:	Config.guid
		$(TBL) $? | $(FORMATTER) $(FORMATTER_OPTS) tmac.n - > $@

Cover.fmtd:	Elm.cover
		$(FORMATTER) $(FORMATTER_OPTS) tmac.n $? > $@

Form.fmtd:	Form.guide
		$(FORMATTER) $(FORMATTER_OPTS) tmac.n $? > $@

Ref.fmtd:	Ref.guide
		$(FORMATTER) $(FORMATTER_OPTS) tmac.n $? > $@

Users.fmtd:	Users.guide
		$(TBL) $? | $(FORMATTER) $(FORMATTER_OPTS) tmac.n - > $@

!NO!SUBS!
