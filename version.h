/* $Id$ */
/******************************************************************************
 * FIDOCONFIG --- library for fidonet configs
 ******************************************************************************
 * Copyright (C) 1998-2002
 *
 * Husky Delopment Team
 *
 * Internet: http://husky.sourceforge.net
 *
 * This file is part of FIDOCONFIG.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library/Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; see file COPYING. If not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * See also http://www.gnu.org
 *****************************************************************************
 */

#ifndef __FIDOCONF__VERSION_H
#define __FIDOCONF__VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fidoconf.h"

/* values for 5th parameter of GenVersionStr() */
typedef enum {
        BRANCH_CURRENT=1, BRANCH_STABLE=2, BRANCH_RELEASE=3
}branch_t;

/* this is version number of FidoConfig */
#define FC_VER_MAJOR 1
#define FC_VER_MINOR 3
#define FC_VER_PATCH 0
#define FC_VER_BRANCH BRANCH_CURRENT

/* Generate version string like
 * programname/platform[-compiler] <major>.<minor>.<patchlevel>-<branch> [<cvs date>]
 *
 * Return malloc'ed pointer
 *
 * Examples:
 * "program/w32-MVC 1.2.3-release"
 * "program/DPMI-DJGPP 1.2.3-stable 01-10-2002"
 * "program/FreeBSD 1.3.0-current 01-10-2002"
 *
 * Require cvs_date.h in module hearer files directory
 */

FCONF_EXT char *GenVersionStr( const char *programname, unsigned major,
   unsigned minor, unsigned patchlevel, unsigned branch, const char *cvsdate );

/* Check version of fidoconfig library
 * return zero if test passed
 * test cvs need for DLL version only, using #include <fidoconf/cvsdate.h>
  const char *fidoconfdate(){
  static
  #include "../fidoconf/cvsdate.h"
  return cvs_date;
  }
  CheckFidoconfigVersion( ..., fidoconfdate());
 */
FCONF_EXT int CheckFidoconfigVersion( int need_major, int need_minor,
                      int need_patch, branch_t need_branch, const char *cvs );

#ifdef __cplusplus
}
#endif

#endif
