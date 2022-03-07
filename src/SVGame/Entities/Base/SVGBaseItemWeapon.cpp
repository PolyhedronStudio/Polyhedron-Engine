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
#include "../../Player/Animations.h"    // Include Player Client Animations.
#include "../../Physics/StepMove.h" // Stepmove funcs.
#include "../../Utilities.h"        // Util funcs.

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
*   Essentially whenever nextWeaponID is set, whether it be by a pick up item or other reasons,
*   we check for it here as to whether it is different from the current active weapon ID. If it is
*   different, we want to engage weapon holstering the first second we get a chance at it.
* 
*   Once the weapon has been holstered and that state has finished processing, we can engage in
*   switching nextWeaponID to be our activeWeaponID and start its Draw weapon process.
* 
*   When a weapon has been drawn, it is allowed to go into other state modes such as idle, or 
*   primary/secondary fire, reload, and others if need be.
* 
*   Animations directly update the gunAnimation variables of the client's player state. With the
*   exception of it not transfering the direct frame to the client, just the mere data for the client
*   to do the animation processing on its own. Instead we use the frame number on the server side of
*   things only to see when an animation has finished playing.
* 
*   Whenever a state switch is engaged, a callback will be fired with the old and new state as its
*   arguments.
* 
*   Whenever an animation has finished playing, a callback will be fired.
* 
*   States are handled using the weapon state flags. Care has to be taken not to exploit this.
***/
void SVGBaseItemWeapon::InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    //// Huuueeeee
    if (!player || !client) {
        return;
    }

    // Debug Print.
    gi.DPrintf("WeaponState(#%i):   Timestamp:(%i)\n", client->weaponState.current, level.timeStamp);

    // Acquire weaponID values.
    int32_t activeWeaponID         = client->persistent.inventory.activeWeaponID;
    int32_t nextWeaponID           = client->persistent.inventory.nextWeaponID;
    int32_t previousActiveWeaponID = client->persistent.inventory.previousActiveWeaponID;
    
    // Less typing.
    using WeaponFlags = ServerClient::WeaponState::Flags;


    // Have an IsAnimating flag for when it really is doing animations.
    // Have a ProcessAnimation flag for when it is even allowed to process animations. This flag
    // does not mean it is animating. It means that it is allowed to process animations if it has any set.
    
    /**
    *   OnAnimationFinished inspection, do a callback if it truly is finished.
    **/
    if (client->weaponState.animationFrame < 0 && client->weaponState.flags & WeaponFlags::IsAnimating) {
        // Call upon Animation Finished callback.
        weapon->InstanceWeaponOnAnimationFinished(player, weapon, client);
        
        // Initiate animation for the current frame and 
        InstanceWeaponProcessAnimation(player, weapon, client);
        //return;
    }

    /**
    *   Weapon State Machine Logic. (It's very minimal nonetheless.)
    **/
    // See if we got a queued state, if we do, override our current weaponstate.
    if (client->weaponState.queued != WeaponState::None) {
        // Set the timestamp of when this current state got set.
        client->weaponState.timeStamp = level.timeStamp;

        gi.DPrintf("WeaponState switched:(From: #%i     To: #%i)   Timestamp:(%i)\n", 
            client->weaponState.current, client->weaponState.queued, client->weaponState.timeStamp);

        // Make the queued weapon state our active one. NOTE: SetCurrentState also calls upon OnSwitchState.
        weapon->InstanceWeaponSetCurrentState(player, weapon, client, client->weaponState.queued);

	    // Reset it to -1.
        client->weaponState.queued = WeaponState::None;

        // 
        //InstanceWeaponProcessAnimation(player, weapon, client);
        return;
    }

    //**
    //*   Weapon Animation Processing.
    //**/
    InstanceWeaponProcessAnimation(player, weapon, client);
    // 
    //// Process only if animating flag is set.
    //if (client->weaponState.flags & WeaponFlags::IsAnimating) {
    //    SG_FrameForTime(&client->weaponState.animationFrame, 
    //        level.timeStamp,
    //        client->playerState.gunAnimationStartTime, 
    //        client->playerState.gunAnimationFrametime, 
    //        client->playerState.gunAnimationStartFrame,
    //        client->playerState.gunAnimationEndFrame, 
    //        client->playerState.gunAnimationLoopCount, 
    //        client->playerState.gunAnimationForceLoop
    //    );
    //    gi.DPrintf("ANIMATIONFRAME = %i\n", client->weaponState.animationFrame);
    //}

    /**
    *   Weapon Switch Logic.
    **/
    if (nextWeaponID) {
        // See if the weapon is free to engage holster mode.
        if (!(client->weaponState.flags & WeaponFlags::IsAnimating) && !(client->weaponState.flags & WeaponFlags::IsHolstered) && client->weaponState.current != WeaponState::Holster) {
            // Set Player Animation. TODO: This'll have to go in the future when we go full pose skeletal on entities.
            client->animation.priorityAnimation = PlayerAnimation::Pain;
            if (client->playerState.pmove.flags & PMF_DUCKED) {
                player->SetAnimationFrame(FRAME_crpain1);   // TODO: Holster Animation instead of pain.
                client->animation.endFrame = FRAME_crpain4;
            } else {
                player->SetAnimationFrame(FRAME_pain301);
                client->animation.endFrame = FRAME_pain304;
            }

            // Add IsHolstered flag.
            client->weaponState.flags |= WeaponFlags::IsHolstered;

            // Queue holster state for next frame.
            weapon->InstanceWeaponQueueNextState(player, weapon, client, WeaponState::Holster);
        // See if the weapon is free to engage draw mode. If it is, let's switch weapons shall we?
        } else if (!(client->weaponState.flags & WeaponFlags::IsAnimating) && (client->weaponState.flags & WeaponFlags::IsHolstered) && client->weaponState.current != WeaponState::Draw) {     
            // Set Player Animation. TODO: This'll have to go in the future when we go full pose skeletal on entities.
            if (client->playerState.pmove.flags & PMF_DUCKED) {
                player->SetAnimationFrame(FRAME_crpain1);   // TODO: Draw Animation instead of pain.
                client->animation.endFrame = FRAME_crpain4;
            } else {
                player->SetAnimationFrame(FRAME_pain301);
                client->animation.endFrame = FRAME_pain304;
            }
            
            // Remove IsHolstered flag.
            client->weaponState.flags &= ~WeaponFlags::IsHolstered;

            // Reset flags for a fresh weapon state.
            client->weaponState.flags = 0;

            // Update client persistent weapon inventory IDs.
            client->persistent.inventory.previousActiveWeaponID = activeWeaponID;
            client->persistent.inventory.activeWeaponID = nextWeaponID;
            client->persistent.inventory.nextWeaponID = 0;

            // We've changed the activeWeaponID. This means we'll have to fetch ourselfes a new pointer.
            weapon = SVGBaseItemWeapon::GetWeaponInstanceByID(client->persistent.inventory.activeWeaponID);

            // Set view model based on ID == 0 aka no weapon, or a valid weapon instance.
            if (weapon) {
                // We got ourselves a valid instance, set view model accordingly.
                if (player->GetModelIndex() == 255) {
                    int32_t i = (weapon->GetViewModelIndex() & 0xff) << 8;
                    player->SetSkinNumber((player->GetNumber()  -  1) | i);
                }

                // Update gun and ammo index.
                client->playerState.gunIndex = weapon->GetViewModelIndex();         
                client->ammoIndex = weapon->GetPrimaryAmmoIdentifier();

                // Queue draw state for next frame.
                weapon->InstanceWeaponQueueNextState(player, weapon, client, ::WeaponState::Draw);
            } else {
                // Update gun and ammo index.
                client->playerState.gunIndex = 0;        
                client->ammoIndex = 0;

                // Skin to 0.
                player->SetSkinNumber(0);

                // If we had no weapon pointer , queue state 'None'.
                InstanceWeaponQueueNextState(player, weapon, client, WeaponState::None);
            }

            // Return, we're done here for this frame.
            //return;
        }
    }





}

