#! /bin/sh
# -*- ksh -*-

# The DefaultPrinter is not defined in a2ps-test.cfg, but that's
# no reason to dump!

. ./defs || exit 1

# grep for just the basename, as on some systems we don't get the full path
# in the error message.
(echo | $CHK -d 2>&1) | grep "$(basename $CHK):" >&5 || exit 1

exit 0
