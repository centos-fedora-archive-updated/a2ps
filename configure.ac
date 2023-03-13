AC_PREREQ([2.71])
AC_COPYRIGHT([Copyright 2023 Free Software Foundation, Inc.])

## ---------------- ##
## Initialization.  ##
## ---------------- ##

AC_INIT([GNU a2ps],[4.15.1],[bug-a2ps@gnu.org])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
m4_pattern_forbid([^jm_[A-Z]])

# Initialize automake
AM_INIT_AUTOMAKE([1.15 foreign subdir-objects silent-rules -Wall])

AC_CONFIG_HEADERS([config.h])

# Define the package name
GNU_PACKAGE="GNU $PACKAGE"
AC_DEFINE_UNQUOTED([GNU_PACKAGE], "$GNU_PACKAGE",
		   [The concatenation of the strings `GNU ', and PACKAGE.])
AC_SUBST(GNU_PACKAGE)

## --------------------- ##
## Checks for programs.  ##
## --------------------- ##

AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PATH_PROG(PERL, perl)

# Check for the C compiler and the various oddities that may exist
AC_PROG_CC
gl_EARLY

dnl Extra warnings with GCC
AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--enable-gcc-warnings],
		  [turn on lots of GCC warnings (for developers)])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=no]
)
if test "$gl_gcc_warnings" = yes; then
  dnl Set up the list of undesired warnings.
  nw=
  nw="$nw -Wsystem-headers"   # Don’t let system headers trigger warnings
  nw="$nw -Wvla -Wstack-protector" # We use variable-length arrays

  gl_MANYWARN_ALL_GCC([warnings])

  dnl Enable all GCC warnings not in this list.
  gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
  for w in $warnings; do
    gl_WARN_ADD([$w])
  done

  dnl Add an extra warning
  gl_WARN_ADD([-Wconversion])

  # When compiling with GCC, prefer -isystem to -I when including system
  # include files, to avoid generating useless diagnostics for the files.
  ISYSTEM='-isystem '
else
  ISYSTEM='-I'
fi
AC_SUBST([ISYSTEM])

AC_USE_SYSTEM_EXTENSIONS

AC_PROG_YACC
AM_PROG_AR

# I want flex, and only flex
AC_PROG_LEX([noyywrap])
AC_MSG_CHECKING([whether LEX is flex])
AS_CASE([`$LEX --version`],
  [flex*],
    [AC_MSG_RESULT([yes])],
  [
    AC_MSG_RESULT([no])
    LEX="$SHELL $missing_dir/missing flex"
    AC_SUBST([LEX_OUTPUT_ROOT], [lex.yy])
    AC_SUBST([LEXLIB], [''])
])

# Libtool.  By default, don't do shared libs, liba2ps is not widely used.
LT_INIT([disable-shared static])

gl_INIT

## ------------------------------ ##
## checks for library functions.  ##
## ------------------------------ ##

# libgc (BDW garbage collector)
PKG_CHECK_MODULES([BDW_GC], [bdw-gc >= 7.2])
AH_BOTTOM([
#define malloc GC_malloc
#define calloc GC_calloc
#define realloc GC_realloc
#ifndef free
#define free GC_free
#endif
#define strdup GC_strdup
#define strndup GC_strndup
#define reallocarray a2ps_reallocarray
])

ad_FUNC_SYSTEMPAPERNAME
AS_IF([test "$ac_cv_lib_paper_systempapername" = "no"],
  [AC_MSG_ERROR([a2ps needs libpaper to work])])

# Internationalization.
AM_GNU_GETTEXT_VERSION([0.20.2])
AM_GNU_GETTEXT([external], [need-ngettext])

# In path separator character.
AC_DEFINE_UNQUOTED([DIRECTORY_SEPARATOR], '/',
		   [The char used to separate component of a path])
# Between paths separator character.
AC_DEFINE_UNQUOTED([PATH_SEPARATOR], ':',
		   [The char used to separate paths.])
# We define SYNTAX_TABLE so that regex uses syntax tables
AC_DEFINE([SYNTAX_TABLE], 1,
	  [Define to enable syntax table support in `regex'.])

## ---------------------------- ##
## Various extentions/options.  ##
## ---------------------------- ##

# Where to put the emacs files
AM_PATH_LISPDIR

# Shall the path to the tools be hardcoded?
ad_ENABLE_PATHS

# What should get installed?
AC_ARG_ENABLE(extensions,
AS_HELP_STRING(--disable-extensions,small foot print installation),
[case "${enableval}" in
  yes|no) ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --disable-extensions) ;;
esac],[enable_extensions=yes])
AM_CONDITIONAL(EXTENSIONS, test x$enable_extensions = xyes)

AC_MSG_CHECKING(for PostScript fonts path)
# Try to find some PostScript fonts.
# Find out if ghostscript is installed
ac_psfont_path=
for ac_dir in /usr/local/ghostscript/fonts \
  /usr/local/share/ghostscript/fonts;
