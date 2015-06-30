/*
 *  elemental-ircd: A slightly useful ircd.
 *  sprintf_rb.c: sprintf helpers.
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

#include <libratbox_config.h>
#include <ratbox_lib.h>

/*
 * rb_vsprintf_append()
 * appends sprintf formatted string to the end of the buffer
 */

int
rb_vsprintf_append(char *str, const char *format, va_list ap)
{
    size_t x = strlen(str);
    return (vsprintf(str + x, format, ap) + x);
}

/*
 * rb_sprintf_append()
 * appends sprintf formatted string to the end of the buffer
 */
int
rb_sprintf_append(char *str, const char *format, ...)
{
    int x;
    va_list ap;
    va_start(ap, format);
    x = rb_vsprintf_append(str, format, ap);
    va_end(ap);
    return (x);
}

/*
 * rb_vsnprintf_append()
 * appends sprintf formatted string to the end of the buffer but not
 * exceeding len
 */

int
rb_vsnprintf_append(char *str, size_t len, const char *format, va_list ap)
{
    size_t x;
    if(len == 0)
        return 0;
    x = strlen(str);

    if(len < x) {
        str[len - 1] = '\0';
        return len - 1;
    }
    return (vsnprintf(str + x, len - x, format, ap) + x);
}

/*
 * rb_snprintf_append()
 * appends snprintf formatted string to the end of the buffer but not
 * exceeding len
 */

int
rb_snprintf_append(char *str, size_t len, const char *format, ...)
{
    int x;
    va_list ap;
    va_start(ap, format);
    x = rb_vsnprintf_append(str, len, format, ap);
    va_end(ap);
    return (x);
}
