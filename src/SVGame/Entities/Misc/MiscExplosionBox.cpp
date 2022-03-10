/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
//
//
*/
#include "../../ServerGameLocal.h"          // SVGame.
#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"            // Util funcs.
#include "../../Physics/StepMove.h" // Stepmove funcs.

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"

// Misc Explosion Box Entity.
#include "MiscExplosionBox.h"

#include "../../Gamemodes/IGamemode.h"
#include "../../World/Gameworld.h"

//
// Constructor/Deconstructor.
//
MiscExplosionBox::MiscExplosionBox(Entity* svEntity) 
    : Base(svEntity) {

}
MiscExplosionBox::~MiscExplosionBox() {

}



//
// Interface functions. 
//
//
//===============
// MiscExplosionBox::Precache
//
//===============
//
void MiscExplosionBox::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache actual barrel model.
    SVG_PrecacheModel("models/objects/barrels/tris.md2");

    // Precache the debris.
    SVG_PrecacheModel("models/objects/debris1/tris.md2");
    SVG_PrecacheModel("models/objects/debris2/tris.md2");
    SVG_PrecacheModel("models/objects/debris3/tris.md2");
}

//
//===============
// MiscExplosionBox::Spawn
//
//===============
//
void MiscExplosionBox::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set solid.
    SetSolid(Solid::BoundingBox);

    // Set move type.
    SetMoveType(MoveType::TossSlide);

    // Set clip mask.
    SetClipMask(CONTENTS_MASK_MONSTERSOLID | CONTENTS_MASK_PLAYERSOLID);

    // Set the barrel model, and model index.
    SetModel("models/objects/barrels/tris.md2");

    // Set the bounding box.
    SetBoundingBox(
        // Mins.
        { -16.f, -16.f, 0.f },
        // Maxs.
        { 16.f, 16.f, 40.f }
    );

    //SetRenderEffects(GetRenderEffects() | RenderEffects::DebugBoundingBox);

    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(100);
    }
    if (!GetHealth()) {
        SetHealth(80);
    }
    if (!GetDamage()) {
        SetDamage(150);
    }

    // We need it to take damage in case we want it to explode.
    SetTakeDamage(TakeDamage::Yes);

    // Setup our MiscExplosionBox callbacks.
    SetUseCallback(&MiscExplosionBox::ExplosionBoxUse);

    SetDieCallback(&MiscExplosionBox::ExplosionBoxDie);
    SetTouchCallback(&MiscExplosionBox::ExplosionBoxTouch);

    // Setup the next think time.
    SetNextThinkTime(level.time + 2.f * FRAMETIME);
    SetThinkCallback(&MiscExplosionBox::ExplosionBoxDropToFloor);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MiscExplosionBox::Respawn
//
//===============
//
void MiscExplosionBox::Respawn() {
    Base::Respawn();
}

//
//===============
// MiscExplosionBox::PostSpawn
//
//===============
//
void MiscExplosionBox::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// MiscExplosionBox::Think
//
//===============
//
void MiscExplosionBox::Think() {
    // Always call parent class method.
    Base::Think();
}

//===============
// MiscExplosionBox::SpawnKey
//
//===============
void MiscExplosionBox::SpawnKey(const std::string& key, const std::string& value) {
	Base::SpawnKey(key, value);
}


//
// Callback Functions.
//

// ==============
// MiscExplosionBox::ExplosionBoxUse
// 
// So that mappers can trigger this entity in order to blow it up
// ==============
void MiscExplosionBox::ExplosionBoxUse( SVGBaseEntity* caller, SVGBaseEntity* activator )
{
    ExplosionBoxDie( caller, activator, 999, GetOrigin() );
}

//
//===============
// MiscExplosionBox::ExplosionBoxDropToFloor
//
// Think callback, to execute the needed physics for this pusher object.
//===============
//
void MiscExplosionBox::ExplosionBoxDropToFloor(void) {
    // First, ensure our origin is +1 off the floor.
    vec3_t traceStart = GetOrigin() + vec3_t{
        0.f, 0.f, 1.f
    };
    
    SetOrigin(traceStart);

    // Calculate the end origin to use for tracing.
    vec3_t traceEnd = traceStart + vec3_t{
        0, 0, -256.f
    };
    
    // Exceute the trace.
    SVGTrace trace = SVG_Trace(traceStart, GetMins(), GetMaxs(), traceEnd, this, CONTENTS_MASK_MONSTERSOLID);
    
    // Return in case we hit anything.
    if (trace.fraction == 1.f || trace.allSolid) {
	    return;
    }
    
    // Set new entity origin.
    SetOrigin(trace.endPosition);
    
    // Link entity back in.
    LinkEntity();

    // Do a check ground for the step move of this pusher.
    SVG_StepMove_CheckGround(this);

    // Setup its next think time, for a frame ahead.
    //SetThinkCallback(&MiscExplosionBox::ExplosionBoxDropToFloor);
    //SetNextThinkTime(level.time + 1.f * FRAMETIME);

    // Do a check ground for the step move of this pusher.
    //SVG_StepMove_CheckGround(this);
    //M_CatagorizePosition(ent); <-- This shit, has to be moved to SVG_Stepmove_CheckGround.
    // ^ <-- if not for that, it either way has to "categorize" its water levels etc.
    // Not important for this one atm.
}

