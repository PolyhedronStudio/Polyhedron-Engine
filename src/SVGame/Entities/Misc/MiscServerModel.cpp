/*
// LICENSE HERE.

//
// MiscServerModel.cpp
//
//
*/
#include "../../g_local.h"          // SVGame.
#include "../../effects.h"          // Effects.
#include "../../utils.h"            // Util funcs.
#include "../../physics/stepmove.h" // Stepmove funcs.

// Server Game Base Entity.
#include "../base/SVGBaseEntity.h"
#include "../base/SVGBaseTrigger.h"

// Misc Server Model Entity.
#include "MiscServerModel.h"



//
// Constructor/Deconstructor.
//
MiscServerModel::MiscServerModel(Entity* svEntity)
    : SVGBaseTrigger(svEntity) {

}
MiscServerModel::~MiscServerModel() {

}



//
// Interface functions. 
//
//
//===============
// MiscServerModel::Precache
//
//===============
//
void MiscServerModel::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache the passed image/model.
    // WID: TODO: We can probably do this check in SetModel, store a bool for it or so.
    // that'd save us from having to parse it again in the Spawn function.
    if (model.find_last_of(".sp2") != std::string::npos) {
        SVG_PrecacheModel(model);
    } else {
        SVG_PrecacheImage(model);
    }

    // Should we precache sound? aka noise?
    if (!noisePath.empty()) {
        precachedNoiseIndex = SVG_PrecacheSound(noisePath);
    }

    //sprites / torchflame1_1.sp2
}

//
//===============
// MiscServerModel::Spawn
//
//===============
//
void MiscServerModel::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set solid.
    SetSolid(Solid::BoundingBox);

    // Set move type.
    SetMoveType(MoveType::None);

    // Since this is a "monster", after all...
    SetFlags(EntityServerFlags::Monster);

    // Set clip mask.
    SetClipMask(CONTENTS_MASK_MONSTERSOLID | CONTENTS_MASK_PLAYERSOLID);

    // Set the barrel model, and model index.
    SetModel(model);

    // Set noise ( in case one is precached. )
    if (precachedNoiseIndex) {
        SetNoiseIndex(precachedNoiseIndex);
        SetSound(GetNoiseIndex());
    }

    // Determine whether the model is a sprite. In case it is, we must set the Translucent flag for it to render properly.
    if (model.find_last_of(".sp2") != std::string::npos) {
        SetRenderEffects(RenderEffects::Translucent);
    }

    // Set the bounding box.
    SetBoundingBox(GetMins(), GetMaxs());

    //SetFlags(EntityFlags::Swim);
    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(40);
    }
    if (!GetHealth()) {
        SetHealth(200);
    }
    if (!GetDamage()) {
        SetDamage(150);
    }

    // Setup the start frame to animate from.
    if (startFrame) {
        SetFrame(startFrame);
    } else {
        SetFrame(0);
    }

    // Set entity to allow taking damage.
    SetTakeDamage(TakeDamage::Yes);

    //// Setup our MiscServerModel callbacks.
    //SetUseCallback(&MiscServerModel::MiscServerModelUse);
    SetThinkCallback(&MiscServerModel::MiscServerModelThink);
    SetDieCallback(&MiscServerModel::MiscServerModelDie);
    //SetTouchCallback(&MiscServerModel::MiscServerModelTouch);

    // Setup the next think time.
    SetNextThinkTime(level.time + 2.f * FRAMETIME);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MiscServerModel::Respawn
//
//===============
//
void MiscServerModel::Respawn() {
    Base::Respawn();
    //gi.DPrintf("MiscServerModel::Respawn();");
}

//
//===============
// MiscServerModel::PostSpawn
//
//===============
//
void MiscServerModel::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
    //gi.DPrintf("MiscServerModel::PostSpawn();");
}

//===============
// MiscServerModel::Think
//
//===============
void MiscServerModel::Think() {
    // Always call parent class method.
    Base::Think();

    // Continue the animation on a per frame basis.
    int32_t currentFrame = GetFrame();

    if (currentFrame > endFrame) {
        SetFrame(startFrame);
    } else {
        SetFrame(currentFrame + 1);
    }

    SetNextThinkTime(level.time + 1 * BASE_1_FRAMETIME);
    //if (GetNoiseIndex()) {
    //    SVG_Sound(this, CHAN_NO_PHS_ADD + CHAN_VOICE, GetSound(), 1.f, ATTN_NONE, 0.f);
    //}

    //gi.DPrintf("MiscServerModel::Think();");
}

