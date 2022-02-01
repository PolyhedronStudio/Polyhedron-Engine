/*
// LICENSE HERE.

// FuncPlat.cpp
*/

#include "../../ServerGameLocal.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"
#include "../../BrushFunctions.h"

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
    Base::Spawn();

    // Zero out angles.
    SetAngles(vec3_zero());

    // Basic properties.
    SetMoveDirection(GetAngles(), true);
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
    SetSpeed(20);
    SetAcceleration(5);
    SetDeceleration(5);

    LinkEntity();

    // Set start and end positions.
    SetStartPosition(GetOrigin());
    SetEndPosition(GetOrigin());

    // Adjust endposition according to height.
    if (GetHeight()) {
        SetEndPosition(GetEndPosition() - vec3_t{0.f, 0.f, GetHeight()});
    } else {
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

    // Setup move info.
    moveInfo.speed = GetSpeed();
    moveInfo.acceleration = GetAcceleration();
    moveInfo.deceleration = GetDeceleration();
  //  moveInfo.wait = GetWaitTime();
    moveInfo.distance = height + 120;
    moveInfo.startOrigin = GetStartPosition();
    moveInfo.startAngles = GetAngles();
    moveInfo.endOrigin = GetEndPosition();
    moveInfo.endAngles = GetAngles();

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

    CalculateMoveSpeed();
    // We set origin here.
    if (!GetTargetName().empty()) {
        moveInfo.state = MoverState::Up;
    } else {
        SetOrigin(GetEndPosition());
        moveInfo.state = MoverState::Bottom;
    }

    LinkEntity();

}

//===============
// FuncPlat::PlatformUse
//===============
void FuncPlat::PlatformUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
    gi.DPrintf("PlatformUse\n");
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
    gi.DPrintf("PlatformBlocked\n");
    SVGBaseEntity* ent;

    if ( !(other->GetServerFlags() & EntityServerFlags::Monster) && !(other->GetClient()) ) {
        // Give it a chance to go away on its own terms (like gibs)
        SVG_InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), 10000, 1, 0, MeansOfDeath::Crush );
        // If it's still there, nuke it
        if ( other->GetHealth() > 0 || other->GetSolid() != Solid::Not ) {
            SVG_BecomeExplosion1( other );
        }
    }

    SVG_InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );

    //if ( GetSpawnFlags() & SF_Crusher ) {
    //    return;
    //}

    if (moveInfo.wait >= 0) {
        if (moveInfo.state == MoverState::Down) {
            for (ent = GetTeamMasterEntity(); nullptr != ent; ent = GetTeamChainEntity()) {
                if (ent->IsSubclassOf<FuncPlat>()) {
                    static_cast<FuncPlat*>(ent)->PlatformGoUp(static_cast<FuncPlat*>(ent)->GetActivator());
                }
            }
        } else if (moveInfo.state == MoverState::Up) {
            for (ent = GetTeamMasterEntity(); nullptr != ent; ent = GetTeamChainEntity()) {
                if (ent->IsSubclassOf<FuncPlat>()) {
                    static_cast<FuncPlat*>(ent)->PlatformGoDown();
                }
            }
        }
    }
}

//===============
// FuncPlat::PlatformTouch
//===============
void FuncPlat::PlatformTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf ) {
    gi.DPrintf("PlatformTouch\n");
    // Clients only.
    if (other->GetClient() == nullptr) {
        return; // Players only; should we have special flags for monsters et al?
    }

    //// Ensure we wait till debounce time is over.
    if ( level.time < debounceTouchTime ) {
        return;
    }

    debounceTouchTime = level.time + 5.0f;

    if ( !messageStr.empty() ) {
        gi.CenterPrintf( other->GetServerEntity(), "%s", messageStr.c_str() );
        gi.Sound( other->GetServerEntity(), CHAN_AUTO, gi.SoundIndex( MessageSoundPath ), 1.0f, ATTN_NORM, 0.0f );
    }

    Use(GetOwner(), other);
}

//===============
// FuncPlat::PlatformGoUp
//===============
void FuncPlat::PlatformGoUp( SVGBaseEntity* activator ) {
    gi.DPrintf("PlatformGoUp\n");
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.startSoundIndex ) {
            gi.Sound( GetServerEntity(), CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.startSoundIndex, 1, ATTN_STATIC, 0.0f );
        }
        SetSound( moveInfo.middleSoundIndex );
    }

    DoGoUp();
}

//===============
// FuncPlat::PlatformGoDown
//===============
void FuncPlat::PlatformGoDown() {
    gi.DPrintf("PlatformGoDown\n");
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.startSoundIndex ) {
            gi.Sound( GetServerEntity(), CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.startSoundIndex, 1, ATTN_STATIC, 0 );
        }
        SetSound( moveInfo.middleSoundIndex );
    }
    
    DoGoDown();
}

//===============
// FuncPlat::DoGoUp
//===============
void FuncPlat::DoGoUp() {
    moveInfo.state = MoverState::Up;
    BrushMoveCalc( moveInfo.startOrigin, OnPlatformHitTop );
}

//===============
// FuncPlat::DoGoDown
//===============
void FuncPlat::DoGoDown() {
    moveInfo.state = MoverState::Down;
    BrushMoveCalc( moveInfo.endOrigin, OnPlatformHitBottom );
}

