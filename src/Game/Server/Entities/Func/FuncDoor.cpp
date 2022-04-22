/*
// LICENSE HERE.

// FuncDoor.cpp
*/

#include "../../ServerGameLocals.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"

#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseMover.h"

#include "../Trigger/TriggerAutoDoor.h"

#include "FuncAreaportal.h"
#include "FuncDoor.h"

#include "../../GameModes/IGameMode.h"
#include "../../World/ServerGameWorld.h"

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
    //if ( GetGameMode()->IsClass( GameModeDeathmatch::ClassInfo ) ) {
    //    SetSpeed( GetSpeed() * 2.0f );
    //}
    if ( !GetAcceleration() ) {
        SetAcceleration( GetSpeed() );
    }
    if ( !GetDeceleration() ) {
        SetDeceleration( GetSpeed() );
    }
    if ( GetWaitTime() == Frametime::zero() ) {
        SetWaitTime( 3s );
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
        SetEffects(GetEffects() | EntityEffectType::AnimCycleAll2hz);
    }
    if ( GetSpawnFlags() & SF_YAxis ) {
        SetEffects(GetEffects() | EntityEffectType::AnimCycleAll30hz);
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
void FuncDoor::DoorUse( IServerGameEntity* other, IServerGameEntity* activator ) {
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
            // Trigger all paired movers.
	        for (IServerGameEntity* ent = dynamic_cast<IServerGameEntity*>(this); ent != nullptr; ent = ent->GetTeamChainEntity()) {
		        // Check it is a derivate of base mover, if not, break out of this loop.
		        if (!ent->IsSubclassOf<SVGBaseMover>()) {
		            gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), ent->GetNumber());
		            break;
		        }

                // Ensure it is a subclass of a door otherwise we can't tell it to go down.
                if ( ent->IsSubclassOf<FuncDoor>() ) {
                    // Unset message so it triggers its display only once.
                    ent->SetMessage( "" );

                    // WID: TODO: Add a flag for whether to unset touch callbacks after being used for the first time.
                    //ent->SetTouchCallback( nullptr );
                    static_cast<FuncDoor*>( ent )->DoorGoDown();
                }
            }

            // Escape out of this function.
            return;
        }
    }

    // Trigger all paired movers.
    for (IServerGameEntity* ent = this; ent != nullptr; ent = ent->GetTeamChainEntity()) {
        // Check it is a derivate of base mover, if not, break out of this loop.
        if (!ent->IsSubclassOf<SVGBaseMover>()) {
	        gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), ent->GetNumber());
            break;
        }

        if ( ent->IsSubclassOf<FuncDoor>() ) {
            ent->SetMessage( "" );
            // WID: TODO: Add a flag for whether to unset touch callbacks after being used for the first time.
            //ent->SetTouchCallback( nullptr );
	        static_cast<FuncDoor*>(ent)->DoorGoUp(dynamic_cast<SVGBaseEntity*>(activator));
        }
    }
}

//===============
// FuncDoor::DoorShotOpen
//===============
void FuncDoor::DoorShotOpen( IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point ) {
    for (IServerGameEntity* ent = this; ent != nullptr; ent = ent->GetTeamChainEntity()) {
	    // Check it is a derivate of base mover, if not, break out of this loop.
	    if (!ent->IsSubclassOf<SVGBaseMover>()) {
	        gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), ent->GetNumber());
	        break;
	    }

        ent->SetHealth( GetMaxHealth() );
        ent->SetTakeDamage( TakeDamage::No );
    }

    GetTeamMasterEntity()->DispatchUseCallback( attacker, attacker );
}

