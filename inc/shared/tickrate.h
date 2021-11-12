// License here.
//
// This file contains all constexpr that define how fast (what framerate, etc) the engine
// operates at.
//
#pragma once

// Set here how fast you want the tick rate to be.
static constexpr uint32_t   BASE_HZ = 50.0;

// Calclate all related values we need to make it work smoothly even if we have
// a nice 250fps, the game must run at 60fps.
static constexpr uint32_t   BASE_FRAMERATE = BASE_HZ;
static constexpr double     BASE_FRAMETIME = 1000.0 / BASE_FRAMERATE;
static constexpr double     BASE_1_FRAMETIME = 1.0 / BASE_FRAMETIME;
static constexpr double     BASE_FRAMETIME_1000 = BASE_FRAMETIME / 1000.0;

// Used for old-school hardcoded values.
static constexpr int32_t    BASE_FRAMEDIVIDER = (int32_t)BASE_FRAMERATE / 10.0;