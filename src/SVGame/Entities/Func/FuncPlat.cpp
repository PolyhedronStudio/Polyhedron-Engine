/*
// LICENSE HERE.

// FuncPlat.cpp
*/

#include "../../ServerGameLocal.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"

#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseMover.h"

#include "../trigger/TriggerAutoPlatform.h"

#include "FuncPlat.h"

//===============
// FuncPlat::ctor
//===============
FuncPlat::FuncPlat( Entity* entity ) 
    : Base( entity ) {

}

//===============
// FuncPlat::Precache
//===============
void FuncPlat::Precache() {
    Base::Precache();

    // Set up the default sounds
    if ( GetSound() != 1 ) {
        moveInfo.startSoundIndex = SVG_PrecacheSound("plats/pt1_strt.wav");
        moveInfo.middleSoundIndex = SVG_PrecacheSound("plats/pt1_mid.wav");
        moveInfo.endSoundIndex = SVG_PrecacheSound("plats/pt1_end.wav");
    }
}

//===============
// FuncPlat::Spawn
//===============
void FuncPlat::Spawn() {
    // Zero out angles here for SetMoveDirection in Base::Spawn.
    SetAngles(vec3_zero());

    // Spawn base class.
    Base::Spawn();

    // Basic properties.
    //SetMoveDirection(GetAngles(), true);
    SetMoveType(MoveType::Push);
    SetSolid(Solid::BSP);
    SetModel(GetModel());

    SetThinkCallback( &SVGBaseEntity::SVGBaseEntityThinkNull );
    SetBlockedCallback(&FuncPlat::PlatformBlocked);
    SetUseCallback(&FuncPlat::PlatformUse);

    // Key/Value setup.
    if (!GetSpeed()) {
        SetSpeed(20.f);
    } else {
        SetSpeed(GetSpeed() * 0.1f);
    }
    if (!GetAcceleration()) {
        SetAcceleration(5.f);
    } else {
        SetAcceleration(GetAcceleration() * 0.5f);
    }
    if (!GetDeceleration()) {
        SetDeceleration(5.f);
    } else {
        SetDeceleration(GetDeceleration() * 0.5f);
    }
    if (!GetDamage()) {
        SetDamage(2);
    }
    if (!GetLip()) {
        SetLip(8);
    }

    // Set start and end positions to origin.
    SetStartPosition(GetOrigin());
    SetEndPosition(GetOrigin());

    if (GetHeight()) {
	    // Adjust endposition according to keyvalue set height.
        SetEndPosition(GetEndPosition() - vec3_t{0.f, 0.f, GetHeight()});
    } else {
        // Adjust endposition based on own calculated height.
        SetEndPosition(GetEndPosition() - vec3_t{0.f, 0.f, (GetMaxs().z - GetMins().z) - GetLip()});
        height = GetEndPosition().z;
    }

    //const float height = GetHeight();
    //const vec3_t boundingBoxMins = GetMins();
    //const vec3_t boundingBoxMaxs = GetMaxs();
    //gi.DPrintf("startPosition = %s\n", vec3_to_str(startPosition).c_str());
    //gi.DPrintf("endPosition = %s\n", vec3_to_str(endPosition).c_str());
    ////startPosition.z += height + GetLip();
    //if (height) {
    //    endPosition.z -= height;
    //} else {
    //    endPosition.z -= (boundingBoxMaxs.z - boundingBoxMins.z) - GetLip();
    //}

    //moveInfo.distance = -(height + GetLip());
    //vec3_t endp = CalculateEndPosition();
    //endPosition.z = endp.z;
    //SetEndPosition(endPosition);

    //gi.DPrintf("height = %f\n", height);
    //gi.DPrintf("startPosition = %s\n", vec3_to_str(startPosition).c_str());
    //gi.DPrintf("endPosition = %s\n", vec3_to_str(endPosition).c_str());

    //// This should never happen
    //if (GetStartPosition() == GetEndPosition()) {
    //    gi.DPrintf("WARNING: func_plat has same start & end position\n");
    //    return;
    //}

    // If it has a targetname, it has to be triggered so set its state to be up.
    //if (!GetTargetName().empty()) {
    //    moveInfo.state = MoverState::Up;
    //    SetMoveDirection(vec3_t{0.f, 1.f, 0.f});
    //    moveInfo.distance = (height + GetLip());
    //    endPosition = CalculateEndPosition();
    //} else {
    //    SetMoveDirection(vec3_t{0.f, -1.f, 0.f});
    //    moveInfo.distance = (height + GetLip());
    //    endPosition = CalculateEndPosition();
    //    SetEndPosition(endPosition);
    //    SetOrigin(endPosition);
    //    moveInfo.state = MoverState::Bottom;
    //}

    //// To simplify logic elsewhere, make non-teamed doors into a team of one
    if ( GetTeam().empty() ) {
        SetTeamMasterEntity( this );
    }

    LinkEntity();
}

