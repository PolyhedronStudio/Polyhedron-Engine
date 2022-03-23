/*
// LICENSE HERE.

// TriggerAutoDoor.cpp
*/

// Core.
#include "../../ServerGameLocals.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"

// Entities.
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseMover.h"
#include "../Func/FuncPlat.h"

// World.
#include "../../World/Gameworld.h"

// Trigger Auto Platform.
#include "TriggerAutoPlatform.h"

//===============
// TriggerAutoPlatform::ctor
//===============
TriggerAutoPlatform::TriggerAutoPlatform( Entity* entity )
	: Base( entity ) {
	debounceTouchTime = 0.0f;
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
void TriggerAutoPlatform::AutoPlatformTouch( SVGBaseEntity* self, SVGBaseEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
	bool isMonster = other->GetServerFlags() & EntityServerFlags::Monster;
	// Alternatively, when we have a BaseMonster class:
	// isMonster = other->IsSubclassOf<BaseMonster>();

	// Only activate if we are a client.
	if (!other->GetClient()) {
		return;
	}

	// Don't activate if we're out of health.
	if (other->GetHealth() <= 0) {
		return;
	}

	// Wait till it can be triggered again.
	if (level.time < debounceTouchTime) {
	    return;
	}

	// Set new debounce time.
	debounceTouchTime = level.time + 3.0f;

	// Ensure that the owner is a func_plat.
	SVGBaseEntity* ownerEntity = GetEnemy();

	if (!ownerEntity->IsSubclassOf<FuncPlat>()) {
		gi.DPrintf("Warning: entity #%i is not a func_plat.\n", ownerEntity->GetNumber());
		return;
	}
	gi.DPrintf("TOUCHED PLATFORM TRIGGER22222222\n");

	// It is save to cast the pointer.
	FuncPlat* platformEntity = dynamic_cast<FuncPlat*>(ownerEntity);
	
	// Fetch move info.
	PushMoveInfo *moveInfo = platformEntity->GetPushMoveInfo();
	
	// Action is determined by move state.
	if (moveInfo->state == MoverState::Bottom) {
		platformEntity->PlatformGoUp();
	} else if (moveInfo->state == MoverState::Top) {
		platformEntity->SetNextThinkTime(level.time + 1 * FRAMETIME);
	}

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
	GetOwner()->Use( other, GetActivator() );
}

//===============
// TriggerAutoPlatform::Create
//===============
TriggerAutoPlatform* TriggerAutoPlatform::Create( SVGBaseEntity* ownerEntity, vec3_t ownerMins, vec3_t ownerMaxs ) {
    TriggerAutoPlatform* autoPlatform = GetGameworld()->CreateClassEntity<TriggerAutoPlatform>();
    autoPlatform->SetOrigin(ownerEntity->GetEndPosition());
	autoPlatform->LinkEntity();
	autoPlatform->Spawn();
	autoPlatform->SetSolid(Solid::Trigger);
	autoPlatform->SetMins( ownerMins );
	autoPlatform->SetMaxs( ownerMaxs );
	autoPlatform->SetOwner( ownerEntity );
	autoPlatform->SetEnemy( ownerEntity );
	autoPlatform->LinkEntity();

	return autoPlatform;
}
