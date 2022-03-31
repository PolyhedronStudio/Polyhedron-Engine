/*
// LICENSE HERE.

//
// SVGBasePlayer.cpp
//
//
*/
#include "../../ServerGameLocals.h"              // SVGame.
#include "../../Effects.h"              // Effects.
#include "../../Entities.h"             // Entities.
#include "../../Player/Client.h"        // Player Client functions.
#include "../../Player/Animations.h"    // Include Player Client Animations.
#include "../../Player/View.h"          // Include Player View functions..
#include "../../Utilities.h"                // Util funcs.

// Game Mode interface.
#include "../../Gamemodes/IGamemode.h"

// World.
#include "../../World/Gameworld.h"

// Class entities.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBaseItemAmmo.h"
#include "../Base/SVGBaseItemWeapon.h"
#include "../Base/SVGBasePlayer.h"

// Constructor/Deconstructor.
SVGBasePlayer::SVGBasePlayer(Entity* svEntity) : Base(svEntity) {

}

/**
*   @brief  Used by game modes to recreate a fresh player entity for the client.
**/
SVGBasePlayer* SVGBasePlayer::Create(Entity* svEntity) {
    // Get gameworld pointer.
    Gameworld* gameworld = GetGameworld();

    // Get pointer to server entities.
    Entity* serverEntities = gameworld->GetServerEntities();

    // Initialize a clean serverEntity.
    svEntity->inUse = true;

    // Set the entity state number.
    svEntity->state.number = svEntity - serverEntities;

    // Delete previous classentity, if existent (older client perhaps).
    gameworld->FreeClassEntity(svEntity);

    // Recreate class SVGBasePlayer entity.
    svEntity->classEntity = gameworld->CreateClassEntity<SVGBasePlayer>(svEntity, false);

    // Last but not least, return this class entity its pointer.
    return dynamic_cast<SVGBasePlayer*>(svEntity->classEntity);
}

//===============
// SVGBasePlayer::Precache
//
//===============
//
void SVGBasePlayer::Precache() {
    Base::Precache();
}

//
//===============
// SVGBasePlayer::Spawn
//
//===============
//
void SVGBasePlayer::Spawn() {
    // Spawn.
    Base::Spawn();

    // When spawned, we aren't on any ground, make sure of that.
    SetGroundEntity(nullptr);
    // Set up the client entity accordingly.
    SetTakeDamage(TakeDamage::Aim);
    // Fresh movetype and solid.
    SetMoveType(MoveType::PlayerMove);
    SetSolid(Solid::OctagonBox);
    // Mass.
    SetMass(200);
    // Undead itself.
    SetDeadFlag(DeadFlags::Alive);
    // Set air finished time so it can respawn kindly.
    SetAirFinishedTime(level.time + 12s);
    // Clip mask this client belongs to.
    SetClipMask(BrushContentsMask::PlayerSolid);
    // Fresh default model.
    SetModel("players/male/tris.md2");
    /*ent->pain = player_pain;*/
    // Fresh water level and type.
    SetWaterLevel(0);
    SetWaterType(0);
    // Fresh flags.
    SetFlags(GetFlags() & ~EntityFlags::NoKnockBack);
    SetServerFlags(GetServerFlags() & ~EntityServerFlags::DeadMonster);
    // Fresh player move bounding box.
    SetMins(vec3_scale(PM_MINS, PM_SCALE));
    SetMaxs(vec3_scale(PM_MAXS, PM_SCALE));
    // Fresh view height.
    SetViewHeight(22);
    // Zero out velocity in case it had any at all.
    SetVelocity(vec3_zero());

    // Fresh effects.
    Base::SetEffects(0);

    // Reset model indexes.
    SetModelIndex(255); // Use the skin specified by its model.
    SetModelIndex2(255);// Custom gun model.
    SetSkinNumber(GetNumber() - 1);	 // Skin is client number. //    ent->state.skinNumber = ent - g_entities - 1; // sknum is player num and weapon number  // weapon number will be added in changeweapon
    
    // Fresh frame for animations.
    SetAnimationFrame(0);

    // Set the die function.
    SetDieCallback(&SVGBasePlayer::SVGBasePlayerDie);

    // Let it be known this client entity is in use again.
    SetInUse(true);
}

//
//===============
// SVGBasePlayer::Respawn
//
//===============
//
void SVGBasePlayer::Respawn() {
    Base::Respawn();
    gi.DPrintf("SVGBasePlayer::Respawn();");
}

//
//===============
// SVGBasePlayer::PostSpawn
//
//===============
//
void SVGBasePlayer::PostSpawn() {
    Base::PostSpawn();
}

//
//===============
// SVGBasePlayer::Think
//
//===============
//
void SVGBasePlayer::Think() {
    // Parent class Think.
    Base::Think();
}

//
//===============
// SVGBasePlayer::SpawnKey
//
// SVGBasePlayer spawn key handling.
//===============
//
void SVGBasePlayer::SpawnKey(const std::string& key, const std::string& value) {
    // Parent class spawnkey.
    Base::SpawnKey(key, value);
}

