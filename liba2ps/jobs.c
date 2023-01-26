/* jobs.c - recording information about the print jobs
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

#include <locale.h>

#include "a2ps.h"
#include "jobs.h"
#include "routines.h"
#include "xstrrpl.h"
#include "hashtab.h"
#include "useropt.h"
#include "caret.h"
#include "metaseq.h"
#include "message.h"
#include "fonts.h"
#include "faces.h"
#include "prange.h"
#include "stream.h"
#include "fjobs.h"
#include "lister.h"
#include "quotearg.h"
#include "userdata.h"
#include "xgethostname.h"

/************************************************************************
 * Information about the user						*
 ************************************************************************/
static inline void
a2ps_job_register_user (a2ps_job *job)
{
  {
    struct userdata u;
    userdata_get (&u);
    if (u.login)
      macro_meta_sequence_add (job, VAR_USER_LOGIN, u.login);
    if (u.name)
      macro_meta_sequence_add (job, VAR_USER_NAME, u.name);
    if (u.comments)
      macro_meta_sequence_add (job, VAR_USER_COMMENTS, u.comments);
    if (u.home)
      macro_meta_sequence_add (job, VAR_USER_HOME, u.home);
  }

  {
    char *host = xgethostname ();
    macro_meta_sequence_add (job, VAR_USER_HOST, host);
  }
}

/*
 * Set the array to NULL values
 */
static void
tmpfiles_reset (struct a2ps_job * job)
{
  size_t i;

  for (i = 0 ; i < cardinalityof (job->tmp_filenames) ; i ++)
    job->tmp_filenames [i] = NULL;
}

/*
 * Unlink all the (may be existing) temp files
 */
void
a2ps_job_unlink_tmpfiles (struct a2ps_job * job)
{
  size_t i;

  /* That of the structure */
  for (i = 0 ; i < cardinalityof (job->tmp_filenames) ; i ++)
    if (job->tmp_filenames[i])
      unlink (job->tmp_filenames[i]);

  /* That of the files */
  da_map (job->jobs, (da_map_func_t) file_job_unlink_tmpfile);
}

/*
 * Create a new a2ps_job (corresponding to a single file produced)
 * and return it initialized (but default values: config files are
 * not read)
 */
