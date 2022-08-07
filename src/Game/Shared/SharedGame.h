/***
*
*	License here.
*
*	@file
*
*	Contains constants, struct-enums, and various other base types that are 
*	shared across client and server game dlls.
*
***/
#pragma once



/**
*
*	Shared Constants:
*
**/
//! Size of the dead body entity queue.
static constexpr int32_t BODY_QUEUE_SIZE = 8;


/**
*
*	Shared Struct-Enums:
*
**/
/**
*   Take Damage.
**/
struct TakeDamage {
    //! Will NOT take damage if hit.
    static constexpr int32_t No     = 0;  
    //! WILL take damage if hit
    static constexpr int32_t Yes    = 1;
    //! When auto targeting is enabled, it'll recognizes this
    static constexpr int32_t Aim    = 2; 
};

/**
*   Dead Flags.
**/
struct DeadFlags {
    static constexpr int32_t Alive = 0;
    static constexpr int32_t Dead = 1;
};

/**
*   Gib Types.
**/
struct GibType {
    static constexpr int32_t Organic = 0;
    static constexpr int32_t Metallic = 1;
};

//! Button Bits.
#include "ButtonBits.h"

//! Entity Effect Types.
#include "EntityEffectTypes.h"

//! Entity Flags.
#include "EntityFlags.h"

// GameMode Flags.
#include "GameModeFlags.h"
//! Item IDs.
#include "ItemIDs.h"
//! MoveTypes.
#include "MoveTypes.h"
//! MuzzleFlashes.
#include "MuzzleFlashTypes.h"
//! Protocol.
#include "Protocol.h"
//! Render Draw Flags.
#include "RenderDrawFlags.h"
//! Temporary Entity Events.
#include "TempEntityEvents.h"
//! Game Framerate/Time.
#include "Time.h"
//! Water Levels.
#include "WaterLevels.h"
//! Weapon States.
#include "WeaponStates.h"


//! Actual base entity system includes.
#include "Entities.h"


/**
*   Skeletal Animation
**/


/**
*   Game Time Utilities.
**/


/**
*	Tracing.
**/


/**
*   Entity Framework
**/


/**
*   Protocol
**/

/**
*	Physics
**/
