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
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"          // Effects.
#include "../../Player/Animations.h"    // Include Player Client Animations.
#include "../../Utilities.h"        // Util funcs.

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseItemWeapon.h"

// Game world.
#include "../../World/ServerGameWorld.h"

//
// Constructor/Deconstructor.
//
SVGBaseItemWeapon::SVGBaseItemWeapon(PODEntity *svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) {

}



/**
*   @brief
**/
void SVGBaseItemWeapon::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache view and world models for the given weapon.
    SVG_PrecacheModel(GetViewModel());
    SVG_PrecacheModel(GetWorldModel());
}

/**
*   @brief
**/
void SVGBaseItemWeapon::Spawn() {
    // Always call parent class method.
    Base::Spawn();
    
    // Set the weapon item world model.
    SetModel(GetWorldModel());

    // Set the config string for this item.
    SVG_SetConfigString(ConfigStrings::Items + itemIdentifier, displayString);
}

/**
*   @brief
**/
void SVGBaseItemWeapon::Respawn() {
    Base::Respawn();
}

/**
*   @brief
**/
void SVGBaseItemWeapon::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

/**
*   @brief
**/
void SVGBaseItemWeapon::Think() {
    // Always call parent class method.
    Base::Think();
}

/**
*   @brief
**/
void SVGBaseItemWeapon::SetRespawn(const Frametime &delay) {
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
    // Make life slightly simpler, TODO: Rename things for more consistency.
    using WeaponFlags = ServerClient::WeaponState::Flags;

    // Sanity.
    if (!player || !client) {
        return;
    }
            //if (((ent->client->latched_buttons | ent->client->buttons) & BUTTON_ATTACK)) {
            //ent->client->latched_buttons &= ~BUTTON_ATTACK;
    /**
    *   Primary/Secondary Fire Logic.
    **/
    if (weapon) {
        if ((client->buttons) & ButtonBits::PrimaryFire) {
            // Do a hard state set, we can't have a weapon need to wait till next frame when willing to fire.
            if (client->weaponState.current == WeaponState::Idle) {
                weapon->InstanceWeaponSetCurrentState(player, weapon, client, WeaponState::PrimaryFire);
            }
        } else if ((client->buttons) & ButtonBits::SecondaryFire) {
            // Do a hard state set, we can't have a weapon need to wait till next frame when willing to fire.
            if (client->weaponState.current == WeaponState::Idle) {
                weapon->InstanceWeaponQueueNextState(player, weapon, client, WeaponState::SecondaryFire);
            }
        } else {

        }
    }

    /**
    *   State Queue Logic.
    **/
    // See if we got a queued state, if we do, override our current weaponstate.
    if (client->weaponState.queued != -1 && !(client->weaponState.flags & WeaponFlags::IsProcessingState)) {
        // Set the timestamp of when this current state got set.
        client->weaponState.timeStamp = level.time;

        //gi.DPrintf("WeaponState Switched:(From:#%i  To:#%i   Timestamp:%i)\n", 
        //    client->weaponState.current, client->weaponState.queued, client->weaponState.timeStamp);

        // Make the queued weapon state our active one. NOTE: SetCurrentState also calls upon OnSwitchState.
        weapon->InstanceWeaponSetCurrentState(player, weapon, client, client->weaponState.queued);

	    // Reset it to -1.
        client->weaponState.queued = -1;
    } 


    /**
    *   State Processing Logic.
    **/
    // Since we had no state queued up to switch to, we'll do a check for whether any states are being processed.
    // If not, ignore, otherwise, call their corresponding process callback.
    if (weapon) {
        switch (client->weaponState.current) {
        case WeaponState::Holster:
                // Execute 'Holster' weapon state.
                weapon->InstanceWeaponProcessHolsterState(player, weapon, client);
            break;
        case WeaponState::Draw:
                // Be sure to update view model weapon right here in case it has been changed.
                weapon->InstanceWeaponUpdateViewModel(player, weapon, client);
                // Execute 'Draw' weapon state.
                weapon->InstanceWeaponProcessDrawState(player, weapon, client);
            break;
        case WeaponState::Reload:
                // Execute 'Reload' weapon state.
                weapon->InstanceWeaponProcessReloadState(player, weapon, client);
            break;
        case WeaponState::PrimaryFire:
                // Execute idle weapon state.
                weapon->InstanceWeaponProcessPrimaryFireState(player, weapon, client);
            break;
        case WeaponState::SecondaryFire:
                // Execute idle weapon state.
                weapon->InstanceWeaponProcessSecondaryFireState(player, weapon, client);
            break;
        // Breakthrough to processing idle state. (Unsets the IsProcessingState bit so it can engage to other stages.)
        case WeaponState::Idle:
        default:
                // Execute idle weapon state.
                weapon->InstanceWeaponProcessIdleState(player, weapon, client);
            break;
        }
    }


    /**
    *   Switch Weapon Logic.
    **/
    // If nextWeaponID != 0, we roll.
    if ( client->persistent.inventory.nextWeaponID != 0 && 
        client->persistent.inventory.nextWeaponID != client->persistent.inventory.activeWeaponID ) {
        // Check whether the weapon is not processing any animations of a certain state, and isn't holstered.
        if (!(client->weaponState.flags & WeaponFlags::IsProcessingState) &&
            !(client->weaponState.flags & WeaponFlags::IsAnimating) &&
            !(client->weaponState.flags & WeaponFlags::IsHolstered) &&
            client->weaponState.current == WeaponState::Idle) {

            // Queue holster state for next frame, prevent this from happening over and over when already in it.
            if (client->weaponState.current != WeaponState::Holster) {
                weapon->InstanceWeaponQueueNextState(player, weapon, client, WeaponState::Holster);
            }
        }

        // Check whether the weapon is not processing any animations of a certain state, and has been holstered.
        if (!(client->weaponState.flags & WeaponFlags::IsProcessingState) &&
            !(client->weaponState.flags & WeaponFlags::IsAnimating) &&
            (client->weaponState.flags & WeaponFlags::IsHolstered) &&
            client->weaponState.current == WeaponState::None) {

            // Exchange weapon IDs.
            client->persistent.inventory.previousActiveWeaponID = client->persistent.inventory.activeWeaponID;
            client->persistent.inventory.activeWeaponID = client->persistent.inventory.nextWeaponID;
            client->persistent.inventory.nextWeaponID = 0;

            // Reset weapon state flags.
            client->weaponState.flags = 0;

            // Queue draw state for next frame, prevent this from happening over and over when already in it.
            if (client->weaponState.current != WeaponState::Draw) {
                // Queue next state.
                weapon->InstanceWeaponQueueNextState(player, weapon, client, WeaponState::Draw);
            }
        }
    }
}

