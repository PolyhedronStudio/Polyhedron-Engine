/***
*
*	License here.
*
*	@file
*
*	SharedGame Base Item implementation.
*
***/
// Needed for the shared headers.
#define CGAME_INCLUDE 1
// Include shared headers.
#include "Shared/Shared.h"
#include "Shared/Refresh.h"

// SharedGame header itself.
#include "Game/Shared/SharedGame.h"

#ifdef SHAREDGAME_CLIENTGAME

// Include CLGbasePacket and CLGBaseLocal Entity types.
#include "../../Client/Entities/IClientGameEntity.h"
#include "../../Client/Entities/Base/CLGBasePacketEntity.h"
#include "../../Client/Entities/Base/CLGBaseLocalEntity.h"
#include "../../Client/Entities/Base/CLGBasePlayer.h"
using SGBasePlayer = CLGBasePlayer;
#endif
#ifdef SHAREDGAME_SERVERGAME 
#include "../../Client/Entities/IServerGameEntity.h"
#include "../../Server/Entities/Base/SVGBaseEntity.h"
#include "../../Server/Entities/Base/SVGBaseTrigger.h"
#include "../../Server/Entities/Base/SVGBasePlayer.h"
using SGBasePlayer = SVGBasePlayer;
#endif
// Item & ItemWeapon.
#include "SGBaseItem.h"


//! Used to store instances that are used for player weapon callbacks.
SGBaseItem* SGBaseItem::itemInstances[ItemID::Maximum];
//! Used for looking up instances by string. TODO: Improve this, it can be done more simply.
std::map<std::string, uint32_t> SGBaseItem::lookupStrings;



//! Constructor/Deconstructor.
SGBaseItem::SGBaseItem(PODEntity *svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity), displayString(displayString), itemIdentifier(identifier) {

}



/**
* 
*   Interface implementation functions.
*
***/
/**
*   @brief
**/
void SGBaseItem::Precache() {
    // Always call parent class method.
    Base::Precache();
}

/**
*   @brief
**/
void SGBaseItem::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set the config string for this item.
    //SVG_SetConfigString(ConfigStrings::Items + itemIdentifier, displayString);

    //// Set solid.
    //SetSolid(Solid::Trigger);

    //// Set move type.
    //SetMoveType(MoveType::Toss);

    //// Set the bounding box.
    //SetBoundingBox(
    //    // Mins.
    //    { -16.f, -16.f, -8.f },
    //    // Maxs.
    //    { 16.f, 16.f, 8 }
    //);

    //// Set default values in case we have none.
    //if (!GetMass()) {
    //    SetMass(40);
    //}

    //// Set entity to allow taking damage (can't explode otherwise.)
    //SetTakeDamage(TakeDamage::No);

    //// Setup our SGBaseItem callbacks.
    //SetUseCallback(&SGBaseItem::BaseItemUse);
    //SetUseInstanceCallback(&SGBaseItem::BaseItemUseInstance);
    //SetTouchCallback(&SGBaseItem::BaseItemTouch);

    //// Start thinking after other entities have spawned. This allows for items to safely
    //// drop on platforms etc.
    //SetNextThinkTime(level.time + 2.5f * FRAMETIME_S);
    //SetThinkCallback(&SGBaseItem::BaseItemDropToFloor);

    //// Link the entity to world, for collision testing.
    //LinkEntity();
}

