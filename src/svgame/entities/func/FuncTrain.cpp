/*
// LICENSE HERE.

// FuncTrain.cpp
*/

#include "../../g_local.h"
#include "../../entities.h"
#include "../../effects.h"

#include "../base/SVGBaseTrigger.h"
#include "../base/SVGBaseMover.h"

#include "../path/PathCorner.h"
#include "FuncTrain.h"

//===============
// FuncTrain::ctor
//===============
FuncTrain::FuncTrain( Entity* entity )
 : Base( entity ) {

}

//===============
// FuncTrain::Spawn
//===============
void FuncTrain::Spawn() {
	SetMoveType( MoveType::Push );
	SetSolid( Solid::BSP );
	SetAngles( vec3_zero() );

	if ( spawnFlags & SF_StopWhenBlocked ) {
		damage = 0;
	} else if ( !damage ) {
		damage = 100;
	}

	SetModel( GetModel() );

	if ( !GetSpeed() ) {
		SetSpeed( 100.0f );
	}

	moveInfo.acceleration = GetSpeed();
	moveInfo.deceleration = GetSpeed();

	LinkEntity();

	if ( targetStr.empty() ) {
		gi.DPrintf( "func_train without a target at %s\n", vec3_to_str( GetAbsoluteCenter() ).c_str() );
	}
}

//===============
// FuncTrain::PostSpawn
//===============
void FuncTrain::PostSpawn() {
	if ( targetStr.empty() ) {
		return;
	}

	SVGBaseEntity* ent = SVG_FindEntityByKeyValue( "targetname", targetStr );
	if ( nullptr == ent ) {
		gi.DPrintf( "FuncTrain: target '%s' not found, maybe you made a typo?\n", targetStr.c_str() );
		return;
	}

	if ( !ent->IsSubclassOf<PathCorner>() ) {
		gi.DPrintf( "FuncTrain: target '%s' is not a path entity\n", targetStr.c_str() );
		return;
	}

	serverEntity->state.origin = ent->GetOrigin() - GetMins();
	LinkEntity();

	// This train has no name, trigger it immediately
	if ( targetNameStr.empty() ) {
		spawnFlags |= SF_StartOn;
	}

	if ( spawnFlags & SF_StartOn ) {
		SetNextThinkTime( level.time + FRAMETIME );
		SetThinkCallback( nullptr );
		activator = this;
	}
}

//===============
// FuncTrain::NextCornerThink
//===============
void FuncTrain::NextCornerThink() {

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
		currentPathEntity->UseTargets( activator );
		currentPathEntity->SetTarget( nextCorner );

		// Do not proceed if we got killed by a killtarget
		if ( GetServerFlags() & EntityServerFlags::Remove ) {
			return;
		}
	}

	if ( moveInfo.wait ) {
		if ( moveInfo.wait > 0.0f ) {
			SetNextThinkTime( level.time + moveInfo.wait );
			SetThinkCallback( &FuncTrain::NextCornerThink );
		} else if ( GetSpawnFlags() & SF_Toggled ) {
			NextCornerThink();
			spawnFlags &= ~SF_StartOn;
			SetVelocity( vec3_zero() );
			SetNextThinkTime( 0.0f );
		}

		if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
			if ( moveInfo.endSoundIndex ) {
				gi.Sound( serverEntity, CHAN_NO_PHS_ADD + CHAN_VOICE, moveInfo.endSoundIndex, 1, ATTN_STATIC, 0 );
			}
			SetSound( 0 );
		}
	} else {
		NextCornerThink();
	}
}

//===============
// FuncTrain::TrainBlocked
//===============
void FuncTrain::TrainBlocked( SVGBaseEntity* other ) {
	if ( !(other->GetServerFlags() & EntityServerFlags::Monster) && !other->GetClient() ) {
		// Give it a chance to go away on its own terms (like gibs)
		SVG_InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), 100000, 1, 0, MeansOfDeath::Crush );
		// If it's still there, nuke it
		if ( other ) {
			SVG_BecomeExplosion1( other );
		}

		return;
	}

	if ( level.time < damageDebounceTime || !GetDamage() ) {
		return;
	}

	damageDebounceTime = level.time + 0.5f;
	SVG_InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );
}