/**
*   @brief  When the weapon isn't occupied processing other states, will switch it to reload state.
**/
void SVGBaseItemWeapon::InstanceWeaponReload(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Sanity.
    if (!player || !weapon || !client) {
        return;
    }

    // Ensure that the player has enough ammo to even be capable of reloading.
    if ( !player->CanReloadWeaponClip( weapon->GetIdentifier() ) ) {
        // TODO:    This is a sad case, got to play a special sound for it AND...
        //          try and switch to another weapon?
        return;
    }

    // Queue a reload state in case this weapon currently isn't processing any state. (Has to be in idle mode.)
    if (client->weaponState.current == WeaponState::Idle && !(client->weaponState.flags & ServerClient::WeaponState::Flags::IsProcessingState)) {
        weapon->InstanceWeaponQueueNextState(player, weapon, client, WeaponState::Reload);
    }
}

/**
*   @brief  Call whenever an animation needs to be processed for another game frame.
**/
void SVGBaseItemWeapon::InstanceWeaponUpdateViewModel(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Sanity check.
    if (!player || !client) {
        return;
    }
    
    /**
    *   View Model & Stats Update Logic.
    **/
    if (weapon) {
        // Update player's view model.
        client->playerState.gunIndex    = weapon->GetViewModelIndex();         
        
        // Assign client's clip ammo, primary, and secondary indices.
        client->clipAmmoIndex           = weapon->GetIdentifier();
        client->primaryAmmoIndex        = weapon->GetPrimaryAmmoIdentifier();
        client->secondaryAmmoIndex      = weapon->GetSecondaryAmmoIdentifier();
        
        // We got ourselves a valid instance, set view model accordingly.
        if (player->GetModelIndex() == 255) {
            int32_t i = (weapon->GetViewModelIndex() & 0xff) << 8;
            player->SetSkinNumber((player->GetNumber()  -  1) | i);
        }
    } else {
        // No weapon, so reset these.
        client->playerState.gunIndex    = 0;        
        client->primaryAmmoIndex        = 0;
        client->secondaryAmmoIndex      = 0;
        client->clipAmmoIndex           = 0;

        // Skin to 0.
        player->SetSkinNumber(0);
    }
}



