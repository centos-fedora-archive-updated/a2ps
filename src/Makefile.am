# Process this file with automake to create Makefile.in
#
# Copyright 1988-2017 Free Software Foundation, Inc.
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

# I don't really understand why I have to put srcdir here, but
# it is needed for yacc and lex files (seems related to #line, but
# I really don't understand why)...
AM_CPPFLAGS = -I.. -I$(srcdir) -I$(top_srcdir)/liba2ps \
	-I$(top_srcdir)/lib -I$(top_builddir)/lib -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS = $(WARN_CFLAGS) $(BDW_GC_FLAGS)

bin_SCRIPTS = a2ps-lpr-wrapper
bin_PROGRAMS = a2ps

a2ps_SOURCES = main.c \
read.c sshread.c ssheet.c select.c generate.c 			\
delegate.c buffer.c versions.c ffaces.c

noinst_HEADERS = main.h \
read.h sshread.h ssheet.h select.h generate.h			\
delegate.h buffer.h versions.h ffaces.h			\
yy2ssh.h lexps.h

noinst_LIBRARIES = libparse.a
libparse_a_SOURCES = parsessh.y lexssh.l lexps.l sheets-map.l
libparse_a_CFLAGS = $(BDW_GC_FLAGS)

BUILT_SOURCES = parsessh.c lexssh.c lexps.c sheets-map.c

AM_YFLAGS = -dtv
LEX = @LEX@

a2ps_LDADD = $(top_builddir)/liba2ps/liba2ps.la $(top_builddir)/liba2ps/libnowarnings.a libparse.a $(LIBINTL) ../lib/libgnu.la -lm $(BDW_GC_LIBS) -lpaper # FIXME: hard coded.

loc-local:
	cd $(srcdir) && $(CLOC) $(CLOC_OPTS) $(a2ps_SOURCES) $(noinst_HEADERS) $(libparse_a_SOURCES) $(bin_SCRIPTS)

EXTRA_DIST = $(BUILT_SOURCES) parsessh.h $(bin_SCRIPTS)
