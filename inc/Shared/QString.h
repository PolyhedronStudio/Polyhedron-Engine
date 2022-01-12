// License here.
#ifndef __SHARED__QSTRING_H__
#define __SHARED__QSTRING_H__

static inline int Q_tolower(int c) {
    if (Q_isupper(c)) {
        c += ('a' - 'A');
    }
    return c;
}

static inline int Q_toupper(int c) {
    if (Q_islower(c)) {
        c -= ('a' - 'A');
    }
    return c;
}

static inline char* Q_strlwr(char* s) {
    char* p = s;

    while (*p) {
        *p = Q_tolower(*p);
        p++;
    }

    return s;
}

static inline char* Q_strupr(char* s) {
    char* p = s;

    while (*p) {
        *p = Q_toupper(*p);
        p++;
    }

    return s;
}

static inline int Q_charhex(int c) {
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return -1;
}

// converts quake char to ASCII equivalent
static inline int Q_charascii(int c) {
    if (Q_isspace(c)) {
        // white-space chars are output as-is
        return c;
    }
    c &= 127; // strip high bits
    if (Q_isprint(c)) {
        return c;
    }
    switch (c) {
        // handle bold brackets
    case 16: return '[';
    case 17: return ']';
    }
    return '.'; // don't output control chars, etc
}

/*
=============
Q_strncpyz

Safe strncpy that ensures a trailing zero
=============
*/
inline void Q_strncpyz(char* dest, const char* src, int32_t destsize) {
    if (!src) {
        Com_Error(ERR_FATAL, "Q_strncpyz: NULL src");
    }
    if (destsize < 1) {
        Com_Error(ERR_FATAL, "Q_strncpyz: destsize < 1");
    }

    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = 0;
}

// portable case insensitive compare
int Q_strcasecmp(const char* s1, const char* s2);
int Q_strncasecmp(const char* s1, const char* s2, size_t n);
char* Q_strcasestr(const char* s1, const char* s2);

#define Q_stricmp   Q_strcasecmp
#define Q_stricmpn  Q_strncasecmp
#define Q_stristr   Q_strcasestr

char* Q_strchrnul(const char* s, int c);
void* Q_memccpy(void* dst, const void* src, int c, size_t size);
void Q_setenv(const char* name, const char* value);

char* COM_SkipPath(const char* pathname);
void COM_StripExtension(const char* in, char* out, size_t size);
void COM_FileBase(char* in, char* out);
void COM_FilePath(const char* in, char* out, size_t size);
size_t COM_DefaultExtension(char* path, const char* ext, size_t size);
char* COM_FileExtension(const char* in);

#define COM_CompareExtension(in, ext) \
    Q_strcasecmp(COM_FileExtension(in), ext)

qboolean COM_IsFloat(const char* s);
qboolean COM_IsUint(const char* s);
qboolean COM_IsPath(const char* s);
qboolean COM_IsWhite(const char* s);

char* COM_Parse(const char** data_p);
// data is an in/out parm, returns a parsed out token
size_t COM_Compress(char* data);

int SortStrcmp(const void* p1, const void* p2);
int SortStricmp(const void* p1, const void* p2);

size_t COM_strclr(char* s);

// buffer safe operations
size_t Q_strlcpy(char* dst, const char* src, size_t size);
size_t Q_strlcat(char* dst, const char* src, size_t size);

size_t Q_concat(char* dest, size_t size, ...) q_sentinel;

size_t Q_vsnprintf(char* dest, size_t size, const char* fmt, va_list argptr);
size_t Q_vscnprintf(char* dest, size_t size, const char* fmt, va_list argptr);
size_t Q_snprintf(char* dest, size_t size, const char* fmt, ...) q_printf(3, 4);
size_t Q_scnprintf(char* dest, size_t size, const char* fmt, ...) q_printf(3, 4);

// Inline utility.
inline const char* Vec3ToString(const vec3_t& v, qboolean rounded = true) {
    // 64 should be enough, no? This function shouldn't be used outside of
    // debugging purposes anyhow...
    static std::string str[64];
    static int strIndex = 0;

    str[strIndex = (strIndex > 7 ? 0 : strIndex + 1)] = vec3_to_str(v, rounded);
    return str[strIndex].c_str();
}

char* va(const char* format, ...) q_printf(1, 2);

#endif // __SHARED__QSTRING_H__
