/* psstat.c - recording information about the PostScript process
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
#include "psstat.h"
#include "jobs.h"
#include "routines.h"
#include "dsc.h"

/************************************************************************
 * Monovalued hash tables						*
 ************************************************************************/
/* Definition of the hash'd structure used for page device definitions */
typedef struct dict_entry
{
  char * key;
  char * value;
  int def;
} dict_entry;

/*
 * Used with the page device(-D), and status dict (-S) definitions 
 */
static unsigned long
key_hash_1 (void const *key)
{
  return_STRING_HASH_1 (((const dict_entry *)key)->key);
}

static unsigned long
key_hash_2 (void const *key)
{
  return_STRING_HASH_2 (((const dict_entry *)key)->key);
}

static int
key_hash_cmp (void const *x, void const *y)
{
  return_STRING_COMPARE (((const dict_entry *)x)->key,
			 ((const dict_entry *)y)->key);
}

static struct hash_table_s *
new_dict_entry_table (void)
{
  static struct hash_table_s * res;

  res = XMALLOC (hash_table);
  hash_init (res, 8,
	     key_hash_1, key_hash_2, key_hash_cmp);
  return res;
}

static void
dict_entry_add (struct hash_table_s * table,
		const char * key, const char * value, int def)
{
  dict_entry * item = XMALLOC (dict_entry);
  item->key = xstrdup (key);
  item->value = xstrdup (value);
  item->def = def;
  hash_insert (table, item);
}

static dict_entry *
dict_entry_get (struct hash_table_s * table, const char * key)
{
  struct dict_entry token;
  token.key = (char *) key;
  return (struct dict_entry *) hash_find_item (table, &token);
}

static void
dict_entry_remove (struct hash_table_s * table, const char * key)
{
  struct dict_entry * item;
  item = dict_entry_get (table, key);
  if (item)
    hash_delete (table, item);
}


/************************************************************************/
/*	Dealing with the structure (creation etc.) that will be kept	*/
/*	in JOB								*/
/************************************************************************/
/* 
 * Private information for the PS generation engine 
 */
struct ps_status *
new_ps_status (void)
{
  struct ps_status * res = XMALLOC (struct ps_status);

  /* The very first line of a PS file */
  res->magic_number = xstrdup ("%!PS-Adobe-3.0");

  /* By default, the list of page number intervals */
  res->page_label_format = xstrdup ("#!s|$p-|, |");

  /* At the beginning, no encoding dict is opened */
  res->opened_encoding = NULL;

  /* After delegating the order is no longer respected */
  res->page_are_ordered = true;

  /* for fonts etc. */
  res->needed_resources = multivalued_table_new ();

  /* Things to put in the preamble */
  res->supplied_resources = multivalued_table_new ();

  /* for setpagedevice */
  res->pagedevice = new_dict_entry_table ();

  /* PS statusdict definitions */
  res->statusdict = new_dict_entry_table ();

  /* The setups read in the files */
  res->setup = output_new ("setup");

  return res;
}

/*
 * Called for each new input session.
 */
void
initialize_ps_status (struct ps_status * status)
{
  /* This one will be malloc'd for each (physical) page.
   * Hence, do not risk to touch it, unless you want to SEGV */
/*  status->page_label = NULL;*/

  /* Reinitialize the ps status */
  status->start_page = true;
  status->start_line = true;
  status->line_continued = false;
  status->is_in_cut = false;
  status->face = Plain;
  status->face_declared = false;	/* One could think in using
					 * an extra value in ->face
					 * (eg No_face), but it gets
					 * painful with switches of
					 * encodings... */
  status->nonprinting_chars = 0;
  status->chars = 0;
  status->line = 0;
  status->column = 0;
  status->wx = 0;
}

/************************************************************************/
/*	Dealing with the various components of this structure		*/
/************************************************************************/
/*
 * Used with the page device definitions (-D)
 */
static void
dict_entry_print (void const * item, FILE * stream)
{
  const dict_entry * tok = (const dict_entry *) item;
  if (tok->def) 
    fprintf (stream, "%s::%s ", tok->key, tok->value);
  else
    fprintf (stream, "%s:%s ", tok->key, tok->value);
}

/*
 * Listing the content of a dict entry hash table
 */
static void
dict_entry_table_dump (struct hash_table_s * table, FILE * stream)
{
  int i;
  struct dict_entry ** items;

  items = (struct dict_entry **) hash_dump (table, NULL, NULL);

  for (i = 0 ; items [i] ; i++)
    dict_entry_print (items [i], stream);
  putc ('\n', stream);
}

