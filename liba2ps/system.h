/* system.h - shared system header with the whole package
   Copyright 1988-2017 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <config.h>

/*-------------------------.
| This guy must be first.  |
`-------------------------*/

#include <alloca.h>
#define ALLOCA(t, n) (alloca (sizeof (t) * (n)))

/*-------------------------------.
| Stdio and missing prototypes.  |
`-------------------------------*/

#include <stdio.h>

/*-------------------.
| Including stdlib.  |
`-------------------*/

#include <stdlib.h>

/*--------------------.
| Including strings.  |
`--------------------*/

#include <string.h>

#define STREQ(s1, s2)		(strcmp ((s1), (s2)) == 0)
#define STRNEQ(s1, s2, n)	(strncmp ((s1), (s2), (n)) == 0)

/*---------------.
| Math headers.  |
`---------------*/

#include <math.h>
#include <errno.h>

/*-------------------.
| Ctype and family.  |
`-------------------*/

#include <ctype.h>

#include <sys/wait.h>
#include <sys/types.h>

/*---------------------------.
| Include unistd and fixes.  |
`---------------------------*/

#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>

/*---------------------------------------.
| Defining various limits on int types.  |
`---------------------------------------*/

#include <limits.h>

/* The extra casts work around common compiler bugs,
   e.g. Cray C 5.0.3.0 when t == time_t.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/*--------------------------------.
| Defining the PATHMAX some way.  |
`--------------------------------*/

#include <sys/param.h>

/*-----------------.
| Time and dates.  |
`-----------------*/

#include <time.h>
#include <sys/time.h>

#include <stdbool.h>

/*---------------------------.
| Take care of NLS matters.  |
`---------------------------*/

#include <locale.h>
#include <gettext.h>

/*----------.
| fnmatch.  |
`-----------*/

#include <fnmatch.h>

/*---------------------.
| Variadic arguments.  |
`---------------------*/

#include <stdarg.h>

#include "xalloc.h"
#include "error.h"

/* Cardinality of a static array. */

#define cardinalityof(ARRAY) (sizeof (ARRAY) / sizeof ((ARRAY)[0]))

#endif /* !defined (SYSTEM_H_) */
