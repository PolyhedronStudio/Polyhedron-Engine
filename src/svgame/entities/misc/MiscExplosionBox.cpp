/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
//
//
*/
#include "../../g_local.h"          // SVGame.
#include "../../effects.h"          // Effects.
#include "../../utils.h"            // Util funcs.
#include "../../physics/stepmove.h" // Stepmove funcs.

// Server Game Base Entity.
#include "../base/SVGBaseEntity.h"

// Misc Explosion Box Entity.
#include "MiscExplosionBox.h"



//
// Constructor/Deconstructor.
//
MiscExplosionBox::MiscExplosionBox(Entity* svEntity) : SVGBaseEntity(svEntity) {

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
    // Precache actual barrel model.
    SVG_PrecacheModel("models/objects/barrels/tris.md2");

    // Precache the debris.
    SVG_PrecacheModel("models/objects/debris1/tris.md2");
    SVG_PrecacheModel("models/objects/debris2/tris.md2");
    SVG_PrecacheModel("models/objects/debris3/tris.md2");

    //gi.DPrintf("MiscExplosionBox::Precache();");
}

//
//===============
// MiscExplosionBox::Spawn
//
//===============
//
void MiscExplosionBox::Spawn() {
    // Ensure we call the base spawn function.
    SVGBaseEntity::Spawn();

    // Set solid.
    SetSolid(Solid::BoundingBox);

    // Set move type.
    SetMoveType(MoveType::Step);

    // Set the barrel model, and model index.
    SetModel("models/objects/barrels/tris.md2");

    // Set the bounding box.
    SetBoundingBox(
        // Mins.
        {
            -16, -16, 0
        },
        // Maxs.
        {
            16, 16, 40
        }
    );

    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(400);
    }
    if (!GetHealth()) {
        SetHealth(10);
    }
    if (!GetDamage())
        SetDamage(150);

    // Set entity to allow taking damage (can't explode otherwise.)
    SetTakeDamage(TakeDamage::Yes);

    // Setup our MiscExplosionBox callbacks.
    SetThink(&MiscExplosionBox::MiscExplosionBoxThink);
    SetDie(&MiscExplosionBox::MiscExplosionBoxDie);
    SetTouch(&MiscExplosionBox::MiscExplosionBoxTouch);

    // Setup the next think time.
    SetNextThinkTime(level.time + 2 * FRAMETIME);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MiscExplosionBox::PostSpawn
//
//===============
//
void MiscExplosionBox::PostSpawn() {
	//gi.DPrintf("MiscExplosionBox::PostSpawn();");
}

//
//===============
// MiscExplosionBox::Think
//
//===============
//
void MiscExplosionBox::Think() {
    SVGBaseEntity::Think();
	//gi.DPrintf("MiscExplosionBox::Think();");
}


//
// Callback Functions.
//
//
//===============
// MiscExplosionBox::MiscExplosionBoxThink
//
// Think callback, to execute the needed physics for this pusher object.
//===============
//
void MiscExplosionBox::MiscExplosionBoxThink(void) {
    // Calculate trace end position.
    vec3_t end = GetOrigin() + vec3_t { 0.f, 0.f, 1.f };

    // Set origin + 1 on the Z axis.
    SetOrigin(GetOrigin() + vec3_t{ 0.f, 0.f, 1.f });
    
    // Calculate the end point for tracing.
    end = GetOrigin() + vec3_t{ 0.f, 0.f, 256.f };

    // Execute the trace.
    trace_t trace = gi.Trace(GetOrigin(), GetMins(), GetMaxs(), end, GetServerEntity(), CONTENTS_MASK_MONSTERSOLID);

    // Return in case of fraction 1 or allSolid.
    if (trace.fraction == 1 || trace.allSolid) {
        return;
    }

    // Set origin to the trace end position.
    SetOrigin(trace.endPosition);

    if (GetServerEntity()->state.number == 12) {
        gi.DPrintf("I think, therefor, as a misc_explobox I AM!\n");
    }
    
    // Link entity for testing.
    LinkEntity();

    // Do a check ground for the step move of this pusher.
    SVG_StepMove_CheckGround(GetServerEntity());
    //M_CatagorizePosition(ent);
}

