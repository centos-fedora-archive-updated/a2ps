dnl Testing delegations for a2ps: a recent version of psutils
dnl
dnl Usage: a2_PSUTILS
dnl
dnl If psutils version 1 patchlevel 17, or psutils version >= 2 is available
dnl     SUBST(PSUTILS) to <nothing>
dnl else
dnl     SUBST(PSUTILS) to `#'

# serial 3

AC_DEFUN([a2_PSUTILS],
[ad_CHECK_PROG(psselect)
ad_CHECK_PROG(psnup)
if test "$COM_psselect" = "#"; then
  COM_PSUTILS="#"
else
  # We found psselect.  Check we either have 1.17 or 2.x
  ac_prog_psselect_banner=`psselect -v 2>&1 | sed 1q`
  if test "${ac_prog_psselect_banner}" != "psselect release 1 patchlevel 17" &&
     test `echo "${ac_prog_psselect_banner}" | cut -d " " -f 2 | cut -d . -f 1` -lt 2; then
    COM_PSUTILS="#"
  fi
fi
if test "$COM_PSUTILS" = "#"; then
  AC_MSG_WARN([===========================================================])
  AC_MSG_WARN([a2ps works much better with psutils.  Available at:])
  AC_MSG_WARN([  https://github.com/rrthomas/psutils/])
  AC_MSG_WARN([You *really* should install it *before* installing a2ps.])
  AC_MSG_WARN([===========================================================])
fi
AC_SUBST(COM_PSUTILS)])
