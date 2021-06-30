/*
// LICENSE HERE.

//
// CoopGameMode.cpp
//
//
*/
#include "../g_local.h"          // SVGame.

// Server Game Base Entity.
#include "../entities/base/SVGBaseEntity.h"

// Game Mode.
#include "CoopGameMode.h"

//
// Constructor/Deconstructor.
//
CoopGameMode::CoopGameMode() : DefaultGameMode() {

}
CoopGameMode::~CoopGameMode() {

}

//
// Interface functions. 
//
//

//
//===============
// CoopGameMode::CanDamage
//
// Template function serves as an example atm.
//===============
//
qboolean CoopGameMode::CanDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor) {
    // Let it be to DefaultGameMode. :)
    return DefaultGameMode::CanDamage(target, inflictor);
}