/***
* 
*   Callback Functions.
*
***/
/**
*   @brief  Callback that is fired any time the player dies. As such, it kindly takes care of doing this.
**/
void SVGBasePlayer::SVGBasePlayerDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
    // Fetch server entity.
    Entity* serverEntity = GetPODEntity();

    // Fetch client.
    gclient_s* client = GetClient();

    // Clear out angular velocity.
    SetAngularVelocity(vec3_zero());

    // Can still take damage.
    SetTakeDamage(TakeDamage::Yes);

    // Let our dead body toss itself.
    SetMoveType(MoveType::Toss);

    // Remove the linked weapon model (third person thingy.)
    SetModelIndex2(0);

    // Our effect type is now: CORPSE. Beautiful dead corpse :P
    Base::SetEffects(EntityEffectType::Corpse);

    // Fetch angles, only maintain the yaw, reset the others.
    SetAngles(vec3_t{ 0.f, GetAngles()[vec3_t::PYR::Yaw], 0.f });

    // Ensure our client entity is playing no sounds anymore.
    Base::SetSound(0);
    client->weaponSound = 0;

    // Retreive maxes, adjust height (z)
    SetMaxs(GetMaxs() - vec3_t{ 0.f, 0.f, -8.f });

    // Change server flags, we're a dead monster now.
    SetServerFlags(GetServerFlags() | EntityServerFlags::DeadMonster);

    // If we're not dead yet, we got some death initializing to do.
    if (!GetDeadFlag()) {
        // Set respawn time.
        SetRespawnTime(level.time + 1s);

        // Ensure we are looking at our killer.
        LookAtKiller(inflictor, attacker);

        // Dead players don't move ;-)
        SetPlayerMoveType(EnginePlayerMoveType::Dead);

        // Update the obituary.
        GetGamemode()->ClientUpdateObituary(this, inflictor, attacker);

        // Toss our weapon, assuming we had any.
        TossWeapon();

        // Show the scoreboard in case of a deathmatch mode.
        //if (deathmatch->value)
        // TODO: Let it be determined by game mode.
        SVG_Command_Score_f(this, client);

        // Let the gamemode know this client died.
        GetGamemode()->ClientDeath(this);
    }

    // Remove powerups.
    SetFlags(GetFlags() & ~EntityFlags::PowerArmor);

    // In case our health went under -40, shred this body to gibs!
    if (GetHealth() < -40) {
        // Play a nasty gib sound, yughh :)
        SVG_Sound(this, SoundChannel::Body, gi.SoundIndex("misc/udeath.wav"), 1, Attenuation::Normal, 0);

        // Throw some gibs around, true horror oh boy.
        // Get gameworld pointer.
	    Gameworld* gameworld = GetGameworld();
        gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
        gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
        gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
        gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
        SVG_ThrowClientHead(this, damage);

        // Can't take damage if we're already busted.
        SetTakeDamage(TakeDamage::No);
    // Normal death.
    } else {
        // Ensure we aren't dead flagged already.
        if (!GetDeadFlag()) {
            static int i = (i + 1) % 3;

            // start a death animation
            SetPriorityAnimation(PlayerAnimation::Death);

            if (client->playerState.pmove.flags & PMF_DUCKED) {
                SetAnimationFrame(FRAME_crdeath1 - 1);
                SetAnimationEndFrame(FRAME_crdeath5);
            }
            else switch (i) {
            case 0:
                SetAnimationFrame(FRAME_death101 - 1);
                SetAnimationEndFrame(FRAME_death106);
                break;
            case 1:
                SetAnimationFrame(FRAME_death201 - 1);
                SetAnimationEndFrame(FRAME_death206);
                break;
            case 2:
                SetAnimationFrame(FRAME_death301 - 1);
                SetAnimationEndFrame(FRAME_death308);
                break;
            }
            SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex(va("*death%i.wav", (rand() % 4) + 1)), 1, Attenuation::Normal, 0);
        }
    }

    // Set the dead flag to: DEAD, duhh.
    SetDeadFlag(DeadFlags::Dead);

    // Link our entity back in for collision purposes.
    LinkEntity();
}


/***
* 
*   Player Functions.
*
***/
/**
*   @brief  Tosses the player's weapon away from himself.
**/
void SVGBasePlayer::TossWeapon() {
    SVGBaseItemWeapon *item = nullptr;
    Entity      *drop= nullptr;
    float       spread = 1.5f;

    // Always allow.
    //if (!deathmatch->value)
    //    return;

    // Get client
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
        return;
    }

    // Fetch active weapon, if any.
    //item = player->GetActiveWeapon();

    //if (!player->GetClient()->persistent.inventory[player->GetClient()->ammoIndex])
    //    item = NULL;
    //if (item && (strcmp(item->pickupName, "Blaster") == 0))
    //    item = NULL;

    if (item) {
        //playerClient->GetClient()->aimAngles[vec3_t::Yaw] -= spread;
        //drop = SVG_DropItem(playerClient->GetPODEntity(), item);
        //playerClient->GetClient()->aimAngles[vec3_t::Yaw] += spread;
        //drop->spawnFlags = ItemSpawnFlags::DroppedPlayerItem;
    }
}

/**
*   @brief  Each player can have two 'noise sources' associated to it. One slot for
*           player personal noises such as jumpin, pain, firing a weapon. The other
*           slot for target noise, such as bullets impacting a wall.
* 
*           Use your imagination to think of what this is useful for ;-P
**/
void SVGBasePlayer::PlayerNoise(SVGBaseEntity* noiseEntity, const vec3_t& noiseOrigin, int32_t noiseType) {
//    Entity     *noise;

//if (deathmatch->value)
//    return;
//if (GetGamemode()->IsClass<DeathmatchGamemode>()) {
//    return;
//}

//if (who->GetFlags() & EntityFlags::NoTarget)
//    return;

//    if (!who->GetPODEntity()->myNoisePtr) {
//        noise = SVG_Spawn();
////        noise->classname = "player_noise";
//        VectorSet(noise->mins, -8, -8, -8);
//        VectorSet(noise->maxs, 8, 8, 8);
//        noise->owner = who->GetPODEntity();
//        noise->serverFlags = EntityServerFlags::NoClient;
//        //who->GetPODEntity()->myNoisePtr = noise;
//
//        noise = SVG_Spawn();
//     //   noise->classname = "player_noise";
//        VectorSet(noise->mins, -8, -8, -8);
//        VectorSet(noise->maxs, 8, 8, 8);
//        noise->owner = who->GetPODEntity();
//        noise->serverFlags = EntityServerFlags::NoClient;
//        //who->GetPODEntity()->myNoise2Ptr = noise;
//    }
//
//    if (type == PNOISE_SELF || type == PNOISE_WEAPON) {
//        noise = who->GetPODEntity()->myNoisePtr;
//        level.soundEntity = noise;
//        level.soundEntityFrameNumber = level.frameNumber;
//    } else { // type == PNOISE_IMPACT
//        noise = who->GetPODEntity()->myNoise2Ptr;
//        level.sound2Entity = noise;
//        level.sound2EntityFrameNumber = level.frameNumber;
//    }
//
//    VectorCopy(where, noise->state.origin);
//    VectorSubtract(where, noise->maxs, noise->absMin);
//    VectorAdd(where, noise->maxs, noise->absMax);
//    noise->teleportTime = level.time;
//    gi.LinkEntity(noise);
}


/***
* 
*   Weapon functions.
*
***/
/**
*   @brief  Gives the player's weapon a chance to "think".
**/
void SVGBasePlayer::WeaponThink() {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) { 
        return;
    }

    // If dead, switch to weapon ID 0 (a space holder, but is essentially no weapon.)
    if (GetGamemode()->IsDeadEntity(this)) {
        ChangeWeapon(0, false);
        return;
    }

    // Let the player's active weapon "think" for this frame.
    SVGBaseItemWeapon *activeWeapon = SVGBaseItemWeapon::GetWeaponInstanceByID(client->persistent.inventory.activeWeaponID);

    //if (client->persistent.inventory.nextWeaponID) {
    //    ChangeWeapon(client->persistent.inventory.nextWeaponID);
    //} else {
    if (activeWeapon) {//}&& client->weaponState.shouldThink) {
	    activeWeapon->InstanceWeaponThink(this, activeWeapon, client);
    }
//    }

    // If the WeaponState is Down(Done holstering), or Finished(with shooting for example), we're allowed to change weapons.
    //if (client->persistent.inventory.nextWeaponID) {
    //    ChangeWeapon(client->persistent.inventory.nextWeaponID);
    //}
}

/**
*   @brief  Adds ammo to the player's inventory.
*   @return True on success, false on failure. (Meaning the player has too much of that ammo type.)
**/
qboolean SVGBasePlayer::GiveAmmo(uint32_t ammoIdentifier, uint32_t amount) {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
        return false;
    }

    // Now we're here, acquire the item instance of the ammo type.
    SVGBaseItemAmmo* ammoInstance = SVGBaseItemAmmo::GetAmmoInstanceByID(ammoIdentifier);

    // If we can't find the instance, return false.
    if (!ammoInstance) {
        return false;
    }

    // Get the cap limit for said ammo type.
    uint32_t ammoCapLimit = ammoInstance->GetCapLimit();

    // Acquire the amount one is carrying of ammo type.
    uint32_t carryingAmount = HasItem(ammoIdentifier);

    // Have we hit the cap limit for this ammo type? Return false.
    if (carryingAmount >= ammoCapLimit) {
        return false;
    }

    // Add ammo amount using a clamp.
    client->persistent.inventory.items[ammoIdentifier] = Clampi(carryingAmount + amount, 0, ammoCapLimit);

    return true;
}
/**
*   @brief  Takes ammo from the player's inventory.
*   @return True on success. If false, the player is out of ammo( <= 0 ). Assuming the first few sanity checks pass.
**/
int32_t SVGBasePlayer::TakeAmmo(uint32_t ammoIdentifier, uint32_t amount) {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
        return 0;
    }

    // Now we're here, acquire the item instance of the ammo type.
    SVGBaseItemAmmo* ammoInstance = SVGBaseItemAmmo::GetAmmoInstanceByID(ammoIdentifier);

    // If we can't find the instance, return false.
    if (!ammoInstance) {
        return 0;
    }

    // Acquire carrying weapon count.
    int32_t carryingAmount = HasItem(ammoIdentifier);

    // Can we even take any more ammo?
    if (carryingAmount <= 0) {
        return 0;
    }

    // When we carry less than we want to take, handle it differently.
    if (carryingAmount < amount) {
        // Obviously our ammo is gone now.
        client->persistent.inventory.items[ammoIdentifier] = 0;

        // So we return the amount that we're carrying as is.
        return carryingAmount;
    }
        
    // Get the cap limit for said ammo type.
    uint32_t ammoCapLimit = ammoInstance->GetCapLimit();
        
    // Subtract ammo amount using a clamp.
    client->persistent.inventory.items[ammoIdentifier] = Clampi(carryingAmount - amount, 0, ammoCapLimit);

    return amount;
}

/**
*   @brief  Adds weapon to the player's inventory.
*   @return True on success, false on failure. (Meaning the player reached the maximum carrying limit.)
**/
qboolean SVGBasePlayer::GiveWeapon(uint32_t weaponIdentifier, uint32_t amount) {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
	    return false;
    }

    // Now we're here, acquire the item instance of the weapon type.
    SVGBaseItemWeapon* weaponInstance = SVGBaseItemWeapon::GetWeaponInstanceByID(weaponIdentifier);

    // If we can't find the instance, return false.
    if (!weaponInstance) {
	    return false;
    }

    // Get limited amount a player can carry of this weapon type.
    uint32_t weaponCarryLimit = weaponInstance->GetCarryLimit();

    // Acquire carrying weapon count.
    uint32_t carryingAmount = HasItem(weaponIdentifier);

    // Have we hit the cap limit for this weapon type? Return false.
    if (carryingAmount >= weaponCarryLimit) {
	    return false;
    }

    // Add weapon amount using a clamp.
    client->persistent.inventory.items[weaponIdentifier] = Clampi(carryingAmount + amount, 0, weaponCarryLimit);

    return true;
}
/**
*   @brief  Takes away a specific amount of weapon type from the player's inventory.
*   @return True on success, false on failure. (Meaning he has none left.)
**/
qboolean SVGBasePlayer::TakeWeapon(uint32_t weaponIdentifier, uint32_t amount) {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
	    return false;
    }

    // Now we're here, acquire the item instance of the weapon type.
    SVGBaseItemWeapon* weaponInstance = SVGBaseItemWeapon::GetWeaponInstanceByID(weaponIdentifier);

    // If we can't find the instance, return false.
    if (!weaponInstance) {
	    return false;
    }

    // Acquire carrying weapon count.
    uint32_t carryingAmount = HasItem(weaponIdentifier);

    // Have we hit the bottom limit for this weapon type? Return false.
    if (carryingAmount <= 0) {
    	return false;
    }

    // Get limited amount a player can carry of this weapon type.
    uint32_t weaponCarryLimit = weaponInstance->GetCarryLimit();

    // Add weapon amount using a clamp.
    client->persistent.inventory.items[weaponIdentifier] = Clampi(carryingAmount - amount, 0, weaponCarryLimit);

    return true;
}

/**
*   @return True if the player has any ammo left for this weapon to refill its clip.
**/
qboolean SVGBasePlayer::CanReloadWeaponClip(uint32_t weaponID) {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
	    return false;
    }

    // Acquire the item instance of the weapon type.
    SVGBaseItemWeapon* weaponInstance = SVGBaseItemWeapon::GetWeaponInstanceByID(weaponID);

    // If we can't find the instance, return false.
    if (!weaponInstance) {
	    return false;
    }

    // See if the player has any ammo left of this weapon type.
    if (HasItem(weaponInstance->GetPrimaryAmmoIdentifier()) >= 1) {
        return true;
    }

    // No ammo left to reload with.
    return false;
}
/**
*   @brief  Refills the weapon's ammo clip.
*   @return True on success, false when the player ran out of ammo to refill with.
**/
qboolean SVGBasePlayer::ReloadWeaponClip(uint32_t weaponID) {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
	    return false;
    }

    // Acquire the item instance of the weapon type.
    SVGBaseItemWeapon* weaponInstance = SVGBaseItemWeapon::GetWeaponInstanceByID(weaponID);

    // If we can't find the instance, return false.
    if (!weaponInstance) {
	    return false;
    }

    // Get ammoID.
    uint32_t ammoID = weaponInstance->GetPrimaryAmmoIdentifier();

    // Calculate how much ammo we desire in order to succesfully reload this clip.
    int32_t inventoryAmmoAmount = HasItem(ammoID);
    int32_t clipAmmoAmount      = client->persistent.inventory.clipAmmo[weaponID];

    // Calculate the amount required to refill this clip with.
    int32_t desiredAmount = weaponInstance->GetClipAmmoLimit() - clipAmmoAmount;

    // Subtract wished for amount from the player's inventory.
    int32_t refillAmount = TakeAmmo(ammoID, desiredAmount);

    // If we got nothing to refill with, we failed.
    if (refillAmount <= 0) {
        return false;
    }

    // Set new clip ammo.
    client->persistent.inventory.clipAmmo[weaponID] += refillAmount;

    // Success.
    return true;
}
/**
*   @brief  Takes ammo from the weapon clip.
*   @return The amount of ammo that was taken from the clip. 0 if the clip is empty.
**/
uint32_t SVGBasePlayer::TakeWeaponClipAmmo(uint32_t weaponID, uint32_t amount) {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
	    return false;
    }

    // Acquire the item instance of the weapon type.
    SVGBaseItemWeapon* weaponInstance = SVGBaseItemWeapon::GetWeaponInstanceByID(weaponID);

    // If we can't find the instance, return false.
    if (!weaponInstance) {
	    return false;
    }

    // Acquire primary ammoID.
    uint32_t ammoID = weaponInstance->GetPrimaryAmmoIdentifier();

    // Get amount of ammo currently in the clip as well as the clip ammo limit.
    uint32_t clipAmmo       = client->persistent.inventory.clipAmmo[weaponID];
    uint32_t clipAmmoLimit  = weaponInstance->GetClipAmmoLimit();

    // Ensure that the clip isn't empty, or filled to the limit.
    if (clipAmmo <= 0) {
        return 0;
    }

    // Special condition in case clipAmmo < amount.
    if (clipAmmo < amount) {
        // Set clip ammo to 0 and return the amount that was left in the clip.
        client->persistent.inventory.clipAmmo[weaponID] = 0;
        // Return leftover.
        return clipAmmo;
    }

    // Clamp the new clip ammo just in case.
    client->persistent.inventory.clipAmmo[weaponID] = Clampi(client->persistent.inventory.clipAmmo[weaponID] - amount, 0, clipAmmoLimit);

    // Return the amount.
    return amount;
}


/**
*   @return The amount this player is holding of the itemIdentifier. (Can be used for ammo, and weapons too.)
**/
int32_t SVGBasePlayer::HasItem(uint32_t itemIdentifier) {
    // Get client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
	    return false;
    }

    // Return amount holding.
    return client->persistent.inventory.items[itemIdentifier];
}

/**
*   @brief  Engages the player to change to the new weapon.
*   @param  weaponIdentifier The identifier used for acquiring the weapon type its instance pointer.
*   @param  storeLastWeapon If set to true it'll store the current active weapon as its last weapon pointer,
*           if set to false it'll set the lastWeapon pointer to nullptr.
*   @return A pointer to the newly activated weapon, nullptr if something went wrong.
**/
SVGBaseItemWeapon* SVGBasePlayer::ChangeWeapon(int32_t weaponID, qboolean storePreviousActiveWeaponID) {
    // Get client, sanity check.
    ServerClient* client = GetClient();
    if (!client) {
        return nullptr;
    }

    // Update previousWeaponID if desired.
    if (storePreviousActiveWeaponID) {
        client->persistent.inventory.previousActiveWeaponID = client->persistent.inventory.activeWeaponID;
    }

    // In case activeWeaponID == 0, we want to set it immediately. Can't have a next weapon to switch to
    // without even having an active weapon now can we? :-)
    if (client->persistent.inventory.activeWeaponID == 0) {
        client->persistent.inventory.activeWeaponID = weaponID;
    } else {
        client->persistent.inventory.nextWeaponID = weaponID;
    }

    // Give this weapon a chance to start thinking.
    SVGBaseItemWeapon *weaponInstance = SVGBaseItemWeapon::GetWeaponInstanceByID(weaponID);

    if (weaponInstance) {
        weaponInstance->InstanceWeaponThink(this, weaponInstance, GetClient());
    }

    //return weaponInstance;
    return weaponInstance;
}
    

/**
*   @brief  Looks into the player entity's client structure for the active instance item weapon.
*   @return Pointer to the instance item weapon that is active for the client.
**/
SVGBaseItemWeapon* SVGBasePlayer::GetActiveWeaponInstance() {
    // Get entity's client.
    ServerClient *client = GetClient();

    // Sanity.
    if (!client) {
        return nullptr;
    }

    // Acquire active weapon instance.
    return SVGBaseItemWeapon::GetWeaponInstanceByID(client->persistent.inventory.activeWeaponID);
}
//
///**
//*   @brief  Sets the player entity's client structure activeWeapon pointer to the instance item weapon.
//**/
//SVGBaseItemWeapon* SVGBasePlayer::SetActiveWeapon(SVGBaseItemWeapon* instanceWeapon) {
//    // Get entity's client.
//    ServerClient *client = GetClient();
//
//    // Sanity.
//    if (!client) {
//        return nullptr;
//    }
//    // Store current active weapon as last weapon.
//    client->persistent.lastWeapon = client->persistent.activeWeapon;
//
//    // Set the instance waepon arg as our active weapon.
//    client->persistent.activeWeapon = instanceWeapon;
//
//    // Return active weapon pointer.
//    return client->persistent.activeWeapon;
//}

/**
*   @brief  Called each frame to check if this player's entity event needs to be set.
**/
void SVGBasePlayer::UpdateEvent() {
    ServerClient* client = GetClient();

    if (!client) {
        return;
    }

    // There already is an active event ID at work.
    if (GetEventID()) {
        return;
    }

    // Are we on-ground and "speeding" hard enough?
    if (GetGroundEntity() && bobMove.XYSpeed > 225) {
        // Do a footstep, from left, to right, left, to right.
        // Do the Bob!
        if ((int)(client->bobTime + bobMove.move) != bobMove.cycle ) {
            SetEventID(EntityEvent::Footstep);
        } else {
            Com_DPrintf("client->bobTime + bobMove[%i]\nbobMove.cycle[%i]\n-----------------\n");
        }
    }
}

/**
*   @brief  Called each frame to reset, and set, this player's entity and render effects.
**/
void SVGBasePlayer::UpdateEffects()
{
    // Set to a clean slate.
    SetEffects(0);
    SetRenderEffects(0);

    // No need to go on in case we are dead or well, the intermission mode is engaged.
    if (GetHealth() <= 0 || level.intermission.time != GameTime::zero()) {
        return;
    }

    // Be sure to show cheaters, lmao, that'll teach them right? * cough *
    if (GetFlags() & EntityFlags::GodMode) {
        SetRenderEffects(GetRenderEffects() | (RenderEffects::RedShell | RenderEffects::GreenShell | RenderEffects::BlueShell));
    }
}

/**
*   @brief  Called each frame to reset, and set, this player's sound effects.
**/
void SVGBasePlayer::UpdateSound() {
    // Check whether the SVGBasePlayer is hooked up to a valid client.
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
        return;
    }

    // Whenever we "fry" by so called lava or slime, we prioritize its sound over weaponry, of course.
    if (GetWaterLevel() && (GetWaterType() & (BrushContents::Lava | BrushContents::Slime))) {
        SetSound(snd_fry);
    // Otherwise, we check if we have any weaponry sound for this frame.
    } else if (client->weaponSound) {
        SetSound(client->weaponSound);
    // If we did not, we clear the sound for this frame.
    } else {
        SetSound(0);
    }
}

/**
*   @brief  Sets the clients view angles to look towards the origin of the killer entity.
**/
void SVGBasePlayer::LookAtKiller(IServerGameEntity* inflictor, IServerGameEntity* attacker)
{
    ServerClient* client = GetClient();

    // Sanity check.
    if (!client) {
        return;
    }

    // Is the attack, not us, or the world?
    if (attacker && attacker != GetGameworld()->GetWorldspawnClassEntity() && attacker != this) {
        float yaw = vec3_to_yaw(attacker->GetOrigin() - GetOrigin());
        SetKillerYaw(yaw);
    // Is the inflictor, and not an attack, NOT us or the WORLD?
    } else if (inflictor && inflictor != GetGameworld()->GetWorldspawnClassEntity() && inflictor != this) {
        float yaw = vec3_to_yaw(inflictor->GetOrigin() - GetOrigin());
        SetKillerYaw(yaw);
    // If none of the above, set the yaw as is.
    } else {
        SetKillerYaw(GetAngles()[vec3_t::Yaw]);
        return;
    }
}


/***
* 
*   View/Bobmove Functionality.
* 
***/
/**
*   @brief Calculates player view roll.
**/
float SVGBasePlayer::CalculateRoll(const vec3_t& angles, const vec3_t& velocity) {
    float side = vec3_dot(velocity, bobMove.right);
    float sign = side < 0 ? -1 : 1;
    side = fabs(side);

    float value = sv_rollangle->value;

    if (side < sv_rollspeed->value)
        side = side * value / sv_rollspeed->value;
    else
        side = value;

    return side * sign;
}

/**
*   @brief Process and apply falling damage effects if needed.
**/
void SVGBasePlayer::CheckFallingDamage()
{
    float   delta;
    int     damage;
    vec3_t  dir;

    // Check whether ent is valid, and a SVGBasePlayer hooked up 
    // to a valid client.
    ServerClient* client = GetClient();

    if (!client) {
        return;
    }

    if (GetModelIndex() != 255)
        return;     // not in the player model

    if (GetMoveType() == MoveType::NoClip || GetMoveType() == MoveType::Spectator)
        return;

    // Calculate delta velocity.
    vec3_t velocity = GetVelocity();

    if ((client->oldVelocity[2] < 0) && (velocity[2] > client->oldVelocity[2]) && (!GetGroundEntity())) {
        delta = client->oldVelocity[2];
    } else {
        if (!GetGroundEntity())
            return;
        delta = velocity[2] - client->oldVelocity[2];
    }
    delta = (delta * delta * 0.0001);

    // never take falling damage if completely underwater
    if (GetWaterLevel() == 3)
        return;
    if (GetWaterLevel() == 2)
        delta *= 0.25;
    if (GetWaterLevel() == 1)
        delta *= 0.5;

    if (delta < 1)
        return;

    if (delta < 15 / 6) {
        SetEventID(EntityEvent::Footstep);
        return;
    }

    client->fallValue = delta * 0.5;
    if (client->fallValue > 40)
        client->fallValue = 40;
    client->fallTime = level.time + FALL_TIME;

    if (delta > 30) {
        if (GetHealth() > 0) {
            if (delta >= 55) {
                SetEventID(EntityEvent::FallFar);
            } else {
                SetEventID(EntityEvent::Fall);
            }
        }
        SetDebouncePainTime(level.time);   // no normal pain sound
        damage = (delta - 30) / 2;
        if (damage < 1)
            damage = 1;
        dir = { 0.f, 0.f, 1.f };

        //if (!deathmatch->value || 
        if (!((int)gamemodeflags->value & GamemodeFlags::NoFallingDamage)) {
	        GetGamemode()->InflictDamage(this, GetGameworld()->GetWorldspawnClassEntity(), GetGameworld()->GetWorldspawnClassEntity(), dir, GetOrigin(), vec3_zero(), damage, 0, 0, MeansOfDeath::Falling);
        }
    } else {
        SetEventID(EntityEvent::FallShort);
        return;
    }
}

