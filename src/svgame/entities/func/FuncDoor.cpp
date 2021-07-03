/*
// LICENSE HERE.

// FuncDoor.cpp
*/

#include "../../g_local.h"
#include "../../effects.h"
#include "../../entities.h"
#include "../../utils.h"
#include "../../physics/stepmove.h"
#include "../../brushfuncs.h"

#include "../base/SVGBaseEntity.h"
#include "../base/SVGBaseTrigger.h"
#include "../base/SVGBaseMover.h"

#include "../trigger/TriggerAutoDoor.h"

#include "FuncDoor.h"

//===============
// FuncDoor::ctor
//===============
FuncDoor::FuncDoor( Entity* entity ) 
	: SVGBaseMover( entity ) {

}

//===============
// FuncDoor::Precache
//===============
void FuncDoor::Precache() {
    Base::Precache();

    // Set up the default sounds
    if ( GetSound() != 1 ) {
        moveInfo.startSoundIndex = gi.SoundIndex( "doors/dr1_strt.wav" );
        moveInfo.middleSoundIndex = gi.SoundIndex( "doors/dr1_mid.wav" );
        moveInfo.endSoundIndex = gi.SoundIndex( "doors/dr1_end.wav" );
    }
}

//===============
// FuncDoor::Spawn
//===============
void FuncDoor::Spawn() {
    vec3_t absoluteMovedir;

    Base::Spawn();

    SetMoveDirection( GetAngles()/*, resetAngles = true*/ );
    SetMoveType( MoveType::Push );
    SetSolid( Solid::BSP );
    SetModel( GetModel() );

    SetBlockedCallback( nullptr /*DoorBlocked*/ );
    SetUseCallback( nullptr /*DoorUse*/ );

    if ( !GetSpeed() ) {
        SetSpeed( 100.0f );
    }
    //if ( game.gameMode->IsClass( GameModeDeathmatch::ClassInfo ) ) {
    //    SetSpeed( GetSpeed() * 2.0f );
    //}
    if ( !GetAcceleration() ) {
        SetAcceleration( GetSpeed() );
    }
    if ( !GetDeceleration() ) {
        SetDeceleration( GetSpeed() );
    }
    if ( !GetWaitTime() ) {
        SetWaitTime( 3.0f );
    }
    if ( !GetLip() ) {
        SetLip( 8.0f );
    }
    if ( !GetDamage() ) {
        SetDamage( 2 );
    }

    // Calculate the end position, with the assumption that start pos = origin
    SetStartPosition( GetOrigin() );
    SetEndPosition( CalculateEndPosition() );

    // If it starts open, swap the positions
    if ( GetSpawnFlags() & FuncDoor::SF_StartOpen ) {
        SwapPositions();
    }

    moveInfo.state = MoverState::Bottom;

    // If the mapper specified health, then make this door
    // openable by shooting it
    if ( GetHealth() ) {
        SetTakeDamage( TakeDamage::Yes );
        SetDieCallback( nullptr /*DoorKilled*/ );
        SetMaxHealth( GetHealth() );
    } else if ( nullptr == serverEntity->targetName ) {
        gi.SoundIndex( "misc/talk.wav" ); // ???
        SetTouchCallback( nullptr /*DoorTouch*/ );
    }

    moveInfo.speed = GetSpeed();
    moveInfo.acceleration = GetAcceleration();
    moveInfo.deceleration = GetDeceleration();
    moveInfo.wait = GetWaitTime();
    moveInfo.startOrigin = GetStartPosition();
    moveInfo.startAngles = GetAngles();
    moveInfo.endOrigin = GetEndPosition();
    moveInfo.endAngles = GetAngles();

    if ( GetSpawnFlags() & SF_Toggle ) {
        serverEntity->state.effects |= EntityEffectType::AnimCycleAll2hz;
    }
    if ( GetSpawnFlags() & SF_YAxis ) {
        serverEntity->state.effects |= EntityEffectType::AnimCycleAll30hz;
    }

    // To simplify logic elsewhere, make non-teamed doors into a team of one
    if ( !GetTeam() ) {
        SetTeamMasterEntity( this );
    }

    LinkEntity();
}

