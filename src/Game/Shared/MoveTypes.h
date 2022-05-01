/***
*
*	License here.
*
*	@file
*
*	SharedGame Entity MoveTypes.
* 
***/
#pragma once


/**
*   Entity MoveTypes.
**/
struct MoveType {
    //! An entity that never moves at all.
    static constexpr int32_t None       = 0;
    //! Spectator movement allows for flying around like noclip, but it does clip to BSP.
    static constexpr int32_t Spectator  = 1;
    //! Similar to spectator movement however it does not clip.
    static constexpr int32_t NoClip     = 2;
    //! No clip to world, push on box contact.
    static constexpr int32_t Push = 3;
    //! No clip to world, stops on box contact
    static constexpr int32_t Stop = 4;

    //! This entity makes use of the player movement code. 
    static constexpr int32_t PlayerMove = 10;    // Gravity. (Player Movement entities use this.)
    //! This entity makes use of the step movement physics.
    static constexpr int32_t Step       = 11;    // Fixed distance per frame, impacted by gravity, supports special edge handling.
    //! Similar to step movement however it does not care for gravity.
    static constexpr int32_t Fly        = 12;
    //! Similar to step fly movement, but with an extra size bounding box for hitting other entities. It's for missiles after all.
    static constexpr int32_t FlyMissile = 13;
    //! Toss is used for when dropping items, dying entities etc. It simply does basic velocity and gravity movement.
    static constexpr int32_t Toss       = 14;
    //! Similar to Toss but doesn't halt to a stop when having landed on-ground.
    static constexpr int32_t TossSlide  = 15;
    //! Similar to Toss but bounces back from the impacted surface instead.
    static constexpr int32_t Bounce     = 16;
};