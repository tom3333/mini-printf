/*
 * The Minimal snprintf() implementation
 *
 * Copyright (c) 2013,2014 Michal Ludvig <michal@logix.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the auhor nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----
 *
 * This is a minimal snprintf() implementation optimised
 * for embedded systems with a very limited program memory.
 * mini_snprintf() doesn't support _all_ the formatting
 * the glibc does but on the other hand is a lot smaller.
 * Here are some numbers from my STM32 project (.bin file size):
 *      no snprintf():      10768 bytes
 *      mini snprintf():    11420 bytes     (+  652 bytes)
 *      glibc snprintf():   34860 bytes     (+24092 bytes)
 * Wasting nearly 24kB of memory just for snprintf() on
 * a chip with 32kB flash is crazy. Use mini_snprintf() instead.
 *
 */

/* *******************************************************************
 * includes
 * ******************************************************************/
#include "mini-printf.h"

#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <stdint.h>

/* *******************************************************************
 * defines
 * ******************************************************************/
# ifndef unlikely
	#define unlikely(x)     __builtin_expect(!!(x), 0)
	//#define unlikely(x)   (__builtin_constant_p(x) ? !!(x) : __branch_check__(x, 0))
# endif

/* *******************************************************************
 * Static Function Prototypes
 * ******************************************************************/
static unsigned int mini_itoa(int value, unsigned int radix, unsigned int uppercase, unsigned int unsig,
	 char *buffer, unsigned int zero_pad);
/* *******************************************************************
 * Global Functions
 * ******************************************************************/
unsigned int mini_strlen(const char *s)
{
	unsigned int len = 0;
	while (s[len] != '\0') len++;
	return len;
}

int mini_vsnprintf(char *buffer, unsigned int buffer_len, const char * const fmt, va_list va)
{
	const char* local_fmt = (char*)fmt;
	char *pbuffer = buffer;
	char bf[24];
	char ch;

	int _putc(char ch)
	{
		if ((unsigned int)((pbuffer - buffer) + 1) >= buffer_len)
			return 0;
		*(pbuffer++) = ch;
		*(pbuffer) = '\0';
		return 1;
	}

	int _puts(char *s, unsigned int len)
	{
		unsigned int i;

		if (buffer_len - (pbuffer - buffer) - 1 < len)
			len = buffer_len - (pbuffer - buffer) - 1;

		/* Copy to buffer */
		for (i = 0; i < len; i++)
			*(pbuffer++) = s[i];
		*(pbuffer) = '\0';

		return len;
	}

	while ((ch=*(local_fmt++))) {
		if ((unsigned int)((pbuffer - buffer) + 1) >= buffer_len)
			break;
		if (ch!='%')
			_putc(ch);
		else {
			char zero_pad = 0;
			char *ptr;
			unsigned int len;

			ch=*(local_fmt++);

			/* Zero padding requested */
			if (ch=='0') {
				ch=*(local_fmt++);
				if (ch == '\0')
					goto end;
				if (ch >= '0' && ch <= '9')
					zero_pad = ch - '0';
				ch=*(local_fmt++);
			}

			switch (ch) {
				case 0:
					goto end;

				case 'i':
					len = mini_itoa(va_arg(va, int), 10, 0, 0, bf, zero_pad);
					_puts(bf, len);
					break;

				case 'u':
				case 'd':
					len = mini_itoa(va_arg(va, unsigned int), 10, 0, (ch=='u'), bf, zero_pad);
					_puts(bf, len);
					break;

				case 'x':
				case 'X':
					len = mini_itoa(va_arg(va, unsigned int), 16, (ch=='X'), 1, bf, zero_pad);
					_puts(bf, len);
					break;

				case 'c' :
					_putc((char)(va_arg(va, int)));
					break;

				case 's' :
					ptr = va_arg(va, char*);
					_puts(ptr, mini_strlen(ptr));
					break;

				default:
					_putc(ch);
					break;
			}
		}
	}
end:
	return pbuffer - buffer;
}


