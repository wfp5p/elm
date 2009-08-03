LANGFILE=LANGS
CATNAME=elm2.5.cat
GENCAT=	gencat/gencat

HDIR=	../hdrs
SDIR=	C/C/C
MSGCAT=	elm2.5.cat
HFILES=	$(HDIR)/s_aliases.h \
	$(HDIR)/s_answer.h \
	$(HDIR)/s_elm.h \
	$(HDIR)/s_elmalias.h \
	$(HDIR)/s_elmrc.h \
	$(HDIR)/s_error.h \
	$(HDIR)/s_fastmail.h \
	$(HDIR)/s_from.h \
	$(HDIR)/s_newalias.h \
	$(HDIR)/s_newmail.h \
	$(HDIR)/s_readmsg.h

MFILES=	$(SDIR)/s_aliases.m \
	$(SDIR)/s_answer.m \
	$(SDIR)/s_elm.m \
	$(SDIR)/s_elmalias.m \
	$(SDIR)/s_elmrc.m \
	$(SDIR)/s_error.m \
	$(SDIR)/s_fastmail.m \
	$(SDIR)/s_from.m \
	$(SDIR)/s_newalias.m \
	$(SDIR)/s_newmail.m \
	$(SDIR)/s_readmsg.m

nls-headers: clean $(HFILES)

# Nothing to do here.  The message catalogs all have the
# same basename, so wait until install to put them somewhere.
all:

install:
	./Install $(LANGFILE) $(GENCAT) $(CATNAME)

$(HFILES): $(MFILES)
	gencat/gencat $(MSGCAT) "$(SDIR)/`basename $@ .h`".m -h $@

clean: FRC
	/bin/rm -f $(MSGCAT)

FRC:
