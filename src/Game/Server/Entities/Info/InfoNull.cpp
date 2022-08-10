/*
// LICENSE HERE.

// InfoNull.cpp
*/

//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"

// This entity never spawns, and as such, no edicts will be wasted on it during spawn time
class InfoNull : public SVGBaseEntity
{
public:
	DefineDummyMapClass( "info_null", InfoNull, SVGBaseEntity );
};
