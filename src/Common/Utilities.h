/***
*
*	License here.
*
*	@file
*
*	Common Utilities.
* 
***/
#pragma once



/***
*
*
*	Various.
*
*
***/
/**
*	@brief	Keep track of paged memory blocks.
**/
void Com_PageInMemory(void *buffer, size_t size);
/**
*	@brief	Helper function to parse an OpenGL-style extension string.
**/
#if USE_REF == REF_GL
unsigned Com_ParseExtensionString(const char *s, const char *const extnames[]);
#endif



/***
*
*
*	Color indexing and parsing, for text display usage.
*
*
***/
enum color_index_t {
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_ORANGE,
    COLOR_PURPLE,
    COLOR_RED,
    COLOR_WHITE,

	COLOR_DEVELOPER,
	COLOR_DEVELOPER_WARNING,

	COLOR_POLYHEDRON,

    COLOR_ALT,
    COLOR_NONE
};

//! Textual representation of all colors serving as a color matching table.
extern const char *const colorStringTable[12];

/**
*	@brief	Parses the color, the string can be a name in our names table, or 
*			Returns COLOR_NONE in case of error.
**/
color_index_t Com_ParseColor(const char *s, color_index_t last);



/***
*
*
*	String Format, and Time.
*
*
***/
#if USE_CLIENT
qboolean Com_ParseTimespec(const char *s, int *frames);
#endif

size_t Com_FormatTime(char *buffer, size_t size, time_t t);
size_t Com_FormatTimeLong(char *buffer, size_t size, time_t t);
size_t Com_TimeDiff(char *buffer, size_t size, time_t *p, time_t now);
size_t Com_TimeDiffLong(char *buffer, size_t size, time_t *p, time_t now);

size_t Com_FormatSize(char *dest, size_t destsize, off_t bytes);
size_t Com_FormatSizeLong(char *dest, size_t destsize, off_t bytes);



/***
*
*
*	String Hashing.
*
*
***/
/**
*	@brief	A case-sensitive simple string hash.
**/
unsigned Com_HashString(const char *s, unsigned size);
/**
*	@brief	A case-insensitive version of Com_HashString that hashes up to 'len'
*			characters.
**/
unsigned Com_HashStringLen(const char *s, size_t len, unsigned size);



/***
*
*
*	Wildcard Comparison.
*
*
***/
qboolean Com_WildCmpEx(const char *filter, const char *string, int term, qboolean ignorecase);
#define Com_WildCmp(filter, string)  Com_WildCmpEx(filter, string, 0, false)