/* dsc.c - recording information about the PostScript resources
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
#include "dsc.h"
#include "jobs.h"
#include "routines.h"
#include "str_ht.h"

/************************************************************************
 * Multivalued hash tables						*
 ************************************************************************/
typedef struct multivalued_entry {
  char * key;			/* E.g., "font", "color" 		*/
  struct string_htable * entries;	/* E.g., "Courier", "Helvetica-Bold"	*/
} multivalued_entry;

static unsigned long
mv_key_hash_1 (struct multivalued_entry *key)
{
  return_STRING_HASH_1 (key->key);
}

static unsigned long
mv_key_hash_2 (struct multivalued_entry *key)
{
  return_STRING_HASH_2 (key->key);
}

static int
mv_key_hash_cmp (struct multivalued_entry *x, struct multivalued_entry *y)
{
  return_STRING_COMPARE (x->key, y->key);
}

/*
 * Create and return a new multivaluated_entry, with key TYPE.
 */
static struct multivalued_entry *
multivalued_entry_new (const char * type)
{
  struct multivalued_entry * res = XMALLOC (struct multivalued_entry);
  res->key = xstrdup (type);
  res->entries = string_htable_new ();
  return res;
}

/*
 * Return the multivalued_entry related to TYPE in TABLE
 * if there is, NULL otherwise.
 */
static struct multivalued_entry *
multivalued_entry_get (struct hash_table_s * table, const char * type)
{
  static struct multivalued_entry token, * res;

  token.key = (char *) type;
  res = (struct multivalued_entry *) hash_find_item (table, &token);
  return res;
}

/*
 * Add a new multivalued_entry with TYPE, if necessary
 */
static void
multivalued_entry_add (struct hash_table_s * table,
		       struct multivalued_entry * item)
{
  hash_insert (table, item);
}

/*
 * In the macro table TABLE, get the sub_table having TYPE (create
 * if necessary), and in this sub table, store a malloc'd copy of VALUE
 */
static void
multivalued_entry_add_couple (hash_table * table,
			      const char * type, const char * value)
{
  struct multivalued_entry * sub_table;

  sub_table = multivalued_entry_get (table, type);
  if (sub_table == NULL) {
    sub_table = multivalued_entry_new (type);
    multivalued_entry_add (table, sub_table);
  }

  string_htable_add (sub_table->entries, value);
}

/*
 * Return the sub hash_table corresponding to TYPE in TABLE
 * NULL if none
 */
static struct string_htable *
multivalued_entry_get_sub_table (struct hash_table_s * table,
				 const char * type)
{
  struct multivalued_entry * item;
  item = multivalued_entry_get (table, type);
  if (!item)
    return NULL;
  return item->entries;
}

/*
 * Return the entries of type TYPE and value VALUE in TABLE
 * if there is.  NULL otherwise
 */
static const char *
multivalued_entry_get_sub_item (struct hash_table_s * table,
				const char * type,
				const char * value)
{
  struct string_htable * sub_table;

  sub_table = multivalued_entry_get_sub_table (table, type);
  if (!sub_table)
    return NULL;
  return string_htable_get (sub_table, value);
}

struct hash_table_s *
multivalued_table_new (void)
{
  static struct hash_table_s * res;

  res = XMALLOC (hash_table);
  hash_init (res, 8,
	     (hash_func_t) mv_key_hash_1,
	     (hash_func_t) mv_key_hash_2,
	     (hash_cmp_func_t) mv_key_hash_cmp);
  return res;
}

/************************************************************************
 * Multivalued hash tables						*
 ************************************************************************/
/*
 * Is this resource already recorded?
 */
int
exist_resource (a2ps_job * job, const char * key, const char * value)
{
  return (multivalued_entry_get_sub_item (job->status->needed_resources,
					  key, value)
	  != NULL);
}

/*
 * Used to record the requirements needed
 */
void
add_supplied_resource (a2ps_job * job, const char * key, const char * value)
{
  multivalued_entry_add_couple (job->status->supplied_resources, key, value);
}

static void
multivalued_entry_dump (FILE * stream, int first,
			const char * fmt_first, const char * fmt_others,
			struct multivalued_entry * entry)
{
  char ** values;
  int i;

  /* Get all the values in a malloc'd storage.
   * We sort them because:
   * 1. it looks better,
   * 2. fewer sources of differences in regression tests */
  values = (char **) string_htable_dump_sorted (entry->entries);
  for (i = 0 ; values[i] ; i++) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    fprintf (stream, first ? fmt_first : fmt_others, entry->key, values[i]);
#pragma GCC diagnostic pop
    first = false;
  }
}