//===============
// FuncDoor::PostSpawn
// 
// func_door in Q2 originally used spawn think functions
// to achieve this. If the entity had health or had a 
// targetname, it'd calculate its move speed, else
// it'd spawn a door trigger around itself
//===============
void FuncDoor::PostSpawn() {
    if ( GetHealth() || !targetNameStr.empty() ) {
        CalculateMoveSpeed();
    } else {
        SpawnDoorTrigger();
    }
}

//===============
// FuncDoor::CalculateMoveSpeed
//===============
void FuncDoor::CalculateMoveSpeed() {
    if ( GetFlags() & EntityFlags::TeamSlave ) {
        return; // Only the team master does this
    }

    FuncDoor* ent = nullptr;
    float min;
    float time;
    float newSpeed;
    float ratio;
    float distance;
    
    // Find the smallest any member of the team will be moving
    min = fabsf( moveInfo.distance );
    for ( ent = dynamic_cast<FuncDoor*>( GetTeamChainEntity() ); ent; ent = dynamic_cast<FuncDoor*>( ent->GetTeamChainEntity() ) ) {
        distance = fabsf( ent->moveInfo.distance );
        if ( distance < min ) {
            min = distance;
        }
    }

    time = min / GetSpeed();

    // Adjust speeds so they will all complete at the same time
    for ( ent = this; ent; ent = dynamic_cast<FuncDoor*>(ent->GetTeamChainEntity()) ) {
        newSpeed = fabsf( ent->moveInfo.distance ) / time;
        ratio = newSpeed / ent->moveInfo.speed;

        if ( ent->moveInfo.acceleration == ent->moveInfo.speed ) {
            ent->moveInfo.acceleration = newSpeed;
        } else {
            ent->moveInfo.acceleration *= ratio;
        }

        if ( ent->moveInfo.deceleration == ent->moveInfo.speed ) {
            ent->moveInfo.deceleration = ent->moveInfo.speed;
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
// FuncDoor::SpawnDoorTrigger
// 
// Nameless, non-shootable doors have an invisible bounding box around them, which
// is slightly larger than the door's bounding box.
// 
// This bad boy spawns it, so keep that in mind if you're running out of edict slots.
//===============
void FuncDoor::SpawnDoorTrigger() {
    FuncDoor* teamMember = nullptr;
    SVGBaseEntity* trigger = nullptr;
    vec3_t mins = GetMins();
    vec3_t maxs = GetMaxs();

    if ( GetFlags() & EntityFlags::TeamSlave ) {
        return; // Only the team leader spawns a trigger
    }
    
    for ( teamMember = dynamic_cast<FuncDoor*>(GetTeamChainEntity()); teamMember; teamMember = dynamic_cast<FuncDoor*>(teamMember->GetTeamChainEntity()) ) {
        AddPointToBounds( teamMember->GetAbsoluteMin(), mins, maxs );
        AddPointToBounds( teamMember->GetAbsoluteMax(), mins, maxs );
    }

    // Expand the trigger box on the horizontal plane by 60 units
    static const vec3_t HullExpand = { 60.0f, 60.0f, 0.0f };
    mins -= HullExpand;
    maxs += HullExpand;

    // Spawn the auto door trigger
    trigger = TriggerAutoDoor::Create( this, maxs, mins );
    
    if ( GetSpawnFlags() & FuncDoor::SF_StartOpen ) {
        UseAreaportals( true );
    }

    CalculateMoveSpeed();
}

//===============
// FuncDoor::UseAreaportals
//===============
void FuncDoor::UseAreaportals( bool open ) const {
    if ( targetStr.empty() ) {
        return;
    }

    SVGBaseEntity* portal = nullptr;
    while ( portal = SVG_FindEntityByKeyValue( "targetname", targetStr, portal ) ) {
        if ( std::string( portal->GetClassName() ) == "func_areaportal" ) {
            gi.SetAreaPortalState( portal->GetStyle(), open );
        }
    }
}
