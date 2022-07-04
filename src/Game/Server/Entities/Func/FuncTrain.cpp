/*
// LICENSE HERE.

// FuncTrain.cpp
*/

#include "../../ServerGameLocals.h"
#include "../../Entities.h"
#include "../../Effects.h"

#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseMover.h"

#include "../Path/PathCorner.h"
#include "FuncTrain.h"

#include "../../GameModes/IGameMode.h"
#include "../../World/ServerGameWorld.h"

//===============
// FuncTrain::ctor
//===============
FuncTrain::FuncTrain( Entity* entity )
 : Base( entity ) {

}

//===============
// FuncTrain::Spawn
//===============
void FuncTrain::Precache() {

}

//===============
// FuncTrain::Spawn
//===============
void FuncTrain::Spawn() {
    // Set angles here.
	SetAngles(vec3_zero());

	// Spawn base.
	Base::Spawn();

	// Re-ensure proper move and solid type.
	SetMoveType( MoveType::Push );
	SetSolid( Solid::BSP );
	SetModel( GetModel() );

	// Use trigger callback.
	SetUseCallback(&FuncTrain::TrainUse);

	// Undo damage value, otherwise it'd kill an object at a stop, looks a bit odd.
	if (GetSpawnFlags() & SF_StopWhenBlocked) {
	    SetDamage(0);
	} else if (!GetDamage()) {
	    SetDamage(100);
	}

	// Ensure we got speed. lol.
	if (!GetSpeed()) {
	    SetSpeed(100.0f);
	}

	// Set initial moveinfo state.
	moveInfo.speed = GetSpeed();
	moveInfo.acceleration = GetSpeed();
	moveInfo.deceleration = GetSpeed();

	// Link it.
	LinkEntity();

	if (!GetTarget().empty()) {
	    SetNextThinkTime(level.time + FRAMERATE_MS);
	    SetThinkCallback(&FuncTrain::FindNextTarget);
	} else {
	    gi.DPrintf("func_train without a target at %s\n", vec3_to_cstr(GetAbsoluteCenter()));
	}
}

//===============
// FuncTrain::PostSpawn
//===============
void FuncTrain::PostSpawn() {
 //   if (GetTargetName().empty()) {
	//	return;
	//}

	//SVGBaseEntity* ent = SVG_FindEntityByKeyValue( "targetname", targetStr );
	//if ( nullptr == ent ) {
	//	gi.DPrintf( "FuncTrain: target '%s' not found, maybe you made a typo?\n", targetStr.c_str() );
	//	return;
	//}

	//if ( !ent->IsSubclassOf<PathCorner>() ) {
	//	gi.DPrintf( "FuncTrain: target '%s' is not a path entity\n", targetStr.c_str() );
	//	return;
	//}

	//SetOrigin(ent->GetOrigin() - GetMins());
	//LinkEntity();

	//// This train has no name, trigger it immediately
	//if ( targetNameStr.empty() ) {
	//	spawnFlags |= SF_StartOn;
	//}

	//if ( spawnFlags & SF_StartOn ) {
	//	SetNextThinkTime( level.time + FRAMETIME );
	//	SetThinkCallback( &FuncTrain::NextCornerThink );
	//	SetActivator(this);
	//}
}

