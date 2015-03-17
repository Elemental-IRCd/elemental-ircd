
/*
 * Modified and hacked into libratbox by Aaron Sethman <androsyn@ratbox.org>
 * The original headers are below..
 * Note that this implementation does not process floating point numbers so
 * you will likely need to fall back to using sprintf yourself to do those...
 * $Id: snprintf.c 26092 2008-09-19 15:13:52Z androsyn $
 */

/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

/*
 * Fri Jul 13 2001 Crutcher Dunnavant <crutcher+kernel@datastacks.com>
 * - changed to provide snprintf and vsnprintf functions
 * So Feb  1 16:51:32 CET 2004 Juergen Quade <quade@hsnr.de>
 * - scnprintf and vscnprintf
 */
#include <libratbox_config.h>
#include <ratbox_lib.h>

static int
skip_atoi(const char **s)
{
    int i = 0;

    while(isdigit(**s))
        i = i * 10 + *((*s)++) - '0';
    return i;
}

/* Decimal conversion is by far the most typical, and is used
 * for /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed
 * using code from
 * http://www.cs.uiowa.edu/~jones/bcd/decimal.html
 * (with permission from the author, Douglas W. Jones). */

/* Formats correctly any integer in [0,99999].
 * Outputs from one to five digits depending on input.
 * On i386 gcc 4.1.2 -O2: ~250 bytes of code. */
static char *
put_dec_trunc(char *buf, unsigned q)
{
    unsigned d3, d2, d1, d0;
    d1 = (q >> 4) & 0xf;
    d2 = (q >> 8) & 0xf;
    d3 = (q >> 12);

    d0 = 6 * (d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xcd) >> 11;
    d0 = d0 - 10 * q;
    *buf++ = d0 + '0';	/* least significant digit */
    d1 = q + 9 * d3 + 5 * d2 + d1;
    if(d1 != 0) {
        q = (d1 * 0xcd) >> 11;
        d1 = d1 - 10 * q;
        *buf++ = d1 + '0';	/* next digit */

        d2 = q + 2 * d2;
        if((d2 != 0) || (d3 != 0)) {
            q = (d2 * 0xd) >> 7;
            d2 = d2 - 10 * q;
            *buf++ = d2 + '0';	/* next digit */

            d3 = q + 4 * d3;
            if(d3 != 0) {
                q = (d3 * 0xcd) >> 11;
                d3 = d3 - 10 * q;
                *buf++ = d3 + '0';	/* next digit */
                if(q != 0)
                    *buf++ = q + '0';	/* most sign. digit */
            }
        }
    }
    return buf;
}

/* Same with if's removed. Always emits five digits */
static char *
put_dec_full(char *buf, unsigned q)
{
    /* BTW, if q is in [0,9999], 8-bit ints will be enough, */
    /* but anyway, gcc produces better code with full-sized ints */
    unsigned d3, d2, d1, d0;
    d1 = (q >> 4) & 0xf;
    d2 = (q >> 8) & 0xf;
    d3 = (q >> 12);

    /* Possible ways to approx. divide by 10 */
    /* gcc -O2 replaces multiply with shifts and adds */
    // (x * 0xcd) >> 11: 11001101 - shorter code than * 0x67 (on i386)
    // (x * 0x67) >> 10:  1100111
    // (x * 0x34) >> 9:    110100 - same
    // (x * 0x1a) >> 8:     11010 - same
    // (x * 0x0d) >> 7:      1101 - same, shortest code (on i386)

    d0 = 6 * (d3 + d2 + d1) + (q & 0xf);
    q = (d0 * 0xcd) >> 11;
    d0 = d0 - 10 * q;
    *buf++ = d0 + '0';
    d1 = q + 9 * d3 + 5 * d2 + d1;
    q = (d1 * 0xcd) >> 11;
    d1 = d1 - 10 * q;
    *buf++ = d1 + '0';

    d2 = q + 2 * d2;
    q = (d2 * 0xd) >> 7;
    d2 = d2 - 10 * q;
    *buf++ = d2 + '0';

    d3 = q + 4 * d3;
    q = (d3 * 0xcd) >> 11;	/* - shorter code */
    /* q = (d3 * 0x67) >> 10; - would also work */
    d3 = d3 - 10 * q;
    *buf++ = d3 + '0';
    *buf++ = q + '0';
    return buf;
}