//===============
// FuncPlat::PostSpawn
//===============
void FuncPlat::PostSpawn() {
    // We spawn this in post spawn so all other calculations have been prepared.
    SpawnPlatformTrigger();

    // Calculate movement speed to use.
    CalculateMoveSpeed();

    // The way how plats work is that they need to be triggered by an other
    // trigger of sorts in case they got a platform name. This means that
    // by default the trigger starts in its up state.
    //
    // Otherwise position it in its endposition and set state to bottom.
    if (!GetTargetName().empty()) {
        moveInfo.state = MoverState::Up;
    } else {
        SetOrigin(GetEndPosition());
        moveInfo.state = MoverState::Bottom;
    }
    
    // Setup move info.
    moveInfo.speed = GetSpeed();
    moveInfo.acceleration = GetAcceleration();
    moveInfo.deceleration = GetDeceleration();
    moveInfo.wait = GetWaitTime();
    moveInfo.distance = height + 120;
    moveInfo.startOrigin = GetStartPosition();
    moveInfo.startAngles = GetAngles();
    moveInfo.endOrigin = GetEndPosition();
    moveInfo.endAngles = GetAngles();

    // Link it.
    LinkEntity();
}

//===============
// FuncPlat::PlatformUse
//===============
void FuncPlat::PlatformUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
    if (HasThinkCallback()) {
        gi.DPrintf("FuncPlat already has a think callback! - returning!!\n");
        return;
    }
    PlatformGoDown();
}

//===============
// FuncPlat::PlatformBlocked
//===============
void FuncPlat::PlatformBlocked( SVGBaseEntity* other ) {
    if (!other) {
        return;
    }

    if ( !(other->GetServerFlags() & EntityServerFlags::Monster) && !(other->GetClient()) ) {
        // Give it a chance to go away on its own terms (like gibs)
        SVG_InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), 10000, 1, 0, MeansOfDeath::Crush );
        // If it's still there, nuke it
        if ( other->GetHealth() > 0 || other->GetSolid() != Solid::Not ) {
            SVG_BecomeExplosion1( other );
        }
    }

    SVG_InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );

    if (moveInfo.state == MoverState::Down) {
	    PlatformGoUp( );
    } else {
	    PlatformGoDown();
    }
}

//===============
// FuncPlat::PlatformGoUp
//===============
void FuncPlat::PlatformGoUp(  ) {
    if (!(GetFlags() & EntityFlags::TeamSlave)) {
	    if (moveInfo.startSoundIndex) {
	        SVG_Sound(this, CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.startSoundIndex, 1, ATTN_STATIC, 0.0f);
	    }
	    SetSound(moveInfo.startSoundIndex);
    }

    DoGoUp();
}

//===============
// FuncPlat::PlatformGoDown
//===============
void FuncPlat::PlatformGoDown() {
    if (!(GetFlags() & EntityFlags::TeamSlave)) {
	    if (moveInfo.startSoundIndex) {
	        SVG_Sound(this, CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.startSoundIndex, 1, ATTN_STATIC, 0.0f);
	    }
	    SetSound(moveInfo.startSoundIndex);
    }
    
    DoGoDown();
}