//===============
// FuncTrain::FindNextTarget
// 
//===============
void FuncTrain::FindNextTarget() {
    if (GetTarget().empty()) {
		gi.DPrintf("FuncTrain: no target!\n");
		return;
	}

	// Find target.
	auto targetEntities = GetGameWorld()->GetGameEntityRange<0, MAX_WIRED_POD_ENTITIES>() | cef::IsValidPointer | cef::HasServerEntity | cef::InUse | cef::HasKeyValue("targetname", GetTarget());

	if (targetEntities.front() != nullptr && targetEntities.front()->IsSubclassOf<PathCorner>()) {
	    PathCorner* pathCorner = dynamic_cast<PathCorner*>(targetEntities.front());

		//if ( !ent->IsSubclassOf<PathCorner>() ) {
		//	gi.DPrintf( "FuncTrain: target '%s' is not a path entity\n", targetStr.c_str() );
		//	return;
		//}
		// Set target to path corner.
	    SetTarget( pathCorner->GetTarget() );


		SetOrigin( pathCorner->GetOrigin() - GetMins() );
		LinkEntity();

		// This train has no name, trigger it immediately
		if (GetTargetName().empty()) {
		    SetSpawnFlags(GetSpawnFlags() | SF_StartOn);
		}

		if (GetSpawnFlags() & SF_StartOn) {
			SetNextThinkTime( level.time + FRAMERATE_MS );
			SetThinkCallback( &FuncTrain::NextCornerThink );
			SetActivator(this);
		}
	} else {
	    gi.DPrintf("FuncTrain: target '%s' not found, maybe you made a typo?\n", targetStr.c_str());
	}
}

//===============
// FuncTrain::SpawnKey
// 
// I've put this here in case we add new KVs
//===============
void FuncTrain::SpawnKey( const std::string& key, const std::string& value ) {
	//if ( key == "speed" ) {
	//	ParseKeyValue( key, value, speed );
	//} else {
		return Base::SpawnKey( key, value );
//	}
}

//===============
// FuncTrain::TrainUse
//===============
void FuncTrain::TrainUse( IServerGameEntity* other, IServerGameEntity* activator ) {
	SetActivator(activator);

	if (GetSpawnFlags()  & SF_StartOn) {
		// Toggling system. On/Off
	    if (~GetSpawnFlags() & SF_Toggled) {
			return;
		}

		SetSpawnFlags(GetSpawnFlags() & ~SF_StartOn);
		SetVelocity( vec3_zero() );
		SetNextThinkTime( 0s );
	} else {
		// Resume path in case we werer traveling along one.
		if ( nullptr != currentPathEntity ) {
			ResumePath();
		} else {
			// Go to next path_corner.
		    SetNextThinkTime(level.time + FRAMERATE_MS);
		    SetThinkCallback(&FuncTrain::NextCornerThink);
			NextCornerThink();
		}
	}

}

//===============
// FuncTrain::NextCornerThink
//===============
void FuncTrain::NextCornerThink() {
    vec3_t destination = vec3_zero();
    PathCorner* pathCornerEntity = nullptr;
	bool first = true;
	bool again = true;

	// Train encountered teleporting path_corners, so pick the next one each time
	// Originally, this used a goto
	while ( again )
	{
		again = false;

		if (GetTarget().empty()) {
			return;
		}

		// TODO: Add a find single entity by targetname to gameworld including other utility classes?

		// Find target.
		auto targetEntities = GetGameWorld()->GetGameEntityRange<0, MAX_WIRED_POD_ENTITIES>() | cef::IsValidPointer | cef::HasServerEntity | cef::InUse | cef::HasKeyValue("targetname", GetTarget());

		if (targetEntities.front() != nullptr && targetEntities.front()->IsSubclassOf<PathCorner>()) {
		    pathCornerEntity = dynamic_cast<PathCorner*>(targetEntities.front());
		    // QUESTIONABLE: Are mappers gonna use non-path_corner ents?

			// We'll probably be rewriting this logic anyway someday...
		    if (!pathCornerEntity->IsSubclassOf<PathCorner>()) {
				gi.DPrintf( "FuncTrain::NextCornerThink: Target '%s' isn't a path_corner\n", GetTarget().c_str() );
				return;
			}
		
			SetTarget(pathCornerEntity->GetTarget());

			if (pathCornerEntity->GetSpawnFlags() & PathCorner::SF_Teleport) {
				if ( !first ) {
					gi.DPrintf("Connected teleport path_corners, see '%s' at '%s'\n", pathCornerEntity->GetTypeInfo()->mapClass, vec3_to_cstr(pathCornerEntity->GetOrigin()));
					return;
				}

				first = false;
				again = true; // loop it
				SetOrigin(pathCornerEntity->GetOrigin() - GetMins());
				SetOldOrigin( GetOrigin() );
				SetEventID( EntityEvent::OtherTeleport );
				LinkEntity();
			}
		}
	}

	moveInfo.wait = pathCornerEntity->GetWaitTime();
	currentPathEntity = static_cast<PathCorner*>(pathCornerEntity);

	destination = pathCornerEntity->GetOrigin() - GetMins();
	moveInfo.state = MoverState::Top;
	moveInfo.startOrigin = GetOrigin();
	moveInfo.endAngles = destination;

	BrushMoveCalc( destination, &FuncTrain::OnWaitAtCorner );
	SetSpawnFlags(GetSpawnFlags() | SF_StartOn);
}

