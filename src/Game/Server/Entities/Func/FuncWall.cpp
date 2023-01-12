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

#include "FuncWall.h"

//===============
// FuncWall::ctor
//===============
FuncWall::FuncWall( Entity* entity )
	: Base( entity ) {

}

//===============
// FuncWall::Spawn
//===============
void FuncWall::Spawn() {
    Base::Spawn();

    SetMoveType( MoveType::Push );
    SetModel( GetModel() );

    if ( GetSpawnFlags() & SF_Animated ) {
        SetEffects(GetEffects() | EntityEffectType::AnimCycleAll2hz);
    } else if ( GetSpawnFlags() & SF_AnimatedFast ) {
        SetEffects(GetEffects() | EntityEffectType::AnimCycleAll30hz);
    }

    // Just a wall
    if ( !(GetSpawnFlags() & 7) ) {
        SetSolid( Solid::BSP );
        LinkEntity();
        return;
    }

    // ???
    if ( !(GetSpawnFlags() & SF_TriggerSpawn) ) {
        spawnFlags |= SF_TriggerSpawn;
    }

    // Yell if the spawnflags are weird
    if ( (GetSpawnFlags() & SF_StartOn) && !(GetSpawnFlags() & SF_Toggle) ) {
        gi.DPrintf( "func_wall has StartOn without Toggle!!!\n" );
        spawnFlags |= SF_Toggle;
    }

    SetUseCallback( &FuncWall::WallUse );

    if ( GetSpawnFlags() & SF_StartOn ) {
        SetSolid( Solid::BSP );
    } else {
        SetSolid( Solid::Not );
        SetServerFlags( GetServerFlags() | EntityServerFlags::NoClient );
    }
	SetAngles( { 0.f, 57.50f, 0.f } );
    LinkEntity();
}

//===============
// FuncWall::WallUse
//===============
void FuncWall::WallUse( IServerGameEntity* other, IServerGameEntity* activator ) {
    if ( GetSolid() == Solid::Not ) {
        SetSolid( Solid::BSP );
        SetServerFlags( GetServerFlags() & ~EntityServerFlags::NoClient );
        SVG_KillBox( this );
    } else {
        SetSolid( Solid::Not );
        SetServerFlags( GetServerFlags() | EntityServerFlags::NoClient );
    }

    LinkEntity();

    if ( !(GetSpawnFlags() & SF_Toggle) ) {
        SetUseCallback( nullptr );
    }
}
