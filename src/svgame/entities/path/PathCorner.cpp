/*
// LICENSE HERE.

// PathCorner.cpp
*/

#include "../../g_local.h"
#include "../../entities.h"

#include "PathCorner.h"

//===============
// PathCorner::ctor
//===============
PathCorner::PathCorner( Entity* entity )
	: Base( entity ) {

}

//===============
// PathCorner::Spawn
//===============
void PathCorner::Spawn() {
    Base::Spawn();

    if ( targetNameStr.empty() ) {
        gi.DPrintf( "path_corner with no targetname at %s\n", vec3_to_str( GetOrigin() ).c_str() );
        return Remove();
    }

    SetSolid( Solid::Trigger );
    SetMaxs( BboxSize );
    SetMins( vec3_negate( BboxSize ) );
    serverEntity->serverFlags |= EntityServerFlags::NoClient;
    LinkEntity();
}

//===============
// PathCorner::OnReachedCorner
//===============
void PathCorner::OnReachedCorner( SVGBaseEntity* traveler ) {
    // Not implemented   
}