//===============
// FuncTrain::ResumePath
//===============
void FuncTrain::ResumePath() {
	vec3_t destination = currentPathEntity->GetOrigin() - GetMins();

	moveInfo.state = MoverState::Top;
	moveInfo.startOrigin = GetOrigin();
	moveInfo.endOrigin = destination;

	BrushMoveCalc( destination, &FuncTrain::OnWaitAtCorner );
	SetSpawnFlags(GetSpawnFlags() | SF_StartOn);
}

//===============
// FuncTrain::WaitAtCorner
//===============
void FuncTrain::WaitAtCorner() {
	// Trigger stuff that is associated with the current path corner
	if ( currentPathEntity->GetPathTarget() ) {
		// Temporarily swap target and pathTarget
		std::string nextCorner = currentPathEntity->GetTarget();
		currentPathEntity->SetTarget( currentPathEntity->GetPathTarget() );
		currentPathEntity->UseTargets( GetActivator() );
		currentPathEntity->SetTarget( nextCorner );

		// Do not proceed if we got killed by a killtarget
		if ( GetServerFlags() & EntityServerFlags::Remove ) {
			return;
		}
	}

	if ( moveInfo.wait != Frametime::zero() ) {
		if ( moveInfo.wait > GameTime::zero() ) {
			SetNextThinkTime( level.time + moveInfo.wait );
			SetThinkCallback( &FuncTrain::NextCornerThink );
		} else if ( GetSpawnFlags() & SF_Toggled ) {
			NextCornerThink();
		    SetSpawnFlags(GetSpawnFlags() & ~SF_StartOn);
			SetVelocity( vec3_zero() );
			SetNextThinkTime( GameTime::zero() );
		}

		if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
			if ( moveInfo.endSoundIndex ) {
				gi.Sound( GetPODEntity(), SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.endSoundIndex, 1, Attenuation::Static, 0 );
			}
			SetSound( 0 );
		}
	} else {
		NextCornerThink();
	}
}

//===============
// FuncTrain::OnWaitAtCorner
//===============
void FuncTrain::OnWaitAtCorner( IServerGameEntity* ent) {
	if ( ent->IsSubclassOf<FuncTrain>() ) {
		dynamic_cast<FuncTrain*>( ent )->WaitAtCorner();
	}
}

//===============
// FuncTrain::TrainBlocked
//===============
void FuncTrain::TrainBlocked( IServerGameEntity* other ) {
	if ( !(other->GetServerFlags() & EntityServerFlags::Monster) && !other->GetClient() ) {
		// Give it a chance to go away on its own terms (like gibs)
		GetGameMode()->InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), 100000, 1, 0, MeansOfDeath::Crush );
		// If it's still there, nuke it
		if ( other ) {
			SVG_BecomeExplosion1( other );
		}

		return;
	}

	if ( level.time < damageDebounceTime || !GetDamage() ) {
		return;
	}

	damageDebounceTime = level.time + 500ms;
	GetGameMode()->InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );
}
