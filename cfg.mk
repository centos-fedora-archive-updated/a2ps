# Configuration for maintainer-makefile
#
# Copyright (c) 2022 Free Software Foundation, Inc.
#
# This file is part of GNU a2ps.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

GNULIB_SRCDIR ?= $(srcdir)/gnulib
gnulib_dir = $(GNULIB_SRCDIR)
manual_title = GNU a2ps

# Set format of NEWS
old_NEWS_hash := 58f198bd1d0a247c3217167d9195d418

# Don't check 3rd-party sources or test files
VC_LIST_ALWAYS_EXCLUDE_REGEX = tests/(g?ps-ref|tstfiles)/|regex.c$$

_makefile_at_at_check_exceptions ?= '&& !/libpath/ && !/date/ && !/LIBDIR/'

ignore_doubled_word_match_RE_ ?= ^doc/a2ps.texi:3699

local-checks-to-skip = \
	sc_bindtextdomain \
	sc_GPL_version \
	sc_indent \
	sc_po_check \
	sc_prohibit_atoi_atof \
	sc_prohibit_gnu_make_extensions \
	sc_prohibit_strncpy \
	sc_space_tab \
	sc_tight_scope \
	sc_trailing_blank \
	sc_unmarked_diagnostics
