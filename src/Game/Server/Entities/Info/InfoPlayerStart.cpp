/*
// LICENSE HERE.

//
// PlayerStart.cpp
//
//
*/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../Base/SVGBaseEntity.h"      // BaseEntity.
#include "InfoPlayerStart.h"            // Class.

// Constructor/Deconstructor.
InfoPlayerStart::InfoPlayerStart(PODEntity *svEntity) 
    : SVGBaseEntity(svEntity) {

}

// Interface functions. 
//===============
// InfoPlayerStart::Precache
//
//===============
void InfoPlayerStart::Precache() {
    Base::Precache();
}

//===============
// InfoPlayerStart::Spawn
//
//===============
void InfoPlayerStart::Spawn() {
    Base::Spawn();
}

//===============
// InfoPlayerStart::PostSpawn
//
//===============
void InfoPlayerStart::PostSpawn() {
    Base::PostSpawn();
}

//===============
// InfoPlayerStart::Think
//
//===============
void InfoPlayerStart::Think() {
    // Parent think.
    Base::Think();
}

//===============
// InfoPlayerStart::SpawnKey
//
//===============
void InfoPlayerStart::SpawnKey(const std::string& key, const std::string& value) {
    Base::SpawnKey(key, value);
}
