/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
//
//
*/
#include "../../g_local.h"     // SVGame.
#include "../../effects.h"     // Effects.
#include "../../utils.h"       // Util funcs.

#include "../base/SVGBaseEntity.h"

#include "PlayerClient.h"

// Constructor/Deconstructor.
PlayerClient::PlayerClient(Entity* svEntity) : SVGBaseEntity(svEntity) {

}
PlayerClient::~PlayerClient() {

}

// Interface functions. 
void PlayerClient::PreCache() {
    gi.DPrintf("MiscExplosionBox::PreCache();");
}
void PlayerClient::Spawn() {
    gi.DPrintf("MiscExplosionBox::Spawn();");
}
void PlayerClient::PostSpawn() {
    gi.DPrintf("MiscExplosionBox::PostSpawn();");
}
void PlayerClient::Think() {
    gi.DPrintf("MiscExplosionBox::Think();");
}

// Functions.