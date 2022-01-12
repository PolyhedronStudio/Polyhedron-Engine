// LICENSE HERE.

//
// os/sdl2/time.cpp
//
//
// SDL2 Time function implementations.
//

#include "Shared/Shared.h"
#include "system/system.h"
#include <SDL.h>


//
//=============================================================================
// Sys_Microseconds
// 
// Returns SDL2 time in Microseconds.
//=============================================================================
//
//uint64_t Sys_Microseconds(void) {
//    static Uint64 base = 0;
//    static Uint64 freq = 0;
//
//    // Initialize timer first time around.
//    if (!base) {
//        // Init SDL Timer for servers?
//        if (!(SDL_WasInit(SDL_INIT_EVERYTHING) && SDL_INIT_TIMER)) {
//            SDL_InitSubSystem(SDL_INIT_TIMER);
//        }
//        base = SDL_GetPerformanceCounter();
//        freq = SDL_GetPerformanceFrequency();
//    }
//    return 1000000ULL * (SDL_GetPerformanceCounter() - base) / freq;
//}
//
////
////=============================================================================
//// Sys_Milliseconds
//// 
//// Returns SDL2 time in Milliseconds.
////=============================================================================
////
//int64_t Sys_Milliseconds(void) {
//    return Sys_Microseconds() / 1000;
//}
