/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "shared/shared.h"

//
//===============
// AngleVectors
// 
// Calculate the forward, right, and up vectors for the given angles.
//===============
//
void AngleVectors(const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up) {
    float        angle;
    float        sr, sp, sy, cr, cp, cy;

    angle = angles.xyz[vec3_t::Yaw] * (M_PI * 2 / 360);
    sy = std::sinf(angle);
    cy = std::cosf(angle);
    angle = angles.xyz[vec3_t::Pitch] * (M_PI * 2 / 360);
    sp = std::sinf(angle);
    cp = std::cosf(angle);
    angle = angles.xyz[vec3_t::Roll] * (M_PI * 2 / 360);
    sr = std::sinf(angle);
    cr = std::cosf(angle);

    if (forward) {
        forward->xyz[0] = cp * cy;
        forward->xyz[1] = cp * sy;
        forward->xyz[2] = -sp;
    }
    if (right) {
        right->xyz[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
        right->xyz[1] = (-1 * sr * sp * sy + -1 * cr * cy);
        right->xyz[2] = -1 * sr * cp;
    }
    if (up) {
        up->xyz[0] = (cr * sp * cy + -sr * -sy);
        up->xyz[1] = (cr * sp * sy + -sr * cy);
        up->xyz[2] = cr * cp;
    }
}

//
//===============
// VectorNormalize
// 
// Normalizes the input vector ptr, and returns its length.
//===============
//
vec_t VectorNormalize(vec3_t &v)
{
    float    length, ilength;

    length = v.xyz[0] * v.xyz[0] + v.xyz[1] * v.xyz[1] + v.xyz[2] * v.xyz[2];
    length = sqrtf(length);         // FIXME

    if (length) {
        ilength = 1 / length;
        v.xyz[0] *= ilength;
        v.xyz[1] *= ilength;
        v.xyz[2] *= ilength;
    }

    return length;

}

//
//===============
// VectorNormalize
// 
// Normalizes the input vector, and stores its results in the out ptr.
// Returns the vector's length.
//===============
//
vec_t VectorNormalize2(const vec3_t& v, vec3_t& out)
{
    float    length, ilength;

    length = v.xyz[0] * v.xyz[0] + v.xyz[1] * v.xyz[1] + v.xyz[2] * v.xyz[2];
    length = sqrtf(length);         // FIXME

    if (length) {
        ilength = 1 / length;
        out.xyz[0] = v.xyz[0] * ilength;
        out.xyz[1] = v.xyz[1] * ilength;
        out.xyz[2] = v.xyz[2] * ilength;
    }

    return length;

}
// Wrapper for legacy code.
vec_t VectorNormalize2(const vec3_t& v, vec_t *out)
{
    float    length, ilength;

    length = v.xyz[0] * v.xyz[0] + v.xyz[1] * v.xyz[1] + v.xyz[2] * v.xyz[2];
    length = sqrtf(length);         // FIXME

    if (length) {
        ilength = 1 / length;
        out[0] = v.xyz[0] * ilength;
        out[1] = v.xyz[1] * ilength;
        out[2] = v.xyz[2] * ilength;
    }

    return length;

}

//
//===============
// ClearBounds
// 
// Clears the Min and Max bounds pointers. (Sets it to the largest around.
// so it can later on add points to it if < ... etc.)
//===============
//
void ClearBounds(vec3_t& mins, vec3_t& maxs) {
    mins.xyz[0] = mins.xyz[1] = mins.xyz[2] = 99999;
    maxs.xyz[0] = maxs.xyz[1] = maxs.xyz[2] = -99999;
}

//
//===============
// AddPointToBounds
// 
// Adds the given vector point to the bounds, in case it exceeds the
// current bounds, it will automatically resize them accordingly.
// 
// Hence, clearbounds, sets the largest possible value around.
//===============
//
void AddPointToBounds(const vec3_t& v, vec3_t& mins, vec3_t& maxs) {
    int        i;
    vec_t    val;

    for (i = 0; i < 3; i++) {
        val = v.xyz[i];
        if (val < mins.xyz[i])
            mins.xyz[i] = val;
        if (val > maxs.xyz[i])
            maxs.xyz[i] = val;
    }
}

//
//===============
// UnionBounds
// 
// Merges 2 bounding boxes together, into the outptr.
//===============
//
void UnionBounds(vec3_t *a, vec3_t *b, vec3_t *c) {
    c[0].xyz[0] = b[0].xyz[0] < a[0].xyz[0] ? b[0].xyz[0] : a[0].xyz[0];
    c[0].xyz[1] = b[0].xyz[1] < a[0].xyz[1] ? b[0].xyz[1] : a[0].xyz[1];
    c[0].xyz[2] = b[0].xyz[2] < a[0].xyz[2] ? b[0].xyz[2] : a[0].xyz[2];

    c[1].xyz[0] = b[1].xyz[0] > a[1].xyz[0] ? b[1].xyz[0] : a[1].xyz[0];
    c[1].xyz[1] = b[1].xyz[1] > a[1].xyz[1] ? b[1].xyz[1] : a[1].xyz[1];
    c[1].xyz[2] = b[1].xyz[2] > a[1].xyz[2] ? b[1].xyz[2] : a[1].xyz[2];
}

//
//===============
// RadiusFromBounds
// 
// Returns the radius length for the given boundaries.
//===============
//
vec_t RadiusFromBounds(const vec3_t& mins, const vec3_t& maxs) {
    int     i;
    vec3_t  corner;
    vec_t   a, b;

    for (i = 0; i < 3; i++) {
        a = std::fabsf(mins.xyz[i]); // MATHLIB: !! WARNING: Q_fabs replaced by std::fabs
        b = std::fabsf(maxs.xyz[i]); // MATHLIB: !! WARNING: Q_fabs replaced by std::fabs
        corner.xyz[i] = a > b ? a : b;
    }

    return VectorLength(corner);
}

//====================================================================================

// unused:
// static const char hexchars[] = "0123456789ABCDEF";

/*
============
COM_SkipPath
============
*/
char* COM_SkipPath(const char* pathname)
{
    char* last;

    if (!pathname) {
        Com_Error(ERR_FATAL, "%s: NULL", __func__);
    }

    last = (char*)pathname;
    while (*pathname) {
        if (*pathname == '/')
            last = (char*)pathname + 1;
        pathname++;
    }
    return last;
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension(const char* in, char* out, size_t size)
{
    char* s;

    Q_strlcpy(out, in, size);

    s = out + strlen(out);

    while (s != out) {
        if (*s == '/') {
            break;
        }
        if (*s == '.') {
            *s = 0;
            break;
        }
        s--;
    }
}

/*
============
COM_FileExtension
============
*/
char* COM_FileExtension(const char* in)
{
    const char* s;
    const char* last;

    if (!in) {
        Com_Error(ERR_FATAL, "%s: NULL", __func__);
    }

    s = in + strlen(in);
    last = s;

    while (s != in) {
        if (*s == '/') {
            break;
        }
        if (*s == '.') {
            return (char*)s;
        }
        s--;
    }

    return (char*)last;
}

/*
============
COM_FileBase
============
*/
void COM_FileBase(char* in, char* out)
{
    char* s, * s2;

    s = in + strlen(in) - 1;

    while (s != in && *s != '.')
        s--;

    for (s2 = s; s2 != in && *s2 != '/'; s2--)
        ;

    if (s - s2 < 2)
        out[0] = 0;
    else {
        s--;
        strncpy(out, s2 + 1, s - s2);
        out[s - s2] = 0;
    }
}

/*
============
COM_FilePath

Returns the path up to, but not including the last /
============
*/
void COM_FilePath(const char* in, char* out, size_t size)
{
    char* s;

    Q_strlcpy(out, in, size);
    s = strrchr(out, '/');
    if (s) {
        *s = 0;
    }
    else {
        *out = 0;
    }
}


/*
==================
COM_DefaultExtension

if path doesn't have .EXT, append extension
(extension should include the .)
==================
*/
size_t COM_DefaultExtension(char* path, const char* ext, size_t size)
{
    char* src;
    size_t  len;

    if (*path) {
        len = strlen(path);
        src = path + len - 1;

        while (*src != '/' && src != path) {
            if (*src == '.')
                return len;                 // it has an extension
            src--;
        }
    }

    len = Q_strlcat(path, ext, size);
    return len;
}

/*
==================
COM_IsFloat

Returns true if the given string is valid representation
of floating point number.
==================
*/
qboolean COM_IsFloat(const char* s)
{
    int c, dot = '.';

    if (*s == '-') {
        s++;
    }
    if (!*s) {
        return false;
    }

    do {
        c = *s++;
        if (c == dot) {
            dot = 0;
        }
        else if (!Q_isdigit(c)) {
            return false;
        }
    } while (*s);

    return true;
}

qboolean COM_IsUint(const char* s)
{
    int c;

    if (!*s) {
        return false;
    }

    do {
        c = *s++;
        if (!Q_isdigit(c)) {
            return false;
        }
    } while (*s);

    return true;
}

qboolean COM_IsPath(const char* s)
{
    int c;

    if (!*s) {
        return false;
    }

    do {
        c = *s++;
        if (!Q_ispath(c)) {
            return false;
        }
    } while (*s);

    return true;
}

qboolean COM_IsWhite(const char* s)
{
    int c;

    while (*s) {
        c = *s++;
        if (Q_isgraph(c)) {
            return false;
        }
    }

    return true;
}

int SortStrcmp(const void* p1, const void* p2)
{
    return strcmp(*(const char**)p1, *(const char**)p2);
}

int SortStricmp(const void* p1, const void* p2)
{
    return Q_stricmp(*(const char**)p1, *(const char**)p2);
}

/*
================
COM_strclr

Operates inplace, normalizing high-bit and removing unprintable characters.
Returns final number of characters, not including the NUL character.
================
*/
size_t COM_strclr(char* s)
{
    char* p;
    int c;
    size_t len;

    p = s;
    len = 0;
    while (*s) {
        c = *s++;
        c &= 127;
        if (Q_isprint(c)) {
            *p++ = c;
            len++;
        }
    }

    *p = 0;

    return len;
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char* va(const char* format, ...)
{
    va_list         argptr;
    static char     buffers[2][0x2800];
    static int      index;

    index ^= 1;

    va_start(argptr, format);
    Q_vsnprintf(buffers[index], sizeof(buffers[0]), format, argptr);
    va_end(argptr);

    return buffers[index];
}

static char     com_token[4][MAX_TOKEN_CHARS];
static int      com_tokidx;

/*
==============
COM_Parse

Parse a token out of a string.
Handles C and C++ comments.
==============
*/
char* COM_Parse(const char** data_p)
{
    int         c;
    int         len;
    const char* data;
    char* s = com_token[com_tokidx++ & 3];

    data = *data_p;
    len = 0;
    s[0] = 0;

    if (!data) {
        *data_p = NULL;
        return s;
    }

    // skip whitespace
skipwhite:
    while ((c = *data) <= ' ') {
        if (c == 0) {
            *data_p = NULL;
            return s;
        }
        data++;
    }

    // skip // comments
    if (c == '/' && data[1] == '/') {
        data += 2;
        while (*data && *data != '\n')
            data++;
        goto skipwhite;
    }

    // skip /* */ comments
    if (c == '/' && data[1] == '*') {
        data += 2;
        while (*data) {
            if (data[0] == '*' && data[1] == '/') {
                data += 2;
                break;
            }
            data++;
        }
        goto skipwhite;
    }

    // handle quoted strings specially
    if (c == '\"') {
        data++;
        while (1) {
            c = *data++;
            if (c == '\"' || !c) {
                goto finish;
            }

            if (len < MAX_TOKEN_CHARS - 1) {
                s[len++] = c;
            }
        }
    }

    // parse a regular word
    do {
        if (len < MAX_TOKEN_CHARS - 1) {
            s[len++] = c;
        }
        data++;
        c = *data;
    } while (c > 32);

finish:
    s[len] = 0;

    *data_p = data;
    return s;
}

/*
==============
COM_Compress

Operates in place, removing excess whitespace and comments.
Non-contiguous line feeds are preserved.

Returns resulting data length.
==============
*/
size_t COM_Compress(char* data)
{
    int     c, n = 0;
    char* s = data, * d = data;

    while (*s) {
        // skip whitespace
        if (*s <= ' ') {
            if (n == 0) {
                n = ' ';
            }
            do {
                c = *s++;
                if (c == '\n') {
                    n = '\n';
                }
                if (!c) {
                    goto finish;
                }
            } while (*s <= ' ');
        }

        // skip // comments
        if (s[0] == '/' && s[1] == '/') {
            n = ' ';
            s += 2;
            while (*s && *s != '\n') {
                s++;
            }
            continue;
        }

        // skip /* */ comments
        if (s[0] == '/' && s[1] == '*') {
            n = ' ';
            s += 2;
            while (*s) {
                if (s[0] == '*' && s[1] == '/') {
                    s += 2;
                    break;
                }
                if (*s == '\n') {
                    n = '\n';
                }
                s++;
            }
            continue;
        }

        // add whitespace character
        if (n) {
            *d++ = n;
            n = 0;
        }

        // handle quoted strings specially
        if (*s == '\"') {
            s++;
            *d++ = '\"';
            do {
                c = *s++;
                if (!c) {
                    goto finish;
                }
                *d++ = c;
            } while (c != '\"');
            continue;
        }

        // handle line feed escape
        if (*s == '\\' && s[1] == '\n') {
            s += 2;
            continue;
        }
        if (*s == '\\' && s[1] == '\r' && s[2] == '\n') {
            s += 3;
            continue;
        }

        // parse a regular word
        do {
            *d++ = *s++;
        } while (*s > ' ');
    }

finish:
    *d = 0;

    return d - data;
}

/*
============================================================================

                    LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_strncasecmp(const char* s1, const char* s2, size_t n)
{
    int        c1, c2;

    do {
        c1 = *s1++;
        c2 = *s2++;

        if (!n--)
            return 0;        /* strings are equal until end point */

        if (c1 != c2) {
            c1 = Q_tolower(c1);
            c2 = Q_tolower(c2);
            if (c1 < c2)
                return -1;
            if (c1 > c2)
                return 1;        /* strings not equal */
        }
    } while (c1);

    return 0;        /* strings are equal */
}

int Q_strcasecmp(const char* s1, const char* s2)
{
    int        c1, c2;

    do {
        c1 = *s1++;
        c2 = *s2++;

        if (c1 != c2) {
            c1 = Q_tolower(c1);
            c2 = Q_tolower(c2);
            if (c1 < c2)
                return -1;
            if (c1 > c2)
                return 1;        /* strings not equal */
        }
    } while (c1);

    return 0;        /* strings are equal */
}

char* Q_strcasestr(const char* s1, const char* s2)
{
    size_t l1, l2;

    l2 = strlen(s2);
    if (!l2) {
        return (char*)s1;
    }

    l1 = strlen(s1);
    while (l1 >= l2) {
        l1--;
        if (!Q_strncasecmp(s1, s2, l2)) {
            return (char*)s1;
        }
        s1++;
    }

    return NULL;
}

/*
===============
Q_strlcpy

Returns length of the source string.
===============
*/
size_t Q_strlcpy(char* dst, const char* src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = ret >= size ? size - 1 : ret;
        memcpy(dst, src, len);
        dst[len] = 0;
    }

    return ret;
}

