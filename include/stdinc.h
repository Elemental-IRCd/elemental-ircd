/*
 *  ircd-ratbox: A slightly useful ircd.
 *  stdinc.h: Pull in all of the necessary system headers
 *
 *  Copyright (C) 2002 Aaron Sethman <androsyn@ratbox.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *
 */

#ifndef INCLUDED_stdinc_h
#define INCLUDED_stdinc_h

#include "ratbox_lib.h"
#include "config.h"		/* Gotta pull in the autoconf stuff */
#include "ircd_defs.h"  /* Needed for some reasons here -- dwr */


#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef STRING_WITH_STRINGS
# include <string.h>
# include <strings.h>
#else
# ifdef HAVE_STRING_H
#  include <string.h>
# else
#  ifdef HAVE_STRINGS_H
#   include <strings.h>
#  endif
# endif
#endif


#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif



#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>

#include <limits.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <sys/stat.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#ifdef strdupa
#define LOCAL_COPY(s) strdupa(s)
#else
#if defined(__INTEL_COMPILER) || defined(__GNUC__)
# define LOCAL_COPY(s) __extension__({ char *_s = alloca(strlen(s) + 1); strcpy(_s, s); _s; })
#else
# define LOCAL_COPY(s) strcpy(alloca(strlen(s) + 1), s) /* XXX Is that allowed? */
#endif /* defined(__INTEL_COMPILER) || defined(__GNUC__) */
#endif /* strdupa */

#endif /* INCLUDED_stdinc_h */