static char *
put_dec(char *buf, unsigned long long int num)
{
    while(1) {
        unsigned rem;
        if(num < 100000)
            return put_dec_trunc(buf, num);
        rem = num % 100000;
        num = num / 100000;
        buf = put_dec_full(buf, rem);
    }
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */

static size_t
number(char *const buf, const size_t size, size_t idx, unsigned long long int num, int base, int field_width, int precision,
       int type)
{
    char sign, tmp[66];
    const char *digits;
    /* we are called with base 8, 10 or 16, only, thus don't need "g..."  */
    static const char small_digits[] = "0123456789abcdefx";	/* "ghijklmnopqrstuvwxyz"; */
    static const char large_digits[] = "0123456789ABCDEFX";	/* "GHIJKLMNOPQRSTUVWXYZ"; */
    int need_pfx = ((type & SPECIAL) && base != 10);
    int i;

    digits = (type & LARGE) ? large_digits : small_digits;
    if(type & LEFT)
        type &= ~ZEROPAD;
    if(base < 2 || base > 36)
        return idx;
    sign = 0;
    if(type & SIGN) {
        if((signed long long int)num < 0) {
            sign = '-';
            num = -(signed long long int)num;
            field_width--;
        } else if(type & PLUS) {
            sign = '+';
            field_width--;
        } else if(type & SPACE) {
            sign = ' ';
            field_width--;
        }
    }
    if(need_pfx) {
        field_width--;
        if(base == 16)
            field_width--;
    }

    /* generate full string in tmp[], in reverse order */
    i = 0;
    if(num == 0)
        tmp[i++] = '0';
    /* Generic code, for any base:
       else do {
       tmp[i++] = digits[do_div(num,base)];
       } while (num != 0);
     */
    else if(base != 10) {
        /* 8 or 16 */
        int mask = base - 1;
        int shift = 3;
        if(base == 16)
            shift = 4;
        do {
            tmp[i++] = digits[((unsigned char)num) & mask];
            num >>= shift;
        } while(num);
    } else {
        /* base 10 */
        i = put_dec(tmp, num) - tmp;
    }

    /* printing 100 using %2d gives "100", not "00" */
    if(i > precision)
        precision = i;
    /* leading space padding */
    field_width -= precision;
    if(!(type & (ZEROPAD + LEFT))) {
        while(--field_width >= 0) {
            if(idx < size)
                buf[idx] = ' ';
            ++idx;
        }
    }
    /* sign */
    if(sign) {
        if(idx < size)
            buf[idx] = sign;
        ++idx;
    }
    /* "0x" / "0" prefix */
    if(need_pfx) {
        if(idx < size)
            buf[idx] = '0';
        ++idx;
        if(base == 16) {
            if(idx < size)
                buf[idx] = digits[16];	/* for arbitrary base: digits[33]; */
            ++idx;
        }
    }
    /* zero or space padding */
    if(!(type & LEFT)) {
        char c = (type & ZEROPAD) ? '0' : ' ';
        while(--field_width >= 0) {
            if(idx < size)
                buf[idx] = c;
            ++idx;
        }
    }
    /* hmm even more zero padding? */
    while(i <= --precision) {
        if(idx < size)
            buf[idx] = '0';
        ++idx;
    }
    /* actual digits of result */
    while(--i >= 0) {
        if(idx < size)
            buf[idx] = tmp[i];
        ++idx;
    }
    /* trailing space padding */
    while(--field_width >= 0) {
        if(idx < size)
            buf[idx] = ' ';
        ++idx;
    }
    return idx;
}

/**
 * vsnprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * The return value is the number of characters which would
 * be generated for the given input, excluding the trailing
 * '\0', as per ISO C99. If you want to have the exact
 * number of characters written into @buf as return value
 * (not including the trailing '\0'), use vscnprintf(). If the
 * return is greater than or equal to @size, the resulting
 * string is truncated.
 *
 * Call this function if you are already dealing with a va_list.
 * You probably want snprintf() instead.
 */
int
rb_vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    int len;
    unsigned long long int num;
    int i, base;
    char c;
    size_t idx;
    const char *s;

    int flags;		/* flags to number() */

    int field_width;	/* width of output field */
    int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
    int qualifier;		/* 'h', 'l', or 'L' for integer fields */
    /* 'z' support added 23/7/1999 S.H.    */
    /* 'z' changed to 'Z' --davidm 1/25/99 */
    /* 't' added for ptrdiff_t */

    /* Reject out-of-range values early.  Large positive sizes are
       used for unknown buffer sizes. */
    if(rb_unlikely(size > INT_MAX)) {
        return 0;
    }

    idx = 0;

    for(; *fmt; ++fmt) {
        if(*fmt != '%') {
            if(idx < size)
                buf[idx] = *fmt;
            ++idx;
            continue;
        }

        /* process flags */
        flags = 0;
repeat:
        ++fmt;		/* this also skips first '%' */
        switch (*fmt) {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        /* get field width */
        field_width = -1;
        if(isdigit(*fmt))
            field_width = skip_atoi(&fmt);
        else if(*fmt == '*') {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if(field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if(*fmt == '.') {
            ++fmt;
            if(isdigit(*fmt))
                precision = skip_atoi(&fmt);
            else if(*fmt == '*') {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if(precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
           *fmt == 'Z' || *fmt == 'z' || *fmt == 't') {
            qualifier = *fmt;
            ++fmt;
            if(qualifier == 'l' && *fmt == 'l') {
                qualifier = 'L';
                ++fmt;
            }
        }

        /* default base */
        base = 10;

        switch (*fmt) {
        case 'c':
            if(!(flags & LEFT)) {
                while(--field_width > 0) {
                    if(idx < size)
                        buf[idx] = ' ';
                    ++idx;
                }
            }
            c = (unsigned char)va_arg(args, int);
            if(idx < size)
                buf[idx] = c;
            ++idx;
            while(--field_width > 0) {
                if(idx < size)
                    buf[idx] = ' ';
                ++idx;
            }
            continue;

        case 's':
            s = va_arg(args, char *);
            if(s == NULL) {
                abort();	/* prefer blowing up vs corrupt data */
            }
            len = rb_strnlen(s, precision);

            if(!(flags & LEFT)) {
                while(len < field_width--) {
                    if(idx < size)
                        buf[idx] = ' ';
                    ++idx;
                }
            }
            for(i = 0; i < len; ++i) {
                if(idx < size)
                    buf[idx] = *s;
                ++idx;
                ++s;
            }
            while(len < field_width--) {
                if(idx < size)
                    buf[idx] = ' ';
                ++idx;
            }
            continue;

        case 'p':
            if(field_width == -1) {
                field_width = 2 * sizeof(void *);
                flags |= ZEROPAD;
            }
            idx = number(buf, size, idx,
                         (unsigned long)va_arg(args, void *),
                         16, field_width, precision, flags);
            continue;


        case 'n':
            /* FIXME:
             * What does C99 say about the overflow case here? */
            if(qualifier == 'l') {
                long *ip = va_arg(args, long *);
                *ip = idx;
            } else if(qualifier == 'Z' || qualifier == 'z') {
                size_t *ip = va_arg(args, size_t *);
                *ip = idx;
            } else {
                int *ip = va_arg(args, int *);
                *ip = idx;
            }
            continue;

        case '%':
            if(idx < size)
                buf[idx] = '%';
            ++idx;
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            if(idx < size)
                buf[idx] = '%';
            ++idx;
            if(*fmt) {
                if(idx < size)
                    buf[idx] = *fmt;
                ++idx;
            } else {
                --fmt;
            }
            continue;
        }
        if(qualifier == 'L')
            num = va_arg(args, long long int);
        else if(qualifier == 'l') {
            num = va_arg(args, unsigned long);
            if(flags & SIGN)
                num = (signed long)num;
        } else if(qualifier == 'Z' || qualifier == 'z') {
            num = va_arg(args, size_t);
        } else if(qualifier == 't') {
            num = va_arg(args, ptrdiff_t);
        } else if(qualifier == 'h') {
            num = (unsigned short)va_arg(args, int);
            if(flags & SIGN)
                num = (signed short)num;
        } else {
            num = va_arg(args, unsigned int);
            if(flags & SIGN)
                num = (signed int)num;
        }
        idx = number(buf, size, idx, num, base, field_width, precision, flags);
    }
    if(size > 0) {
        if(idx < size)
            buf[idx] = '\0';
        else
            buf[size - 1] = '\0';
    }
    /* the trailing null byte doesn't count towards the total */
    return idx;
}

/**
 * snprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @...: Arguments for the format string
 *
 * The return value is the number of characters which would be
 * generated for the given input, excluding the trailing null,
 * as per ISO C99.  If the return is greater than or equal to
 * @size, the resulting string is truncated.
 */
int
rb_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = rb_vsnprintf(buf, size, fmt, args);
    va_end(args);
    return i;
}

/**
 * vsprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * The function returns the number of characters written
 * into @buf. Use vsnprintf() or vscnprintf() in order to avoid
 * buffer overflows.
 *
 * Call this function if you are already dealing with a va_list.
 * You probably want sprintf() instead.
 */
int
rb_vsprintf(char *buf, const char *fmt, va_list args)
{
    return rb_vsnprintf(buf, INT_MAX, fmt, args);
}

/**
 * sprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @fmt: The format string to use
 * @...: Arguments for the format string
 *
 * The function returns the number of characters written
 * into @buf. Use snprintf() or scnprintf() in order to avoid
 * buffer overflows.
 */
int
rb_sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = rb_vsnprintf(buf, INT_MAX, fmt, args);
    va_end(args);
    return i;
}

/*
 * rb_vsprintf_append()
 * appends sprintf formatted string to the end of the buffer
 */

int
rb_vsprintf_append(char *str, const char *format, va_list ap)
{
    size_t x = strlen(str);
    return (rb_vsprintf(str + x, format, ap) + x);
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
    return (rb_vsnprintf(str + x, len - x, format, ap) + x);
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
