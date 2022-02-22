/*
// LICENSE HERE.

// TargetTempEntity.cpp
*/

#include "../../ServerGameLocal.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"

#include "TargetTempEntity.h"

//===============
// TargetTempEntity::ctor
//===============
TargetTempEntity::TargetTempEntity( Entity* entity )
	: Base( entity ) {

}

//===============
// TargetTempEntity::Spawn
//===============
void TargetTempEntity::Spawn() {
	SetUseCallback( &TargetTempEntity::TempEntityUse );
}

//===============
// TargetTempEntity::TempEntityUse
//===============
void TargetTempEntity::TempEntityUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
	gi.WriteByte( ServerGameCommands::TempEntity );
	gi.WriteByte( GetStyle() );
	gi.WriteVector3( GetOrigin() );
	gi.Multicast( GetOrigin(), Multicast::PVS );
}
