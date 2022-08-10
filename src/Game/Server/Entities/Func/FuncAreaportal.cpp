/*
// LICENSE HERE.

// FuncAreaportal.cpp
*/

//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"

#include "FuncAreaportal.h"

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
	gi.SetAreaPortalState( GetStyle(), false );
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
void FuncAreaportal::PortalUse( IServerGameEntity* other, IServerGameEntity* activator ) {
	ActivatePortal( !turnedOn );
}

//===============
// FuncAreaportal::ActivatePortal
//===============
void FuncAreaportal::ActivatePortal( bool open ) {
	gi.SetAreaPortalState( GetStyle(), open );
	turnedOn = open;
}
