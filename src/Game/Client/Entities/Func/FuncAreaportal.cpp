/***
*
*	License here.
*
*	@file
* 
*   Client Side FuncDoor -> Intented to be predicted someday.
*
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"
//! BaseMover.
#include "Game/Client/Entities/Base/CLGBaseLocalEntity.h"
//#include "Game/Entities/Trigger/TriggerAutoDoor.h"
//#include "Game/Entities/Func/FuncAreaportal.h"
#include "Game/Client/Entities/Func/FuncAreaportal.h"
//! Game World.
#include "Game/Client/World/ClientGameWorld.h"

//===============
// FuncAreaportal::ctor
//===============
FuncAreaportal::FuncAreaportal( Entity* entity )
	: Base( entity ) {

}

//===============
// FuncAreaportal::Spawn
//===============
void FuncAreaportal::Spawn() {
	// Start closed.
	//clgi.CM_SetAreaPortalState( GetStyle(), false );
	SetUseCallback( &FuncAreaportal::FuncAreaportalUse );
}

//===============
// FuncAreaportal::SpawnKey
//===============
void FuncAreaportal::SpawnKey( const std::string& key, const std::string& value ) {
	if ( key == "style" ) {
		// Parsed integer.
		int32_t parsedInteger = 0;

		// Parse.
		ParseKeyValue( key, value, parsedInteger);

		// Set style.
		SetStyle(parsedInteger);
	} else {
		Base::SpawnKey( key, value );
	}
}

//===============
// FuncAreaportal::PortalUse
//===============
void FuncAreaportal::FuncAreaportalUse( IClientGameEntity* other, IClientGameEntity* activator ) {
	ActivatePortal( !turnedOn );
}

//===============
// FuncAreaportal::ActivatePortal
//===============
void FuncAreaportal::ActivatePortal( const bool open ) {
	clgi.CM_SetAreaPortalState( GetStyle(), open );
	turnedOn = open;
}