//
//===============
// MiscExplosionBox::MiscExplosionBoxExplode
//
// 'Think' callback that is set when the explosion box is exploding.
// (Has died due to taking damage.)
//===============
//
void MiscExplosionBox::MiscExplosionBoxExplode(void)
{
    // Execute radius damage.
    SVG_RadiusDamage(GetServerEntity(), GetServerEntity()->activator, GetDamage(), NULL, GetDamage() + 40, MeansOfDeath::Barrel);

    // Retrieve origin.
    vec3_t save = GetOrigin();

    // Set the new origin.
    SetOrigin(vec3_fmaf(GetAbsoluteMin(), 0.5f, GetSize()));

    // Calculate speed.
    float speed = 1.5 * (float)GetDamage() / 200.0f;

    // Throw several debris chunks.
    vec3_t randomVec = { crandom(), crandom(), crandom() };
    vec3_t origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris1/tris.md2", speed, origin);

    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris1/tris.md2", speed, origin);

    // bottom corners
    speed = 1.75 * (float)GetDamage() / 200.0f;
    origin = GetAbsoluteMin();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris3/tris.md2", speed, origin);
    origin = GetAbsoluteMin();
    origin.x += GetSize().x;
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris3/tris.md2", speed, origin);
    origin = GetAbsoluteMin();
    origin.y += GetSize().y;
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris3/tris.md2", speed, origin);
    origin = GetAbsoluteMin();
    origin.x += GetSize().x;
    origin.y += GetSize().y;
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris3/tris.md2", speed, origin);

    // A bunch of little chunks
    speed = 2.f * GetDamage() / 200.f;
    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", speed, origin);
    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", speed, origin);
    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", speed, origin);
    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", speed, origin);
    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", speed, origin);
    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", speed, origin);
    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", speed, origin);
    randomVec = { crandom(), crandom(), crandom() };
    origin = GetOrigin() + randomVec * GetSize();
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", speed, origin);
    
    // Reset origin to saved origin.
    SetOrigin(save);

    // Depending on whether we have a ground entity or not, we determine which explosion to use.
    if (GetServerEntity()->groundEntityPtr)
        BecomeExplosion2(GetServerEntity());
    else
        BecomeExplosion1(GetServerEntity());

    // Ensure we have no more think callback pointer set when this entity has "died"
    SetThink(nullptr);
}

//
//===============
// MiscExplosionBox::MiscExplosionBoxDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MiscExplosionBox::MiscExplosionBoxDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    // Entity is dying, it can't take any more damage.
    SetTakeDamage(TakeDamage::No);
    
    // Attacker becomes this entity its "activator".
    if (attacker)
        GetServerEntity()->activator = attacker->GetServerEntity();

    // Setup the next think and think time.
    SetNextThinkTime(level.time + 2 * FRAMETIME);

    // Set think function.
    SetThink(&MiscExplosionBox::MiscExplosionBoxExplode);
}

//
//===============
// MiscExplosionBox::MiscExplosionBoxTouch
//
// 'Touch' callback, to calculate the direction to move into.
//===============
//
void MiscExplosionBox::MiscExplosionBoxTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
    // Safety checks.
    if (!self)
        return;
    if (!other)
        return;

    // Ground entity checks.
    if ((!other->GetServerEntity()->groundEntityPtr) || (other->GetServerEntity()->groundEntityPtr == GetServerEntity()))
        return;

    // Calculate ratio to use.
    float ratio = (float)other->GetServerEntity()->mass / (float)GetServerEntity()->mass;
    
    // Calculate direction.
    vec3_t dir = GetOrigin() - other->GetOrigin();

    // Calculate yaw to use based on direction.
    float yaw = vec3_to_yaw(dir);

    // Last but not least, move a step ahead.
    SVG_StepMove_Walk(GetServerEntity(), yaw, 20 * ratio * FRAMETIME);
    gi.DPrintf("self: '%i' is TOUCHING other: '%i'\n", self->GetServerEntity()->state.number, other->GetServerEntity()->state.number);
}