#! /bin/sh
# -*- ksh -*-

# Test if the basic options work, and produce on stdout only.

. ./defs || exit 1

for p in --version --help --list=features --list=printers --list=options \
         --list=media --list=style-sheets --list=delegations \
	 --list=macro-meta-sequences --list=encodings --list=user-options \
         --list=prologues --list=ppd;
do
  echo $p
  # There should be nothing on stderr */
  $verbose "Running a2ps $p"
  err=`$CHK $p 2>&1 >/dev/null`
  test "x$err" = x
done

exit 0