/***
*
*   Animation Functions.
*
***/
/**
*   @brief    Sets the weapon's animation properties.
*   @param    frameTime Determines the time taken for each frame, this can be used to either speed up or slow down an animation.
**/
void SVGBaseItemWeapon::InstanceWeaponSetAnimation(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient *client, GameTime startTime, int32_t startFrame, int32_t endFrame, int32_t loopCount, qboolean forceLoop, float frameTime) {
    // Sanity.
    if (!client) {
        return;
    }

    // Set IsAnimating flag bit.
    client->weaponState.flags |= ServerClient::WeaponState::Flags::IsAnimating;

    // Time properties.
    client->playerState.gunAnimationStartTime   = startTime.count();
    client->playerState.gunAnimationFrametime   = frameTime;

    // Animation properties.
    client->playerState.gunAnimationStartFrame  = startFrame;
    client->playerState.gunAnimationEndFrame    = endFrame;

    // Loop properties.
    client->playerState.gunAnimationLoopCount   = loopCount;
    client->playerState.gunAnimationForceLoop   = forceLoop;
}

/**
*   @brief  Call whenever an animation needs to be processed for another game frame.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessAnimation(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Process only if animating flag is set.
    if (client->weaponState.flags & ServerClient::WeaponState::Flags::IsAnimating) {
        // Animate for another frame.
        SG_FrameForTime(&client->weaponState.animationFrame, 
            GameTime(level.time),
            GameTime(client->playerState.gunAnimationStartTime), 
            client->playerState.gunAnimationFrametime, 
            client->playerState.gunAnimationStartFrame,
            client->playerState.gunAnimationEndFrame, 
            client->playerState.gunAnimationLoopCount, 
            client->playerState.gunAnimationForceLoop
        );

        // If the animation has finished(frame == -1), automatically unset the IsAnimating flag.
        if (client->weaponState.animationFrame < 0) {
            // Remove IsAnimating flag.
            client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsAnimating;

            // Call upon animation finished callback.
            weapon->InstanceWeaponOnAnimationFinished(player, weapon, client);
        }

        //gi.DPrintf("ProcessAnimation - State:(#%i)    Frame(#%i)\n", client->weaponState.current, client->weaponState.animationFrame);
    }
}

/**
*   @brief  Called only once during the same frame when the weapon has finished playing an animation.
**/
void SVGBaseItemWeapon::InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Sanity.
    if (!client) {
        return;
    }

    // Remove IsAnimating flag.
    client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsAnimating;
}



/***
*
* 
*   State Management Functions.
* 
*
***/
/**
*   @brief  Instantly sets the current state.
**/
void SVGBaseItemWeapon::InstanceWeaponSetCurrentState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient* client, int32_t state) {
    // Sanity.
    if (!client) {
        return;
    }

    // Only call on switch state in case the state isn't identical to old state.
    if (client->weaponState.current != state) {
        // Add IsProcessingState flag in case state isn't 'None'. Remove it otherwise.
        if (state == WeaponState::None) {
            client->weaponState.flags &= ~ServerClient::WeaponState::Flags::IsProcessingState;
        } else {
            client->weaponState.flags |= ServerClient::WeaponState::Flags::IsProcessingState;
        }

        // Store old current state so we can use it for our call to OnSwitchState.
        int32_t oldWeaponState = client->weaponState.current;

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
*   @brief  A callback which can be implemented by weapons in order to set up and
*           prepare for the next state.
* 
*           (Mainly used for setting animations, but can be used for anything really.)
* 
*   @param  newState The current new state that the weapon resides in.
*   @param  oldState Old previous state the weapon was residing in.
**/
void SVGBaseItemWeapon::InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient *client, int32_t newState, int32_t oldState) {
    // It is safe to assume, draw weapon state switch means we got ourselves a new viewmodel, I suppose.
    // Change view model right here, no need to do so elsewhere, or is there?
    if (newState == WeaponState::Draw) {
        weapon->InstanceWeaponUpdateViewModel(player, weapon, client);
    }
}





/***
*
*   State Processing Stubs.
*
***/
/**
*   @brief  Called each frame the weapon is in Draw state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessDrawState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    //gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessDrawState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}
    
/**
*   @brief  Called each frame the weapon is in Holster state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessHolsterState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    //gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessHolsterState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}
/**
*   @brief  Called each frame the weapon is in Holster state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessIdleState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    //gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessIdleState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}
/**
*   @brief  Called each frame the weapon is in Reload state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessReloadState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    //gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessReloadState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}
/**
*   @brief  Called each frame the weapon is in 'Primary Fire' state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessPrimaryFireState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    //gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessPrimaryFireState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}
/**
*   @brief  Called each frame the weapon is in 'Secondary Fire' state.
**/
void SVGBaseItemWeapon::InstanceWeaponProcessSecondaryFireState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) {
    // Debug Print.
    //gi.DPrintf("SVGBaseItemWeapon::InstanceWeaponProcessSecondaryFireState(weaponState.timeStamp: %i    level.timeStamp: %i)\n", client->weaponState.timeStamp, level.timeStamp);
}