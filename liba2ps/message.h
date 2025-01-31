/* message.h - declaration for verbosity sensitive feedback function
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

#ifndef MESSAGE_H_
# define MESSAGE_H_

/* The bigger, the more verbose.  Should be set by application
   (default is 0, set to -1 for to messages) */
extern unsigned int msg_verbosity;

/* Decode ARG which is the verbosity level wanted: either an integer
   specifying directly the bits, or using a comma separated list of
   token which are ARGMATCHed. */

unsigned msg_verbosity_argmatch (const char *option, char *arg);

/* Return non null value if message at LVL should be displayed. */

# define msg_test(lvl) ((lvl) & msg_verbosity)

/* fprintf (TEXT) at verbosity LEVEL. */

# define message(level,text)	\
  do {				\
    if (msg_test (level))	\
      fprintf text;		\
  } while (0)

/* Include the definition of the verbosity levels.  This is
   application dependant. */

# include "msg.h"

#endif /* not MESSAGE_H_ */
