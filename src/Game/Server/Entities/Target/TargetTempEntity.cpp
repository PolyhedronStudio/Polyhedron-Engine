/*
// LICENSE HERE.

// TargetTempEntity.cpp
*/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

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
void TargetTempEntity::TempEntityUse( IServerGameEntity* other, IServerGameEntity* activator ) {
	gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte( ServerGameCommand::TempEntityEvent );
	gi.MSG_WriteUint8(GetStyle());//WriteByte( GetStyle() );
	gi.MSG_WriteVector3( GetOrigin(), false );//WriteVector3( GetOrigin() );
	gi.Multicast( GetOrigin(), Multicast::PVS );
}
