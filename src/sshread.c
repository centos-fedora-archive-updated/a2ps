/* sshread.c - routines of input, and formatting according to the styles
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
#include "sshread.h"
#include "ssheet.h"
#include "routines.h"
#include "buffer.h"
#include "jobs.h"
#include "fjobs.h"
#include "psgen.h"
#include "quotearg.h"

/*
 * Use the information offered by main.c
 */
extern struct a2ps_job *job;

/*
 * Shortcut to call regex upon a buffer, and store in a token
 */
#define buffer_match(buffer,regex,token)                                \
  re_match (regex,                                                      \
            (char *) buffer->value, (regoff_t) buffer->len, (regoff_t) buffer->curr, \
	     token->registers)

/*
 * Structure in which is stored the result of a parsing
 */
struct token
  {
    struct re_registers *registers;
    struct darray *rhs;
  };

#define token_dest(_i_)	\
    ((struct faced_string *) token->rhs->content[_i_])

#define token_dest_fface(_i_)	\
    (token_dest(_i_)->face)

#define token_dest_fflags(_i_)	\
    (fface_get_flags(token_dest_fface(_i_)))

static inline struct token *
token_new (void)
{
  struct token *res = XMALLOC (struct token);
  res->registers = XMALLOC (struct re_registers);
  res->registers->start = XNMALLOC (30, regoff_t);
  res->registers->end = XNMALLOC (30, regoff_t);
  return res;
}

static inline struct darray *
rhs_plain_new (void)
{
  return rhs_new_single (NULL, 0, Plain_fface);
}

/* Where the token and its attributes are stored */
static struct token *token = NULL;
#define token_set_registers(_start_, _len_)                     \
  do {                                                          \
   token->registers->start [0] = (regoff_t) _start_;            \
   token->registers->end [0] = (regoff_t) (_start_ + _len_);    \
 } while (0)

#define token_start(_i_)		\
  token->registers->start [token_dest(_i_)->reg_ref]

#define token_end(_i_)		\
  token->registers->end [token_dest(_i_)->reg_ref]

static struct darray *plain_rhs = NULL;

/****************************************************************/
/*              pretty printing service routines                */
/****************************************************************/
/*
 * Eat characters as long as their in the 2nd alphabet
 * and we are in the buffer.
 */
static inline void
match_word (buffer_t * buffer, struct style_sheet *sheet)
{
  size_t start = buffer->curr;

  do
    buffer->curr++;
  while (sheet->alpha2[(unsigned char) *(buffer->content + buffer->curr)]
	 && !buffer_is_empty (buffer));

  token->rhs = plain_rhs;
  token_dest (0)->face = Plain_fface;
  token->registers->start[0] = (regoff_t) start;
  token->registers->end[0] = (regoff_t) buffer->curr;
}

/****************************************************************/
/*                      lexical analysis routines               */
/****************************************************************/
#define word_regexp(_i_)	\
   (((struct rule *) words->regexps->content[_i_]))
/*
 * Return true if there is a element of WORDS which keywords-match
 * current point of BUFFER.  Fill TOKEN with the matching part.
 */
static inline int
match_keyword (buffer_t * buffer,
	       struct words *words,
	       unsigned char * alphabet)
{
  struct rule **key;
  char *string = buffer->value + buffer->curr;
  int i;
  int res;

  /* First try the words */
  if (words->min[(unsigned) *string])
    for (key = words->max[(unsigned) *string]
         ; words->min[(unsigned) *string] <= key
	 ; key--)
      {
	if (strprefix ((*key)->word, string)
	    && !alphabet[(unsigned char) string[strlen ((*key)->word)]])
	  {
	    token->rhs = (*key)->rhs;
	    token_set_registers (buffer->curr, strlen ((*key)->word));
	    buffer->curr += strlen ((*key)->word);
	    return 1;
	  }
      }

  /* Then the regexps, in reversed order (in order to take the
   * _last_ definition */
  for (i = (int) words->regexps->len - 1; i >= 0; i--)
    {
      res = buffer_match (buffer, word_regexp (i)->regex, token);
      switch (res)
	{
	case -2:
	  fprintf (stderr, "An error occured while matching\n");
	  break;
	case -1:
	  continue;
	default:
	  token->rhs = word_regexp (i)->rhs;
	  buffer->curr += (size_t) res;
	  return 1;
	}
    }

  /* Report that nothing matches */
  return 0;
}

/*
 * Return true if there is a element of WORDS which operators-match
 * current point of BUFFER.  Fill TOKEN with the matching part.
 */
static int
match_operator (buffer_t * buffer,
		struct words *words)
{
  struct rule **key;
  char *string = buffer->value + buffer->curr;
  int i;
  int res;

