/*
// LICENSE HERE.

// FuncDoor.cpp
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

#include "../trigger/TriggerAutoDoor.h"

#include "FuncAreaportal.h"
#include "FuncDoor.h"

//===============
// FuncDoor::ctor
//===============
FuncDoor::FuncDoor( Entity* entity ) 
	: Base( entity ) {

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
    Base::Spawn();

    SetMoveDirection( GetAngles(), true);
    SetMoveType( MoveType::Push );
    SetSolid( Solid::BSP );
    SetModel( GetModel() );

    SetThinkCallback( &SVGBaseEntity::SVGBaseEntityThinkNull );
    SetBlockedCallback( &FuncDoor::DoorBlocked );
    SetUseCallback( &FuncDoor::DoorUse );

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

    // This should never happen
    if ( GetStartPosition() == GetEndPosition() ) {
        gi.DPrintf( "WARNING: func_door has same start & end position\n" );
        return;
    }

    // If it starts open, swap the positions
    if ( GetSpawnFlags() & FuncDoor::SF_StartOpen ) {
        SwapPositions();
    }

    moveInfo.state = MoverState::Bottom;
    
    // If the mapper specified health, then make this door
    // openable by shooting it.
    if ( GetHealth() ) {
        SetTakeDamage( TakeDamage::Yes );
        SetDieCallback( &FuncDoor::DoorShotOpen );
        SetMaxHealth( GetHealth() );
    } else if ( GetTargetName().empty() ) {
    // If the mapper did NOT specify a targetname, then make this 
    // door openable by touching it.
        gi.SoundIndex( MessageSoundPath );
        SetTouchCallback( &FuncDoor::DoorTouch );
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
    if ( GetTeam().empty() ) {
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
    if ( GetHealth() || !GetTargetName().empty()) {
        CalculateMoveSpeed();
    } else {
        SpawnDoorTrigger();
    }
}

//===============
// FuncDoor::DoorUse
//===============
void FuncDoor::DoorUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
    if ( GetFlags() & EntityFlags::TeamSlave ) {
        return;
    }

    // This should never happen
    if ( GetStartPosition() == GetEndPosition() ) {
        gi.DPrintf( "WARNING: func_door has same start & end position\n" );
        return;
    }

    if ( GetSpawnFlags() & SF_Toggle ) {
        if ( moveInfo.state == MoverState::Up || moveInfo.state == MoverState::Top ) {
            // Trigger all paired doors
            for ( SVGBaseEntity* ent = this; nullptr != ent; ent = ent->GetTeamChainEntity() ) {
                if ( ent->IsSubclassOf<FuncDoor>() ) {
                    ent->SetMessage( "" );

                    // WID: TODO: Add a flag for whether to unset touch callbacks after being used for the first time.
                    //ent->SetTouchCallback( nullptr );
                    static_cast<FuncDoor*>( ent )->DoorGoDown();
                }
            }

            return;
        }
    }

    // Trigger all paired doors
    for ( SVGBaseEntity* ent = this; nullptr != ent; ent = ent->GetTeamChainEntity() ) {
        if ( ent->IsSubclassOf<FuncDoor>() ) {
            ent->SetMessage( "" );
            // WID: TODO: Add a flag for whether to unset touch callbacks after being used for the first time.
            //ent->SetTouchCallback( nullptr );
            static_cast<FuncDoor*>( ent )->DoorGoUp( activator );
        }
    }
}

//===============
// FuncDoor::DoorShotOpen
//===============
void FuncDoor::DoorShotOpen( SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point ) {
    SVGBaseEntity* ent;
    for ( ent = GetTeamMasterEntity(); nullptr != ent; ent = GetTeamChainEntity() ) {
        ent->SetHealth( GetMaxHealth() );
        ent->SetTakeDamage( TakeDamage::No );
    }

    GetTeamMasterEntity()->Use( attacker, attacker );
}

//===============
// FuncDoor::DoorBlocked
//===============
void FuncDoor::DoorBlocked( SVGBaseEntity* other ) {
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

    if ( GetSpawnFlags() & SF_Crusher ) {
        return;
    }

    // If a door has a negative wait, it would never come back if blocked,
    // so let it just squash the object to death real fast
    if ( moveInfo.wait >= 0 ) {
        if ( moveInfo.state == MoverState::Down ) {
            for ( ent = GetTeamMasterEntity(); nullptr != ent; ent = GetTeamChainEntity() ) {
                if ( ent->IsSubclassOf<FuncDoor>() ) {
                    static_cast<FuncDoor*>( ent )->DoorGoUp( static_cast<FuncDoor*>( ent )->GetActivator() );
                }
            }
        } else {
            for ( ent = GetTeamMasterEntity(); nullptr != ent; ent = GetTeamChainEntity() ) {
                if ( ent->IsSubclassOf<FuncDoor>() ) {
                    static_cast<FuncDoor*>( ent )->DoorGoDown();
                }
            }
        }
    }
}

//===============
// FuncDoor::DoorTouch
//===============
void FuncDoor::DoorTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf ) {
    // Clients only.
    if (other->GetClient() == nullptr) {
        return; // Players only; should we have special flags for monsters et al?
    }

    // Ensure we wait till debounce time is over.
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
// FuncDoor::DoorGoUp
//===============
void FuncDoor::DoorGoUp( SVGBaseEntity* activator ) {
    if ( moveInfo.state == MoverState::Up ) {
        return; // already going up
    }

    if ( moveInfo.state == MoverState::Top ) {
        // Reset top wait time 
        if ( moveInfo.wait >= 0.0f ) {
            SetNextThinkTime( level.time + moveInfo.wait );
        }
        return;
    }

    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.startSoundIndex ) {
            gi.Sound( GetServerEntity(), CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.startSoundIndex, 1, ATTN_STATIC, 0.0f );
        }
        SetSound( moveInfo.middleSoundIndex );
    }

    moveInfo.state = MoverState::Up;

    DoGoUp();
    UseTargets( activator );
    UseAreaportals( true );
}

//===============
// FuncDoor::DoorGoDown
//===============
void FuncDoor::DoorGoDown() {
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.startSoundIndex ) {
            gi.Sound( GetServerEntity(), CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.startSoundIndex, 1, ATTN_STATIC, 0 );
        }
        SetSound( moveInfo.middleSoundIndex );
    }

    if ( GetMaxHealth() ) {
        SetTakeDamage( TakeDamage::Yes );
        SetHealth( GetMaxHealth() );
    }

    moveInfo.state = MoverState::Down;
    DoGoDown();
}

//===============
// FuncDoor::DoGoUp
//===============
void FuncDoor::DoGoUp() {
    BrushMoveCalc( moveInfo.endOrigin, OnDoorHitTop );
}

//===============
// FuncDoor::DoGoDown
//===============
void FuncDoor::DoGoDown() {
    BrushMoveCalc( moveInfo.startOrigin, OnDoorHitBottom );
}

//===============
// FuncDoor::HitTop
//===============
void FuncDoor::HitTop() {
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.endSoundIndex ) {
            gi.Sound( GetServerEntity(), CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.endSoundIndex, 1.0f, ATTN_STATIC, 0.0f );
        }
        SetSound( 0 );
    }

    moveInfo.state = MoverState::Top;
    if ( GetSpawnFlags() & SF_Toggle ) {
        return;
    }

    if ( moveInfo.wait >= 0.0f ) {
        SetThinkCallback( &FuncDoor::DoorGoDown );
        SetNextThinkTime( level.time + moveInfo.wait );
    }
}

//===============
// FuncDoor::HitBottom
//===============
void FuncDoor::HitBottom() {
    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.endSoundIndex ) {
            gi.Sound( GetServerEntity(), CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.endSoundIndex, 1.0f, ATTN_STATIC, 0.0f );
        }
        SetSound( 0 );
    }

    moveInfo.state = MoverState::Bottom;
    UseAreaportals( false ); // close off area portals
}

//===============
// FuncDoor::OnDoorHitTop
//===============
void FuncDoor::OnDoorHitTop( Entity* self ) {
    if ( self->classEntity->IsSubclassOf<FuncDoor>() ) {
        static_cast<FuncDoor*>( self->classEntity )->HitTop();
    }
}

//===============
// FuncDoor::OnDoorHitBottom
//===============
void FuncDoor::OnDoorHitBottom( Entity* self ) {
    if ( self->classEntity->IsSubclassOf<FuncDoor>() ) {
        static_cast<FuncDoor*>(self->classEntity)->HitBottom();
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
    
    // Find the smallest distance any member of the team will be moving
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
    if ( targetStr.empty()) {
        return;
    }

    SVGBaseEntity* ent = nullptr;
    while ( ent = SVG_FindEntityByKeyValue( "targetname", targetStr, ent ) ) {
        if ( ent->IsClass<FuncAreaportal>() ) {
            static_cast<FuncAreaportal*>(ent)->ActivatePortal( open );
        }
    }
}

//===============
// Light::SpawnKey
//===============
void FuncDoor::SpawnKey(const std::string& key, const std::string& value) {
    if (key == "lip") {
        // Parsed int.
        int32_t parsedInteger = 0;

        // Parse.
        ParseIntegerKeyValue(key, value, parsedInteger);

        // Assign.
        SetLip(parsedInteger);
    }
    // Speed value.
    else if (key == "speed") {
        // Parsed int.
        int32_t parsedInteger = 0;

        // Parse.
        ParseIntegerKeyValue(key, value, parsedInteger);

        // Assign.
        SetSpeed(parsedInteger);
    }
    // Team value.
    else if (key == "team") {
        // Parsed int.
        std::string parsedString = "";

        // Parse.
        ParseStringKeyValue(key, value, parsedString);

        // Assign.
        //SetTeam(parsedString);
    }
    // Parent class spawnkey.
    else {
        SVGBaseEntity::SpawnKey(key, value);
    }
}