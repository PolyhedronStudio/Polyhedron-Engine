/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
//
//
*/
#include "../../ClientGameLocals.h"          // CLGame.
//#include "../../Effects.h"          // Effects.
//#include "../../Utilities.h"            // Util funcs.
//#include "../../Physics/StepMove.h" // Stepmove funcs.

// Server Game Base Entity.
#include "../Base/CLGBasePacketEntity.h"
#include "../Base/CLGBaseTrigger.h"
#include "../Base/CLGBaseLocalEntity.h"
//#include "../Base/CLGBaseTrigger.h"

// World.
#include "../../World/ClientGameWorld.h"

// Misc Explosion Box Entity.
#include "MiscExplosionBox.h"

//#include "../../Gamemodes/IGamemode.h"
//#include "../../World/GameWorld.h"

//
// Constructor/Deconstructor.
//
MiscExplosionBox::MiscExplosionBox(PODEntity* clEntity) 
    : Base(clEntity) {

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
    clgi.R_RegisterModel("models/env/barrels/barrel_red.iqm");

    // Precache the debris.
    clgi.R_RegisterModel("models/objects/debris1/tris.md2");
    clgi.R_RegisterModel("models/objects/debris2/tris.md2");
    clgi.R_RegisterModel("models/objects/debris3/tris.md2");
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

	// Set InUse.
	SetInUse(true);

    // Set solid.
    SetSolid(Solid::OctagonBox);

    // Set move type.
    SetMoveType(MoveType::TossSlide);

    // Set clip mask.
    SetClipMask(BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid);

    // Set the barrel model, and model index.
    SetModel("models/env/barrels/barrel_red.iqm");

    // Set the bounding box.
    SetBoundingBox(
        // Mins.
        { -16.f, -16.f, 0.f },
        // Maxs.
        { 16.f, 16.f, 58.f }
    );
	//if (podEntity && podEntity->clientEntityNumber == 2052) {
	//	SetInUse(true);
	//}
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
    SetTakeDamage(ClientTakeDamage::Yes);

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
// MiscExplosionBox::Think
//
//===============
//
void MiscExplosionBox::Think() {
    // Always call parent class method.
    Base::Think();


//	Com_DPrint("YO DAWG FROM THA MISC_CLIENT_EXPLOBOX YO %i\n", clientEntity->clientEntityNumber);
//	podEntity->currentState.origin = vec3_mix(podEntity->previousState.origin, podEntity->currentState.origin, cl->lerpFraction);
        // Interpolate start and end points for beams
        //refreshEntity.origin = vec3_mix(previousState.origin, currentState.origin, lerpFraction);
        //refreshEntity.oldorigin = vec3_mix(previousState.oldOrigin, currentState.oldOrigin, lerpFraction);
	//SetRenderEffects(RenderEffects::Beam | RenderEffects::DebugBoundingBox);
	//clientEntity->lerpOrigin = vec3_mix(clientEntity->previousState.origin, clientEntity->currentState.origin, cl->lerpFraction);

	// Interpolate origin?
	//ClientGameWorld *gameWorld = GetGameWorld();
	//PODEntity *clientEntity = gameWorld->GetPODEntityByIndex(GetNumber());
	////
	//if (podEntity) {

	//	podEntity->currentState.origin = vec3_mix(podEntity->previousState.origin, podEntity->currentState.origin, cl->lerpFraction);
	//	podEntity->currentState.oldOrigin = vec3_mix(podEntity->previousState.oldOrigin, podEntity->currentState.oldOrigin, cl->lerpFraction);
	//	podEntity->currentState.renderEffects = RenderEffects::Beam;
	////	SetRenderEffects(RenderEffects::Beam);
	////	//Com_DPrint("MiscExploBox #%i: lerpOrigin = %f %f %f, prevOrigin=%f %f %f curOrigin=%f %f %f\n", clientEntity->clientEntityNumber, clientEntity->lerpOrigin.x, clientEntity->lerpOrigin.y, clientEntity->lerpOrigin.z, clientEntity->previousState.origin.x, clientEntity->previousState.origin.y, clientEntity->previousState.origin.z, clientEntity->currentState.origin.x, clientEntity->currentState.origin.y, clientEntity->currentState.origin.z);
	//}
}

void MiscExplosionBox::MiscExplosionBoxInterpolateThink() {
    SetNextThinkTime(level.time + 1.f * FRAMETIME);
    SetThinkCallback(&MiscExplosionBox::MiscExplosionBoxInterpolateThink);
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
void MiscExplosionBox::ExplosionBoxUse( IClientGameEntity* caller, IClientGameEntity* activator )
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
    CLGTraceResult trace = CLG_Trace(traceStart, GetMins(), GetMaxs(), traceEnd, this, BrushContentsMask::MonsterSolid);
    
    // Return in case we hit anything.
    if (trace.fraction == 1.f || trace.allSolid) {
	    return;
    }
    
    // Set new entity origin.
    SetOrigin(trace.endPosition);
    
    // Link entity back in.
    LinkEntity();

    // Do a check ground for the step move of this pusher.
    SG_CheckGround(this);//CLG_StepMove_CheckGround(this);

    // Setup its next think time, for a frame ahead.
    //SetThinkCallback(&MiscExplosionBox::ExplosionBoxDropToFloor);
    //SetNextThinkTime(level.time + 1.f * FRAMETIME);

    // Do a check ground for the step move of this pusher.
    //CLG_StepMove_CheckGround(this);
    //M_CatagorizePosition(ent); <-- This shit, has to be moved to CLG_Stepmove_CheckGround.
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
//    GetGameMode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), NULL, GetDamage() + 40, MeansOfDeath::Barrel);

    // Retrieve origin.
    vec3_t save = GetOrigin();

    // Set the new origin.
    vec3_t debrisSpawnOrigin = vec3_fmaf(GetAbsoluteMin(), 0.5f, GetSize());

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
    // Just a random number event id to test for now.
	SetEventID(43);

	//if (GetGroundEntityHandle()) {
    //    gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
    //    gi.MSG_WriteUint8(TempEntityEvent::Explosion1);//WriteByte(TempEntityEvent::Explosion1);
    //    gi.MSG_WriteVector3(GetOrigin(), false);//WriteVector3(GetOrigin());
    //    gi.Multicast(GetOrigin(), Multicast::PHS);

    //    GetGameMode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

    //    const Frametime save = GetDelayTime();
    //    SetDelayTime(0s);
    //    UseTargets(GetActivator());
    //    SetDelayTime(save);
    //} else {
    //    gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
    //    gi.MSG_WriteUint8(TempEntityEvent::Explosion2);//WriteByte(TempEntityEvent::Explosion2);
    //    gi.MSG_WriteVector3(GetOrigin(), false);//WriteVector3(GetOrigin());
    //    gi.Multicast(GetOrigin(), Multicast::PHS);

    //    GetGameMode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

    //    const Frametime save = GetDelayTime();
    //    SetDelayTime(0s);
    //    UseTargets(GetActivator());
    //    SetDelayTime(save);
    //}
	//podEntity->inUse = true;
    // Ensure we have no more think callback pointer set when this entity has "died"
    SetNextThinkTime(level.time + 1.f * FRAMETIME);
    //SetThinkCallback(&MiscExplosionBox::CLGBaseLocalEntityThinkFree);
	SetThinkCallback(&MiscExplosionBox::CLGBasePacketEntityThinkFree);
}

//
//===============
// MiscExplosionBox::ExplosionBoxDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MiscExplosionBox::ExplosionBoxDie(IClientGameEntity* inflictor, IClientGameEntity* attacker, int damage, const vec3_t& point) {
    // Entity is dying, it can't take any more damage.
    SetTakeDamage(ClientTakeDamage::No);

    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

	// Get the general length of velocity, if it's non 0 we want to give this box time to fly. Teeehee :-)
	const vec3_t normalizedVelocity = vec3_normalize(GetVelocity());

	if (normalizedVelocity.z > 0.1) {
		// Setup the next think and think time.
		uint32_t nextThinkOffset = RandomRangeui(15, 35);
		SetNextThinkTime(level.time + (float)nextThinkOffset * FRAMETIME);
	} else {
		// Setup the next think and think time.
		SetNextThinkTime(level.time + 2 * FRAMETIME);
	}

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
void MiscExplosionBox::ExplosionBoxTouch(IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
   
	// Safety checks.
    if (!other || other == this) {
		Com_DPrint("Explobox: %i is touching a other == this or !other\n", GetNumber());
		return;
    }

    // Ground entity checks.
	GameEntity *groundEntity = ClientGameWorld::ValidateEntity( other->GetGroundEntityHandle() );
    if (!other->GetGroundEntityHandle() || other->GetGroundEntityHandle() == this) {
		return;
    }
	
	//Com_DPrint("ExplosionBox: %i is touching: other.GetNumber() = %i\n", GetNumber(), other->GetNumber());
	if (other->GetNumber() == cl->frame.clientNumber + 1) {
	//	Com_DPrint("PlayerEnt(#%i) is touching: (#%i)\n", other->GetNumber(), GetNumber());
	}

    // Calculate ratio to use.
    double ratio = (static_cast<double>(other->GetMass()) / static_cast<double>(GetMass()));

    // Calculate direction.
    vec3_t dir = GetOrigin() - other->GetOrigin();

    // Calculate yaw to use based on direction.
    double yaw = vec3_to_yaw(dir);

	//Com_DPrint("Origin before pushing #(%i): %f %f %f\n", GetNumber(), GetOrigin().x, GetOrigin().y, GetOrigin().z);
    // Last but not least, move a step ahead.
//    CLG_StepMove_Walk(this, yaw, (30.0 / static_cast<double>(BASE_FRAMEDIVIDER) * ratio * FRAMETIME.count()));
	//Com_DPrint("Origin after pushing #(%i): %f %f %f\n", GetNumber(), GetOrigin().x, GetOrigin().y, GetOrigin().z);
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
//    GameWorld* gameworld = GetGameWorld();

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
//    gameworld->ThrowDebris(this, "models/objects/debris1/tris.md2", origin, speed);
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
 //   GetGameWorld()->ThrowDebris(this, "models/objects/debris2/tris.md2", origin, speed);
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
//    GetGameWorld()->ThrowDebris(this, "models/objects/debris3/tris.md2", origin, speed);
}