//===============
// FuncDoor::DoorBlocked
//===============
void FuncDoor::DoorBlocked( IServerGameEntity* other ) {

    if ( !(other->GetServerFlags() & EntityServerFlags::Monster) && !(other->GetClient()) ) {
        // Give it a chance to go away on its own terms (like gibs)
        GetGameMode()->InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), 10000, 1, 0, MeansOfDeath::Crush );

        // If it's still there, nuke it
        if ( other->GetHealth() > 0 || other->GetSolid() != Solid::Not ) {
            SVG_BecomeExplosion1( other );
        }
    }

    GetGameMode()->InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );

    if ( GetSpawnFlags() & SF_Crusher ) {
        return;
    }

    // If a door has a negative wait, it would never come back if blocked,
    // so let it just squash the object to death real fast
    if ( moveInfo.wait != Frametime::zero() ) {
        if ( moveInfo.state == MoverState::Down ) {
	        for (IServerGameEntity* ent = this; ent != nullptr; ent = ent->GetTeamChainEntity()) {
		        // Check it is a derivate of base mover, if not, break out of this loop.
		        if (!ent->IsSubclassOf<SVGBaseMover>()) {
		            gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), ent->GetNumber());
		            break;
		        }

                // Execute go up.
                if ( ent->IsSubclassOf<FuncDoor>() ) {
                    static_cast<FuncDoor*>( ent )->DoorGoUp( static_cast<FuncDoor*>( ent )->GetActivator() );
                }
            }
        } else {
	        for (IServerGameEntity* ent = this; ent != nullptr; ent = ent->GetTeamChainEntity()) {
		        // Check it is a derivate of base mover, if not, break out of this loop.
		        if (!ent->IsSubclassOf<SVGBaseMover>()) {
		            gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), ent->GetNumber());
		            break;
		        }

                // Execute go down.
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
void FuncDoor::DoorTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
    // Clients only.
    if (other->GetClient() == nullptr) {
        return; // Players only; should we have special flags for monsters et al?
    }

    // Ensure we wait till debounce time is over.
    if ( level.time < debounceTouchTime ) {
        return;
    }

    debounceTouchTime = level.time + 5s;

    if ( !messageStr.empty() ) {
        gi.CenterPrintf( other->GetPODEntity(), "%s", messageStr.c_str() );
        gi.Sound( other->GetPODEntity(), SoundChannel::Auto, gi.SoundIndex( MessageSoundPath ), 1.0f, Attenuation::Normal, 0.0f );
    }

    DispatchUseCallback(GetOwner(), other);
}

//===============
// FuncDoor::DoorGoUp
//===============
void FuncDoor::DoorGoUp( IServerGameEntity* activator ) {
    if ( moveInfo.state == MoverState::Up ) {
        return; // already going up
    }

    if ( moveInfo.state == MoverState::Top ) {
        // Reset top wait time 
        if ( moveInfo.wait != Frametime::zero() ) {
            SetNextThinkTime( level.time + moveInfo.wait );
        }
        return;
    }

    if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
        if ( moveInfo.startSoundIndex ) {
            gi.Sound( GetPODEntity(), SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.startSoundIndex, 1, Attenuation::Static, 0.0f );
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
            gi.Sound( GetPODEntity(), SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.startSoundIndex, 1, Attenuation::Static, 0 );
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
            gi.Sound( GetPODEntity(), SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.endSoundIndex, 1.0f, Attenuation::Static, 0.0f );
        }
        SetSound( 0 );
    }

    moveInfo.state = MoverState::Top;
    if ( GetSpawnFlags() & SF_Toggle ) {
        return;
    }

    if ( moveInfo.wait != Frametime::zero() ) {
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
            gi.Sound( GetPODEntity(), SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.endSoundIndex, 1.0f, Attenuation::Static, 0.0f );
        }
        SetSound( 0 );
    }

    moveInfo.state = MoverState::Bottom;
    UseAreaportals( false ); // close off area portals
}

//===============
// FuncDoor::OnDoorHitTop
//===============
void FuncDoor::OnDoorHitTop( IServerGameEntity* self ) {
    if ( self->IsSubclassOf<FuncDoor>() ) {
        static_cast<FuncDoor*>( self )->HitTop();
    }
}

//===============
// FuncDoor::OnDoorHitBottom
//===============
void FuncDoor::OnDoorHitBottom( IServerGameEntity* self ) {
    if ( self->IsSubclassOf<FuncDoor>() ) {
        static_cast<FuncDoor*>(self)->HitBottom();
    }
}

