/* lister.c - listing data in various formats
   Copyright 1998-2017 Free Software Foundation, Inc.

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

  This file still needs quite a lot of work.  In particular, its
  interaction with tterm, and the fact that the user might want to use
  another stream than the one given at initialization.

 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

#include "xalloc.h"
#include "tterm.h"
#include "lister.h"

/* Information about filling a column.  */
struct world
{
  size_t *widths;	/* Array of the widths of the columns */
  size_t width;		/* Sum of the widths of the columns */
  size_t valid_len;	/* This world respects the width of the line */
};

/* All the needed information for multicolumn listings */

struct multicol
{
  /* Number of spaces between columns. */
  size_t between;

  /* Justification of the items. */
  size_t justify;

  /* Array with information about column filledness.  Indexed by the
     number of columns minus one (used for hypothetical reasonning.
     Only one is actually selected to print.) */
  struct world *worlds;
};

/* List with separators.  See lister_print_separated for more
   information. */

struct separated
{
  /* String between items. */
  const char *separator;

  /* Spaces after the separator. */
  size_t between;

  /* String after the last item. */
  const char *final;
};


struct lister
{
  /* The tinyterm under which displays are done. */
  struct tterm *tterm;

  /* Default output stream. */
  FILE *stream;

  /* Default width function to be applied on items. */
  lister_width_t width_fn;

  /* Default print function to be applied on items. */
  lister_print_t print_fn;

  /* Number of spaces to leave at the beginning of the line. */
  size_t before;

  /* Number of spaces to leave before the end of the line. */
  size_t after;

  /* All the needed information for multicolumn listings. */
  struct multicol multicol;

  /* All the needed information for separated listings. */
  struct separated separated;
};

/* Default setting */
static struct lister lister_default =
{
  /* tterm. Lister's default tterm, is the default tterm */
  NULL,

  /* stream.  We cannot initialize statically to stdout, EGCS won't
     let us do. */
  NULL,

  /* width_fn, print_fn */
  (lister_width_t) strlen,  (lister_print_t) fputs,

  /* Before, after */
  0, 0,

  /* multicol structure. */
  {
    /* between, justify, worlds */
    2, lister_left, NULL
  },

  /* separated structure. */
  {
    /* separator, between, final. */
    ",", 2, "."
  }
};

/* Maximum number of columns ever possible for this display.  */
static size_t max_idx;

/* The minimum width of a colum is 1 character for the name. */
#define MIN_COLUMN_WIDTH	1


/* Initialize the LISTER for the STREAM. */

void
lister_initialize (struct lister *lister, FILE *stream)
{
  struct lister * l = lister ? lister : &lister_default;
  tterm_initialize (NULL, stream);
  l->stream = stream;
}

/* Assuming cursor is at position FROM, indent up to position TO.
   Use a TAB character instead of two or more spaces whenever possible.  */

#define INDENT(_from_,_to_)				\
  do {							\
    size_t from = _from_;				\
    size_t to = _to_;					\
    while (from < to)					\
      {							\
  	if (tabsize > 0 				\
  	    && to / tabsize > (from + 1) / tabsize)	\
  	  {						\
  	    putc ('\t', stream);			\
  	    from += tabsize - from % tabsize;		\
  	  }						\
  	else						\
  	  {						\
  	    putc (' ', stream);				\
  	    from++;					\
  	  }						\
      }							\
  } while (0)

/* FIXME: comment */
static void
init_worlds (struct lister * l)
{
  size_t i;
  size_t line_width = tterm_width (l->tterm);

  /* Prepare the structure to the maximum number of columns, hence do
     not depend on BEFORE, BETWEEN and AFTER which can change while
     running, but only on LINE_WIDTH which is a constant. */
  if (l->multicol.worlds == NULL)
    {
      l->multicol.worlds = XNMALLOC (line_width, struct world);
      for (i = 0; i < line_width; ++i)
	l->multicol.worlds[i].widths = XNMALLOC (i + 1, size_t);
    }

  max_idx = ((line_width
	      - l->before - l->after - l->multicol.between)
             / (MIN_COLUMN_WIDTH + l->multicol.between));
  if (max_idx == 0)
    max_idx = 1;

  for (i = 0; i < max_idx; ++i)
    {
      size_t j;

      l->multicol.worlds[i].valid_len = 1;
      l->multicol.worlds[i].width = (i + 1) * MIN_COLUMN_WIDTH;

      for (j = 0; j <= i; ++j)
	l->multicol.worlds[i].widths[j] = MIN_COLUMN_WIDTH;
    }
}