/**
*   @brief  In order for an item to respawn we first check if it is a member of a team, if it is
*           then we'll go and look for a random entity in the list to respawn.
**/
void SGBaseItem::Respawn() {
    Base::Respawn();

    //// Fetch team entity.
    ////GameEntity *teamMaster = GetTeamMasterEntity();
    //GameEntity* slaveEntity = this;

    ////// Count and random index.
    ////if (!GetTeam().empty()) {
    ////    int32_t slaveCount = 0;
    ////    uint32_t slaveChoice = 0;

    ////    for (count = 0, slaveEntity = teamMaster; teamMaster; slaveEntity = teamMaster->GetTeamChainEntity(), count++) {
    ////        // Generate random index within range of the counted amount of team entities.
    ////        slaveChoice = RandomRangeui(0, count);

    ////        // Fetch the entity we had sought for.
    ////        for (count = 0, slaveEntity = teamMasterEntity; count < slaveChoice; slaveEntity = slaveEntity->GetTeamChainEntity(), count++) {

    ////        }
    ////    }
    ////}

    //// Time to respawn the entity.
    //if (slaveEntity) {
    //    // Remove NoClient flag so the clients can see this entity again.
    //    slaveEntity->SetServerFlags(slaveEntity->GetServerFlags() & ~EntityServerFlags::NoClient);

    //    // Reset the entity to SOlid::Trigger so clients can pick it up again.
    //    slaveEntity->SetSolid(Solid::Trigger);

    //    // Attach a respawn effect event to the entity.
    //    slaveEntity->SetEventID(EntityEvent::ItemRespawn);

    //    // Link it back in.
    //    slaveEntity->LinkEntity();

    //    // Set think callback to fall to floor (just in case).
    //    SetThinkCallback(&SGBaseItem::BaseItemDropToFloor);
    //} else {
    //	SVG_DPrint("No slave entity found for SGBaseItem(#" + GetState().number + std::string(") at origin: ") + vec3_to_str(GetOrigin()) + "\n");
    //}
}

/**
*   @brief
**/
void SGBaseItem::PostSpawn() {
    //// Always call parent class method.
    //Base::PostSpawn();
}

/**
*   @brief
**/
void SGBaseItem::Think() {
    //// Always call parent class method.
    //Base::Think();
}



/**
*
*   Instance Interface implementation functions.
*
**/
void SGBaseItem::InstanceSpawn() {
    SetUseInstanceCallback(&SGBaseItem::BaseItemUseInstance);
}



/**
* 
*   Entity functions.
*
***/
/**
*   @brief Engages this item in respawn mode waiting for the set delay to pass before respawning.
**/
void SGBaseItem::SetRespawn(const Frametime& delay) {
    //// Ensure that for the time being the item doesn't appear to clients.
    //SetServerFlags(GetServerFlags() | EntityServerFlags::NoClient);

    //// Ensure it isn't solid so client's can't receive another pickup.
    //SetSolid(Solid::Not);

    //// Set the next think callback to dispatch when the delay is over with.
    //SetNextThinkTime(level.time + delay);
    //SetThinkCallback(&SGBaseItem::BaseItemDoRespawn);

    //// Relink entity.
    //LinkEntity();
}

/**
*   @brief  Use for item instances, calls their "Use Item" callback.
* 
*   @details    'UseInstance' is not to be confused with the general 'Use' dispatch 
*               function for trigger callbacks.
**/
void SGBaseItem::UseInstance(GameEntity* user, SGBaseItem* item) {
    //// Safety check.
    //if (useInstanceFunction == nullptr) {
	   // return;
    //}

    //// Execute 'Use Item' callback function.
    //(this->*useInstanceFunction)(user, item);
}


/**
* 
*   Base Entity interface Callbacks.
*
***/
/**
*   @brief Callback for when being triggered. Also known as "Use".
**/
void SGBaseItem::BaseItemUse( GameEntity* caller, GameEntity* activator )
{
    //BaseItemDie( caller, activator, 999, GetOrigin() );
}



/**
* 
*   Item Entity interface Callbacks.
*
***/
/**
*   @brief Callback for item instance usage.
**/
void SGBaseItem::BaseItemUseInstance(GameEntity* user, SGBaseItem* item) {

}