//
//===============
// MiscExplosionBox::MiscExplosionBoxExplode
//
// 'Think' callback that is set when the explosion box is exploding.
// (Has died due to taking damage.)
//===============
//
void MiscExplosionBox::MiscExplosionBoxExplode(void) {
    // Execute radius damage.
    GetGamemode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), NULL, GetDamage() + 40, MeansOfDeath::Barrel);

    // Retrieve origin.
    vec3_t save = GetOrigin();

    // Set the new origin.
    SetOrigin(vec3_fmaf(GetAbsoluteMin(), 0.5f, GetSize()));

    // Throw several "debris1/tris.md2" chunks.
    SpawnDebris1Chunk();
    SpawnDebris1Chunk();

    // Bottom corners
    vec3_t origin = GetAbsoluteMin();
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.x += GetSize().x;
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.y += GetSize().y;
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.x += GetSize().x;
    origin.y += GetSize().y;
    SpawnDebris3Chunk(origin);

    // Spawn 8 "debris2/tris.md2" chunks.
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();

    // Reset origin to saved origin.
    SetOrigin(save);

    // Depending on whether we have a ground entity or not, we determine which explosion to use.
    if (GetGroundEntity()) {
        gi.MSG_WriteUint8(ServerGameCommand::TempEntity);//WriteByte(ServerGameCommand::TempEntity);
        gi.MSG_WriteUint8(TempEntityEvent::Explosion1);//WriteByte(TempEntityEvent::Explosion1);
        gi.MSG_WriteVector3(GetOrigin(), false);//WriteVector3(GetOrigin());
        gi.Multicast(GetOrigin(), Multicast::PHS);

        GetGamemode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

        float save;
        save = GetDelayTime();
        SetDelayTime(0.0f);
        UseTargets(GetActivator());
        SetDelayTime(save);
    } else {
        gi.MSG_WriteUint8(ServerGameCommand::TempEntity);//WriteByte(ServerGameCommand::TempEntity);
        gi.MSG_WriteUint8(TempEntityEvent::Explosion2);//WriteByte(TempEntityEvent::Explosion2);
        gi.MSG_WriteVector3(GetOrigin(), false);//WriteVector3(GetOrigin());
        gi.Multicast(GetOrigin(), Multicast::PHS);

        GetGamemode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

        float save;
        save = GetDelayTime();
        SetDelayTime(0.0f);
        UseTargets(GetActivator());
        SetDelayTime(save);
    }

    // Ensure we have no more think callback pointer set when this entity has "died"
    SetNextThinkTime(level.time + 1.f * FRAMETIME);
    SetThinkCallback(&MiscExplosionBox::SVGBaseEntityThinkFree);
}

//
//===============
// MiscExplosionBox::ExplosionBoxDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MiscExplosionBox::ExplosionBoxDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    // Entity is dying, it can't take any more damage.
    SetTakeDamage(TakeDamage::No);

    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

    // Setup the next think and think time.
    SetNextThinkTime(level.time + 2 * FRAMETIME);

    // Set think function.
    SetThinkCallback(&MiscExplosionBox::MiscExplosionBoxExplode);
}

//
//===============
// MiscExplosionBox::ExplosionBoxTouch
//
// 'Touch' callback, to calculate the direction to move into.
//===============
//
void MiscExplosionBox::ExplosionBoxTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
    // Safety checks.
    if (!other || other == this) {
	    return;
    }

    // Ground entity checks.
    if (!other->GetGroundEntity() || other->GetGroundEntity() == this) {
	    return;
    }

    // Calculate ratio to use.
    float ratio = ((float)other->GetMass()) / ((float) GetMass());

    // Calculate direction.
    vec3_t dir = GetOrigin() - other->GetOrigin();

    // Calculate yaw to use based on direction.
    float yaw = vec3_to_yaw(dir);

    // Last but not least, move a step ahead.
    SVG_StepMove_Walk(this, yaw, (20.f / BASE_FRAMEDIVIDER) * ratio * FRAMETIME);
}


//
//===============
// MiscExplosionBox::SpawnDebris1Chunk
// 
// Function to spawn "debris1/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris1Chunk() {
    // Acquire a pointer to the game world.
    Gameworld* gameworld = GetGameworld();

    // Speed to throw debris at.
    float speed = 1.5 * (float)GetDamage() / 200.0f;

    // Calculate random direction vector.
    vec3_t randomDirection = {
        RandomRangef(2.f, 0.f), //crandom(),
        RandomRangef(2.f, 0.f),//crandom(),
        RandomRangef(2.f, 0.f),//crandom()      
    };

    // Calculate origin to spawn them at.
    vec3_t origin = GetOrigin() + randomDirection * GetSize();

    // Throw debris!
    gameworld->ThrowDebris(this, "models/objects/debris1/tris.md2", origin, speed);
}


//
//===============
// MiscExplosionBox::SpawnDebris2Chunk
//
// Function to spawn "debris2/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris2Chunk() {
    // Speed to throw debris at.
    float speed = 2.f * GetDamage() / 200.f;

    // Calculate random direction vector.
    vec3_t randomDirection = {
        RandomRangef(2.f, 0.f), //crandom(),
        RandomRangef(2.f, 0.f),//crandom(),
        RandomRangef(2.f, 0.f),//crandom()      
    };

    // Calculate origin to spawn them at.
    vec3_t origin = GetOrigin() + randomDirection * GetSize();

    // Last but not least, throw debris.
    GetGameworld()->ThrowDebris(this, "models/objects/debris2/tris.md2", origin, speed);
}

//
//===============
// MiscExplosionBox::SpawnDebris3Chunk
// 
// Function to spawn "debris3/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris3Chunk(const vec3_t &origin) {
    // Speed to throw debris at.
    float speed = 1.75 * (float)GetDamage() / 200.0f;

    // Throw debris!
    GetGameworld()->ThrowDebris(this, "models/objects/debris3/tris.md2", origin, speed);
}