/*
===============
Q_strlcat

Returns length of the source and destinations strings combined.
===============
*/
size_t Q_strlcat(char* dst, const char* src, size_t size)
{
    size_t ret, len = strlen(dst);

    if (len >= size) {
        Com_Error(ERR_FATAL, "%s: already overflowed", __func__);
    }

    ret = Q_strlcpy(dst + len, src, size - len);
    ret += len;

    return ret;
}

/*
===============
Q_concat

Returns number of characters that would be written into the buffer,
excluding trailing '\0'. If the returned value is equal to or greater than
buffer size, resulting string is truncated.
===============
*/
size_t Q_concat(char* dest, size_t size, ...)
{
    va_list argptr;
    const char* s;
    size_t len, total = 0;

    va_start(argptr, size);
    while ((s = va_arg(argptr, const char*)) != NULL) {
        len = strlen(s);
        if (total + len < size) {
            memcpy(dest, s, len);
            dest += len;
        }
        total += len;
    }
    va_end(argptr);

    if (size) {
        *dest = 0;
    }

    return total;
}

/*
===============
Q_vsnprintf

Returns number of characters that would be written into the buffer,
excluding trailing '\0'. If the returned value is equal to or greater than
buffer size, resulting string is truncated.
===============
*/
size_t Q_vsnprintf(char* dest, size_t size, const char* fmt, va_list argptr)
{
    int ret;

    if (size > INT_MAX)
        Com_Error(ERR_FATAL, "%s: bad buffer size", __func__);

#ifdef _WIN32
    if (size) {
        ret = _vsnprintf(dest, size - 1, fmt, argptr);
        if (ret < 0 || ret >= size - 1)
            dest[size - 1] = 0;
    }
    else {
        ret = _vscprintf(fmt, argptr);
    }
#else
    ret = vsnprintf(dest, size, fmt, argptr);
#endif

    // exploit the fact -1 becomes SIZE_MAX > size
    return (size_t)ret;
}