//===============
// FuncPlat::DoGoUp
//===============
void FuncPlat::DoGoUp() {
    if (!(GetFlags() & EntityFlags::TeamSlave)) {
	    if (moveInfo.middleSoundIndex) {
	        SVG_Sound(this, CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.middleSoundIndex, 1, ATTN_STATIC, 0.0f);
	    }
	    SetSound(moveInfo.middleSoundIndex);
    }
    moveInfo.state = MoverState::Up;
    BrushMoveCalc( moveInfo.startOrigin, OnPlatformHitTop );
}

//===============
// FuncPlat::DoGoDown
//===============
void FuncPlat::DoGoDown() {
    if (!(GetFlags() & EntityFlags::TeamSlave)) {
	    if (moveInfo.middleSoundIndex) {
	        SVG_Sound(this, CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.middleSoundIndex, 1, ATTN_STATIC, 0.0f);
	    }
	    SetSound(moveInfo.middleSoundIndex);
    }

    moveInfo.state = MoverState::Down;
    BrushMoveCalc( moveInfo.endOrigin, OnPlatformHitBottom );
}

//===============
// FuncPlat::HitTop
//===============
void FuncPlat::HitTop() {
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.endSoundIndex ) {
            SVG_Sound( this, CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.endSoundIndex, 1.0f, ATTN_STATIC, 0.0f );
        }
        SetSound( 0 );
    }

    moveInfo.state = MoverState::Top;

    SetThinkCallback( &FuncPlat::PlatformGoDown );
    SetNextThinkTime( level.time + 3);
}

//===============
// FuncPlat::HitBottom
//===============
void FuncPlat::HitBottom() {
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.endSoundIndex ) {
            SVG_Sound( this, CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.endSoundIndex, 1.0f, ATTN_STATIC, 0.0f );
        }
        SetSound( 0 );
    }
    SetThinkCallback( &SVGBaseEntity::SVGBaseEntityThinkNull );
    moveInfo.state = MoverState::Bottom;
}

//===============
// FuncPlat::OnPlatformHitTop
//===============
void FuncPlat::OnPlatformHitTop( SVGBaseEntity* self ) {
    if (!self->IsSubclassOf<FuncPlat>()) {
	    gi.DPrintf("Warning: In function %s entity #%i is not a subclass of func_plat\n", __func__, self->GetNumber());
        return;
    }
    
    // Cast.
    FuncPlat* platEntity = static_cast<FuncPlat*>(self);
	platEntity->HitTop();
}

//===============
// FuncPlat::OnPlatformHitBottom
//===============
void FuncPlat::OnPlatformHitBottom( SVGBaseEntity* self ) {
    if (!self->IsSubclassOf<FuncPlat>()) {
	    gi.DPrintf("Warning: In function %s entity #%i is not a subclass of func_plat\n", __func__, self->GetNumber());
        return;
    }
    
    // Cast.
    FuncPlat* platEntity = static_cast<FuncPlat*>(self);
	platEntity->HitBottom();
}

//===============
// FuncPlat::CalculateMoveSpeed
//===============
void FuncPlat::CalculateMoveSpeed() {
    //if ( GetFlags() & EntityFlags::TeamSlave ) {
    //    return; // Only the team master does this
    //}

    //FuncPlat* ent = nullptr;
    float min = 0.f;
    float time = 0.f;
    float newSpeed = 0.f;
    float ratio = 0.f;
    float distance = 0.f;
    FuncPlat *ent = nullptr;

    // Find the smallest distance any member of the team will be moving
    min = fabsf( moveInfo.distance );
    for (ent = dynamic_cast<FuncPlat*>(GetTeamChainEntity()); (ent != nullptr && ent->IsSubclassOf<SVGBaseMover>()); ent = dynamic_cast<FuncPlat*>(ent->GetTeamChainEntity())) {
        distance = fabsf( ent->moveInfo.distance );
        if ( distance < min ) {
            min = distance;
        }
    }

    time = min / GetSpeed();

    // Adjust speeds so they will all complete at the same time
    for (ent = dynamic_cast<FuncPlat*>(GetTeamChainEntity()); (ent != nullptr && ent->IsSubclassOf<SVGBaseMover>()); ent = dynamic_cast<FuncPlat*>(ent->GetTeamChainEntity())) {
        newSpeed = fabsf( ent->moveInfo.distance ) / time;
        ratio = newSpeed / ent->moveInfo.speed;

        if ( ent->moveInfo.acceleration == ent->moveInfo.speed ) {
            ent->moveInfo.acceleration = newSpeed;
        } else {
            ent->moveInfo.acceleration *= ratio;
        }

        if ( ent->moveInfo.deceleration == ent->moveInfo.speed ) {
            ent->moveInfo.deceleration = newSpeed;
        } else {
            ent->moveInfo.deceleration *= ratio;
        }

        // Update moveInfo variables and class member variables
        ent->SetAcceleration( ent->moveInfo.acceleration );
        ent->SetDeceleration( ent->moveInfo.deceleration );
        ent->moveInfo.speed = newSpeed;
        ent->SetSpeed( newSpeed );
    }
}