/**
*   @brief Process and apply world specific effects if needed.
**/
void SVGBasePlayer::CheckWorldEffects()
{
    int32_t waterLevel, oldWaterLevel;

    // Check whether ent is valid, and a SVGBasePlayer hooked up 
    // to a valid client.
    ServerClient* client = GetClient();

    if (!client)
        return;

    if (GetMoveType() == MoveType::NoClip || GetMoveType() == MoveType::Spectator) {
        SetAirFinishedTime(level.time + 12s); // don't need air
        return;
    }

    // Get gameworld pointer.
    Gameworld* gameworld = GetGameworld();

    // Retreive waterLevel.
    waterLevel = GetWaterLevel();
    oldWaterLevel = client->oldWaterLevel;
    client->oldWaterLevel = waterLevel;

    // Just entered a water volume sound effect.
    if (!oldWaterLevel && waterLevel) {
        PlayerNoise(this, GetOrigin(), PlayerNoiseType::Self);
        if (GetWaterType() & BrushContents::Lava) {
            SVG_Sound(this, SoundChannel::Body, gi.SoundIndex("player/lava_in.wav"), 1, Attenuation::Normal, 0);
        } else if (GetWaterType() & BrushContents::Slime) {
            SVG_Sound(this, SoundChannel::Body, gi.SoundIndex("player/watr_in.wav"), 1, Attenuation::Normal, 0);
        } else if (GetWaterType() & BrushContents::Water) {
            SVG_Sound(this, SoundChannel::Body, gi.SoundIndex("player/watr_in.wav"), 1, Attenuation::Normal, 0);
        }
        
        SetFlags(GetFlags() | EntityFlags::InWater);

        // clear damage_debounce, so the pain sound will play immediately
        SetDebounceDamageTime(level.time - 1s);
    }

    // Just completely exited a water volume sound effect.
    if (oldWaterLevel && ! waterLevel) {
        PlayerNoise(this, GetOrigin(), PlayerNoiseType::Self);
        SVG_Sound(this, SoundChannel::Body, gi.SoundIndex("player/watr_out.wav"), 1, Attenuation::Normal, 0);
        SetFlags(GetFlags() & ~EntityFlags::InWater);
    }

    // Head just going under water effect.
    if (oldWaterLevel != 3 && waterLevel == 3) {
        SVG_Sound(this, SoundChannel::Body, gi.SoundIndex("player/watr_un.wav"), 1, Attenuation::Normal, 0);
    }

    // Head just coming out of water effect.
    if (oldWaterLevel == 3 && waterLevel != 3) {
        if (GetAirFinishedTime() < level.time) {
            // gasp for air
            SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex("player/gasp1.wav"), 1, Attenuation::Normal, 0);
            PlayerNoise(this, GetOrigin(), PlayerNoiseType::Self);
        } else  if (GetAirFinishedTime() < level.time + 11s) {
            // just break surface
            SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex("player/gasp2.wav"), 1, Attenuation::Normal, 0);
        }
    }

    // Check for drowning effects.
    if (waterLevel == 3) {
        // if out of air, start drowning
        if (GetAirFinishedTime() < level.time) {
            // drown!
            if (GetNextDrownTime() < level.time && GetHealth() > 0) {
                SetNextDrownTime(level.time + 1s);

                // take more damage the longer underwater
                SetDamage(GetDamage() + 2);
                if (GetDamage() > 15) {
                    SetDamage(15);
                }

                // play a gurp sound instead of a normal pain sound
                if (GetHealth() <= GetDamage()) {
                    SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex("player/drown1.wav"), 1, Attenuation::Normal, 0);
                } else if (rand() & 1) {
                    SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex("*gurp1.wav"), 1, Attenuation::Normal, 0);
                } else {
                    SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex("*gurp2.wav"), 1, Attenuation::Normal, 0);
                }

                SetDebouncePainTime(level.time);

                GetGamemode()->InflictDamage(this, gameworld->GetWorldspawnClassEntity(), gameworld->GetWorldspawnClassEntity(), vec3_zero(), GetOrigin(), vec3_zero(), GetDamage(), 0, DamageFlags::NoArmorProtection, MeansOfDeath::Water);
            }
        }
    } else {
        SetAirFinishedTime(level.time + 12s);
        SetDamage(2);
    }

    // Check for sizzle damage
    if (waterLevel && (GetWaterType() & (BrushContents::Lava | BrushContents::Slime))) {
        if (GetWaterType() & BrushContents::Lava) {
            if (GetHealth() > 0
                && GetDebouncePainTime() <= level.time) {
                if (rand() & 1)
                    SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex("player/burn1.wav"), 1, Attenuation::Normal, 0);
                else
                    SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex("player/burn2.wav"), 1, Attenuation::Normal, 0);
                SetDebouncePainTime(level.time + 1s);
            }

            GetGamemode()->InflictDamage(this, gameworld->GetWorldspawnClassEntity(), gameworld->GetWorldspawnClassEntity(), vec3_zero(), GetOrigin(), vec3_zero(), 3 * waterLevel, 0, 0, MeansOfDeath::Lava);
        }

        if (GetWaterType() & BrushContents::Slime) {
            GetGamemode()->InflictDamage(this, gameworld->GetWorldspawnClassEntity(), gameworld->GetWorldspawnClassEntity(), vec3_zero(), GetOrigin(), vec3_zero(), 1 * waterLevel, 0, 0, MeansOfDeath::Slime);
        }
    }
}

/**
*   @brief Process and apply damage feedback effects.
**/
void SVGBasePlayer::ApplyDamageFeedback() {
    // Check whether ent is valid, and a SVGBasePlayer hooked up 
    // to a valid client.
    ServerClient* client = GetClient();
    if (!client) {
        return;
    }

    // Flash the backgrounds behind the status numbers.
    client->playerState.stats[PlayerStats::Flashes] = 0;
    if (client->damages.blood) {
        client->playerState.stats[PlayerStats::Flashes] |= 1;
    }
    if (client->damages.armor && !(GetFlags() & EntityFlags::GodMode)) {
        client->playerState.stats[PlayerStats::Flashes] |= 2;
    }

    // Total points of damage shot at the player this frame.
    float totalDamageCount = (client->damages.blood + client->damages.armor + client->damages.powerArmor);
    
    // Return in case of not having taken any damage at all.
    if (totalDamageCount == 0) {
        return;
    }

    // Start a pain animation if still in the player model
    if (client->animation.priorityAnimation < PlayerAnimation::Pain && GetModelIndex() == 255) {
        // Pain is our priority player animation.
        client->animation.priorityAnimation = PlayerAnimation::Pain;
        // In case of being ducked, we only have a single one.
        if (client->playerState.pmove.flags & PMF_DUCKED) {
            SetAnimationFrame(FRAME_crpain1 - 1);
            client->animation.endFrame = FRAME_crpain4;
        } else {
            // Otherwise, we shift through them circulary.
            static int32_t painAnimation = (painAnimation + 1) % 3;
            switch (painAnimation) {
                case 0:
                    SetAnimationFrame(FRAME_pain101 - 1);
                    client->animation.endFrame = FRAME_pain104;
                break;
                case 1:
                    SetAnimationFrame(FRAME_pain201 - 1);
                    client->animation.endFrame = FRAME_pain204;
                break;
                case 2:
                    SetAnimationFrame(FRAME_pain301 - 1);
                    client->animation.endFrame = FRAME_pain304;
                break;
            }
        }
    }

    // Store the real damage count.
    float realDamageCount = totalDamageCount;

    // It isn't <= 0, but still has a count, so we want to make sure it is visibly evident.
    if (totalDamageCount < 10) {
        totalDamageCount = 10;
    }

    // Play an apropriate pain sound
    if ((level.time > GetDebouncePainTime()) && !(GetFlags() & EntityFlags::GodMode)) {
        // Left pain audio file number index.
        int32_t l = 0;
        // Right pain audio file number index.
        int32_t r = 1 + (rand() & 1);

        // Set debounce pain time.
        SetDebouncePainTime(level.time + 700ms);

        // Set left index based on health value.
        if (GetHealth() < 25) {
            l = 25;
        } else if (GetHealth() < 50) {
            l = 50;
        } else if (GetHealth() < 75) {
            l = 75;
        } else {
            l = 100;
        }

        // Play pain sound based on damage taken.
        SVG_Sound(this, SoundChannel::Voice, gi.SoundIndex(va("*pain%i_%i.wav", l, r)), 1, Attenuation::Normal, 0);
    }

    // The total alpha of the blend is always proportional to count.
    if (client->damageAlpha < 0.f) {
        client->damageAlpha = 0.f;
    }
    client->damageAlpha += count * 0.01f;
    if (client->damageAlpha < 0.2f) {
        client->damageAlpha = 0.2f;
    }
    if (client->damageAlpha > 0.6f) {
        client->damageAlpha = 0.6f;     // don't go too saturated
    }

    
    // Reset damageblend for a fresh start. The final color of the blend will 
    // vary based on how much was absorbed by different armors.
    client->damageBlend = vec3_zero();

    // Power Armor blend.
    if (client->damages.powerArmor) {
        client->damageBlend = vec3_fmaf(client->damageBlend, (float)client->damages.powerArmor / realDamageCount, { 0.0f, 1.0f, 0.0f });
    }
    // Damage Armor blend.
    if (client->damages.armor) {
        client->damageBlend = vec3_fmaf(client->damageBlend, (float)client->damages.armor / realDamageCount, {1.0f, 1.0f, 1.0f});
    }
    // Damage Blood blend.
    if (client->damages.blood) {
        client->damageBlend = vec3_fmaf(client->damageBlend, (float)client->damages.blood / realDamageCount, {1.0f, 0.0f, 0.0f});
    }

    // Calculate view angle kicks
    float knockBackKick = std::fabs(client->damages.knockBack);

    // A kick of 0 means no there is no need to adjust view at all.
    if (knockBackKick && GetHealth() > 0) {
        knockBackKick = knockBackKick * 100.0 / (float)GetHealth();

        if (knockBackKick  < count * 0.5f) {
            knockBackKick = count * 0.5f;
        }
        if (knockBackKick > 50) {
            knockBackKick = 50;
        }

        // Calculate a normalized kick force vector.
        vec3_t kickForce = vec3_normalize(client->damages.fromOrigin - GetOrigin());

        // Calculate side force and apply to view damage roll.
        float side = vec3_dot(kickForce, bobMove.right);
        client->viewDamage.roll = knockBackKick * side * 0.3f;

        // Calculate pitch force and apply to view damage pitch.
        side = -vec3_dot(kickForce, bobMove.forward);
        client->viewDamage.pitch = knockBackKick * side * 0.3f;

        // Set the time for view damage to hold.
        client->viewDamage.time = level.time + DAMAGE_TIME;
    }

    // Reset totals.
    client->damages.powerArmor  = 0;
    client->damages.knockBack   = 0;
    client->damages.blood       = 0;
    client->damages.armor       = 0;
}

