/*
 *  elemental-ircd: A slightly useful ircd.
 *  sprintf_rb.h: sprintf helpers.
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2002-2012 ircd-ratbox development team
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

#ifndef RB_LIB_H
# error "Do not use snprintf.h directly"
#endif

#ifndef SPRINTF_IRC
#define SPRINTF_IRC

int rb_sprintf_append(char *str, const char *format, ...)
__format_printf(2, 3);

int rb_snprintf_append(char *str, size_t len, const char *format, ...)
__format_printf(3, 4);

int rb_vsnprintf_append(char *str, const size_t size, const char *fmt, va_list args);
int rb_vsprintf_append(char *str, const char *fmt, va_list args);

#endif /* SPRINTF_IRC */
