// License here.
#ifndef __SHARED__QSTRING_H__
#define __SHARED__QSTRING_H__

static inline int PH_ToLower(int c) {
    if (PH_IsUpper(c)) {
        c += ('a' - 'A');
    }
    return c;
}

static inline int PH_ToUpper(int c) {
    if (PH_IsLower(c)) {
        c -= ('a' - 'A');
    }
    return c;
}

static inline char* PH_StringLower(char* s) {
    char* p = s;

    while (*p) {
        *p = PH_ToLower(*p);
        p++;
    }

    return s;
}

static inline char* PH_StringUpper(char* s) {
    char* p = s;

    while (*p) {
        *p = PH_ToUpper(*p);
        p++;
    }

    return s;
}

static inline int PH_CharHex(int c) {
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
static inline int PH_CharASCII(int c) {
    if (PH_IsSpace(c)) {
        // white-space chars are output as-is
        return c;
    }
    c &= 127; // strip high bits
    if (PH_IsPrint(c)) {
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
PH_StringCopyZ

Safe strncpy that ensures a trailing zero
=============
*/
inline void PH_StringCopyZ(char* dest, const char* src, int32_t destsize) {
    if (!src) {
        Com_Error(ErrorType::Fatal, "PH_StringCopyZ: NULL src");
    }
    if (destsize < 1) {
        Com_Error(ErrorType::Fatal, "PH_StringCopyZ: destsize < 1");
    }

    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = 0;
}

// portable case insensitive compare
int PH_StringCaseCompare(const char* s1, const char* s2);
int PH_StringNumberCaseCompare(const char* s1, const char* s2, size_t n);
char* PH_StringCaseString(const char* s1, const char* s2);


static inline const int PH_StringCompare(const char* s1, const char* s2) { return PH_StringCaseCompare(s1, s2); }
static inline const int PH_StringCompareN(const char* s1, const char* s2, size_t n) { return PH_StringNumberCaseCompare(s1, s2, n); }
static inline char* PH_StringiString(const char* s1, const char* s2) { return PH_StringCaseString(s1, s2); }


char* PH_StringCharNul(const char* s, int c);
void* Q_memccpy(void* dst, const void* src, int c, size_t size);
void Q_setenv(const char* name, const char* value);

char* COM_SkipPath(const char* pathname);
void COM_StripExtension(const char* in, char* out, size_t size);
void COM_FileBase(char* in, char* out);
void COM_FilePath(const char* in, char* out, size_t size);
size_t COM_DefaultExtension(char* path, const char* ext, size_t size);
char* COM_FileExtension(const char* in);

//#define COM_CompareExtension(in, ext) \
//    PH_StringCaseCompare(COM_FileExtension(in), ext)
static inline int COM_CompareExtension(const char* in, const char* ext) { return PH_StringCaseCompare(COM_FileExtension(in), ext); }

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
