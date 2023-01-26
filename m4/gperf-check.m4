dnl
dnl AC_PROG_GPERF (MINIMUM-VERSION)
dnl
dnl Check for availability of gperf.
dnl Abort if not found or if current version is not up to par.
dnl
dnl requires AX_COMPARE_VERSION from autoconf-archive

AC_DEFUN([AC_PROG_GPERF],[
	AC_PATH_PROG(GPERF, gperf, no)
	if test "$GPERF" = no; then
		AC_MSG_ERROR(Could not find gperf)
	fi
	min_gperf_version=ifelse([$1], ,2.7,$1)
	gperf_version=$($GPERF --version | head -n 1 | cut -d" " -f3)
        AC_MSG_CHECKING([gperf version $gperf_version >= $min_gperf_version])
        AX_COMPARE_VERSION([$gperf_version], [ge], [$min_gperf_version],
          [AC_MSG_RESULT([yes])],
          [AC_MSG_RESULT([no])
          no_gperf=yes]
        )
        AS_IF([test "$no_gperf" = yes],
          [AC_MSG_ERROR("gperf at least version $min_gperf_version is required!")]
        )
])
