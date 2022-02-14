/*
// LICENSE HERE.

//
// SVGBaseItem.cpp
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

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/PlayerClient.h"

// Misc Explosion Box Entity.
#include "SVGBaseItem.h"


//
// Constructor/Deconstructor.
//
SVGBaseItem::SVGBaseItem(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity), displayString(displayString), itemIdentifier(identifier) {

}
SVGBaseItem::~SVGBaseItem() {

}



//
// Interface functions. 
//
//
//===============
// SVGBaseItem::Precache
//
//===============
//
void SVGBaseItem::Precache() {
    // Always call parent class method.
    Base::Precache();
}

//
//===============
// SVGBaseItem::Spawn
//
//===============
//
void SVGBaseItem::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set the config string for this item.
    SVG_SetConfigString(ConfigStrings::Items + itemIdentifier, displayString);

    // Set solid.
    SetSolid(Solid::Trigger);

    // Set move type.
    SetMoveType(MoveType::Toss);

    // Set the bounding box.
    SetBoundingBox(
        // Mins.
        { -16.f, -16.f, -16.f },
        // Maxs.
        { 16.f, 16.f, 16 }
    );

    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(40);
    }

    // Set entity to allow taking damage (can't explode otherwise.)
    SetTakeDamage(TakeDamage::No);

    // Setup our SVGBaseItem callbacks.
    SetUseCallback(&SVGBaseItem::BaseItemUse);
    SetTouchCallback(&SVGBaseItem::BaseItemTouch);

    // Start thinking after other entities have spawned. This allows for items to safely
    // drop on platforms etc.
    SetNextThinkTime(level.time + 2.5f * FRAMETIME);
    SetThinkCallback(&SVGBaseItem::BaseItemDropToFloor);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// SVGBaseItem::Respawn
//
// In order for an item to respawn we first check if it is a member of a team, if it is
// then we'll go and look for a random entity in the list to respawn.
//===============
//
void SVGBaseItem::Respawn() {
    Base::Respawn();

    // Fetch team entity.
//    SVGBaseEntity *teamMaster = GetTeamMasterEntity();
    SVGBaseEntity* slaveEntity = this;

    //// Count and random index.
    //if (!GetTeam().empty()) {
    //    int32_t slaveCount = 0;
    //    uint32_t slaveChoice = 0;

    //    for (count = 0, slaveEntity = teamMaster; teamMaster; slaveEntity = teamMaster->GetTeamChainEntity(), count++) {
    //        // Generate random index within range of the counted amount of team entities.
    //        slaveChoice = RandomRangeui(0, count);

    //        // Fetch the entity we had sought for.
    //        for (count = 0, slaveEntity = teamMasterEntity; count < slaveChoice; slaveEntity = slaveEntity->GetTeamChainEntity(), count++) {

    //        }
    //    }
    //}

    // Time to respawn the entity.
    if (slaveEntity) {
        // Remove NoClient flag so the clients can see this entity again.
        slaveEntity->SetServerFlags(slaveEntity->GetServerFlags() & ~EntityServerFlags::NoClient);

        // Reset the entity to SOlid::Trigger so clients can pick it up again.
        slaveEntity->SetSolid(Solid::Trigger);

        // Attach a respawn effect event to the entity.
        slaveEntity->SetEventID(EntityEvent::ItemRespawn);

        // Link it back in.
        slaveEntity->LinkEntity();

        gi.DPrintf("BaseItem::Respawn");

        // Set think callback to fall to floor (just in case).
        SetThinkCallback(&SVGBaseItem::BaseItemDropToFloor);
    } else {
        gi.DPrintf("No slaveEntity found...");
    }
}

//
//===============
// SVGBaseItem::PostSpawn
//
//===============
//
void SVGBaseItem::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// SVGBaseItem::Think
//
//===============
//
void SVGBaseItem::Think() {
    // Always call parent class method.
    Base::Think();
}


//
// Entity functions.
//
//===============
// SVGBaseItem::SetRespawn
// 
//===============
void SVGBaseItem::SetRespawn(const float delay) {
    // Ensure that for the time being the item doesn't appear to clients.
    SetServerFlags(GetServerFlags() | EntityServerFlags::NoClient);

    // Ensure it isn't solid so client's can't receive another pickup.
    SetSolid(Solid::Not);

    // Set the next think callback to dispatch when the delay is over with.
    SetNextThinkTime(level.time + delay);
    SetThinkCallback(&SVGBaseItem::BaseItemDoRespawn);

    // Relink entity.
    LinkEntity();
}


//
// Callback Functions.
//
//===============
// SVGBaseItem::BaseItemUse
// 
//===============
void SVGBaseItem::BaseItemUse( SVGBaseEntity* caller, SVGBaseEntity* activator )
{
    //BaseItemDie( caller, activator, 999, GetOrigin() );
}

//===============
// SVGBaseItem::BaseItemDropToFloor
//
// 'DropToFloor' callback ensures the item falls on top of whichever entity is below it. (World or others)
//===============
void SVGBaseItem::BaseItemDropToFloor() {
    // First, ensure our origin is +1 off the floor.
    vec3_t newOrigin = GetOrigin() + vec3_t { 0.f, 0.f, 1 };

    SetOrigin(newOrigin);
    
    // Calculate the end origin to use for tracing.
    vec3_t end = newOrigin + vec3_t { 0, 0, -256.f };
    
    // Exceute the trace.
    SVGTrace trace = SVG_Trace(newOrigin, GetMins(), GetMaxs(), end, this, CONTENTS_MASK_PLAYERSOLID);
    
    // Return in case we hit anything.
    if (trace.fraction == 1 || trace.allSolid) {
	    return;
    }
    
    // Set new entity origin.
    SetOrigin(trace.endPosition);
    
    // Check for ground.
    SVG_StepMove_CheckGround(this);

    // If the entity has a team...
    //if (!GetTeam().empty()) {
    //    SetFlags(GetFlags() & ~EntityFlags::TeamSlave);
    //    SVGBaseEntity *teamChainEntity = GetTeamChainEntity();
    //    SetTeamChainEntity(nullptr);
    //    SetServerFlags(GetServerFlags() | EntityServerFlags::NoClient);
    //    SetSolid(Solid::Not);
    //    if (this == GetTeamMasterEntity()) {
    //        SetNextThinkTime(level.time + FRAMETIME);
    //        SetThinkCallback(&SVGBaseItem::BaseItemDoRespawn);
    //    }
    //}

    // Unset think callback.
    SetThinkCallback(nullptr);

    // Relink entity.
    LinkEntity();
}

//===============
// SVGBaseItem::BaseItemDoRespawn
//
// 'DoRespawn' callback calls the entity item's Respawn function.
//===============
void SVGBaseItem::BaseItemDoRespawn() {
    Respawn();
}


//===============
// SVGBaseItem::BaseItemTouch
//
// 'Touch' callback, triggers a pickup event if the entity is allowed to pick up this item.
//===============
void SVGBaseItem::BaseItemTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
    // Safety checks.
    if (!self || !other || self == other)
        return;

    // We need an active client.
    if (!other->GetClient()) {
        return;
    }

    // Dead players can't pick up items.
    if (other->GetHealth() < 1) {
        return;
    }

    // Ensure it is a (sub-)class of PlayerClient
    if (!other->IsSubclassOf<PlayerClient>()) {
        return;
    }

    // Last but not least, ensure we got a pickup callback to dispatch.
    if (!HasPickupCallback()) {
        return;
    }

    // Cast it.
    PlayerClient* playerEntity = dynamic_cast<PlayerClient*>(other);

    // Pick up the item.
    qboolean tookItem = (this->*pickupFunction)(other);

    if (tookItem) {
        // Flash the screen.
        ServerClient* client = playerEntity->GetClient();
        client->bonusAlpha = 0.25f;

        // Show icon and name on status bar.
        //other->GetClient->playerState.stats[STAT_PICKUP_ICON] = SVG_PrecacheImage(GetIcon());
        //other->GetClient->playerState.stats[STAT_PICKUP_ICON] = SVG_PrecacheImage(GetIcon());
        //other->client->pickupMessageTime = level.time + 3.0;

        // change selected item
        //if (ent->item->Use)
        //    other->client->persistent.selectedItem = other->client->playerState.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);
    }

    // Check for whether the item has used its trigger targets.
    if (!(GetSpawnFlags() & ItemSpawnFlags::TargetsUsed)) {
        // It hasn't triggered targets yet, do so now.
        UseTargets();

        // Add flag that it has used its targets.
        SetSpawnFlags(ItemSpawnFlags::TargetsUsed);
    }

    // If we didn't take the item, return.
    if (!tookItem)
        return;

    // If the respawn spawnflag is set, let it respawn.
    if (GetFlags() & EntityFlags::Respawn) {
        SetRespawn(2);
    } else {
        Remove();
    }
}