/* Set the tiny term of LISTER to TTERM.  Returns the previous value. */

struct tterm *
lister_tterm_set (struct lister * lister, struct tterm *tterm)
{
  struct lister * l = lister ? lister : &lister_default;
  struct tterm *old = l->tterm;
  l->tterm = tterm;
  return old;
}

/* Set the width of the white prefix in LISTER to SIZE.  Returns the
   previous value. */

size_t
lister_before_set (struct lister * lister, size_t size)
{
  struct lister * l = lister ? lister : &lister_default;
  size_t old = l->before;
  l->before = size;
  return old;
}

/* Set the width of the white suffix in LISTER to SIZE.  Returns the
   previous value. */

size_t
lister_after_set (struct lister * lister, size_t size)
{
  struct lister * l = lister ? lister : &lister_default;
  size_t old = l->after;
  l->after = size;
  return old;
}


static size_t
lister_vertical_format (struct lister * l,
			void **items, size_t item_number,
			lister_width_t item_width_fn,
			struct world ** line_fmt)
{
  size_t max_cols;	/* Maximum number of columns usable */
  size_t cols;
  size_t itemno;
  size_t item_width;
  struct multicol * m = &l->multicol;
  size_t available_width = tterm_width (l->tterm) - l->after - l->before;
  struct world * worlds = m->worlds;

  /* Normally the maximum number of columns is determined by the
     screen width.  But if few files are available this might limit it
     as well.  */
  max_cols = max_idx > item_number ? item_number : max_idx;

  /* Compute the maximum number of possible columns.  */
  for (itemno = 0; itemno < item_number; ++itemno)
    {
      size_t i;

      item_width = item_width_fn (items[itemno]);

      for (i = 0; i < max_cols; ++i)
	{
	  if (worlds[i].valid_len)
	    {
	      size_t effective_width = available_width - i * m->between;
	      size_t idx = itemno / ((item_number + i) / (i + 1));
	      size_t real_width = item_width;

	      if (real_width > worlds[i].widths[idx])
		{
		  worlds[i].width += (real_width - worlds[i].widths[idx]);
		  worlds[i].widths[idx] = real_width;
		  worlds[i].valid_len = worlds[i].width <= effective_width;
		}
	    }
	}
    }

  /* Find maximum allowed columns.  */
  for (cols = max_cols; cols > 1; --cols)
    {
      if (worlds[cols - 1].valid_len)
	break;
    }

  *line_fmt = &worlds[cols - 1];
  return cols;
}

void
lister_fprint_vertical (struct lister * lister, _GL_UNUSED FILE *unused,
			void **items, size_t item_number,
			lister_width_t item_width_fn,
			lister_print_t item_print_fn)
{
  struct world *line_fmt;
  size_t itemno;		/* Index size_to files. */
  size_t row;			/* Current row. */
  size_t col_width;     	/* Width of longest file name + frills. */
  size_t item_width;		/* Width of each file name + frills. */
  size_t pos;			/* Current character column. */
  size_t cols;			/* Number of files across. */
  size_t rows;			/* Maximum number of files down. */
  struct lister * l = lister ? lister : &lister_default;
  struct multicol * m = &l->multicol;
  size_t tabsize = tterm_tabsize (l->tterm);
  FILE *stream = l->stream;

  init_worlds (l);

  /* If not known, compute the actually number of elements.
     FIXME: there are probably more intelligent scheme to get item_number
     on the fly, but currently it is need in lister_vertical_format
     real soon, so just compute it right now. */
  if (item_number == (size_t) -1)
    {
      for (item_number = 0 ; items[item_number] ; item_number++)
	/* nothing */ ;
    }

  cols = lister_vertical_format (l, items, item_number,
				 item_width_fn, &line_fmt);

  /* Calculate the number of rows that will be in each column except possibly
     for a short column on the right. */
  rows = item_number / cols + (item_number % cols != 0);

  for (row = 0; row < rows; row++)
    {
      size_t col = 0;
      size_t nextpos, nextcolpos;
      itemno = row;

      /* Print the next row.  */
      pos = 0;
      nextcolpos = nextpos = l->before;
      while (1)
	{
	  col_width = line_fmt->widths[col++];
          item_width = (*item_width_fn) (items[itemno]);
	  nextpos += (col_width - item_width) * m->justify /2 ;
	  nextcolpos += col_width + m->between;

          INDENT (pos, nextpos);
          (*item_print_fn) (items[itemno], stream);

	  itemno += rows;
	  if (itemno >= item_number)
	    break;

	  pos = nextpos + item_width;
	  nextpos = nextcolpos;
	}
      putc ('\n', stream);
    }
}

