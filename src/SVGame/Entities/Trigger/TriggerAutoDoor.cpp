/*
// LICENSE HERE.

// TriggerAutoDoor.cpp
*/

// Core.
#include "../../ServerGameLocal.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"

// Entities.
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseMover.h"

#include "../Func/FuncDoor.h"

#include "TriggerAutoDoor.h"

//===============
// TriggerAutoDoor::ctor
//===============
TriggerAutoDoor::TriggerAutoDoor( Entity* entity )
	: Base( entity ) {
	debounceTouchTime = 0.0f;
}

//===============
// TriggerAutoDoor::Spawn
//===============
void TriggerAutoDoor::Spawn() {
	Base::Spawn();

	SetSolid( Solid::Trigger );
	SetMoveType( MoveType::None );
	SetTouchCallback( &TriggerAutoDoor::AutoDoorTouch );
	LinkEntity();
}

//===============
// TriggerAutoDoor::AutoDoorTouch
//===============
void TriggerAutoDoor::AutoDoorTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf ) {
	bool isMonster = other->GetServerFlags() & EntityServerFlags::Monster;
	// Alternatively, when we have a BaseMonster class:
	// isMonster = other->IsSubclassOf<BaseMonster>();

	if ( other->GetHealth() <= 0 ) {
		return; // If you're dead, you can't pass
	}
	// Only players and monsters are allowed
	if ( !(isMonster) && (!other->GetClient()) ) {
		return;
	}
	// Unless the no monster flag is set
	if ( (GetOwner()->GetSpawnFlags() & FuncDoor::SF_NoMonsters) && (isMonster) ) {
		return;
	}
	// If it's not the time to activate the door yet, then don't
	if ( level.time < debounceTouchTime ) {
		return;
	}
	debounceTouchTime = level.time + 1.0f;
	// Trigger our door
	GetOwner()->Use( other, other );
}

//===============
// TriggerAutoDoor::Create
//===============
TriggerAutoDoor* TriggerAutoDoor::Create( SVGBaseEntity* ownerEntity, vec3_t ownerMins, vec3_t ownerMaxs ) {
	TriggerAutoDoor* autoDoor = SVG_CreateClassEntity<TriggerAutoDoor>();
	autoDoor->SetOwner( ownerEntity );
	autoDoor->SetMaxs( ownerMaxs );
	autoDoor->SetMins( ownerMins );
	autoDoor->Spawn();
	return autoDoor;
}
