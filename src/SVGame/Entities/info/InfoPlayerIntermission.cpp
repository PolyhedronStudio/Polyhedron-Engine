/*
// LICENSE HERE.

//
// PlayerStart.cpp
//
//
*/
#include "../../g_local.h"              // SVGame.
#include "../base/SVGBaseEntity.h"      // BaseEntity.
#include "InfoPlayerStart.h"
#include "InfoPlayerIntermission.h"     // Class.

// Constructor/Deconstructor.
InfoPlayerIntermission::InfoPlayerIntermission(Entity* svEntity) 
    : InfoPlayerStart(svEntity) {

}
InfoPlayerIntermission::~InfoPlayerIntermission() {

}

// Interface functions. 
//===============
// InfoPlayerStart::Precache
//
//===============
void InfoPlayerIntermission::Precache() {
    Base::Precache();
}

//===============
// InfoPlayerStart::Spawn
//
//===============
void InfoPlayerIntermission::Spawn() {
    Base::Spawn();
}

//===============
// InfoPlayerIntermission::PostSpawn
//
//===============
void InfoPlayerIntermission::PostSpawn() {
    Base::PostSpawn();
}

//===============
// InfoPlayerIntermission::Think
//
//===============
void InfoPlayerIntermission::Think() {
    // Parent think.
    Base::Think();
}

//===============
// InfoPlayerIntermission::SpawnKey
//
//===============
void InfoPlayerIntermission::SpawnKey(const std::string& key, const std::string& value) {
    Base::SpawnKey(key, value);
}
