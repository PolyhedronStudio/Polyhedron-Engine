/*
// LICENSE HERE.

//
// MiscClientModel.cpp
//
//
*/
#include "../../ClientGameLocals.h"          // SVGame.

// Server Game Base Entity.
#include "../Base/CLGBaseLocalEntity.h"
//#include "../Base/CLGBaseTrigger.h"

// World.
#include "../../World/ClientGameWorld.h"

// Misc Server Model Entity.
#include "MiscClientModel.h"



//
// Constructor/Deconstructor.
//
MiscClientModel::MiscClientModel(PODEntity *svEntity)
    : Base(svEntity) {

}



//
// Interface functions. 
//
//
//===============
// MiscClientModel::Precache
//
//===============
//
void MiscClientModel::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache the passed image/model.
    // WID: TODO: We can probably do this check in SetModel, store a bool for it or so.
    // that'd save us from having to parse it again in the Spawn function.
    if (model.find_last_of(".sp2") != std::string::npos) {
        clgi.R_RegisterModel(model.c_str());
    } else {
        clgi.R_RegisterPic(model.c_str());
    }

    // Should we precache sound? aka noise?
    if (!noisePath.empty()) {
        precachedNoiseIndex = clgi.S_RegisterSound(noisePath.c_str());
    }
}