/* Same as LISTER_FPRINT_VERTICAL, but using LISTER's default value
   for width_fn and print_fn. */

void
lister_print_vertical (struct lister *lister,
		       void **items, size_t item_number)
{
  struct lister *l = lister ? lister : &lister_default;
  lister_fprint_vertical (lister, NULL,
			  items, item_number,
			  l->width_fn, l->print_fn);
}


/* Listing in horizontal format.  Columns are built to minimize the
   number of needed rows.  For instance:

   +---------------------------+
   | a   bbbbb  c  dddddddd    |
   | ee  ff     g  h           |
   | i   j      k  lllllllllll |
   +---------------------------+

   */

static size_t
lister_horizontal_format (struct lister * l,
			  void **items, size_t item_number,
			  lister_width_t item_width_fn,
			  struct world **line_fmt)
{
  size_t max_cols;	/* Maximum number of columns usable */
  size_t cols;
  size_t itemno;
  size_t item_width;
  struct multicol * m = &l->multicol;
  size_t available_width = tterm_width (l->tterm) - l->after - l->before;
  struct world * worlds = m->worlds;

  /* Normally the maximum number of columns is determined by the
     screen width.  But if few files are available this might limit it
     as well.  */
  max_cols = max_idx > item_number ? item_number : max_idx;

  /* Compute the maximum file name width.  */
  for (itemno = 0; itemno < item_number; ++itemno)
    {
      size_t i;

      item_width = (*item_width_fn) (items[itemno]);

      for (i = 0; i < max_cols; ++i)
	{
	  if (worlds[i].valid_len)
	    {
	      size_t effective_width = available_width - i * m->between;
	      size_t idx = itemno % (i + 1);
	      size_t real_width = item_width;

	      if (real_width > worlds[i].widths[idx])
		{
		  worlds[i].width += (real_width - worlds[i].widths[idx]);
		  worlds[i].widths[idx] = real_width;
		  worlds[i].valid_len = worlds[i].width <= effective_width;
		}
	    }
	}
    }

  /* Find maximum allowed columns.  */
  for (cols = max_cols; cols > 1; --cols)
    {
      if (worlds[cols - 1].valid_len)
	break;
    }

  *line_fmt = &worlds[cols - 1];
  return cols;
}

/* FIXME: document */

void
lister_fprint_horizontal (struct lister * lister, _GL_UNUSED FILE *unused,
			  void **items, size_t item_number,
			  lister_width_t item_width_fn,
			  lister_print_t item_print_fn)
{
  struct world *line_fmt;
  size_t itemno;
  size_t col_width;
  size_t item_width;
  size_t cols;
  size_t pos;
  size_t nextpos, nextcolpos;
  struct lister * l = lister ? lister : &lister_default;
  struct multicol * m = &l->multicol;
  size_t tabsize = tterm_tabsize (l->tterm);
  FILE *stream = l->stream;

  init_worlds (l);

  /* If not known, compute the actually number of elements.
     FIXME: there are probably more intelligent scheme to get item_number
     on the fly, but currently it is need in lister_vertical_format
     real soon, so just compute it right now. */
  if (item_number == (size_t) -1)
    {
      for (item_number = 0 ; items[item_number] ; item_number++)
	/* nothing */ ;
    }

  cols = lister_horizontal_format (l, items, item_number,
				   item_width_fn, &line_fmt);

  /* Print first entry.  */
  pos = 0;
  nextcolpos = nextpos = l->before;

  /* Now the rest.  */
  for (itemno = 0; itemno < item_number; ++itemno)
    {
      size_t col = itemno % cols;

      item_width = strlen (items[itemno]);
      col_width = line_fmt->widths[col];

      if (col == 0 && itemno != 0)
	{
	  putc ('\n', stream);
	  pos = 0;
	  nextcolpos = nextpos = l->before;
	}

      nextpos += (col_width - item_width) * m->justify / 2;
      INDENT (pos, nextpos);
      (*item_print_fn) (items [itemno], stream);
      pos = nextpos + item_width;
      nextcolpos += col_width + m->between;
      nextpos = nextcolpos;
    }
  putc ('\n', stream);
}


