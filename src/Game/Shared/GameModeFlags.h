/***
*
*	License here.
*
*	@file
*
*	SharedGame GameMode Flags. Originally you'd go nuts in Quake 2 because these would
*	be inquired about all over the place within random if statements.
*
*	That problem has now long been resolved thanks to the actual use of GameMode classes.
*
*	Even so, it's probably not a bad idea to keep some flags around. In fact, some are still
*	being made use of. :-)
* 
***/
#pragma once


/**
*   @brief  GameMode specific flags such as FixedFOV, InstantItems, No Friendly Fire etc.
**/
struct GameModeFlags {
    //! When set falling doesn't apply any demage to clients.
    static constexpr int16_t NoFallingDamage    = (1 << 0);
	//! Whether the server should keep the game going over and over within the same map(level).
    static constexpr int16_t SameLevel          = (1 << 1);
	//! Base Teams on Skins.
    static constexpr int16_t SkinTeams          = (1 << 2);
	//! Base Teams on Models.
    static constexpr int16_t ModelTeams         = (1 << 3);
    //! When set, instantly respawns a client after death. Denying them the chance to wait and press a key first.
    static constexpr int16_t ForceRespawn       = (1 << 4);
    //! Whether to use a fixed Field Of View or not.
    static constexpr int16_t FixedFOV           = (1 << 5);
};

///**
//*   @brief  GameMode specific flags such as FixedFOV, InstantItems, No Friendly Fire etc.
//**/
//struct GameModeFlags {
//    static constexpr int16_t NoHealthItems      = (1 << 0);
//    static constexpr int16_t NoItems            = (1 << 1);
//    static constexpr int16_t NoFallingDamage    = (1 << 2);
//    static constexpr int16_t SameLevel          = (1 << 3);
//    static constexpr int16_t SkinTeams          = (1 << 4);
//    static constexpr int16_t ModelTeams         = (1 << 5);
//    static constexpr int16_t NoFriendlyFire     = (1 << 6);
//    static constexpr int16_t ForceRespawn       = (1 << 7);
//    static constexpr int16_t InfiniteAmmo       = (1 << 8);
//    static constexpr int16_t FixedFOV           = (1 << 9);
//};