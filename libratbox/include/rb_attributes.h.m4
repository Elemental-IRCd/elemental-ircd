/*
 *  elemental-ircd: A slightly useful ircd.
 *  attributes.h: Preprocessor macros for compiler attributes
 *
 *  Copyright (C) 2015 elemental-ircd development team
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 */

changequote([,])
changecom()

#ifndef RB_ATTRIB_H
#define RB_ATTRIB_H 1

#ifdef __GNUC__
/* Optimize for true or false branches */
#define rb_likely(x)       __builtin_expect(!!(x), 1)
#define rb_unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define rb_likely(x)       (!!(x))
#define rb_unlikely(x)     (!!(x))
#endif

define(attribute, [dnl
define([m_name], [regexp($1, [^[^(]+], [\&])])dnl
#if !defined(m_name)
#define [$1] __attribute__(([$2]))
#endif
divert(1)dnl
#if !defined(m_name)
#define [$1]
#endif
divert(0)dnl
])

/* Warn on unchecked returns */
attribute(__must_check, warn_unused_result)

/* Function never returns */
attribute(__noreturn, noreturn)

/* Validate and Type-check printf arguments */
attribute(__format(type, fmt, args), [format(type, fmt, args)])

/* Mark argument as unused */
attribute(__unused, unused)

/* Mark a function as depricated */
attribute(__deprecated, deprecated)

/* Non-null return value */
#if defined(__COVERITY__)
attribute(__returns_nonnull, returns_nonnull)
#endif

/* function returns new memory (and warn on unchecked return) */
attribute(__malloc, [malloc, warn_unused_result])

undivert(1)
#endif
