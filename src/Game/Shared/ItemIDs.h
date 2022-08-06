/***
*
*	License here.
*
*	@file
*
*	SharedGame ItemIDs. Each item has its own identifier, including weaponry.
* 
***/
#pragma once



//! Shared header.
#include "../../Shared/Shared.h"

/**
*   @brief Item identifiers.
**/
struct ItemID { 
    /***
    * Weapons.
    ***/
    //! Bare hands.
    static constexpr uint32_t Barehands     = 1;
    //! Pistol.
    static constexpr uint32_t Beretta       = 2;
    //! SMG.
    static constexpr uint32_t SMG           = 3;
    //! Shotgun.
    static constexpr uint32_t Shotgun       = 4;
    //! Last item slot that can be used for weapons.
    static constexpr uint32_t MaxWeapons    = 64;

    /***
    * Ammo.
    ***/
    //! 9 millimeter ammo.
    static constexpr uint32_t Ammo9mm       = 65;
    //! 9 millimeter ammo.
    static constexpr uint32_t MaxAmmos      = 85;    

    /***
    * Medical Stats Items.
    ***/
    //! Mega Health.
    static constexpr uint32_t MegaHealth = 86;

    /**
    *   ... :-)
    **/
    //! Total amount of items.
    static constexpr uint32_t Total = 87; 
    //! Maximum amount of allowed items.
    static constexpr uint32_t Maximum = 255;
};
