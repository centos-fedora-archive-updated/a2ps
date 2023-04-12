# Process this file with automake to create Makefile.in
#
# Copyright 1995-2023 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

AM_CPPFLAGS = -I$(top_builddir) -I$(srcdir) -I$(top_srcdir)/lib -I$(top_builddir)/lib \
	-DLOCALEDIR=\"$(localedir)\" -DSYSCONFFILE=\"$(sysconfdir)/a2ps.cfg\" -DHAVE_CONFIG_H
AM_CFLAGS = $(WARN_CFLAGS) $(BDW_GC_FLAGS)

#
# Definition of the local target
#
noinst_LTLIBRARIES = liba2ps.la
noinst_LIBRARIES = libnowarnings.a

# --debug, --defines, --verbose
AM_YFLAGS = -dtv
# Use a struct, handle duplicates, produce ANSI-C
GPERFFLAGS = -t -D -L ANSI-C
BUILT_SOURCES = lexppd.h parseppd.c liba2ps.h confg.c

liba2psheaders = encoding.h media.h jobs.h output.h		\
routines.h psgen.h prolog.h faces.h useropt.h			\
gen.h printers.h psstat.h caret.h metaseq.h options.h		\
dsc.h fonts.h ppd.h prange.h stream.h document.h		\
fjobs.h	common.h madir.h filalign.h lexppd.h			\
system.h a2ps.h liba2ps.h yy2ppd.h gc-stubs.h

liba2pssources = encoding.c media.c jobs.c output.c		\
routines.c psgen.c prolog.c faces.c useropt.c			\
gen.c printers.c psstat.c caret.c metaseq.c options.c		\
dsc.c ppd.c prange.c stream.c document.c			\
fjobs.c common.c madir.c filalign.c gc-stubs.c

mylibitheaders = msg.h msg.c message.h xstrrpl.h getshline.h pathwalk.h	\
darray.h dstring.h printlen.h pair_ht.h filtdir.h str_ht.h		\
title.h xbackupfile.h getnum.h tterm.h lister.h userdata.h xobstack.h

mylibitsources = message.c xstrrpl.c gc_calloc.c getshline.c pathwalk.c	\
darray.c dstring.c printlen.c pair_ht.c filtdir.c str_ht.c		\
title.c xbackupfile.c getnum.c tterm.c lister.c userdata.c

libitheaders = argv.h hashtab.h

libitsources = argv.c hashtab.c

noinst_HEADERS = $(liba2psheaders) $(libitheaders) $(mylibitheaders)

libnowarnings_a_CFLAGS = $(BDW_GC_FLAGS)
libnowarnings_a_SOURCES = fonts.l lexppd.l parseppd.y regex.c regex.h confg.c confg.h

liba2ps_la_LDFLAGS = $(BDW_GC_LIBS)
liba2ps_la_SOURCES = $(liba2pssources) $(libitsources) $(mylibitsources)
liba2ps_la_LIBADD = ../lib/libgnu.la $(LIBINTL) $(LIBSOCKET) $(GETHOSTNAME_LIB)

#
# Handling the Gperf code
#
GPERF = gperf
confg.c: confg.gperf
	$(GPERF) $(GPERFFLAGS) $< >$@

#
# Building the header for using liba2ps
#
noinst_HEADERS += liba2ps.h
liba2ps_h_sources = liba2ps.h.in liba2ps.h.extract

liba2ps_h_extract_sources = faces.h gen.h jobs.h confg.h

liba2ps.h.extract: $(liba2ps_h_extract_sources) Makefile.am
	@echo "Extracting liba2ps.h info"
	@rm -rf liba2ps.h.extract 2> /dev/null
	@for f in $(liba2ps_h_extract_sources);			\
	do							\
	  $(AWK) '/liba2ps.h:begin/, /liba2ps.h:end/' $(srcdir)/$$f	\
	     | sed -e "s|/\* liba2ps.h:begin.*$$|/* From $$f */|g"	\
	     | sed -e "s|/\* liba2ps.h:end.*$$||g"		\
	     >> liba2ps.h.extract;				\
	done

liba2ps.h: $(liba2ps_h_sources)
	@echo "Building liba2ps.h";					\
	files=`grep liba2ps.h:include: $(srcdir)/liba2ps.h.in	\
	       | sed -e 's/^.*liba2ps.h:include:\([^ ]*\).*$$/\1/'`;	\
	cp $(srcdir)/liba2ps.h.in liba2ps.h.tmp1;			\
	for f in $$files;						\
	do								\
	  echo "Inlining file $$f";					\
	  ff=`echo $$f | sed -e 's#/#\\\\/#g'`;				\
	  cat liba2ps.h.tmp1						\
	    | sed -e "/liba2ps.h:include:$$ff/r $(srcdir)/$$f"		\
	    | sed -e "s|^.*liba2ps.h:include:$$f.*$$|\/* File $$ff *\/|g"\
		> liba2ps.h.tmp2;					\
	  mv liba2ps.h.tmp2 liba2ps.h.tmp1;				\
	done;								\
	mv liba2ps.h.tmp1 liba2ps.h

loc-local:
	cd $(srcdir) && $(CLOC) $(CLOC_OPTS) $(liba2pssources) $(liba2psheaders) $(libitsources) $(libitheaders) $(mylibitsources) $(mylibitheaders) $(libnowarnings_a_SOURCES) $(liba2ps_h_sources)

EXTRA_DIST = liba2ps.h $(liba2ps_h_sources) liba2ps.h $(BUILT_SOURCES) parseppd.h confg.gperf