  /* First the words */
  if (words->min[(unsigned char) *string])
    for (key = words->max[(unsigned char) *string]
         ; words->min[(unsigned char) *string] <= key
	 ; key--)
      {
	if (strprefix ((*key)->word, string))
	  {
	    token->rhs = (*key)->rhs;
	    token_set_registers (buffer->curr, strlen ((*key)->word));
	    buffer->curr += strlen ((*key)->word);
	    return 1;
	  }
      }

  /* Then the regexps, in reversed order (in order to take the
   * _last_ definition */
  for (i = (int) words->regexps->len - 1; i >= 0; i--)
    {
      res = buffer_match (buffer, word_regexp (i)->regex, token);
      switch (res)
	{
	case -2:
	  fprintf (stderr, "An error occured while matching\n");
	  break;
	case -1:
	  continue;
	default:
	  token->rhs = word_regexp (i)->rhs;
	  buffer->curr += (size_t) res;
	  return 1;
	}
    }

  /* Report failure */
  return 0;
}

/*
 * If buffer+*curr begins with a sequence, return that sequence.
 * Otherwise NULL
 */
#ifdef SEQ
#undef SEQ
#endif
#define SEQ(_i_) 	\
   ((struct sequence *) sheet->sequences->content [i])
static inline struct sequence *
match_sequence (buffer_t * buffer, struct style_sheet *sheet)
{
  int i;
  int res;
  char *string = buffer->value + buffer->curr;

  /* In reversed order (in order to take the _last_ definition */
  for (i = (int) sheet->sequences->len - 1; i >= 0; i--)
    {
      if (SEQ (i)->open->regex)
	{
	  /* The regexp patterns */
	  res = buffer_match (buffer, SEQ (i)->open->regex, token);
	  switch (res)
	    {
	    case -2:
	      fprintf (stderr, "An error occured while matching\n");
	      break;
	    case -1:
	      continue;
	    default:
	      token->rhs = SEQ (i)->open->rhs;
	      buffer->curr += (size_t) res;
	      return SEQ (i);
	    }
	}
      else
	{
	  /* It's a string */
	  if (strprefix (SEQ (i)->open->word, string))
	    {
	      token_set_registers (buffer->curr, strlen (SEQ (i)->open->word));
	      token->rhs = SEQ (i)->open->rhs;
	      buffer->curr += strlen (SEQ (i)->open->word);
	      return SEQ (i);
	    }
	}
    }
  return NULL;
}

/*
 * Put in token the token recognized.
 * The number of token read, 0 if nothing left
 */
static inline int
ssh_get_token (buffer_t * buffer, struct style_sheet *sheet)
{
  static int return_to_plain = false;
  /* NULL if not in a sequence currently */
  static struct sequence *sequence = NULL;

  if (buffer_is_empty (buffer))
    {
      buffer_get (buffer);

      /* We don't trust liba2ps for the line numbers, because
       * if a2ps skips some lines (e.g., --strip-level, or INVISIBLE),
       * liba2ps will number upon output lines, not input lines,
       * which is what is expected */
      (CURRENT_FILE (job))->lines = buffer->line;

      if (buffer->len == 0)
	{
	  /* end of file: reset values */
	  /* If this is a new file, it must not depend on the trailling
	   * parameters of the previous file */
	  sequence = NULL;
	  return_to_plain = false;
	  return 0;
	}
    }

  if (return_to_plain)
    {
      return_to_plain = false;
      token->rhs = plain_rhs;
      token_dest (0)->face = Plain_fface;
    }

  if (sequence)
    {
      /* escape: not converted when in a sequence */
      if (match_operator (buffer, sequence->exceptions))
	return 1;
      /* end of sequence ? */
      if (match_operator (buffer, sequence->close))
	{
	  return_to_plain = true;
	  sequence = NULL;
	  return 1;
	}
      /* We are in a sequence not to be closed yet.
       * Advance of 1 char */
      token->rhs = plain_rhs;
      token_dest (0)->face = sequence->face;
      token->registers->start[0] = (regoff_t) buffer->curr++;
      token->registers->end[0] = (regoff_t) buffer->curr;
      return 1;
    }
  else
    {				/* (not in sequence) */
      if ((sequence = match_sequence (buffer, sheet)))
	return 1;
      else if (sheet->alpha1[(unsigned char) buffer->content[buffer->curr]])
	{
	  /* we are in a word since this was a char belonging to the
	   * first alphabet */
	  if (match_keyword (buffer, sheet->keywords, sheet->alpha2)
	      || match_operator (buffer, sheet->operators))
	    {
	      return_to_plain = true;
	      return 1;
	    }
	  else
	    {
	      /* since some characters may be used inside an identifier
	       * (eg, x' = x in claire) but can also be used to open
	       * a special sequence (eg, 'x' in claire), then we must read
	       * the whole word, and print in.
	       */
	      match_word (buffer, sheet);
	      return 1;
	    }
	}
      else if (match_operator (buffer, sheet->operators))
	{
	  return_to_plain = true;
	  return 1;
	}
    }

  /* We did not recognize something special */
  token->rhs = plain_rhs;
  token->registers->start[0] = (regoff_t) buffer->curr++;
  token->registers->end[0] = (regoff_t) buffer->curr;
  return 1;
}