a2ps_job *
a2ps_job_new (void)
{
  time_t tim;
  struct tm *tm;
  a2ps_job * res;
  char * cp;

  res = XMALLOC (a2ps_job);

  /* Specify the quotation style. */
  set_quoting_style (NULL, escape_quoting_style);

  /* Set the NLS on */
  setlocale (LC_TIME, "");
  setlocale (LC_MESSAGES, "");
  setlocale (LC_CTYPE, "");

  bindtextdomain (PACKAGE, LOCALEDIR);
  bindtextdomain (PACKAGE "-gnulib", LOCALEDIR);
  textdomain (PACKAGE);

  /* Initialize tinyterm and lister. */
  lister_initialize (NULL, stdout);
  lister_before_set (NULL, 2);

  /* Set verbosity level right now to help debugging through the
     envvar A2PS_VERBOSITY. */
  msg_verbosity = 2;
  if ((cp = getenv ("A2PS_VERBOSITY")) && *cp)
    msg_verbosity = msg_verbosity_argmatch ("$A2PS_VERBOSITY", cp);

  /* The arguments */
  res->argv = NULL;
  res->argc = 0;

  a2ps_common_reset (&res->common);
  res->file_command = NULL;	/* Command to use to call file(1)	*/

  /*
   * Data that library needs (mostly read from config files)
   */
  res->media = new_medium_table ();	/* Media defined by the user 	*/

  /* Short cuts defined by the user */
  res->user_options = user_options_table_new ();

  /* Honor what the user said in its environment */
  if ((cp = getenv ("SIMPLE_BACKUP_SUFFIX")))
    simple_backup_suffix = cp;
  res->backup_type = get_version ("$VERSION_CONTROL",
				  getenv ("VERSION_CONTROL"));

  /* Get current time information */
  tim = time (NULL);
  tm = localtime (&tim);
  memcpy (&(res->run_tm), tm, sizeof (*tm));

  res->sheets = 0;
  res->pages = 0;
  res->lines_folded = 0;
  res->total_files = 0;
  res->orientation = portrait;
  res->duplex = simplex;
  res->columns = 1;
  res->rows = 1;
  res->madir = madir_rows;	/* by default, write horizontally	*/
  res->virtual = 0;
  res->copies = 1;
  res->margin = 0;

  /* Map to know where is the information related to the encodings */
  res->encodings_map = encodings_map_new ();

  /* Chunk in which PS def of the fonts is stored */
  res->ps_encodings = output_new ("PS encodings");

  res->page_prefeed = false;	/* No page prefeed			*/

  /* Make sure not to be happy to use a not initialized array */
  init_face_eo_font (res);

  /* virtual file name given to stdin */
  res->stdin_filename = xstrdup ("stdin");

  /*
   * Related to the output
   */
  res->output_format = ps;	/* By default, make PostScript 		*/
  /* Reset the printers modules	*/
  res->printers = a2ps_printers_new (&res->common);
  res->output_stream = NULL;

  res->folding = true;		/* Line folding option 			*/
  res->numbering = 0;		/* Line numbering option 		*/
  res->unprintable_format = caret;/* Replace non printable char by ^M etc. */
  res->interpret = true;	/* Interpret TAB, FF and BS chars option */
  res->print_binaries = false;	/* Force printing for binary files 	*/
  /*  Use default behavior from previous versions here. */
  res->file_align = file_align_page;
  res->border = true;		/* print the surrounding border ?	*/
  res->debug = false;		/* No debugging				*/
  res->prolog = xstrdup ("bw");	/* default ps header file		*/
  res->medium_request = LIBPAPER_MEDIUM;/* default paper is libpaper default */
  res->medium = NULL;
  res->tabsize = 8;		/* length of tabulations		*/
  res->lines_requested = 0;	/* lines per page			*/
  res->columns_requested = 0;	/* columns per page			*/
  res->fontsize = 0.0;		/* Size of a char for body font 	*/
  res->encoding = NULL; 	/* What is the current char set ?	*/
  res->requested_encoding_name = NULL;	/* Encoding requested by -X 	*/
  res->requested_encoding = NULL;/* Encoding requested by -X		*/
  res->saved_encoding = NULL;/* Encoding requested by -X		*/
  res->encodings = encodings_table_new ();


  /* Map to know the name of the files describing fonts	*/
  res->fonts_map = fonts_map_new ();
  res->font_infos = font_info_table_new ();

  /* Title of the job */
  res->title = xstrdup ("a2ps output");	/* Default title */

  /* Headers and footers */
  res->header = NULL;
  res->center_title = NULL;
  res->left_title = NULL;
  res->right_title = NULL;
  res->left_footer = NULL;
  res->footer = NULL;
  res->right_footer = NULL;
  res->water = NULL;
  * res->tag1 = '\0';
  * res->tag2 = '\0';
  * res->tag3 = '\0';
  * res->tag4 = '\0';

  /* Definition of the macro meta sequences	*/
  res->macro_meta_sequences = macro_meta_sequence_table_new ();
  a2ps_job_register_user (res);

  /* Private info for PS generation */
  res->status = new_ps_status();

  /* Where the diverted output is stored */
  res->divertion = output_new ("Main trunk");

  /* Null tmp names. */
  tmpfiles_reset (res);

  /* List of the pages to print */
  res->page_range = page_range_new ();

  /* List of the jobs */
  res->jobs = da_new ("List of the jobs", 10,
		      da_linear, 10,
		      (da_print_func_t) file_job_self_print, NULL);
  return res;
}

/*
 * Finish the initialization
 * Typically must be called after that the a2ps.cfg was read
 * so that the lib_path is known, so that the files to be
 * read at initialization time get read now
 */
void
a2ps_job_finalize (struct a2ps_job * job)
{
  /* Finalize the shared mem */
  a2ps_common_finalize (&job->common,
			macro_meta_sequence_get (job, VAR_USER_HOME));

  /* Finalize the printers module */
  a2ps_printers_finalize (job->printers);

  /* Map to know where is the information related to the encodings */
  load_main_encodings_map (job);

  /* Now that the encoding.map is read, make sure to update
     the encoding.  It must be correct */
  job->requested_encoding =
    get_encoding_by_alias (job, job->requested_encoding_name);
  if (!job->requested_encoding)
    error (EXIT_FAILURE, 0, _("unknown encoding `%s'"),
	   quotearg (UNNULL (job->requested_encoding_name)));

  /* Get the right medium */
  job->medium = a2ps_get_medium (job, job->medium_request);

  /* Map to know the name of the files describing fonts	*/
  load_main_fonts_map (job);
}

/*
 * Print diagnostics and delete temp files
 */
void
a2ps_job_free (struct a2ps_job * job)
{
  if (msg_test (msg_file))
    da_self_print (job->jobs, stderr);

  if (msg_test (msg_file))
    output_self_print (job->divertion, stderr);

  /* Unlink and free the temporary files */
  a2ps_job_unlink_tmpfiles (job);
}
