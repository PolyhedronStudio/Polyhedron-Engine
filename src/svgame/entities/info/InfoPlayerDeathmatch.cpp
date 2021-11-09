/*
// LICENSE HERE.

//
// InfoPlayerDeathmatch.cpp
//
//
*/
#include "../../g_local.h"              // SVGame.
#include "../base/SVGBaseEntity.h"      // BaseEntity.
#include "InfoPlayerStart.h"
#include "InfoPlayerDeathmatch.h"            // Class.

// Constructor/Deconstructor.
InfoPlayerDeathmatch::InfoPlayerDeathmatch(Entity* svEntity) 
    : InfoPlayerStart(svEntity) {

}
InfoPlayerDeathmatch::~InfoPlayerDeathmatch() {

}

// Interface functions. 
//===============
// InfoPlayerDeathmatch::Precache
//
//===============
void InfoPlayerDeathmatch::Precache() {
    Base::Precache();
}

//===============
// InfoPlayerDeathmatch::Spawn
//
//===============
void InfoPlayerDeathmatch::Spawn() {
    Base::Spawn();
}

//===============
// InfoPlayerDeathmatch::PostSpawn
//
//===============
void InfoPlayerDeathmatch::PostSpawn() {
    Base::PostSpawn();
}

//===============
// InfoPlayerDeathmatch::Think
//
//===============
void InfoPlayerDeathmatch::Think() {
    // Parent think.
    Base::Think();
}

//===============
// InfoPlayerDeathmatch::SpawnKey
//
//===============
void InfoPlayerDeathmatch::SpawnKey(const std::string& key, const std::string& value) {
    Base::SpawnKey(key, value);
}
