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

#ifndef RB_ATTRIB_H
#define RB_ATTRIB_H 1

#ifdef __GNUC__

/* Optimize for true or false branches */
#define rb_likely(x)       __builtin_expect(!!(x), 1)
#define rb_unlikely(x)     __builtin_expect(!!(x), 0)

/* Warn on unchecked returns */
#define __rb_must_check    __attribute__((warn_unused_result))

/* Warn on null arguments */
#define __rb_nonnull(...)    __attribute__((nonull(__VA_ARGS__)))

/* Function never returns */
#define __rb_noreturn       __attribute__((noreturn))

/* Validate and Type-check printf arguments */
#define __rb_format_printf(fmt, args) __attribute__ ((format(printf, fmt, args)));

#else  /* __GNUC__ */

#define rb_likely(x)       (x)
#define rb_unlikely(x)     (x)

#define __rb_must_check
#define __rb_nonull
#define __rb_noreturn
#define __rb_format_printf

#endif /* __GNUC__ */

#endif /* RB_ATTRIB_H */