//===============
// MiscServerModel::SpawnKey
//
//===============
void MiscServerModel::SpawnKey(const std::string& key, const std::string& value) {
    if (key == "model") {
        ParseStringKeyValue(key, value, model);
    } else if (key == "boundingboxmins") {
        ParseVector3KeyValue(key, value, boundingBoxMins);
        SetMins(boundingBoxMins);
    } else if (key == "boundingboxmaxs") {
        ParseVector3KeyValue(key, value, boundingBoxMaxs);
        SetMaxs(boundingBoxMaxs);
    } else if (key == "endframe") {
        ParseIntegerKeyValue(key, value, endFrame);
    } else if (key == "startframe") {
        ParseIntegerKeyValue(key, value, startFrame);
    } else if (key == "mass") {
        uint32_t parsedMass = 0;
        ParseUnsignedIntegerKeyValue(key, value, parsedMass);
        SetMass(parsedMass);
    } else if (key == "health") {
        uint32_t parsedHealth = 0;
        ParseUnsignedIntegerKeyValue(key, value, parsedHealth);
        SetMaxHealth(parsedHealth);
        SetHealth(parsedHealth);
    } else if (key == "effects") {
        uint32_t parsedEffects = 0;
        ParseUnsignedIntegerKeyValue(key, value, parsedEffects);
        SetEffects(parsedEffects);
    } else if (key == "rendereffects") {
        uint32_t parsedRenderEffects = 0;
        ParseUnsignedIntegerKeyValue(key, value, parsedRenderEffects);
        SetRenderEffects(parsedRenderEffects);
    } else if (key == "noise") {
        std::string parsedNoisePath = "";

        ParseStringKeyValue(key, value, parsedNoisePath);
        noisePath = parsedNoisePath;
    } else {
        Base::SpawnKey(key, value);
    }
}

//
// Callback Functions.
//

//// ==============
//// MiscServerModel::MiscServerModelUse
//// 
//// So that mappers can trigger this entity in order to blow it up
//// ==============
//void MiscServerModel::MiscServerModelUse(SVGBaseEntity* caller, SVGBaseEntity* activator) {
//    MiscServerModelDie(caller, activator, 999, GetOrigin());
//}


