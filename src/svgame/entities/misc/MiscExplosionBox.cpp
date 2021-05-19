/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
//
//
*/
#include "../../g_local.h"     // SVGame.
#include "../../effects.h"     // Effects.
#include "../../utils.h"       // Util funcs.

#include "../base/SVGBaseEntity.h"

#include "MiscExplosionBox.h"


void barrel_touch(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf)

{
    //float   ratio;
    //vec3_t  v;

    //if ((!other->groundEntityPtr) || (other->groundEntityPtr == self))
    //    return;

    //ratio = (float)other->mass / (float)GetServerEntity()->mass;
    //VectorSubtract(GetServerEntity()->state.origin, other->state.origin, v);
    ////    M_walkmove(self, vectoyaw(v), 20 * ratio * FRAMETIME);
}


void barrel_explode(Entity* self) {

}
void barrel_delay(Entity* self, Entity* inflictor, Entity* attacker, int damage, const vec3_t& point)
{
}

// Constructor/Deconstructor.
MiscExplosionBox::MiscExplosionBox(Entity* svEntity) : SVGBaseEntity(svEntity) {

}
MiscExplosionBox::~MiscExplosionBox() {

}

// Interface functions. 
void MiscExplosionBox::PreCache() {
	gi.DPrintf("MiscExplosionBox::PreCache();");
}
void MiscExplosionBox::Spawn() {
    // Ensure we call the base spawn function.
    SVGBaseEntity::Spawn();

    gi.ModelIndex("models/objects/debris1/tris.md2");
    gi.ModelIndex("models/objects/debris2/tris.md2");
    gi.ModelIndex("models/objects/debris3/tris.md2");

    // Set solid.
    SetSolid(Solid::BoundingBox);

    // Set move type.
    SetMoveType(MoveType::Step);

    //GetServerEntity()->model = "models/objects/barrels/tris.md2";
    // Set the barrel model, and model index.
    SetModel("models/objects/barrels/tris.md2");
    GetServerEntity()->state.modelIndex = gi.ModelIndex(GetServerEntity()->model);

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
void MiscExplosionBox::PostSpawn() {
	gi.DPrintf("MiscExplosionBox::PostSpawn();");
}
void MiscExplosionBox::Think() {
    SVGBaseEntity::Think();
	gi.DPrintf("MiscExplosionBox::Think();");
}

// Functions.
extern void SVG_StepMove_CheckGround(Entity* ent);
void MiscExplosionBox::MiscExplosionBoxThink(void) {
    vec3_t      end;
    trace_t     trace;

    GetServerEntity()->state.origin[2] += 1;
    VectorCopy(GetServerEntity()->state.origin, end);
    end[2] -= 256;

    trace = gi.Trace(GetServerEntity()->state.origin, GetServerEntity()->mins, GetServerEntity()->maxs, end, GetServerEntity(), CONTENTS_MASK_MONSTERSOLID);

    if (trace.fraction == 1 || trace.allSolid)
    return;

    VectorCopy(trace.endPosition, GetServerEntity()->state.origin);

    if (GetServerEntity()->state.number == 12) {
        gi.DPrintf("I think, therefor, as a misc_explobox I AM!\n");
    }
    gi.LinkEntity(GetServerEntity());
    SVG_StepMove_CheckGround(GetServerEntity());
    //M_CatagorizePosition(ent);
}
void MiscExplosionBox::MiscExplosionBoxExplode(void)
{
    vec3_t  org;
    float   spd;
    vec3_t  save;

    SVG_RadiusDamage(GetServerEntity(), GetServerEntity()->activator, GetServerEntity()->damage, NULL, GetServerEntity()->damage + 40, MeansOfDeath::Barrel);

    VectorCopy(GetServerEntity()->state.origin, save);
    VectorMA(GetServerEntity()->absMin, 0.5, GetServerEntity()->size, GetServerEntity()->state.origin);

    // a few big chunks
    spd = 1.5 * (float)GetServerEntity()->damage / 200.0;
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris1/tris.md2", spd, org);
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris1/tris.md2", spd, org);

    // bottom corners
    spd = 1.75 * (float)GetServerEntity()->damage / 200.0;
    VectorCopy(GetServerEntity()->absMin, org);
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(GetServerEntity()->absMin, org);
    org[0] += GetServerEntity()->size[0];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(GetServerEntity()->absMin, org);
    org[1] += GetServerEntity()->size[1];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(GetServerEntity()->absMin, org);
    org[0] += GetServerEntity()->size[0];
    org[1] += GetServerEntity()->size[1];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris3/tris.md2", spd, org);

    // a bunch of little chunks
    spd = 2 * GetServerEntity()->damage / 200;
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", spd, org);
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", spd, org);
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", spd, org);
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", spd, org);
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", spd, org);
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", spd, org);
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", spd, org);
    org[0] = GetServerEntity()->state.origin[0] + crandom() * GetServerEntity()->size[0];
    org[1] = GetServerEntity()->state.origin[1] + crandom() * GetServerEntity()->size[1];
    org[2] = GetServerEntity()->state.origin[2] + crandom() * GetServerEntity()->size[2];
    SVG_ThrowDebris(GetServerEntity(), "models/objects/debris2/tris.md2", spd, org);

    VectorCopy(save, GetServerEntity()->state.origin);
    if (GetServerEntity()->groundEntityPtr)
        BecomeExplosion2(GetServerEntity());
    else
        BecomeExplosion1(GetServerEntity());

    SetThink(nullptr);
}

void MiscExplosionBox::MiscExplosionBoxDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    GetServerEntity()->takeDamage = TakeDamage::No;
    GetServerEntity()->nextThinkTime = level.time + 2 * FRAMETIME;
    if (attacker)
        GetServerEntity()->activator = attacker->GetServerEntity();

    SetThink(&MiscExplosionBox::MiscExplosionBoxExplode);
}

void MiscExplosionBox::MiscExplosionBoxTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
    float   ratio;
    vec3_t  v;

    if (!self)
        return;
    if (!other)
        return;

    if ((!other->GetServerEntity()->groundEntityPtr) || (other->GetServerEntity()->groundEntityPtr == GetServerEntity()))
        return;

    ratio = (float)other->GetServerEntity()->mass / (float)GetServerEntity()->mass;
    VectorSubtract(GetServerEntity()->state.origin, other->GetServerEntity()->state.origin, v);
    float yaw = vec3_to_yaw(v);

    SVG_StepMove_Walk(GetServerEntity(), yaw, 20 * ratio * FRAMETIME);
    gi.DPrintf("self: '%i' is TOUCHING other: '%i'\n", self->GetServerEntity()->state.number, other->GetServerEntity()->state.number);
}