/*
 * Specify the needed resources to the PS prologue
 */
void
dump_supplied_resources (FILE * stream, a2ps_job * job)
{
  int i;

  multivalued_entry ** list;
  list = ((multivalued_entry **)
	  hash_dump (job->status->supplied_resources, NULL, NULL));

  for (i = 0 ; list [i] ; i ++)
    /* i is used as a clue that it is the first */
    multivalued_entry_dump (stream, i == 0,
			    "%%%%DocumentSuppliedResources: %s %s\n",
			    "%%%%+ %s %s\n",
			    list [i]);
}

/*
 * Used to record the requirements needed
 */
void
add_needed_resource (a2ps_job * job, const char * key, const char * value)
{
  multivalued_entry_add_couple (job->status->needed_resources, key, value);
}

/*
 * Returned the needed resource if it is known,
 * NULL otherwise
 */
static const char *
needed_resource_get (a2ps_job * job, const char * key, const char * value)
{
  return multivalued_entry_get_sub_item (job->status->needed_resources,
					 key, value);
}

/*
 * Dump the needed resources _BUT_ the colors
 */
void
dump_needed_resources (FILE * stream, a2ps_job * job)
{
  int i;
  int first = 1;

  multivalued_entry ** list;
  list = ((multivalued_entry **)
	  hash_dump (job->status->needed_resources, NULL, NULL));

  for (i = 0 ; list [i] ; i ++)
    {
      /* Don't print the colors, because they have another section */
      if (STREQ (list [i]-> key, "color")
	  /* nor files, since they are yet included */
	  || STREQ (list [i]-> key, "file"))
	continue;

      multivalued_entry_dump (stream, first,
			      "%%%%DocumentNeededResources: %s %s\n",
			      "%%%%+ %s %s\n",
			      list [i]);
      first = false;
    }
}

/*
 * Colors used by the document
 */
void
add_process_color (a2ps_job * job, const char * value)
{
  multivalued_entry_add_couple (job->status->needed_resources,
				"color", value);
}

/*
 * Dump the needed colors
 */
void
dump_process_color (FILE * stream, a2ps_job * job)
{
  struct string_htable * color_table;

  color_table = multivalued_entry_get_sub_table (job->status->needed_resources,
						 "color");

  if (color_table)
    {
      int i;
      char ** colors = (char **) string_htable_dump_sorted (color_table);

      if (*colors != NULL) {
	fputs ("%%DocumentProcessColors: ", stream);
	for (i = 0 ; colors [i] ; i++)
	  fprintf (stream, "%s ", colors [i]);
	putc ('\n', stream);
      }
    }
}

/************************************************************************/
/*	Handling the fonts						*/
/************************************************************************/
/*
 * We will need this fonts.
 * Depending whether it is part of the 13 standard fonts, consider
 * it to be a Needed or an IncludedResource.
 */
void
add_required_font (a2ps_job * job, const char * name)
{
  if (a2ps_printers_font_known_p (job->printers, name))
    {
      /* This is a regular ps fonts.
       * `Needed' it, and `Include" it.
       * We do it only if not yet done to avoid multiple %%Include */
      if (!needed_resource_get (job, "font", name)) {
	add_needed_resource (job, "font", name);
	output (job->divertion, "%%%%IncludeResource: font %s\n", name);
      }
    } else {
      /* This is not a known font.
       * `Supplie' it, and include it */
      add_supplied_resource (job, "font", name);
    }
}

/*
 * Return a malloc'd char ** in which are stored
 * the required_fonts (if there are, NULL otherwise)
 */
char **
required_fonts_get (a2ps_job * job)
{
  struct string_htable * font_table;
  font_table =
    multivalued_entry_get_sub_table (job->status->supplied_resources,
				     "font");

  if (font_table)
    return (char **) string_htable_dump_sorted (font_table);
  return NULL;
}

/*
 * Dump the setup code read in the various prologue (.pro and .ps)
 * files.  The hard part is that we don't want to dump too
 * many definitions of fonts, to avoid running out of memory on
 * too old PS level 1 printers.
 * Nevertheless, I still wait for somebody to tell me if this is
 * really needed (useful is sure, needed is not)
 */
void
dump_setup (FILE * stream, a2ps_job * job)
{
  output_dump (job->status->setup, stream);
}