//
//===============
// MiscClientModel::Spawn
//
//===============
//
void MiscClientModel::Spawn() {
    // Always call parent class method.
    Base::Spawn();

    // Set solid.
    SetSolid(Solid::OctagonBox);

    // Set move type.
    SetMoveType(MoveType::None);

    // Since this is a "monster", after all...
    //SetServerFlags(EntityServerFlags::Monster);
    
    // Set clip mask.
    SetClipMask(BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid);

    // Set the barrel model, and model index.
    if ( !model.empty() ) {
		SetModel( model );
	}

    // Set noise ( in case one is precached. )
    if (precachedNoiseIndex) {
        SetNoiseIndexA(precachedNoiseIndex);
        SetSound(GetNoiseIndexA());
    }

    // Determine whether the model is a sprite. In case it is, we must set the Translucent flag for it to render properly.
    if (model.find_last_of(".sp2") != std::string::npos) {
        SetRenderEffects(RenderEffects::Translucent);
    }

    // Set the bounding box.
    SetBoundingBox(GetMins(), GetMaxs());

    // Set default values in case we have none.
    if ( !GetMass() ) {
        SetMass(40);
    }
    if ( !GetHealth() ) {
        SetHealth(200);
    }
    if ( !GetDamage() ) {
        SetDamage(150);
    }

    // Setup the start frame to animate from.
    //if (startFrame) {
    //    SetAnimationFrame(startFrame);
    //} else {
    //    SetAnimationFrame(0);
    //}
	
	//SetStyle(5);

    // Set entity to allow taking damage.
    SetTakeDamage( TakeDamage::No );

    //// Setup our MiscClientModel callbacks.
    SetThinkCallback( &MiscClientModel::MiscServerModelThink );
    SetDieCallback( &MiscClientModel::MiscServerModelDie );

    // Setup the next think time.
    SetNextThinkTime( level.time + 2.f * FRAMETIME );

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MiscClientModel::Respawn
//
//===============
//
void MiscClientModel::Respawn() {
    Base::Respawn();
}

//
//===============
// MiscClientModel::PostSpawn
//
//===============
//
void MiscClientModel::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//===============
// MiscClientModel::Think
//
//===============
void MiscClientModel::Think() {
    // Always call parent class method.
    Base::Think();


    //if (GetNoiseIndexA()) {
    //    SVG_Sound(this, SoundChannel::IgnorePHS + SoundChannel::Voice, GetSound(), 1.f, Attenuation::None, 0.f);
    //}

    //gi.DPrintf("MiscClientModel::Think();");
}

//===============
// MiscClientModel::SpawnKey
//
//===============
void MiscClientModel::SpawnKey(const std::string& key, const std::string& value) {
    if (key == "model") {
        ParseKeyValue( key, value, model );
    } else if (key == "boundingboxmins") {
        ParseKeyValue( key, value, boundingBoxMins );
        SetMins( boundingBoxMins );
    } else if (key == "boundingboxmaxs") {
        ParseKeyValue( key, value, boundingBoxMaxs );
        SetMaxs( boundingBoxMaxs );
    } else if (key == "endframe") {
        ParseKeyValue( key, value, endFrame );
    } else if (key == "startframe") {
        ParseKeyValue( key, value, startFrame );
    } else if (key == "mass") {
        uint32_t parsedMass = 0;
        ParseKeyValue( key, value, parsedMass );
        SetMass( parsedMass );
    } else if (key == "health") {
        uint32_t parsedHealth = 0;
        ParseKeyValue( key, value, parsedHealth );
        SetMaxHealth( parsedHealth );
        SetHealth( parsedHealth );
    } else if (key == "effects") {
        uint32_t parsedEffects = 0;
        ParseKeyValue( key, value, parsedEffects );
        SetEffects( parsedEffects);
    } else if (key == "rendereffects") {
        uint32_t parsedRenderEffects = 0;
        ParseKeyValue( key, value, parsedRenderEffects );
        SetRenderEffects( parsedRenderEffects );
	} else if ( key == "style" ) {
		uint32_t parsedStyle = 0;
		
		ParseKeyValue( key, value, parsedStyle );
		SetStyle( parsedStyle );
	} else if (key == "customLightStyle") {
		std::string parsedCustomLightStyle = "";
		
		ParseKeyValue( key, value, parsedCustomLightStyle );
		SetCustomLightStyle( parsedCustomLightStyle );
    //}
	} else if (key == "noise") {
        std::string parsedNoisePath = "";

        ParseKeyValue( key, value, parsedNoisePath );
        noisePath = parsedNoisePath;
    } else {
        Base::SpawnKey( key, value );
    }
}

//===============
// MiscClientModel::MiscServerModelThink
//
// Think callback, to execute the needed physics for this pusher object.
//===============
void MiscClientModel::MiscServerModelThink(void) {
    // First, ensure our origin is +1 off the floor.
    //vec3_t newOrigin = GetOrigin() + vec3_t{
    //    0.f, 0.f, 1.f
    //};

    //SetOrigin(newOrigin);
    
    //// Calculate the end origin to use for tracing.
    //vec3_t end = newOrigin + vec3_t{
    //    0, 0, -256.f
    //};
    //
    //
    //// Exceute the trace.
    //SVGTraceResult trace = SVG_Trace(newOrigin, GetMins(), GetMaxs(), end, this, BrushContentsMask::MonsterSolid);
    //
    //// Return in case we hit anything.
    //if (trace.fraction == 1 || trace.allSolid)
    //    return;
    //
    //// Set new entity origin.
    //SetOrigin(trace.endPosition);
    float currentFrame = GetAnimationFrame();
    float nextFrame = GetAnimationFrame() + 1.f;

    if (nextFrame > endFrame) {

        //if (GetHealth() > 0) {
	        nextFrame = startFrame;
	    //}
    }

    SetAnimationFrame(nextFrame);

    ////
    //// Calculate direction.
    ////
    //if (GetHealth() > 0) {
    //    vec3_t currentMoveAngles = GetAngles();
    //
    //    // Direction vector between player and other entity.
    //    vec3_t wishMoveAngles = GetGameWorld()->GetGameEntities()[1]->GetOrigin() - GetOrigin();

    //    //  
    //    vec3_t newModelAngles = vec3_euler(wishMoveAngles);
    //    newModelAngles.x = 0;

    //    SetAngles(newModelAngles);

    //    // Calculate yaw to use based on direction.
    //    float yaw = vec3_to_yaw(wishMoveAngles);

    //    // Last but not least, move a step ahead.
    //    SVG_StepMove_Walk(this, yaw, 90 * FRAMETIME);
    //}

    // Link entity back in.
    LinkEntity();
    
    // Check for ground.
    //SVG_StepMove_CheckGround(this);

    // Setup its next think time, for a frame ahead.
    SetNextThinkTime(level.time + 1.f * FRAMETIME);
}

//===============
// MiscClientModel::MiscServerModelDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MiscClientModel::MiscServerModelDie(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point) {
    //// Entity is dying, it can't take any more damage.
    //SetTakeDamage(TakeDamage::No);

    //// Attacker becomes this entity its "activator".
    //SetActivator(attacker);
    //
    //// Set movetype to dead, solid dead.
    //SetMoveType(MoveType::None);
    //SetSolid(Solid::Not);
    //LinkEntity();
    //// Play a nasty gib sound, yughh :)
    //SVG_Sound(this, SoundChannel::Body, gi.PrecacheSound("misc/gibdeath1.wav"), 1, Attenuation::Normal, 0);

    //// Throw some gibs around, true horror oh boy.
    //ServerGameWorld* gameworld = GetGameWorld();
    //gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
    //gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
    //gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);
    //gameworld->ThrowGib(this, "models/objects/gibs/sm_meat/tris.md2", damage, GibType::Organic);

    ////SVG_ThrowClientHead(this, damage);
    //SetEndFrame(119.f);
    //SetStartFrame(4.f);
    //// Setup the next think and think time.
    SetNextThinkTime(level.time + 1 * FRAMETIME);

    // Set think function.
    SetThinkCallback(&MiscClientModel::CLGBaseLocalEntityThinkFree);
}