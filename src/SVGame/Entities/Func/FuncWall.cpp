/*
// LICENSE HERE.

// TriggerRelay.cpp
*/

#include "../../ServerGameLocal.h"
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
        serverEntity->state.effects |= EntityEffectType::AnimCycleAll2hz;
    } else if ( GetSpawnFlags() & SF_AnimatedFast ) {
        serverEntity->state.effects |= EntityEffectType::AnimCycleAll30hz;
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

    LinkEntity();
}

//===============
// FuncWall::WallUse
//===============
void FuncWall::WallUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
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