do
  if test "cd $ac_dir 2>/dev/null && echo *.afm"; then
    ac_psfont_path="$ac_psfont_path:$ac_dir";
  fi
done
dnl # Remove the leading `:'
if test -n "$ac_psfont_path"; then
  PSFONT_PATH=`echo "$ac_psfont_path" | sed -e 's/^://'`
fi
AC_SUBST(PSFONT_PATH)
AC_MSG_RESULT([$ac_psfont_path])

## ---------------------------------------------------------- ##
## Write a first sketch of a2ps.cfg.  The user should check.  ##
## ---------------------------------------------------------- ##

#
# Check for a file program that follows symlinks
#
ad_PROG_FILE_LINK
AC_SUBST(FILE_LINK)

#
# default encoding to use (defaulted to latin1)
#
AC_ARG_WITH(encoding,
AS_HELP_STRING([--with-encoding=enc],
               [specify the default encoding (ascii, latin1, latin2... latin6, hp etc.) [default=latin1]]),
ENCODING=$with_encoding,
ENCODING=latin1)
AC_SUBST(ENCODING)

########################
# Nice special printers
########################
# Look for GV or Ghostview
ad_CHECK_PROGS([gv, ghostview])
if test "$COM_gv$COM_ghostview" = "##"; then
  COM_DISPLAY="#"
fi
AC_SUBST(COM_DISPLAY)

#################################
# Helping people with delegations
#################################
# "Compressed" delegation
ad_CHECK_PROG(gzip,
[a2ps works better with gzip.
Consider fetching gzip at any ftp site proposing GNU programs])
ad_CHECK_PROGS([bzip, bzip2])

# DVI delegation
ad_CHECK_PROG(dvips)

# ImageMagick for most image formats
ad_CHECK_PROG(convert)

# HTML delegation
ad_CHECK_PROGS([html2ps])

# PDF delegation
ad_CHECK_PROGS([pdf2ps])

# Outputting PDF
ad_CHECK_PROGS([ps2pdf])
AC_SUBST(COM_PS2PDF)

# PostScript delegation.  Check for psutils >= 1.17
a2_PSUTILS

# ROFF delegation
ad_CHECK_PROG(grog,
[a2ps works better with GNU roff.
Consider fetching groff from https://ftp.gnu.org/pub/groff])

# Texinfo delegation
ad_CHECK_PROGS([makeinfo, tex, latex])
COM_TEXI="$COM_tex$COM_makeinfo$COM_dvips"
COM_LATEX="$COM_latex$COM_dvips"
AC_SUBST(COM_TEXI)
AC_SUBST(COM_LATEX)

## ---------- ##
## Epilogue.  ##
## ---------- ##

AC_CONFIG_SUBDIRS(ogonkify)

# Shell scripts
AC_CONFIG_FILES([contrib/card], [chmod +x contrib/card])
AC_CONFIG_FILES([contrib/fixps], [chmod +x contrib/fixps])
AC_CONFIG_FILES([contrib/pdiff], [chmod +x contrib/pdiff])
AC_CONFIG_FILES([contrib/lp2], [chmod +x contrib/lp2])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  doc/Makefile
  man/Makefile
  tests/Makefile tests/defs
  lib/Makefile
  liba2ps/Makefile
  build-aux/Makefile
  etc/Makefile etc/a2ps_cfg etc/a2ps-site.cfg
  po/Makefile.in
  po-gnulib/Makefile.in
  sheets/Makefile
  encoding/Makefile
  ps/Makefile
  afm/Makefile
  contrib/Makefile
  contrib/emacs/Makefile
  fonts/Makefile
  ppd/Makefile
])

dnl help2man
dnl Set a value even if not found, so that x-to-1 gives a better error.
AC_PATH_PROG([HELP2MAN], [help2man], [help2man])
CROSS_COMPILING=$cross_compiling
AC_SUBST([CROSS_COMPILING])
AC_CONFIG_FILES([build-aux/x-to-1], [chmod +x build-aux/x-to-1])

# Code counting
AM_EXTRA_RECURSIVE_TARGETS([loc])
AC_PATH_PROG(CLOC, cloc, true)
CLOC_OPTS="--autoconf --force-lang=C,h --force-lang=C,extract"
AC_SUBST([CLOC_OPTS])

AC_OUTPUT

AC_MSG_RESULT([
*****************************************************************
* a2ps thinks it should be configured the following way:
* - your charset is `$ENCODING'
* - to ask `file(1)' to follow symlinks, run `$FILE_LINK'
*   (`file(1)' is used to guess the type of the files to print.)
*
* If a2ps is wrong somewhere, fix the `System Dependent parameters'
* in `etc/a2ps-site.cfg'.  See the README file.
*****************************************************************
])
