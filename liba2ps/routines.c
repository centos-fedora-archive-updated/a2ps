/* routines.c - general use routines
   Copyright 1995-2017 Free Software Foundation, Inc.

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
#include "routines.h"
#include "message.h"
#include "quotearg.h"


/*
 * Convert a list of string of valid chars to an yes/no array
 */
void
string_to_array (unsigned char arr[256], const unsigned char * string)
{
  int c;

  for (c = 0 ; c < 256 ; c++)
    arr [c] = false;
  for ( /* nothing */ ; *string ; string ++)
    arr [*string] = true;
}

/*
 * return true iff there are no upper case chars
 */
int
is_strlower (const char * string)
{
  for (/* skip */; *string != '\0'; string++)
    if (isupper(*string))
      return false;
  return true;
}

/* Copy the LEN first characters of SRC into DST in lower case.
   DST[LEN] is set to \0.  */

static inline char *
_strncpylc (char *dst, const char *src, size_t len)
{
  for (size_t i = 0 ; i < len ; i++)
    dst[i] = (char) tolower (src[i]);
  dst[len] = '\0';
  return dst;
}

char *
strnlower (char *string, size_t len)
{
  return _strncpylc (string, string, len);
}

char *
strlower (char *string)
{
  return _strncpylc (string, string, strlen (string));
}

char *
strcpylc (char *dst, const char *src)
{
  return _strncpylc (dst, src, strlen (src));
}

/*
 * Count the number of occurrence of C in S
 */
int
strcnt (unsigned char *s, unsigned char c)
{
  int res;
  for (res = 0 ; *s ; s++)
    if (*s == c)
      res++;
  return res;
}

/*
 * Extract a substring for START, of LENGTH, making sure to
 * set the trailing '\0' (return pos of \0)
 */
char *
strsub (char * dest, const char * string, size_t start, size_t length)
{
  char * end = stpncpy (dest, string + start, length);
  *end = '\0';
  return end;
}

/*
 * fopen, but exits on failure
 */
FILE *
xfopen (const char * filename, const char * rights, const char * format)
{
  FILE * res;

  message (msg_file,
	   (stderr, "%s-fopen (%s)\n", rights, quotearg (filename)));
  res = fopen (filename, rights);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  if (!res)
    error (1, errno, format, quotearg (filename));
#pragma GCC diagnostic pop
  return res;
}

FILE *
xrfopen (const char * filename)
{
  return xfopen (filename, "r", _("cannot open file `%s'"));
}

FILE *
xwfopen (const char * filename)
{
  return xfopen (filename, "w", _("cannot create file `%s'"));
}



/*
 * Like popen, but exit upon failure
 */
FILE *
xpopen (const char * filename, const char * rights, const char * format)
{
  FILE * res;

  message (msg_file,
	   (stderr, "%s-popen (%s)\n", rights, filename));
  res = popen (filename, rights);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  if (!res)
    error (1, errno, format, quotearg (filename));
#pragma GCC diagnostic pop
  return res;
}

FILE *
xrpopen (const char * filename)
{
  return xpopen (filename, "r", _("cannot open a pipe on `%s'"));
}

FILE *
xwpopen (const char * filename)
{
  return xpopen (filename, "w", _("cannot open a pipe on `%s'"));
}

/*
 * Copy the content of IN into OUT
 */
void
streams_copy (FILE * in, FILE * out)
{
  size_t read_length;
  char buf [BUFSIZ];

  while ((read_length = fread (buf, sizeof (char), sizeof (buf), in)))
    fwrite (buf, sizeof (char), read_length, out);
}

/*
 * Dump the content of the file FILENAME onto STREAM.
 * Used when honoring a subcontract.
 */
void
stream_dump (FILE * stream, const char * filename)
{
  FILE * fp;

  message (msg_tool | msg_file, (stderr, "Dumping file `%s'\n", filename));

  fp = xrfopen (filename);
  streams_copy (fp, stream);
  fclose (fp);
}

/*
 * Unlink the file FILENAME.
 */
void
unlink2 (PARAM_UNUSED void * dummy, const char * filename)
{
  message (msg_tool | msg_file, (stderr, "Unlinking file `%s'\n", filename));

  /* Don't complain if you can't unlink.  Who cares of a tmp file? */
  unlink (filename);
}

/*
 * Securely generate a temp file, and make sure it gets
 * deleted upon exit.
 */
static char **	tempfiles;
static unsigned	ntempfiles;

static void
cleanup_tempfiles(void)
{
	while (ntempfiles--)
		unlink(tempfiles[ntempfiles]);
}

char *
safe_tempnam(const char *pfx)
{
	char	*filename;
        const char *dirname;
	int	fd;

	if (!(dirname = getenv("TMPDIR")))
		dirname = "/tmp";

	tempfiles = (char **) realloc(tempfiles,
			(ntempfiles+1) * sizeof(char *));
	if (tempfiles == NULL)
		return NULL;

	filename = malloc(strlen(dirname) + strlen(pfx) + sizeof("/XXXXXX"));
	if (!filename)
		return NULL;

	sprintf(filename, "%s/%sXXXXXX", dirname, pfx);

	if ((fd = mkstemp(filename)) < 0) {
		free(filename);
		return NULL;
	}
	close(fd);

	if (ntempfiles == 0)
		atexit(cleanup_tempfiles);
	tempfiles[ntempfiles++] = filename;

	return filename;
}