//===============
// FuncDoor::CalculateMoveSpeed
//===============
void FuncDoor::CalculateMoveSpeed() {
    if ( GetFlags() & EntityFlags::TeamSlave ) {
        return; // Only the team master does this
    }

    float min = 0.f;
    float time = 0.f;
    float newSpeed = 0.f;
    float ratio = 0.f;
    float distance = 0.f;
    
    // Find the smallest distance any member of the team will be moving
    min = fabsf( moveInfo.distance );
    for (IServerGameEntity* ent = this; ent != nullptr; ent = ent->GetTeamChainEntity()) {
	    // Check it is a derivate of base mover, if not, break out of this loop.
	    if (!ent->IsSubclassOf<SVGBaseMover>()) {
	        gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), ent->GetNumber());
	        break;
	    }

        // Cast to base mover and fetch moveinfo.
        SVGBaseMover* moverEntity = static_cast<SVGBaseMover*>(ent);
	    PushMoveInfo* moveInfo = moverEntity->GetPushMoveInfo();

        // Calculate distance.
        distance = fabsf( moveInfo->distance );
        if ( distance < min ) {
            min = distance;
        }
    }

    // Calculate time.
    time = min / GetSpeed();

    // Adjust speeds so they will all complete at the same time
    for (IServerGameEntity* ent = this; ent != nullptr; ent = ent->GetTeamChainEntity()) {
	    // Check it is a derivate of base mover, if not, break out of this loop.
	    if (!ent->IsSubclassOf<SVGBaseMover>()) {
	        gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), ent->GetNumber());
	        break;
	    }

	    // Cast to base mover and fetch moveinfo.
	    SVGBaseMover* moverEntity = static_cast<SVGBaseMover*>(ent);
	    PushMoveInfo* moveInfo = moverEntity->GetPushMoveInfo();

        newSpeed = fabsf( moveInfo->distance ) / time;
        ratio = newSpeed / moveInfo->speed;

        if ( moveInfo->acceleration == moveInfo->speed ) {
            moveInfo->acceleration = newSpeed;
        } else {
            moveInfo->acceleration *= ratio;
        }

        if ( moveInfo->deceleration == moveInfo->speed ) {
            moveInfo->deceleration = moveInfo->speed;
        } else {
            moveInfo->deceleration *= ratio;
        }

        // Update moveInfo variables and class member variables
        moverEntity->SetAcceleration( moveInfo->acceleration );
        moverEntity->SetDeceleration( moveInfo->deceleration );
        moveInfo->speed = newSpeed;
        moverEntity->SetSpeed( newSpeed );
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
    SVGBaseMover*  teamMember = nullptr;
    SVGBaseEntity* trigger = nullptr;
    vec3_t mins = GetMins();
    vec3_t maxs = GetMaxs();

    if ( GetFlags() & EntityFlags::TeamSlave ) {
        return; // Only the team leader spawns a trigger
    }
    
    for (IServerGameEntity* teamMember = dynamic_cast<SVGBaseEntity*>(GetTeamChainEntity()); teamMember != nullptr; teamMember = teamMember->GetTeamChainEntity()) {
	    // Check it is a derivate of base mover, if not, break out of this loop.
	    if (!teamMember->IsSubclassOf<SVGBaseMover>()) {
	        gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), teamMember->GetNumber());
	        break;
	    }

        AddPointToBounds( teamMember->GetAbsoluteMin(), mins, maxs );
        AddPointToBounds( teamMember->GetAbsoluteMax(), mins, maxs );
    }

    // Expand the trigger box on the horizontal plane by 60 units
    static const vec3_t HullExpand = { 60.0f, 60.0f, 0.0f };
    mins -= HullExpand;
    maxs += HullExpand;

    // Spawn the auto door trigger
    trigger = TriggerAutoDoor::Create( this, mins, maxs );
    
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
    for (auto& areaPortalEntity : GetGameWorld()->GetGameEntityRange<0, MAX_EDICTS>() | 
        cef::IsValidPointer | cef::HasServerEntity | cef::InUse | cef::IsSubclassOf<FuncAreaportal>() | cef::HasKeyValue("targetname", targetStr)) {
	    dynamic_cast<FuncAreaportal*>(areaPortalEntity)->ActivatePortal(open);
    }
    
        //while ( ent = SVG_FindEntityByKeyValue( "targetname", targetStr, ent ) ) {
    //    if ( ent->IsClass<FuncAreaportal>() ) {
    //        static_cast<FuncAreaportal*>(ent)->ActivatePortal( open );
    //    }
    //}
}

//===============
// Light::SpawnKey
//===============
void FuncDoor::SpawnKey(const std::string& key, const std::string& value) {
    if (key == "lip") {
        // Parsed int.
        int32_t parsedInteger = 0;

        // Parse.
        ParseKeyValue(key, value, parsedInteger);

        // Assign.
        SetLip(parsedInteger);
    }
    // Speed value.
    else if (key == "speed") {
        // Parsed int.
        int32_t parsedInteger = 0;

        // Parse.
        ParseKeyValue(key, value, parsedInteger);

        // Assign.
        SetSpeed(parsedInteger);
    }
    // Team value.
    else if (key == "team") {
        // Parsed int.
        std::string parsedString = "";

        // Parse.
        ParseKeyValue(key, value, parsedString);

        // Assign.
        //SetTeam(parsedString);
    }
    // Parent class spawnkey.
    else {
        Base::SpawnKey(key, value);
    }
}