/**
*   @brief  Call whenever an animation needs to be processed for another game frame.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessAnimation(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Check if the flag for processing animations is set. If not, escape this function.
    if (!client->weaponState.flags & ServerClient::WeaponState::Flags::ProcessAnimation) {
        return;
    }
    
    // Process only if animating flag is set.
    if (client->weaponState.flags & ServerClient::WeaponState::Flags::IsAnimating) {
        SG_FrameForTime(&client->weaponState.animationFrame, 
            level.timeStamp,
            client->playerState.gunAnimationStartTime, 
            client->playerState.gunAnimationFrametime, 
            client->playerState.gunAnimationStartFrame,
            client->playerState.gunAnimationEndFrame, 
            client->playerState.gunAnimationLoopCount, 
            client->playerState.gunAnimationForceLoop
        );
        
        // If the animation has finished(frame == -1), automatically unset the IsAnimating flag.
        if (client->weaponState.animationFrame < 0) {
            client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsAnimating;
        }

        gi.DPrintf("InstanceWeaponProcessAnimation - State:(#%i)    animationFrame(#%i)\n", client->weaponState.current, client->weaponState.animationFrame);
    }
}

/**
*   @brief  Called each frame the weapon is in Draw state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessDrawState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessDrawState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}
    
/**
*   @brief  Called each frame the weapon is in Holster state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessHolsterState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessHolsterState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}
/**
*   @brief  Called each frame the weapon is in Holster state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessIdleState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessIdleState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}

/**
* @brief    Sets the weapon's animation properties.
* @param    frameTime Determines the time taken for each frame, this can be used to either speed up or slow down an animation.
**/
void SVGBaseItemWeapon::InstanceWeaponSetAnimation(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient *client, int64_t startTime, int32_t startFrame, int32_t endFrame, int32_t loopCount, qboolean forceLoop, float frameTime) {
    // Sanity.
    if (!client) {
        return;
    }

    // Set IsAnimating flag bit.
    client->weaponState.flags |= ServerClient::WeaponState::Flags::IsAnimating;

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
void SVGBaseItemWeapon::InstanceWeaponSetCurrentState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient* client, int32_t state) {
    // Sanity.
    if (!client) {
        return;
    }

    // Store old current state so we can use it for our call to OnSwitchState.
    int32_t oldWeaponState = client->weaponState.current;

    // Only call on switch state in case the state isn't identical to old state.
    if (client->weaponState.current != state) {
        // Assign new state.
        client->weaponState.current = state;

        // Call switch state.
        weapon->InstanceWeaponOnSwitchState(player, weapon, client, state, oldWeaponState);
    }
}

/** 
*   @brief  Queues a state and sets it as the next current state when the state currently processing has finished.
**/
void SVGBaseItemWeapon::InstanceWeaponQueueNextState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient* client, int32_t state) {
    // Sanity.
    if (!client) {
        return;
    }

    client->weaponState.queued = state;
}

/**
* @brief    A callback which can be implemented by weapons in order to set up and
*           prepare for the next state.
* 
*           (Mainly used for setting animations, but can be used for anything really.)
* 
* @param newState The current new state that the weapon resides in.
* @param oldState Old previous state the weapon was residing in.
**/
void SVGBaseItemWeapon::InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient *client, int32_t newState, int32_t oldState) {
}

/**
* @brief    Sets the weapon's animation properties.
* @param    frameTime Determines the time taken for each frame, this can be used to either speed up or slow down an animation.
**/
void SVGBaseItemWeapon::InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Sanity.
    if (!client) {
        return;
    }

    // Remove IsAnimating flag.
    client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsAnimating;
}
