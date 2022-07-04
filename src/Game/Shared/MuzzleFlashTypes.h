/***
*
*	License here.
*
*	@file
*
*	Muzzle Flashes are effects applied to an entity itself and as such take place at its origin.
*
***/
#pragma once


/**
*   @brief  Muzzle Flashes are effects applied to an entity itself and as such
*           take place at its origin.
**/
struct MuzzleFlashType {
    //! Shows a respawn particle effect at the entity's origin.
    static constexpr uint8_t Respawn     = 0;
    //! Shows an item specific respawn particle effect at the entity's origin.
    static constexpr uint8_t ItemRespawn = 1;
    //! Shows a login particle effect. (When a client connects and spawns for the first time in a game.)
    static constexpr uint8_t Login       = 2;
    //! Shows a logout particle effect. (When a client disconnects from a game.)
    static constexpr uint8_t Logout      = 3;

    //! Shows a Pistol muzzleflash effect.
    static constexpr uint8_t Smg45        = 16;
    //! Shows a MachineGun muzzleflash effect.
    static constexpr uint8_t MachineGun     = 17;
    //! Shows a Shotgun muzzleflash effect.
    static constexpr uint8_t Shotgun        = 18;
    //! Shows a SuperShotgun muzzleflash effect.
    static constexpr uint8_t SuperShotgun   = 19;
};

/**
*   @brief  Monster entity specific muzzleflash types.
**/
struct MonsterMuzzleFlashType {
    static constexpr int32_t FictionalMonsterWeapon = 0;
};