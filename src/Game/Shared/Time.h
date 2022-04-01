/***
*
*	License here.
*
*	@file
*
*	Contains declarations and definitions of types used for handling time in
*	both of the game modules. It makes use of std::chrono to simplify things
*	for us.
*
***/
#pragma once


//! Time of a game's frame.
static constexpr Frametime FRAMETIME_S = Frametime(1.0f / BASE_HZ);

//! Literal to return the 'frame counts'. Should be avoided to use.
using Frames = std::chrono::duration<int64_t, std::ratio<1, BASE_HZ>>;
static constexpr Frames operator""_hz(uint64_t frame) {
	return Frames(frame);
}

//! Milliseconds per frame.
static constexpr GameTime FRAMERATE_MS = duration_cast<GameTime>(FRAMETIME_S);