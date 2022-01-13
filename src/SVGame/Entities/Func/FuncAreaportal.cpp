/*
// LICENSE HERE.

// FuncAreaportal.cpp
*/

#include "../../ServerGameLocal.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"
#include "../../BrushFunctions.h"

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
		ParseIntegerKeyValue( key, value, serverEntity->style );
	} else {
		Base::SpawnKey( key, value );
	}
}

//===============
// FuncAreaportal::PortalUse
//===============
void FuncAreaportal::PortalUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
	ActivatePortal( !turnedOn );
}

//===============
// FuncAreaportal::ActivatePortal
//===============
void FuncAreaportal::ActivatePortal( bool open ) {
	gi.SetAreaPortalState( GetStyle(), open );
	turnedOn = open;
}
