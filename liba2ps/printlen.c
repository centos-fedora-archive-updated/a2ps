/* printlen.c - return number of chars used by a printf like call
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

/*
  Note: not all the cases are implemented, so improve it
  before use it!
*/

#include <config.h>

#include "system.h"
#include "printlen.h"


static int
int_printflen (const char *format, va_list ap)
{
  const char *cp;
  size_t total_width = 0;

  for (cp = format ; *cp ; cp++)
    {
      if (*cp != '%')
	total_width++;
      else
	{
	  /* A `% ' is occuring */
	  cp++;
	  while (strchr ("-+ #0", *cp))
	    ++cp;
	  if (*cp == '*')
	    ++cp;
	  if (*cp == '.')
	    {
	      ++cp;
	      if (*cp == '*')
		++cp;
	    }
	  while (strchr ("hlL", *cp))
	    ++cp;
	  /* Currently not enough cases are covered */
	  switch (*cp)
	    {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
	    case 'c':
	      (void) va_arg (ap, int);
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      (void) va_arg (ap, double);
	      break;
	    case 's':
	      total_width += strlen (va_arg (ap, char *));
	      break;
	    case 'p':
	    case 'n':
	      (void) va_arg (ap, char *);
	      break;
	    }
	}
    }
  return (int) total_width;
}

int
vprintflen (const char *format,  va_list args)
{
  va_list ap;
  int ret;
  
  va_copy (ap, args);
  
  ret = int_printflen (format, ap);
  
  va_end(ap);
  
  return ret;
}

int
printflen (const char *format, ...)
{
  va_list args;
  int res;

  va_start (args, format);

  res = vprintflen (format, args);
  va_end (args);

  return res;
}
