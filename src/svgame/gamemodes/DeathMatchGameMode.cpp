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
    // Be sure to initialize the entity.
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

//===============
// DeathMatchGameMode::ClientUpdateObituary.
// 
//===============
void DeathMatchGameMode::ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) {
    std::string message = ""; // String stating what happened to whichever entity. "suicides", "was squished" etc.
    std::string messageAddition = ""; // String stating what is additioned to it, "'s shrapnell" etc. Funny stuff.

    //if (coop->value && attacker->client)
    //    meansOfDeath |= MOD_FRIENDLY_FIRE;

    // Set a bool for whether we got friendly fire.
    qboolean friendlyFire = meansOfDeath & MeansOfDeath::FriendlyFire;
    // Quickly remove it from meansOfDeath again, our bool is set. This prevents it from 
    // sticking around when we process the next entity/client.
    int32_t finalMeansOfDeath = meansOfDeath & ~MeansOfDeath::FriendlyFire; // Sum of things, final means of death.

    // Determine the means of death.
    switch (finalMeansOfDeath) {
        case MeansOfDeath::Suicide:
            message = "suicides";
            break;
        case MeansOfDeath::Falling:
            message = "cratered";
            break;
        case MeansOfDeath::Crush:
            message = "was squished";
            break;
        case MeansOfDeath::Water:
            message = "sank like a rock";
            break;
        case MeansOfDeath::Slime:
            message = "melted";
            break;
        case MeansOfDeath::Lava:
            message = "does a back flip into the lava";
            break;
        case MeansOfDeath::Explosive:
        case MeansOfDeath::Barrel:
            message = "blew up";
            break;
        case MeansOfDeath::Exit:
            message = "found a way out";
            break;
        case MeansOfDeath::Splash:
        case MeansOfDeath::TriggerHurt:
            message = "was in the wrong place";
            break;
    }

    // Check if the attacker hurt himself, if so, ... n00b! :D 
    if (attacker == self) {
        switch (finalMeansOfDeath) {
            case MeansOfDeath::GrenadeSplash:
                message = "tripped on his own grenade";
                break;
            case MeansOfDeath::RocketSplash:
                message = "blew himself up";
                break;
            default:
                message = "killed himself";
                break;
        }
    }

    // Generated a message?
    if (message != "") {
        gi.BPrintf(PRINT_MEDIUM, "%s %s.\n", self->GetClient()->persistent.netname, message.c_str());
        //if (deathmatch->value)
        self->GetClient()->respawn.score--;
        self->SetEnemy(NULL);
        return;
    }

    // Set 'self' its attacker entity pointer.
    self->SetEnemy(attacker);

    // If we have an attacker, and it IS a client...
    if (attacker && attacker->GetClient()) {
        switch (finalMeansOfDeath) {
            case MeansOfDeath::Blaster:
                message = "was blasted by";
                break;
            case MeansOfDeath::Shotgun:
                message = "was gunned down by";
                break;
            case MeansOfDeath::SuperShotgun:
                message = "was blown away by";
                messageAddition = "'s super shotgun";
                break;
            case MeansOfDeath::Machinegun:
                message = "was machinegunned by";
                break;
            case MeansOfDeath::Grenade:
                message = "was popped by";
                messageAddition = "'s grenade";
                break;
            case MeansOfDeath::GrenadeSplash:
                message = "was shredded by";
                messageAddition = "'s shrapnel";
                break;
            case MeansOfDeath::Rocket:
                message = "ate";
                messageAddition = "'s rocket";
                break;
            case MeansOfDeath::RocketSplash:
                message = "almost dodged";
                messageAddition = "'s rocket";
                break;
            case MeansOfDeath::TeleFrag:
                message = "tried to invade";
                messageAddition = "'s personal space";
                break;
        }

        // In case we have a message, proceed.
        if (message != "") {
            // Print it.
            gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s.\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClient()->persistent.netname, messageAddition.c_str());
            
            // WID: Old piec of code // if (deathmatch->value) {
            if (friendlyFire)
                attacker->GetClient()->respawn.score--;
            else
                attacker->GetClient()->respawn.score++;
            // WID: Old piec of code // }
            return;
        }
    }

    // Check for monster deaths here.
    if (attacker->GetServerFlags() & EntityServerFlags::Monster) {
        // Fill in message here
        // aka if (attacker->classname == "monster_1337h4x0r")
        // Then we do...
        // Also we gotta adjust that ->classname thing, but this is a template, cheers :)
        //if (!message.empty()) {
        //    gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClassName(), messageAddition.c_str());
        //    if (deathmatch->value) {
        //        if (friendlyFire)
        //            attacker->GetClient()->respawn.score--;
        //        else
        //            attacker->GetClient()->respawn.score++;
        //    }
        //    return;
        //}
    }

    // Inform the client died.
    gi.BPrintf(PRINT_MEDIUM, "%s died.\n", self->GetClient()->persistent.netname);

    // WID: This was an old piece of code, keeping it so people know what..// if (deathmatch->value)
    // Get the client, and change its current score.
    self->GetClient()->respawn.score--;
}