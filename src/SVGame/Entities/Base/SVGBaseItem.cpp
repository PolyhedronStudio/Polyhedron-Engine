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

// Misc Explosion Box Entity.
#include "SVGBaseItem.h"


//
// Constructor/Deconstructor.
//
SVGBaseItem::SVGBaseItem(Entity* svEntity) 
    : Base(svEntity) {

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

    // Set solid.
    SetSolid(Solid::Trigger);

    // Set move type.
    SetMoveType(MoveType::Toss);

    // Set the barrel model, and model index.
    SetModel("models/objects/barrels/tris.md2");

    // Set the bounding box.
    SetBoundingBox(
        // Mins.
        { -16.f, -16.f, 0.f },
        // Maxs.
        { 16.f, 16.f, 16.f }
    );

    //SetFlags(EntityFlags::PowerArmor);

    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(40);
    }
    //if (!GetHealth()) {
    //    SetHealth(150);
    //}
//    SetHealth(999);
//    if (!GetDamage()) {
//        SetDamage(150);
//    }

    // Set entity to allow taking damage (can't explode otherwise.)
    SetTakeDamage(TakeDamage::No);

    // Setup our SVGBaseItem callbacks.
    SetUseCallback(&SVGBaseItem::BaseItemUse);
    SetThinkCallback(&SVGBaseItem::BaseItemThink);
    SetDieCallback(&SVGBaseItem::BaseItemDie);
    SetTouchCallback(&SVGBaseItem::BaseItemTouch);

    // Start thinking after other entities have spawned. This allows for items to safely
    // drop on platforms etc.
    SetNextThinkTime(level.time + 2.f * FRAMETIME);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// SVGBaseItem::Respawn
//
//===============
//
void SVGBaseItem::Respawn() {
    Base::Respawn();
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
// Callback Functions.
//

//===============
// SVGBaseItem::BaseItemUse
// 
// 
//===============
void SVGBaseItem::BaseItemUse( SVGBaseEntity* caller, SVGBaseEntity* activator )
{
    BaseItemDie( caller, activator, 999, GetOrigin() );
}

//
//===============
// SVGBaseItem::BaseItemThink
//
// 
//===============
//
void SVGBaseItem::BaseItemThink(void) {


    // Setup its next think time, for a frame ahead.
    SetThinkCallback(&SVGBaseItem::BaseItemThink);
    SetNextThinkTime(level.time + 1.f * FRAMETIME);
}

//
//===============
// SVGBaseItem::BaseItemDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void SVGBaseItem::BaseItemDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {

}

//
//===============
// SVGBaseItem::BaseItemTouch
//
// 'Touch' callback, to calculate the direction to move into.
//===============
//
void SVGBaseItem::BaseItemTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
    // Safety checks.
    if (!self)
        return;
    if (!other)
        return;
    // TODO: Move elsewhere in baseentity, I guess?
    // Prevent this entity from touching itself.
    if (self == other)
        return;

    // Call actual pickup function if we have any.
    if (HasPickupCallback()) {
        (this->*pickupFunction)(other);
    }
}