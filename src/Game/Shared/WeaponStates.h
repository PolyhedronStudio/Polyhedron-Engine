/***
*
*	License here.
*
*	@file
*
*	SharedGame WeaponStates are used to determine what kind of actual logic
*	to process for the current state of the weapon.
*
*	Specifics are explained below.
* 
***/
#pragma once


/**
*   @brief  Used to determine the state a weapon is currently in.
**/
struct WeaponState {
    //! None state, meaning it has no logic to process.
    static constexpr int32_t None = 0;
    //! Draw state, when set it'll process a draw animation.
    static constexpr int32_t Holster = 1;
    //! Holster state, when set it'll process a holster animation.
    static constexpr int32_t Draw = 2;
    //! Idle state, when set it'll play idle animations at random intervals.
    static constexpr int32_t Idle = 3;
    //! Reload, when set it'll try and reload.
    static constexpr int32_t Reload = 4;
    //! Primary Fire. Speaks for itself.
    static constexpr int32_t PrimaryFire = 5;
    //! Secondary Fire. This too, speaks for itself.
    static constexpr int32_t SecondaryFire = 6;
};