//===============
// FuncPlat::HitTop
//===============
void FuncPlat::HitTop() {
    gi.DPrintf("HitTop\n");
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.endSoundIndex ) {
            gi.Sound( GetServerEntity(), CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.endSoundIndex, 1.0f, ATTN_STATIC, 0.0f );
        }
        SetSound( 0 );
    }

    moveInfo.state = MoverState::Top;

    SetThinkCallback( &FuncPlat::PlatformGoDown );
    SetNextThinkTime( level.time + 3 );
}

//===============
// FuncPlat::HitBottom
//===============
void FuncPlat::HitBottom() {
    gi.DPrintf("HitBottom\n");
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.endSoundIndex ) {
            gi.Sound( GetServerEntity(), CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.endSoundIndex, 1.0f, ATTN_STATIC, 0.0f );
        }
        SetSound( 0 );
    }
    SetThinkCallback( &SVGBaseEntity::SVGBaseEntityThinkNull );
    moveInfo.state = MoverState::Bottom;
}

//===============
// FuncPlat::OnPlatformHitTop
//===============
void FuncPlat::OnPlatformHitTop( Entity* self ) {
    gi.DPrintf("OnPlatformHitTop\n");
    if ( self->classEntity->IsSubclassOf<FuncPlat>() ) {
        static_cast<FuncPlat*>( self->classEntity )->HitTop();
    }
}

//===============
// FuncPlat::OnPlatformHitBottom
//===============
void FuncPlat::OnPlatformHitBottom( Entity* self ) {
    gi.DPrintf("OnPlatformHitBottom\n");
    if ( self->classEntity->IsSubclassOf<FuncPlat>() ) {
        static_cast<FuncPlat*>(self->classEntity)->HitBottom();
    }
}

//===============
// FuncPlat::CalculateMoveSpeed
//===============
void FuncPlat::CalculateMoveSpeed() {
    //if ( GetFlags() & EntityFlags::TeamSlave ) {
    //    return; // Only the team master does this
    //}

    //FuncPlat* ent = nullptr;
    //float min = 0.f;
    //float time = 0.f;
    //float newSpeed = 0.f;
    //float ratio = 0.f;
    //float distance = 0.f;

    //// Find the smallest distance any member of the team will be moving
    //min = fabsf( moveInfo.distance );
    //for ( ent = dynamic_cast<FuncPlat*>( GetTeamChainEntity() ); ent; ent = dynamic_cast<FuncPlat*>( ent->GetTeamChainEntity() ) ) {
    //    distance = fabsf( ent->moveInfo.distance );
    //    if ( distance < min ) {
    //        min = distance;
    //    }
    //}
    //distance *= 2;
    //time = min / GetSpeed();

    //// Adjust speeds so they will all complete at the same time
    //for ( ent = this; ent; ent = dynamic_cast<FuncPlat*>(ent->GetTeamChainEntity()) ) {
    //    newSpeed = fabsf( ent->moveInfo.distance ) / time;
    //    ratio = newSpeed / ent->moveInfo.speed;

    //    if ( ent->moveInfo.acceleration == ent->moveInfo.speed ) {
    //        ent->moveInfo.acceleration = newSpeed;
    //    } else {
    //        ent->moveInfo.acceleration *= ratio;
    //    }

    //    if ( ent->moveInfo.deceleration == ent->moveInfo.speed ) {
    //        ent->moveInfo.deceleration = ent->moveInfo.speed;
    //    } else {
    //        ent->moveInfo.deceleration *= ratio;
    //    }

    //    // Update moveInfo variables and class member variables
    //    ent->SetAcceleration( ent->moveInfo.acceleration );
    //    ent->SetDeceleration( ent->moveInfo.deceleration );
    //    ent->moveInfo.speed = newSpeed;
    //    ent->SetSpeed( newSpeed );
    //}
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
    // Create the trigger.
    vec3_t mins = GetMins();
    vec3_t maxs = GetMaxs();

    // Trigger hull mins/maxs.
    static const vec3_t hullMinsExpand = { 25.f, 25.f, 0.0f };
    static const vec3_t hullMaxsExpand = { -25.f, -25.f, 8.0f };

    FuncPlat* teamMember = nullptr;
    for ( teamMember = dynamic_cast<FuncPlat*>(GetTeamChainEntity()); teamMember; teamMember = dynamic_cast<FuncPlat*>(teamMember->GetTeamChainEntity()) ) {
        AddPointToBounds( teamMember->GetAbsoluteMin(), mins, maxs );
        AddPointToBounds( teamMember->GetAbsoluteMax(), mins, maxs );
    }
    // Start calculation of the new trigger mins/maxs.
    vec3_t triggerMins = mins - hullMinsExpand;
    vec3_t triggerMaxs = maxs + hullMaxsExpand;

    const vec3_t &startPosition = GetStartPosition();
    const vec3_t &endPosition = GetEndPosition();
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

    // At last, create platform trigger entity.
    SVGBaseEntity *trigger = TriggerAutoPlatform::Create( this, triggerMins, triggerMaxs );

    // Calculate move speeds.
    //CalculateMoveSpeed();
}

////===============
//// FuncPlat::UseAreaportals
////===============
//void FuncPlat::UseAreaportals( bool open ) const {
//    if ( targetStr.empty()) {
//        return;
//    }
//
//    SVGBaseEntity* ent = nullptr;
//    while ( ent = SVG_FindEntityByKeyValue( "targetname", targetStr, ent ) ) {
//        if ( ent->IsClass<FuncAreaportal>() ) {
//            static_cast<FuncAreaportal*>(ent)->ActivatePortal( open );
//        }
//    }
//}

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