/**
*   @brief Callback for when an entity touches this item.
**/
void SGBaseItem::BaseItemTouch(GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
    //// Safety checks.
    //if (!self || !other || self == other)
    //    return;

    //// We need an active client.
    //if (!other->GetClient()) {
    //    return;
    //}

    //// Dead players can't pick up items.
    //if (other->GetHealth() < 1) {
    //    return;
    //}

    //// Ensure it is a (sub-)class of SGBasePlayer
    //if (!other->IsSubclassOf<SGBasePlayer>()) {
    //    return;
    //}

    //// Last but not least, ensure we got a pickup callback to dispatch.
    //if (!HasPickupCallback()) {
    //    return;
    //}

    //// Cast it.
    //SGBasePlayer* playerEntity = dynamic_cast<SGBasePlayer*>(other);

    //// Pick up the item.
    //qboolean tookItem = (this->*pickupFunction)(other);

    //if (tookItem) {
    //    // Flash the screen.
    //    ServerClient* client = playerEntity->GetClient();
    //    client->bonusAlpha = 0.25f;

    //    // Show icon and name on status bar.
    //    //other->GetClient->playerState.stats[STAT_PICKUP_ICON] = SVG_PrecacheImage(GetIcon());
    //    //other->GetClient->playerState.stats[STAT_PICKUP_ICON] = SVG_PrecacheImage(GetIcon());
    //    //other->client->pickupMessageTime = level.time + 3.0;

    //    // change selected item
    //    //if (ent->item->Use)
    //    //    other->client->persistent.selectedItem = other->client->playerState.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);
    //}

    //// Check for whether the item has used its trigger targets.
    //if (!(GetSpawnFlags() & ItemSpawnFlags::TargetsUsed)) {
    //    // It hasn't triggered targets yet, do so now.
    //    UseTargets();

    //    // Add flag that it has used its targets.
    //    SetSpawnFlags(ItemSpawnFlags::TargetsUsed);
    //}

    //// If we didn't take the item, return.
    //if (!tookItem)
    //    return;

    //// If the respawn spawnflag is set, let it respawn.
    //if (GetFlags() & EntityFlags::Respawn) {
    //    SetRespawn(2s);
    //} else {
    //    Remove();
    //}
}

/**
*   @brief Callback for executing drop to floor behavior.
**/
void SGBaseItem::BaseItemDropToFloor() {
 //   // First, ensure our origin is +1 off the floor.
 //   vec3_t newOrigin = GetOrigin() + vec3_t { 0.f, 0.f, 1 };

 //   SetOrigin(newOrigin);

 //   // Calculate the end origin to use for tracing.
 //   vec3_t end = newOrigin + vec3_t { 0, 0, -256.f };

 //   // Exceute the trace.
 //   SGTraceResult trace = SG_Trace(newOrigin, GetMins(), GetMaxs(), end, this, BrushContentsMask::PlayerSolid);

 //   // Return in case we hit anything.
 //   if (trace.fraction == 1 || trace.allSolid) {
	//return;
 //   }

 //   // Set new entity origin.
 //   SetOrigin(trace.endPosition);

 //   // Check for ground.
 //   SG_CheckGround(this);//SVG_StepMove_CheckGround(this);

 //   // If the entity has a team...
 //   //if (!GetTeam().empty()) {
 //   //    SetFlags(GetFlags() & ~EntityFlags::TeamSlave);
 //   //    GameEntity *teamChainEntity = GetTeamChainEntity();
 //   //    SetTeamChainEntity(nullptr);
 //   //    SetServerFlags(GetServerFlags() | EntityServerFlags::NoClient);
 //   //    SetSolid(Solid::Not);
 //   //    if (this == GetTeamMasterEntity()) {
 //   //        SetNextThinkTime(level.time + FRAMETIME_S);
 //   //        SetThinkCallback(&SGBaseItem::BaseItemDoRespawn);
 //   //    }
 //   //}

 //   // Unset think callback.
 //   SetThinkCallback(nullptr);

 //   // Relink entity.
 //   LinkEntity();
}

/**
*   @brief Callback meant to be used by SetThink so one can delay a call to Respawn.
**/
void SGBaseItem::BaseItemDoRespawn() { Respawn(); }