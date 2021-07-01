/*
// LICENSE HERE.

//
// PlayerStart.cpp
//
//
*/
#include "../../g_local.h"              // SVGame.

#include "../base/SVGBaseEntity.h"      // BaseEntity.

#include "InfoPlayerStart.h"            // Class.

// Constructor/Deconstructor.
InfoPlayerStart::InfoPlayerStart(Entity* svEntity) 
    : SVGBaseEntity(svEntity) {

}
InfoPlayerStart::~InfoPlayerStart() {

}

// Interface functions. 
void InfoPlayerStart::Precache() {
    Base::Precache();
}

void InfoPlayerStart::Spawn() {
    Base::Spawn();
}

void InfoPlayerStart::PostSpawn() {
    Base::PostSpawn();
}

void InfoPlayerStart::Think() {
    // Parent think.
    Base::Think();
}