/* Same as LISTER_FPRINT_HORIZONTAL, but using LISTER's default value
   for width_fn and print_fn. */

void
lister_print_horizontal (struct lister *lister,
			 void **items, size_t item_number)
{
  struct lister *l = lister ? lister : &lister_default;
  lister_fprint_horizontal (lister, NULL,
			   items, item_number,
			   l->width_fn, l->print_fn);
}

/*
   Listing thing separated by spaces and strings.  For instance:

   +------------------------------+
   |  one, two, three, four, five,|
   |  six, seven, eight, nine,    |
   |  ten, eleven.                |
   +------------------------------+

   Note that in the first line, `,' is written though there is not
   enough room for the ` ' behind.  Note also that `ten' is not
   written on the second line, because there is no room enough for the
   `,'.

   This corresponds to the settings: before = 2, after = 0,
   between_width = 1, between_string = `,' after_string = `.'.
*/

void
lister_fprint_separated (struct lister * lister, _GL_UNUSED FILE *unused,
			 void **items, size_t item_number,
			 lister_width_t item_width_fn,
			 lister_print_t item_print_fn)
{
  size_t itemno;
  size_t pos;
  struct lister * l = lister ? lister : &lister_default;
  struct separated * s = &l->separated;
  size_t final_width = strlen (s->final);
  size_t separator_width = strlen (s->separator);
  size_t tabsize = tterm_tabsize (l->tterm);
  FILE *stream = l->stream;

  /* The BEFORE prefix must not be `smartly' hidden in line_width,
     since INDENT needs the absolute position on the screen in order
     to synchronize correctly the tabulations.  */
  size_t line_width = tterm_width (l->tterm) - l->after;
  size_t old_pos;

  pos = l->before;
  INDENT (0, pos);

  for (itemno = 0;
       (item_number != (size_t) -1
	? (itemno < item_number)
	: items[itemno] != NULL);
       itemno++)
    {
      old_pos = pos;

      pos += (*item_width_fn) (items[itemno]);
      pos += (itemno + 1 < item_number) ? separator_width : final_width;

      if (itemno)
	{
	  if (pos + s->between > line_width)
	    {
	      putc ('\n', stream);
	      INDENT (0, l->before);
	      pos = pos - old_pos + l->before;
	    }
	  else
	    {
	      INDENT (old_pos, old_pos + s->between);
	      pos += s->between;
	    }
	}
      (*item_print_fn) (items[itemno], stream);
      fputs ((itemno + 1 < item_number) ? s->separator : s->final, stream);
    }
  putc ('\n', stream);
}

/* Same as LISTER_FPRINT_SEPARATED, but using LISTER's default value
   for width_fn and print_fn. */

void
lister_print_separated (struct lister *lister,
			void **items, size_t item_number)
{
  struct lister *l = lister ? lister : &lister_default;
  lister_fprint_separated (lister, NULL,
			   items, item_number,
			   l->width_fn, l->print_fn);
}


#ifdef TEST

const char * program_name = "lister";

int
main (int argc, char **argv)
{
  static const char * liste[] =
  {
    "1", "22", "333", "4444", "55555", "666666", "7777777",
    "88888888", "999999999"
  };
  lister_init (stdout);

  lister_fprint_separated (NULL,
			   liste, sizeof (liste) / sizeof (*liste),
			   strlen, fputs);
  return 0;
}
#endif
