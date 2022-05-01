/***
*
*	License here.
*
*	@file
*
*	SharedGame Entity Fkags. These are set and used during in-game gameplay.
* 
***/
#pragma once


/**
*   @brief Entity Flags. These are set in-game during gameplay.
**/
struct EntityFlags {
    static constexpr int32_t Fly            = 1;
    static constexpr int32_t Swim           = 2;        //! Implied immunity to drowining
    static constexpr int32_t ImmuneLaser    = 4;
    static constexpr int32_t InWater        = 8;
    static constexpr int32_t GodMode        = 16;
    static constexpr int32_t NoTarget       = 32;
    static constexpr int32_t ImmuneToSlime  = 64;
    static constexpr int32_t ImmuneToLava   = 128;
    static constexpr int32_t PartiallyOnGround = 256;   //! Not all corners are valid
    static constexpr int32_t WaterJump      = 512;      //! Player jumping out of water
    static constexpr int32_t TeamSlave      = 1024;     //! Not the first on the team
    static constexpr int32_t NoKnockBack    = 2048;
    static constexpr int32_t PowerArmor     = 4096;     //! Power armor (if any) is active
    static constexpr int32_t Respawn        = 0x80000000;   //! Used for item respawning
};