/* Page device definitions */
void
output_pagedevice (a2ps_job * job)
{
  dict_entry ** entries = NULL;
  dict_entry ** entry;
  entries = (dict_entry **) hash_dump (job->status->pagedevice, NULL, NULL);
  
  if (!*entries) 
    return;
  
  /* Protect calls to setpagedevice through a stopped environment.
   * (Cf. PDF spec, by Adobe) */
  output (job->divertion, "\
%%%% Pagedevice definitions:\n\
countdictstack\n\
%% Push our own mark, since there can be several PS marks pushed depending\n\
%% where the failure really occured.\n\
/a2ps_mark\n\
{\n");

  /* Each Pagedevice */
  for (entry = entries ; *entry ; entry++)
    output (job->divertion, "\
%%%%BeginFeature: *%s %c%s\n\
  (<<) cvx exec /%s (%s) cvx exec (>>) cvx exec setpagedevice\n\
%%%%EndFeature\n",
      (*entry)->key, toupper ((*entry)->value[0]), 
      (*entry)->value + 1,
      (*entry)->key, (*entry)->value);

  /* Close the stopped env, and clear the stack */
  output (job->divertion, "\
} stopped\n\
%% My cleartomark\n\
{ /a2ps_mark eq { exit } if } loop\n\
countdictstack exch sub dup 0 gt\n\
{\n\
  { end } repeat\n\
}{\n\
  pop\n\
} ifelse\n");
}

/* FIXME: Find some better scheme.  But I don't want to do that before
   4.11.  This is the same routine as above, but which fputs instead of
   output. 

   This routine will only be called when only a single delegated job
   is output, therefore a2ps's prologue will not be output, therefore
   there is a very high chance (contrary to the previous item) that
   the psutils have neutralized the setpagedevice operator.  Here
   we really want to use it, hence the `systemdict /setpagedevice get exec'
   sequence instead of just `setpagdevice'.
*/

void
pagedevice_dump (FILE *stream, a2ps_job * job)
{
  dict_entry ** entries = NULL;
  dict_entry ** entry;
  entries = (dict_entry **) hash_dump (job->status->pagedevice, NULL, NULL);
  
  if (!*entries) 
    return;
  
  /* Protect calls to setpagedevice through a stopped environment.
   * (Cf. PDF spec, by Adobe) */
  fputs ("\
%% Pagedevice definitions:\n\
countdictstack\n\
% Push our own mark, since there can be several PS marks pushed depending\n\
% where the failure really occured.\n\
/a2ps_mark\n\
{\n", stream);

  /* Each Pagedevice */
  for (entry = entries ; *entry ; entry++)
    fprintf (stream, "\
%%%%BeginFeature: *%s %c%s\n\
  (<<) cvx exec /%s (%s) cvx exec (>>) cvx exec\n\
  systemdict /setpagedevice get exec\n\
%%%%EndFeature\n",
      (*entry)->key, toupper ((*entry)->value[0]), 
      (*entry)->value + 1,
      (*entry)->key, (*entry)->value);

  /* Close the stopped env, and clear the stack */
  fputs ("\
} stopped\n\
% My cleartomark\n\
{ /a2ps_mark eq { exit } if } loop\n\
countdictstack exch sub dup 0 gt\n\
{\n\
  { end } repeat\n\
}{\n\
  pop\n\
} ifelse\n", stream);
}

void
dump_requirements (FILE * stream, struct a2ps_job * job)
{
  dict_entry ** entries = NULL;
  dict_entry ** entry;
  entries = (dict_entry **) hash_dump (job->status->pagedevice, NULL, NULL);
  /* Dump only if there is something to say */
  if (*entries)
    {
      fputs ("%%Requirements: ", stream);
      for (entry = entries ; *entry ; entry++) 
	fprintf (stream, "%s ", (*entry)->key);
      putc ('\n', stream);
    }
  /* We don't want this one which breaks some collating systems
     output (job->divertion, "numcopies(%d)", job->copies);
     */
}

void
setpagedevice (a2ps_job * job, const char * key, const char * value)
{
  dict_entry_add (job->status->pagedevice, key, value, false);
}

void
delpagedevice (a2ps_job * job, const char * key)
{
  dict_entry_remove (job->status->pagedevice, key);
}

/*
 * For --list-options
 */
void
list_pagedevice (a2ps_job * job, FILE * stream)
{
  dict_entry_table_dump (job->status->pagedevice, stream);
}

/*
 * Used with the status dict definitions (-S)
 */
void
setstatusdict (a2ps_job * job, const char * key, const char * value, int def)
{
  dict_entry_add (job->status->statusdict, key, value, def);
}

void
delstatusdict (a2ps_job * job, const char * key)
{
  dict_entry_remove (job->status->statusdict, key);
}

void
output_statusdict (a2ps_job * job)
{
  dict_entry ** entries = NULL;
  dict_entry ** entry;

  entries = (dict_entry **) hash_dump (job->status->statusdict, NULL, NULL);
  if (*entries) {
    output (job->divertion, "%% Statustdict definitions:\n");
    output (job->divertion, "statusdict begin\n");
    for ( entry = entries ; *entry ; entry++)
      if ((*entry)->def) 
	output (job->divertion, "  /%s %s def\n",
		(*entry)->key, (*entry)->value);
      else
	output (job->divertion, "  %s %s\n", 
		(*entry)->value, (*entry)->key);
    output (job->divertion, "end\n");
  }
}

/*
 * For --list-options
 */
void
list_statusdict (a2ps_job * job, FILE * stream)
{
  dict_entry_table_dump (job->status->statusdict, stream);
}
