/*
// LICENSE HERE.

// TriggerAutoDoor.cpp
*/

//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "Game/Server/Effects.h"
#include "Game/Server/Entities.h"
#include "Game/Server/Utilities.h"

// Entities.
#include "Game/Server/Entities/Base/SVGBaseTrigger.h"
#include "Game/Server/Entities/Base/SVGBaseLinearMover.h"
#include "Game/Server/Entities/Func/FuncPlat.h"

// World.
#include "Game/Server/World/ServerGameWorld.h"

// Trigger Auto Platform.
#include "Game/Server/Entities/Trigger/TriggerAutoPlatform.h"

//===============
// TriggerAutoPlatform::ctor
//===============
TriggerAutoPlatform::TriggerAutoPlatform( Entity* entity )
	: Base( entity ) {
}

//===============
// TriggerAutoPlatform::Spawn
//===============
void TriggerAutoPlatform::Spawn() {
	Base::Spawn();
	// Initialize brush trigger.
	InitBrushTrigger();

	// Set touch callback.
	SetTouchCallback( &TriggerAutoPlatform::AutoPlatformTouch );

	// Link in.
	LinkEntity();
}

//===============
// TriggerAutoPlatform::AutoPlatformTouch
//===============
void TriggerAutoPlatform::AutoPlatformTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
	// Only activate if we are a client.
	if ( !other ) { 
		return;
	}
	
	// Alternatively, when we have a DECENT BaseMonster class:
	const bool isMonster = other->GetServerFlags() & EntityServerFlags::Monster;

	if ( (!other->GetClient() && !isMonster) ) {
		return;
	}
	// Unless the no monster flag is set
	//if ( (GetOwner()->GetSpawnFlags() & FuncPlat::SF_NoMonsters) && (isMonster) ) {
	//	return;
	//}

	// Don't activate if we're out of health.
	if (other->GetHealth() <= 0) {
		return;
	}

	// Wait till it can be triggered again.
	if (level.time < debounceTouchTime) {
	    return;
	}

	// Set new debounce time.
	debounceTouchTime = level.time + 3s; // TODO: This is likely better eh? GetOwner()->GetWaitTime();

	// Ensure that the owner is a func_plat.
	IServerGameEntity* ownerEntity = GetEnemy();

	if (!ownerEntity->IsSubclassOf<FuncPlat>()) {
		gi.DPrintf("Warning: entity #%i is not a func_plat.\n", ownerEntity->GetNumber());
		return;
	}

	// It is save to cast the pointer.
	FuncPlat* platformEntity = dynamic_cast<FuncPlat*>(ownerEntity);
	
	// Fetch move info.
//	PushMoveInfo *moveInfo = platformEntity->GetPushMoveInfo();
	
	//// Action is determined by move state.
	//if (moveInfo->state == MoverState::Bottom) {
	//	platformEntity->Callback_EngageRaiseMove();
	//} else if (moveInfo->state == MoverState::Top) {
	//	platformEntity->Callback_EngageLowerMove();
	//	//platformEntity->SetNextThinkTime(level.time + FRAMERATE_MS);
	//	//platformEntity->SetThinkCallback( &FuncPlat::Callback_EngageLowerMove );
	//}

	//if ( other->GetHealth() <= 0 ) {
	//	return; // If you're dead, you can't pass
	//}
	//// Only players and monsters are allowed
	//if ( !(isMonster) && (!other->GetClient()) ) {
	//	return;
	//}
	//////// Unless the no monster flag is set
	//////if ( (GetOwner()->GetSpawnFlags() & FuncPlat::SF_NoMonsters) && (isMonster) ) {
	//////	return;
	//////}
	////// If it's not the time to activate the platform yet, then don't

	// Trigger our platform
	GetOwner()->DispatchUseCallback( other, GetActivator() );
}

//===============
// TriggerAutoPlatform::Create
//===============
TriggerAutoPlatform* TriggerAutoPlatform::Create( SVGBaseEntity *ownerEntity, const vec3_t &triggerOrigin, const vec3_t &triggerMins, const vec3_t &triggerMaxs  ) {
    TriggerAutoPlatform* autoPlatform = GetGameWorld()->CreateGameEntity< TriggerAutoPlatform >();
    autoPlatform->SetOrigin( triggerOrigin );
	autoPlatform->LinkEntity();
	autoPlatform->Spawn();
	autoPlatform->SetSolid( Solid::Trigger );
	autoPlatform->SetMins( triggerMins );
	autoPlatform->SetMaxs( triggerMaxs );
	autoPlatform->SetOwner( ownerEntity );
	autoPlatform->SetEnemy( ownerEntity );
	autoPlatform->LinkEntity();

	return autoPlatform;
}
