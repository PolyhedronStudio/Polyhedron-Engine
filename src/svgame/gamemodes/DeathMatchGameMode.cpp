/*
// LICENSE HERE.

//
// DeathMatchGameMode.cpp
//
//
*/
#include "../g_local.h"          // SVGame.
#include "../effects.h"     // Effects.
#include "../entities.h"    // Entities.
#include "../utils.h"       // Util funcs.

// Server Game Base Entity.
#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/PlayerClient.h"

// Weapons.h
#include "../player/client.h"
#include "../player/hud.h"
#include "../player/weapons.h"
#include "../player/view.h"

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
    value = Info_ValueForKey(userInfo, "spectator");
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

//===============
// DeathMatchGameMode::ClientBegin
// 
// Called when a client is ready to be placed in the game after connecting.
//===============
void DeathMatchGameMode::ClientBegin(Entity* serverEntity) {
    SVG_InitEntity(serverEntity);

    SVG_InitClientRespawn(serverEntity->client);

    // locate ent at a spawn point
    SVG_PutClientInServer(serverEntity);

    if (level.intermission.time) {
        HUD_MoveClientToIntermission(serverEntity);
    } else {
        // send effect
        gi.WriteByte(SVG_CMD_MUZZLEFLASH);
        //gi.WriteShort(ent - g_entities);
        gi.WriteShort(serverEntity->state.number);
        gi.WriteByte(MuzzleFlashType::Login);
        gi.Multicast(serverEntity->state.origin, MultiCast::PVS);
    }

    gi.BPrintf(PRINT_HIGH, "%s entered the game\n", serverEntity->client->persistent.netname);

    // make sure all view stuff is valid
    SVG_ClientEndServerFrame((PlayerClient*)serverEntity->classEntity);
}