int mini_snprintf(char* buffer, unsigned int buffer_len, const char * const fmt, ...)
{
	int ret;
	va_list va;
	va_start(va, fmt);
	ret = mini_vsnprintf(buffer, buffer_len, fmt, va);
	va_end(va);

	return ret;
}





unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
    unsigned long result = 0,value;

    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((*cp == 'x') && isxdigit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    }
    while (isxdigit(*cp) &&
           (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

/**
 * simple_strtol - convert a string to a signed long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
long simple_strtol(const char *cp,char **endp,unsigned int base)
{
    if(*cp=='-')
        return -simple_strtoul(cp+1,endp,base);
    return simple_strtoul(cp,endp,base);
}

/**
 * simple_strtoull - convert a string to an unsigned long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
unsigned long long simple_strtoull(const char *cp,char **endp,unsigned int base)
{
    unsigned long long result = 0,value;

    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((*cp == 'x') && isxdigit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    }
    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
                                                               ? toupper(*cp) : *cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

/**
 * simple_strtoll - convert a string to a signed long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
long long simple_strtoll(const char *cp,char **endp,unsigned int base)
{
    if(*cp=='-')
        return -simple_strtoull(cp+1,endp,base);
    return simple_strtoull(cp,endp,base);
}

static int skip_atoi(const char **s)
{
    int i=0;

    while (isdigit(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
}





#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define PLUS    4               /* show plus */
#define SPACE   8               /* space if plus */
#define LEFT    16              /* left justified */
#define SPECIAL 32              /* 0x */
#define LARGE   64              /* use 'ABCDEF' instead of 'abcdef' */

/**
 * vsscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	format of buffer
 * @args:	arguments
 */
int mini_vsscanf(const char * buf, const char * fmt, va_list args)
{
	const char *str = buf;
	char *next;
	char digit;
	int num = 0;
	int qualifier;
	int base;
	int field_width;
	int is_sign = 0;

	while(*fmt && *str) {
		/* skip any white space in format */
		/* white space in format matchs any amount of
		 * white space, including none, in the input.
		 */
		if (isspace(*fmt)) {
			while (isspace(*fmt))
				++fmt;
			while (isspace(*str))
				++str;
		}

		/* anything that is not a conversion must match exactly */
		if (*fmt != '%' && *fmt) {
			if (*fmt++ != *str++)
				break;
			continue;
		}

		if (!*fmt)
			break;
		++fmt;

		/* skip this conversion.
		 * advance both strings to next white space
		 */
		if (*fmt == '*') {
			while (!isspace(*fmt) && *fmt)
				fmt++;
			while (!isspace(*str) && *str)
				str++;
			continue;
		}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);

		/* get conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
		    *fmt == 'Z' || *fmt == 'z') {
			qualifier = *fmt++;
			if (unlikely(qualifier == *fmt)) {
				if (qualifier == 'h') {
					qualifier = 'H';
					fmt++;
				} else if (qualifier == 'l') {
					qualifier = 'L';
					fmt++;
				}
			}
		}
		base = 10;
		is_sign = 0;

		if (!*fmt || !*str)
			break;

		switch(*fmt++) {
		case 'c':
		{
			char *s = (char *) va_arg(args,char*);
			if (field_width == -1)
				field_width = 1;
			do {
				*s++ = *str++;
			} while (--field_width > 0 && *str);
			num++;
		}
		continue;
		case 's':
		{
			char *s = (char *) va_arg(args, char *);
			if(field_width == -1)
				field_width = INT_MAX;
			/* first, skip leading white space in buffer */
			while (isspace(*str))
				str++;

			/* now copy until next white space */
			while (*str && !isspace(*str) && field_width--) {
				*s++ = *str++;
			}
			*s = '\0';
			num++;
		}
		continue;
		case 'n':
			/* return number of characters read so far */
		{
			int *i = (int *)va_arg(args,int*);
			*i = str - buf;
		}
		continue;
		case 'o':
			base = 8;
			break;
		case 'x':
		case 'X':
			base = 16;
			break;
		case 'i':
                        base = 0;
		case 'd':
			is_sign = 1;
		case 'u':
			break;
		case '%':
			/* looking for '%' in str */
			if (*str++ != '%')
				return num;
			continue;
		default:
			/* invalid format; stop here */
			return num;
		}

		/* have some sort of integer conversion.
		 * first, skip white space in buffer.
		 */
		while (isspace(*str))
			str++;

		digit = *str;
		if (is_sign && digit == '-')
			digit = *(str + 1);

		if (!digit
                    || (base == 16 && !isxdigit(digit))
                    || (base == 10 && !isdigit(digit))
                    || (base == 8 && (!isdigit(digit) || digit > '7'))
                    || (base == 0 && !isdigit(digit)))
				break;

		switch(qualifier) {
		case 'H':	/* that's 'hh' in format */
			if (is_sign) {
				signed char *s = (signed char *) va_arg(args,signed char *);
				*s = (signed char) simple_strtol(str,&next,base);
			} else {
				unsigned char *s = (unsigned char *) va_arg(args, unsigned char *);
				*s = (unsigned char) simple_strtoul(str, &next, base);
			}
			break;
		case 'h':
			if (is_sign) {
				short *s = (short *) va_arg(args,short *);
				*s = (short) simple_strtol(str,&next,base);
			} else {
				unsigned short *s = (unsigned short *) va_arg(args, unsigned short *);
				*s = (unsigned short) simple_strtoul(str, &next, base);
			}
			break;
		case 'l':
			if (is_sign) {
				long *l = (long *) va_arg(args,long *);
				*l = simple_strtol(str,&next,base);
			} else {
				unsigned long *l = (unsigned long*) va_arg(args,unsigned long*);
				*l = simple_strtoul(str,&next,base);
			}
			break;
		case 'L':
			if (is_sign) {
				long long *l = (long long*) va_arg(args,long long *);
				*l = simple_strtoll(str,&next,base);
			} else {
				unsigned long long *l = (unsigned long long*) va_arg(args,unsigned long long*);
				*l = simple_strtoull(str,&next,base);
			}
			break;
		case 'Z':
		case 'z':
		{
			size_t *s = (size_t*) va_arg(args,size_t*);
			*s = (size_t) simple_strtoul(str,&next,base);
		}
		break;
		default:
			if (is_sign) {
				int *i = (int *) va_arg(args, int*);
				*i = (int) simple_strtol(str,&next,base);
			} else {
				unsigned int *i = (unsigned int*) va_arg(args, unsigned int*);
				*i = (unsigned int) simple_strtoul(str,&next,base);
			}
			break;
		}
		num++;

		if (!next)
			break;
		str = next;
	}
	return num;
}

