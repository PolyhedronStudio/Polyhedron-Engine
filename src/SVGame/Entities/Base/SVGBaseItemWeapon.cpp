/*
// LICENSE HERE.

//
// SVGBaseItemWeapon.cpp
//
// Base class to create item entities from.
//
// Gives the following functionalities:
// TODO: Explain what.
//
*/
#include "../../ServerGameLocal.h"  // SVGame.
#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"        // Util funcs.
#include "../../Physics/StepMove.h" // Stepmove funcs.

#include <SharedGame/SkeletalAnimation.h>

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseItemWeapon.h"

// Game world.
#include "../../World/Gameworld.h"

//
// Constructor/Deconstructor.
//
SVGBaseItemWeapon::SVGBaseItemWeapon(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) {

}
SVGBaseItemWeapon::~SVGBaseItemWeapon() {

}



//
// Interface functions. 
//
//
//===============
// SVGBaseItemWeapon::Precache
//
//===============
//
void SVGBaseItemWeapon::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache view and world models for the given weapon.
    SVG_PrecacheModel(GetViewModel());
    SVG_PrecacheModel(GetWorldModel());
}

//
//===============
// SVGBaseItemWeapon::Spawn
//
//===============
//
void SVGBaseItemWeapon::Spawn() {
    // Always call parent class method.
    Base::Spawn();
    
    // Set the weapon item world model.
    SetModel(GetWorldModel());

    // Set the config string for this item.
    SVG_SetConfigString(ConfigStrings::Items + itemIdentifier, displayString);
}

//
//===============
// SVGBaseItemWeapon::Respawn
//===============
//
void SVGBaseItemWeapon::Respawn() {
    Base::Respawn();
}

//
//===============
// SVGBaseItemWeapon::PostSpawn
//===============
//
void SVGBaseItemWeapon::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// SVGBaseItemWeapon::Think
//===============
//
void SVGBaseItemWeapon::Think() {
    // Always call parent class method.
    Base::Think();
}


//===============
// SVGBaseItemWeapon::SetRespawn
//===============
void SVGBaseItemWeapon::SetRespawn(const float delay) {
    Base::SetRespawn(delay);
}



/***
* 
* 
*   Instance functions.
* 
* 
***/
void SVGBaseItemWeapon::InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Huuueeeee
    if (!player || !client) {
        return;
    }

    /**
    *   Pre-Animation and State logic.
    **/
    if (client->weaponState.currentAnimationFrame < 0) {
        // Call upon Animation Finished callback.
        InstanceWeaponOnAnimationFinished(player, weapon, client);
        
        // Set it back to 0 to prevent it from repeating the animation finished callback.
        client->weaponState.currentAnimationFrame = 0;
        //return;
    }

    /**
    *   Weapon State Machine Logic. (It's very minimal nonetheless.)
    **/
    // See if we got a queued state, if we do, override our current weaponstate.
    if (client->weaponState.queuedState != -1){
        // Set the timestamp of when this current state got set.
        client->weaponState.stateTimestamp = level.timeStamp;

        // Make the queued weapon state our active one. NOTE: SetCurrentState also calls upon OnSwitchState.
        InstanceWeaponSetCurrentState(player, client, client->weaponState.queuedState);

	    // Reset it to -1.
        client->weaponState.queuedState = -1;
        
        gi.DPrintf("WeaponState switched to: %i at timestamp: %i\n", client->weaponState.currentState, client->weaponState.stateTimestamp);
    }

    /**
    *   Weapon Animation Processing.
    **/
    SG_FrameForTime(&client->weaponState.currentAnimationFrame, 
        level.timeStamp, 
        client->playerState.gunAnimationStartTime, 
        client->playerState.gunAnimationFrametime, 
        client->playerState.gunAnimationStartFrame,
        client->playerState.gunAnimationEndFrame, 
        client->playerState.gunAnimationLoopCount, 
        client->playerState.gunAnimationForceLoop
    );
}

/**
* @brief   A callback which can be implemented by weapons in order to fire code one time
*          when the weapon has switched to a new state. 
* 
*          (Mainly used for setting animations, but can be used for anything really.)
* 
* @param newState The current new state that the weapon resides in.
* @param oldState Old previous state the weapon was residing in.
**/
void SVGBaseItemWeapon::InstanceWeaponOnSwitchState(SVGBasePlayer *player, ServerClient *client, int32_t newState, int32_t oldState) {}

void SVGBaseItemWeapon::InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {}
/**
* @brief    Sets the weapon's animation properties.
* @param    frameTime Determines the time taken for each frame, this can be used to either speed up or slow down an animation.
**/
void SVGBaseItemWeapon::InstanceWeaponSetAnimation(SVGBasePlayer *player, ServerClient *client, uint32_t startTime, uint16_t startFrame, uint16_t endFrame, uint32_t loopCount, qboolean forceLoop, float frameTime) {
    // Sanity.
    if (!client) {
        return;
    }

    // Time properties.
    client->playerState.gunAnimationStartTime   = startTime;
    client->playerState.gunAnimationFrametime   = frameTime;

    // Animation properties.
    client->playerState.gunAnimationStartFrame  = startFrame;
    client->playerState.gunAnimationEndFrame    = endFrame;

    // Loop properties.
    client->playerState.gunAnimationLoopCount   = loopCount;
    client->playerState.gunAnimationForceLoop   = forceLoop;
}

/**
*   @brief  Instantly sets the current state.
**/
void SVGBaseItemWeapon::InstanceWeaponSetCurrentState(SVGBasePlayer *player, ServerClient* client, int32_t state) {
    if (!player || !client) {
        return;
    }

    // Store old current state so we can use it for our call to OnSwitchState.
    int32_t oldWeaponState = client->weaponState.currentState;

    // Assign new state.
    client->weaponState.currentState = state;

    // Only call on switch state in case the state isn't identical to old state.
    if (oldWeaponState != state) {
        InstanceWeaponOnSwitchState(player, client, state, oldWeaponState);
    }
}

/** 
*   @brief  Queues a state and sets it as the next current state when the state currently processing has finished.
**/
void SVGBaseItemWeapon::InstanceWeaponQueueNextState(SVGBasePlayer *player, ServerClient* client, int32_t state) {
    if (!player || !client) {
        return;
    }
    client->weaponState.queuedState = state;
}
