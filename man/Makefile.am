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

man_MANS = $(regular_mans) $(irregular_mans)
man_aux = a2ps.x card.x fixps.x lp2.x pdiff.x

regular_mans = card.1 fixps.1 lp2.1 pdiff.1
irregular_mans = a2ps.1 a2ps-lpr-wrapper.1

EXTRA_DIST = $(man_MANS) $(man_aux) common.x a2ps-lpr-wrapper.1
MAINTAINERCLEANFILES = $(man_MANS)

# Depend on configure.ac to get version number changes.
$(man_MANS): $(top_srcdir)/configure.ac common.x

# Depend on the source file containing the --help text.
$(regular_mans): %.1: $(top_srcdir)/contrib/%.in
a2ps.1: $(top_srcdir)/src/main.c

# Build command for man pages
# Exit gracefully if $@ is not writeable, such as during distcheck!
do_build = "if ( touch $${name%.*}.w && rm -f $${name%.*}.w; ) >/dev/null 2>&1; then \
	  $(top_builddir)/build-aux/x-to-1 \
		$(PERL) $(HELP2MAN) --include=$(srcdir)/common.x \
		$$exe $(srcdir)/$${name%.*}.x $$name; \
	fi"

a2ps.1: a2ps.x
	name=$@ exe=$(top_builddir)/src/a2ps; eval $(do_build)
card.1: card.x
	name=$@ exe=$(top_builddir)/contrib/card; eval $(do_build)
lp2.1: lp2.x
	name=$@ exe=$(top_builddir)/contrib/lp2; eval $(do_build)
fixps.1: fixps.x
	name=$@ exe=$(top_builddir)/contrib/fixps; eval $(do_build)
pdiff.1: pdiff.x
	name=$@ exe=$(top_builddir)/contrib/pdiff; eval $(do_build)
