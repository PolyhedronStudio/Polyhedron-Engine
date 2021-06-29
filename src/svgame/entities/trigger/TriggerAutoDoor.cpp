/*
// LICENSE HERE.

// TriggerAutoDoor.cpp
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

#include "../func/FuncDoor.h"

#include "TriggerAutoDoor.h"

//===============
// TriggerAutoDoor::ctor
//===============
TriggerAutoDoor::TriggerAutoDoor( Entity* entity )
	: SVGBaseTrigger( entity ) {
	debounceTouchTime = 0.0f;
}

//===============
// TriggerAutoDoor::Spawn
//===============
void TriggerAutoDoor::Spawn() {
	SVGBaseTrigger::Spawn();

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
TriggerAutoDoor* TriggerAutoDoor::Create( SVGBaseEntity* ownerEntity, vec3_t ownerMaxs, vec3_t ownerMins ) {
	TriggerAutoDoor* autoDoor = static_cast<TriggerAutoDoor*>(SVG_SpawnClassEntity( SVG_Spawn(), "trigger_auto_door" ));
	autoDoor->SetOwner( ownerEntity );
	autoDoor->SetMaxs( ownerMaxs );
	autoDoor->SetMins( ownerMins );
	autoDoor->Spawn();
}
