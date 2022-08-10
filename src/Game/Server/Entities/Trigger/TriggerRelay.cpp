/*
// LICENSE HERE.

// TriggerRelay.cpp
*/

//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"

#include "TriggerRelay.h"

//===============
// TriggerRelay::ctor
//===============
TriggerRelay::TriggerRelay( Entity* entity )
	: Base( entity ) {
}

//===============
// TriggerRelay::Spawn
//===============
void TriggerRelay::Spawn() {
	Base::Spawn();
	SetUseCallback( &TriggerRelay::RelayUse );
}

//===============
// TriggerRelay::RelayUse
//===============
void TriggerRelay::RelayUse( IServerGameEntity* other, IServerGameEntity* activator ) {
	UseTargets( activator );
}
