/*
// LICENSE HERE.

//
// PlayerStart.cpp
//
//
*/
#include "../../ServerGameLocals.h"              // SVGame.
#include "../Base/SVGBaseEntity.h"      // BaseEntity.
#include "InfoNotNull.h"            // Class.

// Constructor/Deconstructor.
InfoNotNull::InfoNotNull(PODEntity *svEntity) 
    : Base(svEntity) {

}

// Interface functions. 
//===============
// InfoNotNull::Precache
//
//===============
void InfoNotNull::Precache() {
    Base::Precache();
}

//===============
// InfoNotNull::Spawn
//
//===============
void InfoNotNull::Spawn() {
    Base::Spawn();

	SetInUse(true);
	SetMins(vec3_t{-16.f, -16.f, -16.f});
	SetMaxs(vec3_t{16.f, 16.f, 16.f});
	SetMoveType(MoveType::None);
	SetSolid(Solid::Not);
	std::string x = GetTargetName();
	gi.DPrintf("InfoNotNull: TargetName: %s\n", x.c_str());
	LinkEntity();
}

//===============
// InfoNotNull::PostSpawn
//
//===============
void InfoNotNull::PostSpawn() {
    Base::PostSpawn();
}

//===============
// InfoNotNull::Think
//
//===============
void InfoNotNull::Think() {
    // Parent think.
    Base::Think();
}

//===============
// InfoNotNull::SpawnKey
//
//===============
void InfoNotNull::SpawnKey(const std::string& key, const std::string& value) {
    Base::SpawnKey(key, value);
}
