/*
// LICENSE HERE.

//
// DeathMatchGameMode.cpp
//
//
*/
#include "../g_local.h"          // SVGame.

// Server Game Base Entity.
#include "../entities/base/SVGBaseEntity.h"

// Game Mode.
#include "DeathMatchGameMode.h"

//
// Constructor/Deconstructor.
//
DeathMatchGameMode::DeathMatchGameMode() : DefaultGameMode() {

}
DeathMatchGameMode::~DeathMatchGameMode() {

}

//
// Interface functions. 
//
//

//
//===============
// DeathMatchGameMode::CanDamage
//
// Template function serves as an example atm.
//===============
//
qboolean DeathMatchGameMode::CanDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor) {
    // Let it be to DefaultGameMode. :)
    return DefaultGameMode::CanDamage(target, inflictor);
}