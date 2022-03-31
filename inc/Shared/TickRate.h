// License here.
//
// This file contains all constexpr that define how fast (what framerate, etc) the engine
// operates at.
//
#pragma once

// From here on out (game modules) we use chrono literals namespace by default.
using namespace std::chrono_literals;



//! Measurement used keeping track of time in the game.
using GameTime = std::chrono::milliseconds;
//! Normalized(0 to 1.0) 0.#th of a second per frame time measurement.
using Frametime = std::chrono::duration<double>;


/**
*	@brief	This is the actual framerate which the game logic will simulate at.
*			(Client, Server and including Shared game modules.)
**/
static constexpr uint64_t   BASE_HZ = 60.0;

// Calclate all related values we need to make it work smoothly even if we have
// a nice 250fps, the game must run at 50fps.
static constexpr uint64_t   BASE_FRAMERATE = BASE_HZ;
static constexpr double     BASE_FRAMETIME = 1000.0 / BASE_FRAMERATE;
static constexpr double     BASE_1_FRAMETIME = 1.0 / BASE_FRAMETIME;
static constexpr double     BASE_FRAMETIME_1000 = BASE_FRAMETIME / 1000.0;

// Used for old-school hardcoded values.
static constexpr int64_t    BASE_FRAMEDIVIDER = (int64_t)BASE_FRAMERATE / 10.0;

/**
*	@brief	This is the actual framerate which the game logic will simulate at.
*			(Client, Server and including Shared game modules.)
**/
//! Frames Per Second that the simulation runs at.
//static constexpr uint64_t BASE_FRAMERATE		= 60;
//
////! Milliseconds per frame.
//static constexpr Time BASE_FRAMETIME = Time(1000ms / BASE_FRAMERATE);
//
////! Seconds per frame.
//static constexpr TimeFrame BASE_1_FRAMETIME = TimeFrame(1.0 / BASE_FRAMETIME.count());
//
////! Frametime: 0.###th of a Second.
//static constexpr TimeFrame BASE_FRAMETIME_1000 = duration_cast<Time>(BASE_FRAMETIME);