#define GRAB_TAG(_tag_)                                          \
  do {                                                           \
    strncat (_tag_,                                              \
             buffer->content + token_start (i),                  \
             (unsigned) (token_end (i) - token_start (i)));      \
  } while (0)

/*
 * ssh-Pretty print a file to postscript
 */
void
ssh_print_postscript (struct a2ps_job *Job,
		      buffer_t * buffer,
		      struct style_sheet *sheet)
{
  struct fface_s fface;
  /* To grab the encoding switching instruction */
  char bufenc[512];
  int grabbing_encoding = false;
  size_t i;

  *bufenc = '\0';
  fface = Plain_fface;

  /* I must do this because of the broken handling of the registers in
   * regex.  Until I find a way to ensure enough place in the
   * registers, without having to trust regex. */
  if (!token)
    {
      token = token_new ();
      plain_rhs = rhs_plain_new ();
    }

  while (ssh_get_token (buffer, sheet) != 0)
    for (i = 0; i < token->rhs->len; i++)
      {
	/* Is a new face ? */
	if (!fface_squ (fface, token_dest_fface (i)))
	  {
	    /* Reset dynamic markers */
	    if (token_dest_fflags (i) & ff_Tag1)
	      *Job->tag1 = '\0';
	    if (token_dest_fflags (i) & ff_Tag2)
	      *Job->tag2 = '\0';
	    if (token_dest_fflags (i) & ff_Tag3)
	      *Job->tag3 = '\0';
	    if (token_dest_fflags (i) & ff_Tag4)
	      *Job->tag4 = '\0';
	    if (grabbing_encoding
		&& (!(token_dest_fflags (i) & ff_Encoding)))
	      {
		/* Grabbing of the encoding name is completed */
		struct encoding *newenc;
		newenc = get_encoding_by_alias (job, bufenc);
/*          encoding_build_faces_wx (job, newenc); */
		if (!newenc)
		  error (0, 0, _ ("unknown encoding `%s', ignored"),
			 quotearg (bufenc));
		else
		  ps_switch_encoding (Job, newenc);
		*bufenc = '\0';
		grabbing_encoding = false;
	      }
	    fface = token_dest (i)->face;
	  }

	/* See if there are some information to grab */
	if (token_dest (i)->string)
	  {
	    if (fface_get_flags (fface) & ff_Tag1)
	      strcat (Job->tag1, token_dest (i)->string);
	    if (fface_get_flags (fface) & ff_Tag2)
	      strcat (Job->tag2, token_dest (i)->string);
	    if (fface_get_flags (fface) & ff_Tag3)
	      strcat (Job->tag3, token_dest (i)->string);
	    if (fface_get_flags (fface) & ff_Tag4)
	      strcat (Job->tag4, token_dest (i)->string);
	    /* Grab the dynamic encodings */
	    if (fface_get_flags (fface) & ff_Encoding)
	      {
		grabbing_encoding = true;
		strcat (bufenc, token_dest (i)->string);
	      }
	  }
	else
	  {
	    if (fface_get_flags (fface) & ff_Tag1)
	      GRAB_TAG (Job->tag1);
	    if (fface_get_flags (fface) & ff_Tag2)
	      GRAB_TAG (Job->tag2);
	    if (fface_get_flags (fface) & ff_Tag3)
	      GRAB_TAG (Job->tag3);
	    if (fface_get_flags (fface) & ff_Tag4)
	      GRAB_TAG (Job->tag4);
	    /* Grab the dynamic encodings */
	    if (fface_get_flags (fface) & ff_Encoding)
	      {
		grabbing_encoding = true;
		GRAB_TAG (bufenc);
	      }
	  }

	/*
	 * If not invisible, give it to liba2ps for printing.
	 */
	if (!(fface_get_flags (fface) & ff_Invisible))
	  {
	    if (token_dest (i)->string)
	      ps_print_string (Job, token_dest (i)->string,
			       fface.face);
	    else
	      ps_print_buffer (Job, buffer->content,
			       (unsigned) token_start (i), (unsigned) token_end (i),
			       fface.face);
	  }
      }
}
