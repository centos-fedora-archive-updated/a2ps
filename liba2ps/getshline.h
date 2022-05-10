/* getshline.h - read a meaningful line from a file
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

#ifndef _GETSHLINE_H_
# define _GETSHLINE_H_ 1

/* These routines are devoted to reading meaningful lines.  By
 * meaningful is meant, not empty, and not behind a COMMENTOR symbol.
 * These lines are ended by a DELIMITER, but may be continued if
 * DELIMITER is preceded by a DELIMITER_QUOTE.
 */

ptrdiff_t
getshline (char **_lineptr, size_t *_n, FILE *_stream);

ptrdiff_t
getshdelim (char **_lineptr, size_t *_n,
		    int _delimiter, int _delimiter_quote, int _commentor,
		    FILE *_stream);


/* These routines are the pending of the above routines, keeping
 * track of the meaning full line interval read.  When calling them
 * *LASTLINE should be the number of the last line read (hence 0
 * for the first line), upon return, *FIRSTLINE is the line number
 * where begun the line returned, and *LASTLINE, the line number
 * where it ended
 */

/* A basic example is available at the bottom of getshline.c */
ptrdiff_t
getshline_numbered (unsigned *_firstline, unsigned *_lastline,
                    char **_lineptr, size_t *_n,
                    FILE *_stream);

ptrdiff_t
getshdelim_numbered (unsigned *_firstline, unsigned *_lastline,
                     char **_lineptr, size_t *_n,
                     int _delimiter, int _delimiter_quote,
                     int _commentor, FILE *_stream);

#endif /* not defined(_GETSHLINE_H_) */
