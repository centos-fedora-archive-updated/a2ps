/* ssheet.h - definition of the languages style sheets
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

#ifndef _SSHEET_H_
#define _SSHEET_H_

#include "a2ps.h"
#include "darray.h"
#include "ffaces.h"
#include "hashtab.h"
#include "jobs.h"
#include "regex.h"

enum case_sensitiveness
{
  case_sensitive, case_insensitive, case_undefined
};

/*
 * Used by the scanner and parser to return a pattern,
 * which may enclose NUL
 */
struct pattern
{
  char * pattern;
  size_t len;
};

/*
 * Now, keywords, and operators share the same underlying
 * structure, because, though keywords don't need a SYMBOL field,
 * they are so close, that implementation should be the same.
 * This may be a sign that OO would have been appreciated :)
 */
struct faced_string
{
  char * string;		/* the destination string	*/
  int reg_ref;			/* there is no destination string,
				 * but the model is the REG_REF expression
				 * caught by the regexp (e.g. 1 for \1 */
  struct fface_s face;
};


/*--------------------------------------.
| A rule is composed of its lhs and rhs |
`--------------------------------------*/

struct rule
{
  char *word;
  struct re_pattern_buffer *regex;
  struct darray * rhs;
};

struct words
{
  /* darrays of struct rule * */
  struct darray * strings;	/* those which matchers are strings */
  struct darray * regexps;	/* those which matchers are regexps */
  struct rule ** min [256];
  struct rule ** max [256];
};

struct sequence
{
  struct rule * open;
  struct fface_s face;
  struct words * exceptions;		/* Exception, i.e. \" between ""  */
  struct words * close;			/* closing alternatives */
} ;

struct style_sheet
{
  /* index of the language (compare with command-line option) */
  const char * key;

  /* Nice looking name of the style */
  const char * name;

  /* Who wrote it, and when */
  const char * author;
  int version[4];

  /* What version of a2ps is required? */
  int requirement[4];

  /* Note describing the mode or the language */
  const char * documentation;

  /* Does it have ancestors (i.e., this one is an extension of
   * its ancestors).  It is a list of keys, of course */
  struct darray * ancestors;

  /* case sensitiveness for keywords and rules */
  enum case_sensitiveness sensitiveness;

  /* definition of the "words" (keywords and rules):
   * a char belonging to a first alphabet (alpha1),
   * and any number of chars belonging to the second (alpha2). */
  unsigned char alpha1 [256];
  unsigned char alpha2 [256];

  /* list of keywords for this language */
  struct words * keywords;

  /* same as keywords BUT there is no need to be preceded and followed
   * by not in alpha2. In other words, these are not "words" of the
   * alphabet, but any sequence of chars */
  struct words * operators;

  /* darray of the sequences (strings, documentations, etc.) */
  struct darray * sequences;
};

/*	Notes
 *
 * - global exceptions is made for languages such as tcl where, whereever the
 * rule appears, it must not be "understood", e.g. \" and \\ are
 * written directly.
 *
 * - local exception is made for ���! languages such as ada where "" is the
 * empty string, but appearing in a string, "" denotes ".
 */


/*
 * The faced_string
 */
struct faced_string *
faced_string_new (char * string, int reg_ref, struct fface_s fface);

/*
 * The version numbers
 */
void style_sheet_set_version (struct style_sheet * sheet,
				      const char * version_string);
int style_sheet_set_requirement (struct style_sheet * sheet,
					 const char * version_string);

/*
 * The destinations (part of rule)
 */
struct darray * rhs_new (void);
struct darray *
rhs_new_single (char * string, int reg_ref, struct fface_s fface);
void rhs_add (struct darray * dest, struct faced_string * str);
void rhs_self_print (struct darray * rhs, FILE * stream);

/*
 * Dealing with the keywords, rules and operators
 */
struct rule * rule_new (char * word,
				struct pattern * pattern,
				struct darray * destination,
				const char *filename, size_t line);
struct rule * keyword_rule_new (char * word,
					struct pattern * pattern,
					struct darray * destination,
					const char *filename, size_t line);

/*
 * Dealing with the sequences
 */
struct pattern *
new_pattern (char * pattern, size_t len);

struct sequence * sequence_new
   	(struct rule * Open,
		 struct fface_s in_face,
		 struct words * Close,
		 struct words * exceptions);
struct sequence * new_C_string_sequence (const char * delimitor);
void sequence_self_print (struct sequence * v, FILE * stream);


/*
 * Dealing with the struct words
 */
struct words *
words_new (const char * name_strings, const char * name_regexps,
		   size_t size, size_t increment);
void words_add_string (struct words * words, struct rule * rule);
void words_add_regex (struct words * words, struct rule * rule);
void words_set_no_face (struct words * words, struct fface_s face);
void words_merge_rules_unique (struct words * words,
					 struct words * new);

/*
 * Dealing with the style sheets
 */
struct style_sheet * new_style_sheet (const char * name);
void style_sheet_finalize (struct style_sheet * sheet);
void style_sheet_self_print (struct style_sheet * sheet,
				     FILE * stream);

/*
 * Dealing with the hash table for style sheets
 */
struct hash_table_s * new_style_sheets (void);
struct style_sheet * get_style_sheet (const char * name);

/************************************************************************/
/*				style selection				*/
/************************************************************************/
/*
 * List the style sheets
 */
void list_style_sheets_short (FILE * stream);
void list_style_sheets_long (FILE * strea);
void list_style_sheets_html (FILE * strea);
void list_style_sheets_texinfo (FILE * strea);

#endif /* not defined _SSHEET_H_ */