/*
===============
Q_vscnprintf

Returns number of characters actually written into the buffer,
excluding trailing '\0'. If buffer size is 0, this function does nothing
and returns 0.
===============
*/
size_t Q_vscnprintf(char* dest, size_t size, const char* fmt, va_list argptr)
{
    size_t ret;

    if (!size)
        return 0;

    ret = Q_vsnprintf(dest, size, fmt, argptr);
    if (ret < size)
        return ret;

    return size - 1;
}

/*
===============
Q_snprintf

Returns number of characters that would be written into the buffer,
excluding trailing '\0'. If the returned value is equal to or greater than
buffer size, resulting string is truncated.
===============
*/
size_t Q_snprintf(char* dest, size_t size, const char* fmt, ...)
{
    va_list argptr;
    size_t  ret;

    va_start(argptr, fmt);
    ret = Q_vsnprintf(dest, size, fmt, argptr);
    va_end(argptr);

    return ret;
}

/*
===============
Q_scnprintf

Returns number of characters actually written into the buffer,
excluding trailing '\0'. If buffer size is 0, this function does nothing
and returns 0.
===============
*/
size_t Q_scnprintf(char* dest, size_t size, const char* fmt, ...)
{
    va_list argptr;
    size_t  ret;

    va_start(argptr, fmt);
    ret = Q_vscnprintf(dest, size, fmt, argptr);
    va_end(argptr);

    return ret;
}

