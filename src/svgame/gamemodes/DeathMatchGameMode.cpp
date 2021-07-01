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

//
//===============
// DeathMatchGameMode::ClientCanConnect
//
// Extends from DefaultGameMode, in order to allow for spectators to be around.
//===============
//
qboolean DeathMatchGameMode::ClientCanConnect(Entity* serverEntity, char* userInfo) {
    // Check to see if they are on the banned IP list
    char* value = Info_ValueForKey(userInfo, "ip");
    if (SVG_FilterPacket(value)) {
        Info_SetValueForKey(userInfo, "rejmsg", "Banned.");
        return false;
    }

    // Check for whether the client is a spectator.
    value = Info_ValueForKey(userInfo, "isSpectator");
    int32_t i = 0, numspec = 0;

    if (*spectator_password->string &&
        strcmp(spectator_password->string, "none") &&
        strcmp(spectator_password->string, value)) {
        Info_SetValueForKey(userInfo, "rejmsg", "Spectator password required or incorrect.");
        return false;
    }

    // Count total spectators.
    for (i = numspec = 0; i < maxClients->value; i++)
        if (g_entities[i + 1].inUse && g_entities[i + 1].client->persistent.isSpectator)
            numspec++;

    // Reject in case we exceeded the limit.
    if (numspec >= maxspectators->value) {
        Info_SetValueForKey(userInfo, "rejmsg", "Server isSpectator limit is full.");
        return false;
    }

    // We CAN connect :)
    return true;
}