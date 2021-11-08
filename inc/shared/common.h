// License here.
#pragma once

enum ErrorType{
    ERR_FATAL,          // exit the entire game with a popup window
    ERR_DROP,           // print to console and disconnect from game
    ERR_DISCONNECT,     // like drop, but not an error
    ERR_RECONNECT       // make server broadcast 'reconnect' message
};

enum PrintType{
    PRINT_ALL,          // general messages
    PRINT_TALK,         // print in green color
    PRINT_DEVELOPER,    // only print when "developer 1"
    PRINT_WARNING,      // print in yellow color
    PRINT_ERROR,        // print in red color
    PRINT_NOTICE        // print in cyan color
};

//
// Contains the client load states, clg_local.h can expand upon it with custom
// states. They can send a text name for the loading state to show in display.
//
enum LoadState {
    LOAD_NONE,
    LOAD_MAP,
    LOAD_MODELS,
    LOAD_IMAGES,
    LOAD_CLIENTS,
    LOAD_SOUNDS
};

//-----------------
// WATISDEZE: We don't want these defined in clgame.h
//-----------------
void    Com_Error(ErrorType code, const char* fmt, ...)
q_noreturn q_printf(2, 3);
#ifndef CGAME_INCLUDE
void    Com_LPrintf(PrintType type, const char* fmt, ...)
q_printf(2, 3);
#define Com_Printf(...) Com_LPrintf(PRINT_ALL, __VA_ARGS__)
#define Com_DPrintf(...) Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__)
#define Com_WPrintf(...) Com_LPrintf(PRINT_WARNING, __VA_ARGS__)
#define Com_EPrintf(...) Com_LPrintf(PRINT_ERROR, __VA_ARGS__)
#endif // CGAME_INCLUDE

// game print flags
#define PRINT_LOW           0       // pickup messages
#define PRINT_MEDIUM        1       // death messages
#define PRINT_HIGH          2       // critical messages
#define PRINT_CHAT          3       // chat messages 