/**
*   @details    Calculates the view offset for this player.
*               Calculates t
*               Fall from 128 : 400 = 160000
*               Fall from 256 : 580 = 336400
*               Fall from 384 : 720 = 518400
*               Fall from 512 : 800 = 640000
*               Fall from 640 : 960 =
*               damage = deltavelocity * deltavelocity * 0.0001
**/
void SVGBasePlayer::CalculateViewOffset()
{
    float bob = 0;
    float ratio = 0;
    float delta = 0;

    // Check whether ent is valid, and a SVGBasePlayer hooked up 
    // to a valid client.
    ServerClient* client = GetClient();

    if (!client) {
        return;
    }

    // If dead, set a fixed angle and don't add any kick
    if (GetDeadFlag()) {
        client->playerState.kickAngles = vec3_zero();

        client->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
        client->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
        client->playerState.pmove.viewAngles[vec3_t::Yaw] = client->killerYaw;
    } else {
        // Fetch client kick angles.
        vec3_t newKickAngles = client->playerState.kickAngles = client->kickAngles; //ent->client->playerState.kickAngles;

        // Add pitch(X) and roll(Z) angles based on damage kick
        ratio = ((client->viewDamage.time - level.time) / DAMAGE_TIME);
        if (ratio < 0) {
            ratio = client->viewDamage.pitch = client->viewDamage.roll = 0;
        }
        newKickAngles[vec3_t::Pitch] += ratio * client->viewDamage.pitch;
        newKickAngles[vec3_t::Roll] += ratio * client->viewDamage.roll;

        // Add pitch based on fall kick
        ratio = (client->fallTime - level.time) / FALL_TIME;
        if (ratio < 0) {
            ratio = 0;
        }
        newKickAngles[vec3_t::Pitch] += ratio * client->fallValue;

        // Add angles based on velocity
        delta = vec3_dot(GetVelocity(), bobMove.forward);
        newKickAngles[vec3_t::Pitch] += delta * run_pitch->value;

        delta = vec3_dot(GetVelocity(), bobMove.right);
        newKickAngles[vec3_t::Roll] += delta * run_roll->value;

        // Add angles based on bob
        delta = bobMove.fracSin * bob_pitch->value * bobMove.XYSpeed;
        // Adjust for crouching.
        if (client->playerState.pmove.flags & PMF_DUCKED) {
            delta *= 6;
        }
        newKickAngles[vec3_t::Pitch] += delta;
        delta = bobMove.fracSin * bob_roll->value * bobMove.XYSpeed;

        // Adjust for crouching.
        if (client->playerState.pmove.flags & PMF_DUCKED) {
            delta *= 6;
        }
        if (bobMove.cycle & 1) {
            delta = -delta;
        }
        newKickAngles[vec3_t::Roll] += delta;

        // Last but not least, assign new kickangles to player state.
        client->playerState.kickAngles = newKickAngles;
    }

    //
    // Calculate new view offset.
    //
    // Start off with the base entity viewheight. (Set by Player Move code.)
    vec3_t newViewOffset = {
        0.f,
        0.f,
        (float)GetViewHeight()
    };

    // Add fall impact view punch height.
    ratio = (client->fallTime - level.time) / FALL_TIME;
    if (ratio < 0) {
        ratio = 0;
    }
    newViewOffset.z -= ratio * client->fallValue * 0.4f;

    // Add bob height.
    bob = bobMove.fracSin * bobMove.XYSpeed * bob_up->value;
    if (bob > 6) {
        bob = 6.f;
    }
    newViewOffset.z += bob;

    // Add kick offset
    newViewOffset += client->kickOrigin;

    // Clamp the new view offsets, and finally assign them to the player state.
    // Clamping ensures that they never exceed the non visible, but physically 
    // there, player bounding box.
    client->playerState.pmove.viewOffset = vec3_clamp(newViewOffset,
        GetMins(),  //{ -14, -14, -22 },
        GetMaxs()   //{ 14,  14, 30 }
    );
}