//===============
// FuncPlat::SpawnPlatformTrigger
// 
// Platforms have an invisible bounding box on them, which
// is slightly smaller than the platform's bounding box.
// 
// This bad boy spawns it, so keep that in mind if you're running out of edict slots.
//===============
void FuncPlat::SpawnPlatformTrigger() {
    // Get mins and max.
    vec3_t mins = GetMins();
    vec3_t maxs = GetMaxs();

    // Start calculation of the new trigger mins/maxs.
    vec3_t triggerMins = mins + vec3_t{ 25.0f, 25.0f, 0.f };
    vec3_t triggerMaxs = maxs + vec3_t{ -25.f, -25.f, 8.f };

    const vec3_t startPosition = GetStartPosition();
    const vec3_t endPosition = GetEndPosition();
    const float& lip = GetLip();

    // Calculate a slightly larger box.
    triggerMins.x = mins.x + 25.f;
    triggerMins.y = mins.y + 25.f;
    triggerMins.z = mins.z;

    triggerMaxs.x = maxs.x - 25.f;
    triggerMaxs.y = maxs.y - 25.f;
    triggerMaxs.z = maxs.z + 8;

    // Adjust height.
    triggerMins.z = triggerMaxs.z - (startPosition.z - endPosition.z + lip);

    // For PlatLowTriggered state we need a different maxs.
    if (GetSpawnFlags() & SF_PlatLowTriggered) {
        triggerMaxs.z = triggerMins.z + 8;
    }

    // Scale it.
    if (triggerMaxs.x - triggerMins.x <= 0.f) {
        triggerMins.x = (mins.x + maxs.x) * 0.5f;
        triggerMaxs.x = triggerMins.x + 1;
    }
    if (triggerMaxs.y - triggerMins.y <= 0.f) {
        triggerMins.y = (mins.y + maxs.y) * 0.5f;
        triggerMaxs.y = triggerMins.y + 1;
    }

    FuncPlat* teamMember = nullptr;
    vec3_t    teamMins = GetMins();
    vec3_t    teamMaxs = GetMaxs();
    
    // Add points to the generated bounding box for the trigger.
    for (SVGBaseEntity* teamMember = GetTeamChainEntity(); teamMember != nullptr; teamMember = teamMember->GetTeamChainEntity()) {
	    // Check it is a derivate of base mover, if not, break out of this loop.
	    if (!teamMember->IsSubclassOf<SVGBaseMover>()) {
	        gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), teamMember->GetNumber());
	        break;
	    }

        AddPointToBounds(teamMember->GetAbsoluteMin(), triggerMins, triggerMaxs);
	    AddPointToBounds(teamMember->GetAbsoluteMax(), triggerMins, triggerMaxs);
    }
    
    // At last, create platform trigger entity.
    SVGBaseEntity *trigger = TriggerAutoPlatform::Create( this, triggerMins, triggerMaxs );
}

//===============
// Light::SpawnKey
//===============
void FuncPlat::SpawnKey(const std::string& key, const std::string& value) {
    // Height value.
    if (key == "height") {
        // Parsed int.
        float parsedFloat = 0.f;

        // Parse.
        ParseFloatKeyValue(key, value, parsedFloat);

        // Assign.
        SetHeight(parsedFloat);
    }
    // Parent class spawnkey.
    else {
        Base::SpawnKey(key, value);
    }
}