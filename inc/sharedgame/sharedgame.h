/*
// LICENSE HERE.

//
// sharedgame/sharedgame.h
//
//
// Contains the definitions of the import, and export API structs.
//
*/

#ifndef __SHAREDGAME_SHAREDGAME_H__
#define __SHAREDGAME_SHAREDGAME_H__

//-----------------
// Button Bits
// 
// These are used for game logic. They are set in clg_input.cpp. One is free
// to use up the remaining slots for their own custom needs.
//-----------------
constexpr uint32_t BUTTON_ATTACK		= (1 << 0);
constexpr uint32_t BUTTON_USE			= (1 << 1);
constexpr uint32_t BUTTON_WALK			= (1 << 2);
constexpr uint32_t BUTTON_UNUSED_0		= (1 << 3);
constexpr uint32_t BUTTON_UNUSED_1		= (1 << 4);
constexpr uint32_t BUTTON_UNUSED_2		= (1 << 5);
constexpr uint32_t BUTTON_UNUSED_3		= (1 << 6);
constexpr uint32_t BUTTON_ANY			= (1 << 7);

#endif // __SHAREDGAME_SHAREDGAME_H__