void SVGBasePlayer::CalculateGunOffset() {
    // Check whether ent is valid, and a SVGBasePlayer hooked up 
    // to a valid client.
    ServerClient* client = GetClient();

    if (!client) {
        return;
    }

    // Calculate gun angles based on view bob.
    client->playerState.gunAngles[vec3_t::Roll] = bobMove.XYSpeed * bobMove.fracSin * 0.005f;
    client->playerState.gunAngles[vec3_t::Yaw]  = bobMove.XYSpeed * bobMove.fracSin * 0.01f;

    // Negate roll and yaw based on which bob cycle step we're in.
    if (bobMove.cycle & 1) {
        client->playerState.gunAngles[vec3_t::Roll] = -client->playerState.gunAngles[vec3_t::Roll];
        client->playerState.gunAngles[vec3_t::Yaw]  = -client->playerState.gunAngles[vec3_t::Yaw];
    }

    // Calculate pitch.
    client->playerState.gunAngles[vec3_t::Pitch] = bobMove.XYSpeed * bobMove.fracSin * 0.005f;

    // Calculate gun angles from delta view movement.
    for (int32_t i = 0 ; i < 3 ; i++) {
        // Calculate delta value.
        float delta = client->oldViewAngles[i] - client->playerState.pmove.viewAngles[i];

        // Keep delta within sane bounds.
        if (delta > 180) {
            delta -= 360;
        }
        if (delta < -180) {
            delta += 360;
        }
        if (delta > 45) {
            delta = 45;
        }
        if (delta < -45) {
            delta = -45;
        }

        // Special handling for gun angle Yaw.
        if (i == vec3_t::Yaw) {
            client->playerState.gunAngles[vec3_t::Roll] += 0.1f * delta;
        }

        // Apply delta to gun angles.
        client->playerState.gunAngles[i] += 0.2f * delta;
    }

    // gun height
    client->playerState.gunOffset = vec3_zero();


    // gun_x / gun_y / gun_z are development tools
    for (int32_t i = 0 ; i < 3 ; i++) {
        client->playerState.gunOffset[i] += bobMove.forward[i] * (gun_y->value);
        client->playerState.gunOffset[i] += bobMove.right[i] * gun_x->value;
        client->playerState.gunOffset[i] += bobMove.up[i] * (-gun_z->value);
    }
}

//
//===============
// SVGBasePlayer::CalculateScreenBlend
// 
//===============
//
void SVGBasePlayer::CalculateScreenBlend() {
    ServerClient* client = GetClient();

    // Sanity.
    if (!client) {
        return;
    }

    // Clear blend values.
    client->playerState.blend[0] = client->playerState.blend[1] = client->playerState.blend[2] = client->playerState.blend[3] = 0;

    // Calculate view origin to use for PointContents.
    vec3_t viewOrigin = GetOrigin() + client->playerState.pmove.viewOffset;
    int32_t contents = gi.PointContents(viewOrigin);

    // Set render definition flags.
    if (contents & (BrushContents::Water | BrushContents::Slime | BrushContents::Lava)) {
        client->playerState.rdflags |= RDF_UNDERWATER;
    } else {
        client->playerState.rdflags &= ~RDF_UNDERWATER;
    }

    // Add screen blends for Liquid surface contents.
    if (contents & (BrushContents::Solid | BrushContents::Lava)) {
        AddScreenBlend(1.0f, 0.3f, 0.0f, 0.6f, client->playerState.blend);
    } else if (contents & BrushContents::Slime) {
        AddScreenBlend(0.0f, 0.1f, 0.05f, 0.6f, client->playerState.blend);
    } else if (contents & BrushContents::Water) {
        AddScreenBlend(0.5f, 0.3f, 0.2f, 0.4f, client->playerState.blend);
    }

    // Damage Alpha blend.
    if (client->damageAlpha > 0) {
        AddScreenBlend(client->damageBlend[0], client->damageBlend[1], client->damageBlend[2], client->damageAlpha, client->playerState.blend);
    }

    // Bonus Alpha blend.
    if (client->bonusAlpha > 0) {
        AddScreenBlend(0.85f, 0.7f, 0.3f, client->bonusAlpha, client->playerState.blend);
    }

    // Cool down the damage alpha value and ensure it won't go under 0.
    client->damageAlpha -= 0.06f;
    if (client->damageAlpha < 0) {
        client->damageAlpha = 0;
    }

    // Cool down the bonus alpha value and ensure it won't go under 0.
    client->bonusAlpha -= 0.1f;
    if (client->bonusAlpha < 0) {
        client->bonusAlpha = 0;
    }
}

void SVGBasePlayer::UpdateAnimationFrame() {
    // Check whether ent is valid, and a SVGBasePlayer hooked up 
    // to a valid client.
    ServerClient* client = GetClient();

    // Sanity.
    if (!client) {
        return;
    }

    // Need to be a player model.
    if (GetModelIndex() != 255) { 
        return;
    }

    // Are we ducking?
    qboolean isDucking = (client->playerState.pmove.flags & PMF_DUCKED ? true : false);
    qboolean isRunning = (bobMove.XYSpeed ? true : false);

    // Transition to new animation of necessary.
    if (isDucking != client->animation.isDucking && client->animation.priorityAnimation < PlayerAnimation::Death) {
        goto newanim;
    }
    if (isRunning != client->animation.isRunning && client->animation.priorityAnimation == PlayerAnimation::Basic) {
        goto newanim;
    }
    if (!GetGroundEntity() && client->animation.priorityAnimation <= PlayerAnimation::Wave) {
        goto newanim;
    }

    // Reverse animation if it is prioritized.
    if (client->animation.priorityAnimation == PlayerAnimation::Reverse) {
        if (GetAnimationFrame() > client->animation.endFrame) {
            SetAnimationFrame(GetAnimationFrame() - 0.2f);
            return;
        }
    // Otherwise continue the animation playback.
    } else if (GetAnimationFrame() < client->animation.endFrame) {
        // Continue an animation
        SetAnimationFrame(GetAnimationFrame() + 0.2f);
        return;
    }

    // Return in case of death.
    if (client->animation.priorityAnimation == PlayerAnimation::Death) {
        return;     // stay there
    }

    if (client->animation.priorityAnimation == PlayerAnimation::Jump) {
        // Return in case of no ground entity, how can one jump otherwise?
        if (!GetGroundEntity()) {
            return;
        }
        // Silly old Q2 animation stuff.
        client->animation.priorityAnimation = PlayerAnimation::Wave;
        SetAnimationFrame(FRAME_jump3);
        client->animation.endFrame = FRAME_jump6;
        return;
    }

newanim:
    // Transit to either a running or standing frame.
    client->animation.priorityAnimation = PlayerAnimation::Basic;
    client->animation.isDucking = isDucking;
    client->animation.isRunning = isRunning;

    // Got ground?
    if (!GetGroundEntity()) {
        // Jump.
        client->animation.priorityAnimation = PlayerAnimation::Jump;
        if (GetAnimationFrame() != FRAME_jump2) {
            SetAnimationFrame(FRAME_jump1);
        }
        client->animation.endFrame = FRAME_jump2;
    // Keep running.
    } else if (isRunning) {
        // Running
        if (isDucking) {
            SetAnimationFrame(FRAME_crwalk1);
            client->animation.endFrame = FRAME_crwalk6;
        } else {
            SetAnimationFrame(FRAME_run1);
            client->animation.endFrame = FRAME_run6;
        }
    // Standing animations.
    } else {
        // standing
        if (isDucking) {
            SetAnimationFrame(FRAME_crstnd01);
            client->animation.endFrame = FRAME_crstnd19;
        } else {
            SetAnimationFrame(FRAME_stand01);
            client->animation.endFrame = FRAME_stand40;
        }
    }
}