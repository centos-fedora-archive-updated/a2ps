/* userdata.h - get data on users (from /etc/passwd)
   Copyright 1999-2017 Free Software Foundation, Inc.

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

#ifndef USERDATA_H_
# define USERDATA_H_

struct userdata
{
  char *login;
  char *name;
  char *comments;
  char *home;
};

void userdata_get (struct userdata *udata);
#endif /* !USERDATA_H_ */
