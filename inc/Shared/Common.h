// License here.
#pragma once

enum ErrorType{
    ERR_FATAL,          // Exit the entire game with a popup window
    ERR_DROP,           // Print to console and disconnect from game
    ERR_DISCONNECT,     // Like drop, but not an error
    ERR_RECONNECT       // Make server broadcast 'reconnect' message
};

enum PrintType{
    PRINT_ALL,          // General messages
    PRINT_TALK,         // Print in green color
    PRINT_DEVELOPER,    // Only print when "developer 1"
    PRINT_WARNING,      // Print in yellow color
    PRINT_ERROR,        // Print in red color
    PRINT_NOTICE        // Print in cyan color
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
// WATISDEZE: We don't want these defined in CLGame.h
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