//===============
// MiscServerModel::MiscServerModelThink
//
// Think callback, to execute the needed physics for this pusher object.
//===============
void MiscServerModel::MiscServerModelThink(void) {
    // First, ensure our origin is +1 off the floor.
    vec3_t newOrigin = GetOrigin() + vec3_t{
        0.f, 0.f, 1.f
    };

    SetOrigin(newOrigin);
    //
    ////    // Calculate the end origin to use for tracing.
    vec3_t end = newOrigin + vec3_t{
        0, 0, -256.f
    };
    //
    //    // Exceute the trace.
    SVGTrace trace = SVG_Trace(newOrigin, GetMins(), GetMaxs(), end, this, CONTENTS_MASK_MONSTERSOLID);
    ////
    ////    // Return in case we hit anything.
    if (trace.fraction == 1 || trace.allSolid)
        return;
    ////
    ////    // Set new entity origin.
    SetOrigin(trace.endPosition);
    //
    //     //
    //    // Check for ground.
    SVG_StepMove_CheckGround(this);
    //
    //    // Setup its next think time, for a frame ahead.
    SetNextThinkTime(level.time + 1 * BASE_1_FRAMETIME);
    //    // Link entity back in.
    LinkEntity();

    //
    //    //// Do a check ground for the step move of this pusher.
    //SVG_StepMove_CheckGround(this);
    //    //M_CatagorizePosition(ent); <-- This shit, has to be moved to SVG_Stepmove_CheckGround.
    //    // ^ <-- if not for that, it either way has to "categorize" its water levels etc.
    //    // Not important for this one atm.
}
//
////
////===============
//// MiscServerModel::MiscServerModelExplode
////
//// 'Think' callback that is set when the explosion box is exploding.
//// (Has died due to taking damage.)
////===============
////
//void MiscServerModel::MiscServerModelExplode(void) {
//    // Execute radius damage.
//    SVG_InflictRadiusDamage(this, GetActivator(), GetDamage(), NULL, GetDamage() + 40, MeansOfDeath::Barrel);
//
//    // Retrieve origin.
//    vec3_t save = GetOrigin();
//
//    // Set the new origin.
//    SetOrigin(vec3_fmaf(GetAbsoluteMin(), 0.5f, GetSize()));
//
//    // Throw several "debris1/tris.md2" chunks.
//    SpawnDebris1Chunk();
//    SpawnDebris1Chunk();
//
//    // Bottom corners
//    vec3_t origin = GetAbsoluteMin();
//    SpawnDebris3Chunk(origin);
//    origin = GetAbsoluteMin();
//    origin.x += GetSize().x;
//    SpawnDebris3Chunk(origin);
//    origin = GetAbsoluteMin();
//    origin.y += GetSize().y;
//    SpawnDebris3Chunk(origin);
//    origin = GetAbsoluteMin();
//    origin.x += GetSize().x;
//    origin.y += GetSize().y;
//    SpawnDebris3Chunk(origin);
//
//    // Spawn 8 "debris2/tris.md2" chunks.
//    SpawnDebris2Chunk();
//    SpawnDebris2Chunk();
//    SpawnDebris2Chunk();
//    SpawnDebris2Chunk();
//    SpawnDebris2Chunk();
//    SpawnDebris2Chunk();
//    SpawnDebris2Chunk();
//    SpawnDebris2Chunk();
//
//    // Reset origin to saved origin.
//    SetOrigin(save);
//
//    // Depending on whether we have a ground entity or not, we determine which explosion to use.
//    if (GetGroundEntity())
//        SVG_BecomeExplosion2(this);
//    else
//        SVG_BecomeExplosion1(this);
//
//    // Ensure we have no more think callback pointer set when this entity has "died"
//    SetThinkCallback(nullptr);
//}
//
////
////===============
//// MiscServerModel::MiscServerModelDie
////
//// 'Die' callback, the explosion box has been damaged too much.
////===============
////
void MiscServerModel::MiscServerModelDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    // Entity is dying, it can't take any more damage.
    SetTakeDamage(TakeDamage::No);

    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

    // Play a nasty gib sound, yughh :)
    SVG_Sound(this, CHAN_BODY, gi.SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);

    // Throw some gibs around, true horror oh boy.
    SVG_ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
    SVG_ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
    SVG_ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
    SVG_ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
    //SVG_ThrowClientHead(this, damage);
    
    // Setup the next think and think time.
    SetNextThinkTime(level.time + 1 * FRAMETIME);

    // Set think function.
    SetThinkCallback(&MiscServerModel::SVGBaseEntityThinkFree);
}
//
////
////===============
//// MiscServerModel::MiscServerModelTouch
////
//// 'Touch' callback, to calculate the direction to move into.
////===============
////
//void MiscServerModel::MiscServerModelTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
//    // Safety checks.
//    if (!self)
//        return;
//    if (!other)
//        return;
//    // TODO: Move elsewhere in baseentity, I guess?
//    // Prevent this entity from touching itself.
//    if (this == other)
//        return;
//
//    // Ground entity checks.
//    if ((!other->GetGroundEntity()) || (other->GetGroundEntity() == this))
//        return;
//
//    // Calculate ratio to use.
//    float ratio = (float)other->GetMass() / (float)GetMass();
//
//    // Calculate direction.
//    vec3_t dir = GetOrigin() - other->GetOrigin();
//
//    // Calculate yaw to use based on direction.
//    float yaw = vec3_to_yaw(dir);
//
//    // Last but not least, move a step ahead.
//    SVG_StepMove_Walk(this, yaw, 40 * ratio * FRAMETIME);
//    //gi.DPrintf("self: '%i' is TOUCHING other: '%i'\n", self->GetServerEntity()->state.number, other->GetServerEntity()->state.number);
//}