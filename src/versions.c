/* versions.c - handling standard version numbers
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

#include <config.h>

#include "a2ps.h"
#include "versions.h"
#include "quotearg.h"
#include "routines.h"

/************************************************************************/
/*	The version handling						*/
/************************************************************************/
void
version_set_to_null (version_t version)
{
  size_t n;

  for (n = 0 ; n < VERSION_LENGTH ; n++)
    version[n] = 0;
}

bool
version_null_p (version_t version)
{
  size_t n;

  for (n = 0 ; n < VERSION_LENGTH ; n++)
    if (version[n])
      return false;
  return true;
}

int
version_cmp (version_t v1, version_t v2)
{
  int n;

  for (n = 0 ; n < VERSION_LENGTH ; n++)
    if (v1[n] < v2[n])
      return -1;
    else if (v1[n] > v2[n])
      return 1;

  return 0;
}

void
version_cpy (version_t d, version_t s)
{
  memcpy (d, s, VERSION_LENGTH * sizeof(s[0]));
}

void
version_self_print (version_t version, FILE * stream)
{
  if (version[2])
    fprintf (stream, "%d.%d%c",
	     version[0], version[1], version[2]);
  else
    fprintf (stream, "%d.%d",
	     version[0], version[1]);
}

/*
 * Return the length occupied by this version number once printed
 */
size_t
version_length (version_t version)
{
#define short_int_len(_i_) ((_i_) < 10 ? 1 : 2)
  if (version[2])
    return 2U
      + short_int_len(version[0])
      + short_int_len(version[1])
      + 1U;
  else
    return 1U
      + short_int_len(version[0])
      + short_int_len(version[1]);
}

void
version_add (version_t v1, version_t v2)
{
  int n;

  for (n = 0 ; n < VERSION_LENGTH ; n++)
    v1[n] += v2[n];
}

/*
 * Valid versions numbers are:
 * digit.digit
 * digit.digitchar
 */
void
string_to_version (const char * version_string, version_t version)
{
  char d;

  switch (sscanf (version_string, "%d.%d%c",
		  &(version[0]), &(version[1]), &d))
    {
    case 2:
      version[2] = 0;
      break;
    case VERSION_LENGTH:
      version[2] = d - 'a' + 1;
      break;
    default:
      error (1, 0,
	     _("invalid version number `%s'"), quotearg (version_string));
      break;
    };
}
