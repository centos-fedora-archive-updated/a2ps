/* document.c - handle report of various documenting formats
   Copyright 1988-2022 Free Software Foundation, Inc.

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
#include "document.h"
#include "xstrrpl.h"

/* The rules for authors to Texinfo */
#define AUTHORS_TO_TEXINFO 	\
   "@", "@@",			\
   NULL

/* The rules to convert documentation to another format */
/* 1. Plain ASCII */
#define DOC_TO_PLAIN \
  "url(", "",		\
  ")url(", " (",	\
  ")url", ")",		\
  "samp(", "`",		\
  ")samp", "'",		\
  "emph(", "*",		\
  ")emph", "*",		\
  "code(", "",		\
  ")code", "",		\
  "@example\n",		"",	\
  "@end example\n", 	"",	\
  "@end example", 	"",	/* Just in case */	\
  "@itemize\n",		"",	\
  "@end itemize", 	"",	\
  "@item\n",		" - ",	\
  "@@",			"@",	\
  NULL

/* 2. Towards HTML */
#define DOC_TO_HTML \
  "url(", "<a href=\"",	\
  ")url(", "\">",	\
  ")url", "</a>",	\
  "emph(", "<emph>",	\
  ")emph", "</emph>'",	\
  "samp(", "`<code>",	\
  ")samp", "</code>'",	\
  "code(", "<code>",	\
  ")code", "</code>",	\
  "@example",		"<pre>",	\
  "@end example", 	"</pre>",	\
  "@itemize",		"<ul>",	\
  "@end itemize", 	"</ul>",	\
  "@item\n",		"<li>",	\
  "@@",			"@",	\
  NULL

/* 3. Towards Texinfo */
#define DOC_TO_TEXINFO \
  "emph(", "@emph{",	\
  ")emph", "}",		\
  "samp(", "@samp{",	\
  ")samp", "}",		\
  "code(", "@code{",	\
  ")code", "}",		\
  "url(", "@href{",	\
  ")url(", ",",		\
  ")url", "}",		\
  "@itemize",		"@itemize @minus",	\
  NULL


/************************************************************************/
/*      The authors list handling                                       */
/************************************************************************/
/*
 * Split the authors and print them on STREAM using AUTHOR_FMT
 * (which %1s is clean name, and %2s is the email), separated
 *  with BETWEEN).  They must be separated with ',', and
 * use this convention "First Last <email>".
 */
static void
authors_print (const char * authors, FILE * stream,
	       const char *before,
	       const char *author_fmt, const char *between,
	       const char *after)
{
  char *cp, *author, *email;
  const char *authors_end;
  bool first = true;

  if (!authors)
    return;

  /* Work on a copy */
  astrcpy (cp, authors);
  authors_end = cp + strlen (authors);
  cp = strtok (cp, ",");

  while (cp)
    {
      author = cp;
      email = author + strcspn (author, "<");
      *(email - 1) = '\0';
      email++;
      if (email > authors_end)
        return;
      *(email + strcspn (email, ">")) = '\0';
      *(email - 1) = '\0';
      if (first)
        {
          fputs (before, stream);
          first = false;
        }
      else
        fputs (between, stream);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
      fprintf (stream, author_fmt, author, email);
#pragma GCC diagnostic pop
      cp = strtok (NULL, ",");
    }
  if (!first)
    fputs (after, stream);
}

/*
 * Plain : nothing to change
 */
void
authors_print_plain (const char * authors, FILE * stream,
		     const char *before)
{
  authors_print (authors, stream,
		 before, "%s <%s>", ", ", ".\n");
}

/*
 * HTML : nothing to change
 */
void
authors_print_html (const char * authors, FILE * stream,
		    const char *before)
{
  authors_print (authors, stream,
		 before,
		 "<a href=\"mailto:%2$s\">%1$s</a>", ", ",
		 ".\n");
}

/*
 * Plain : nothing to change
 */
void
authors_print_texinfo (const char * authors, FILE * stream,
		       const char *before)
{
  /* We must quote the @ of the emails */
  char *cp = xvstrrpl ((const char *) authors,
                       AUTHORS_TO_TEXINFO);

  /* Don't print the email, that makes too wide output. */
  authors_print (cp, stream,
		 before, "%s", ", ", ".\n");
}
/************************************************************************/
/*      The documentation handling                                      */
/************************************************************************/
/* 1. Plain ASCII */
void
documentation_print_plain (const char * documentation,
			   const char *format, FILE * stream)
{
  if (!documentation)
    return;

  char *cp = xvstrrpl (documentation, DOC_TO_PLAIN);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  fprintf (stream, format, cp);
#pragma GCC diagnostic pop
}

/* 2. Towards HTML */
void
documentation_print_html (const char * documentation,
			  const char *format, FILE * stream)
{
  if (!documentation)
    return;

  char *cp = xvstrrpl (documentation, DOC_TO_HTML);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  fprintf (stream, format, cp);
#pragma GCC diagnostic pop
}

/* 3. Towards Texinfo */
void
documentation_print_texinfo (const char * documentation,
			     const char *format, FILE * stream)
{
  if (!documentation)
    return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  char *cp = xvstrrpl (documentation, DOC_TO_TEXINFO);
  fprintf (stream, format, cp);
#pragma GCC diagnostic pop
}