/**
 * sscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	formatting of buffer
 * @...:	resulting arguments
 */
int mini_sscanf(const char * buf, const char * fmt, ...)
{
	va_list args;
	int i;

	va_start(args,fmt);
	i = mini_vsscanf(buf,fmt,args);
	va_end(args);
	return i;
}


static unsigned int mini_itoa(int value, unsigned int radix, unsigned int uppercase, unsigned int unsig,
	 char *buffer, unsigned int zero_pad)
{
	char	*pbuffer = buffer;
	int	negative = 0;
	unsigned int	i, len;

	/* No support for unusual radixes. */
	if (radix > 16)
		return 0;

	if (value < 0 && !unsig) {
		negative = 1;
		value = -value;
	}

	/* This builds the string back to front ... */
	do {
		int digit = value % radix;
		*(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
		value /= radix;
	} while (value > 0);

	for (i = (pbuffer - buffer); i < zero_pad; i++)
		*(pbuffer++) = '0';

	if (negative)
		*(pbuffer++) = '-';

	*(pbuffer) = '\0';

	/* ... now we reverse it (could do it recursively but will
	 * conserve the stack space) */
	len = (pbuffer - buffer);
	for (i = 0; i < len / 2; i++) {
		char j = buffer[i];
		buffer[i] = buffer[len-i-1];
		buffer[len-i-1] = j;
	}

	return len;
}