char* Q_strchrnul(const char* s, int c)
{
    while (*s && *s != c) {
        s++;
    }
    return (char*)s;
}

/*
===============
Q_memccpy

Copies no more than 'size' bytes stopping when 'c' character is found.
Returns pointer to next byte after 'c' in 'dst', or NULL if 'c' was not found.
===============
*/
void* Q_memccpy(void* dst, const void* src, int c, size_t size)
{
    byte* d = (byte*)dst; // CPP: Cast
    const byte* s = (const byte*)src; // CPP: Cast

    while (size--) {
        if ((*d++ = *s++) == c) {
            return d;
        }
    }

    return NULL;
}

void Q_setenv(const char* name, const char* value)
{
#ifdef _WIN32
    if (!value) {
        value = "";
    }
#if (_MSC_VER >= 1400)
    _putenv_s(name, value);
#else
    _putenv(va("%s=%s", name, value));
#endif
#else // _WIN32
    if (value) {
        setenv(name, value, 1);
    }
    else {
        unsetenv(name);
    }
#endif // !_WIN32
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
char* Info_ValueForKey(const char* s, const char* key)
{
    // use 4 buffers so compares work without stomping on each other
    static char value[4][MAX_INFO_STRING];
    static int  valueindex;
    char        pkey[MAX_INFO_STRING];
    char* o;

    valueindex++;
    if (*s == '\\')
        s++;
    while (1) {
        o = pkey;
        while (*s != '\\') {
            if (!*s)
                goto fail;
            *o++ = *s++;
        }
        *o = 0;
        s++;

        o = value[valueindex & 3];
        while (*s != '\\' && *s) {
            *o++ = *s++;
        }
        *o = 0;

        if (!strcmp(key, pkey))
            return value[valueindex & 3];

        if (!*s)
            goto fail;
        s++;
    }

fail:
    o = value[valueindex & 3];
    *o = 0;
    return o;
}

/*
==================
Info_RemoveKey
==================
*/
void Info_RemoveKey(char* s, const char* key)
{
    char* start;
    char    pkey[MAX_INFO_STRING];
    char* o;

    while (1) {
        start = s;
        if (*s == '\\')
            s++;
        o = pkey;
        while (*s != '\\') {
            if (!*s)
                return;
            *o++ = *s++;
        }
        *o = 0;
        s++;

        while (*s != '\\' && *s) {
            s++;
        }

        if (!strcmp(key, pkey)) {
            o = start; // remove this part
            while (*s) {
                *o++ = *s++;
            }
            *o = 0;
            s = start;
            continue; // search for duplicates
        }

        if (!*s)
            return;
    }

}


/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing.
Also checks the length of keys/values and the whole string.
==================
*/
qboolean Info_Validate(const char* s)
{
    size_t len, total;
    int c;

    total = 0;
    while (1) {
        //
        // validate key
        //
        if (*s == '\\') {
            s++;
            if (++total == MAX_INFO_STRING) {
                return false;    // oversize infostring
            }
        }
        if (!*s) {
            return false;    // missing key
        }
        len = 0;
        while (*s != '\\') {
            c = *s++;
            if (!Q_isprint(c) || c == '\"' || c == ';') {
                return false;    // illegal characters
            }
            if (++len == MAX_INFO_KEY) {
                return false;    // oversize key
            }
            if (++total == MAX_INFO_STRING) {
                return false;    // oversize infostring
            }
            if (!*s) {
                return false;    // missing value
            }
        }

        //
        // validate value
        //
        s++;
        if (++total == MAX_INFO_STRING) {
            return false;    // oversize infostring
        }
        if (!*s) {
            return false;    // missing value
        }
        len = 0;
        while (*s != '\\') {
            c = *s++;
            if (!Q_isprint(c) || c == '\"' || c == ';') {
                return false;    // illegal characters
            }
            if (++len == MAX_INFO_VALUE) {
                return false;    // oversize value
            }
            if (++total == MAX_INFO_STRING) {
                return false;    // oversize infostring
            }
            if (!*s) {
                return true;    // end of string
            }
        }
    }

    return false; // quiet compiler warning
}

/*
============
Info_SubValidate
============
*/
size_t Info_SubValidate(const char* s)
{
    size_t len;
    int c;

    len = 0;
    while (*s) {
        c = *s++;
        c &= 127;       // strip high bits
        if (c == '\\' || c == '\"' || c == ';') {
            return SIZE_MAX;  // illegal characters
        }
        if (++len == MAX_QPATH) {
            return MAX_QPATH;  // oversize value
        }
    }

    return len;
}

/*
==================
Info_SetValueForKey
==================
*/
qboolean Info_SetValueForKey(char* s, const char* key, const char* value)
{
    char    newi[MAX_INFO_STRING], * v;
    size_t  l, kl, vl;
    int     c;

    // validate key
    kl = Info_SubValidate(key);
    if (kl >= MAX_QPATH) {
        return false;
    }

    // validate value
    vl = Info_SubValidate(value);
    if (vl >= MAX_QPATH) {
        return false;
    }

    Info_RemoveKey(s, key);
    if (!vl) {
        return true;
    }

    l = strlen(s);
    if (l + kl + vl + 2 >= MAX_INFO_STRING) {
        return false;
    }

    newi[0] = '\\';
    memcpy(newi + 1, key, kl);
    newi[kl + 1] = '\\';
    memcpy(newi + kl + 2, value, vl + 1);

    // only copy ascii values
    s += l;
    v = newi;
    while (*v) {
        c = *v++;
        c &= 127;        // strip high bits
        if (Q_isprint(c))
            *s++ = c;
    }
    *s = 0;

    return true;
}

/*
==================
Info_NextPair
==================
*/
void Info_NextPair(const char** string, char* key, char* value)
{
    char* o;
    const char* s;

    *value = *key = 0;

    s = *string;
    if (!s) {
        return;
    }

    if (*s == '\\')
        s++;

    if (!*s) {
        *string = NULL;
        return;
    }

    o = key;
    while (*s && *s != '\\') {
        *o++ = *s++;
    }
    *o = 0;

    if (!*s) {
        *string = NULL;
        return;
    }

    o = value;
    s++;
    while (*s && *s != '\\') {
        *o++ = *s++;
    }
    *o = 0;

    *string = s;
}

/*
==================
Info_Print
==================
*/
void Info_Print(const char* infostring)
{
    char    key[MAX_INFO_STRING];
    char    value[MAX_INFO_STRING];

    while (1) {
        Info_NextPair(&infostring, key, value);
        if (!infostring)
            break;

        if (!key[0])
            strcpy(key, "<MISSING KEY>");

        if (!value[0])
            strcpy(value, "<MISSING VALUE>");

        Com_Printf("%-20s %s\n", key, value);
    }
}

