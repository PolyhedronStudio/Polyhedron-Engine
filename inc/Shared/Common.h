// License here.
#pragma once

enum ErrorType{
    ERR_FATAL,          // Exit the entire game with a popup window
    ERR_DROP,           // Print to console and disconnect from game
    ERR_DISCONNECT,     // Like drop, but not an error
    ERR_RECONNECT       // Make server broadcast 'reconnect' message
};

//struct ErrorType {
//    //! Exit the entire game with a popup window.
//    static constexpr int32_t Fatal      = 0;
//    //! Print error to console and disconnect from game.
//    static constexpr int32_t Drop       = 1;
//    //! Disconnect from game, but unlike drop, doesn't print to console.
//    static constexpr int32_t Disconnect = 2;
//    //! Make server broadcast 'reconnect' message
//    static constexpr int32_t Reconnect  = 3;
//};

enum PrintType{
    PRINT_ALL,          // General messages
    PRINT_TALK,         // Print in green color
    PRINT_DEVELOPER,    // Only print when "developer 1"
    PRINT_WARNING,      // Print in yellow color
    PRINT_ERROR,        // Print in red color
    PRINT_NOTICE        // Print in cyan color
};

//struct PrintType {
//    //! General messages.
//    static constexpr int32_t All        = 0;
//    //! Print in green color.
//    static constexpr int32_t Talk       = 1;
//    //! Only print when cvar 'developer' >= 1
//    static constexpr int32_t Developer  = 2;
//    //! Print in orange color.
//    static constexpr int32_t Warning    = 3;
//    //! Print in red color.
//    static constexpr int32_t Error      = 4;
//    //! Print in cyan color.
//    static constexpr int32_t Notice     = 5;
//};

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

//struct LoadState {
//    static constexpr int32_t None       = 0;
//    static constexpr int32_t Map        = 1;
//    static constexpr int32_t Models     = 2;
//    static constexpr int32_t Images     = 3;
//    static constexpr int32_t Clients    = 4;
//    static constexpr int32_t Sounds     = 5;
//};

//-----------------
// WATISDEZE: We don't want these defined in CLGame.h
//-----------------
void    Com_Error(int32_t errorType, const char* fmt, ...)
q_noreturn q_printf(2, 3);
#ifndef CGAME_INCLUDE
void    Com_LPrintf(int32_t printType, const char* fmt, ...)
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