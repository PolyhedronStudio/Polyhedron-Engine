/*
// LICENSE HERE.

//
// CoopGamemode.cpp
//
//
*/
#include "../ServerGameLocal.h"
#include "../Effects.h"
#include "../Entities.h"
#include "../Utilities.h"

// Entities.
#include "../Entities/Base/SVGBaseEntity.h"
#include "../Entities/Base/SVGBasePlayer.h"

// Game Mode.
#include "CoopGamemode.h"

// World.
#include "../World/Gameworld.h"

//
// Constructor/Deconstructor.
//
CoopGamemode::CoopGamemode() : DefaultGamemode() {

}
CoopGamemode::~CoopGamemode() {

}

//
// Interface functions. 
//
//

qboolean CoopGamemode::CanSaveGame(qboolean isDedicatedServer) {
    // Can't save deathmatch games.
    return true;
}

//
//===============
// CoopGamemode::CanDamage
//
// Template function serves as an example atm.
//===============
//
qboolean CoopGamemode::CanDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor) {
    // Let it be to DefaultGamemode. :)
    return DefaultGamemode::CanDamage(target, inflictor);
}

//===============
// CoopGamemode::ClientUpdateObituary.
// 
//===============
void CoopGamemode::ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) {
    std::string message; // String stating what happened to whichever entity. "suicides", "was squished" etc.
    std::string messageAddition; // String stating what is additioned to it, "'s shrapnell" etc. Funny stuff.

    // If the attacker is a client, we know it was a friendly fire in this coop mode. :)
    if (attacker->GetClient())
        meansOfDeath |= MeansOfDeath::FriendlyFire;

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
            gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClient()->persistent.netname, messageAddition.c_str());

            if (friendlyFire) {
                attacker->GetClient()->respawn.score--;
            } else {
                attacker->GetClient()->respawn.score++;
            }

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
        //    gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClassname(), messageAddition.c_str());
    //        if (friendlyFire)
    //            attacker->GetClient()->respawn.score--;
    //        else
    //            attacker->GetClient()->respawn.score++;
        //    return;
        //}
    }

    // Inform the client died.
    gi.BPrintf(PRINT_MEDIUM, "%s died.\n", self->GetClient()->persistent.netname);

    // WID: This was an old piece of code, keeping it so people know what..// if (deathmatch->value)
    // Get the client, and change its current score.
    self->GetClient()->respawn.score--;
}

//===============
// CoopGamemode::RespawnClient
// 
// Respawns a client after intermission and hitting a button.
//===============
void CoopGamemode::RespawnClient(SVGBasePlayer* player) {
    // Spectator's don't leave bodies
    if (player->GetMoveType() != MoveType::Spectator) {
	    SpawnClientCorpse(player);
    }

    // Remove no client flag so this player becomes visible to clients.
    player->SetServerFlags(player->GetServerFlags() & ~EntityServerFlags::NoClient);

    // Add a teleportation effect
    player->SetEventID(EntityEvent::PlayerTeleport);

    // Hold in place briefly
    ServerClient* serverClient = player->GetClient();

    // Hold in place for 14 frames and set pmove flags to teleport so the player can
    // respawn somewhere safe without it interpolating its positions.
    serverClient->playerState.pmove.flags = PMF_TIME_TELEPORT;
    serverClient->playerState.pmove.time = 14;

    // Setup respawn time.
    serverClient->respawnTime = level.time;

    // Last but not least, deploy player.
    PlacePlayerInGame(player);
}

//===============
// CoopGamemode::RespawnAllClients
//
// Respawn all valid client entities who's health is < 0.
//===============
void CoopGamemode::RespawnAllClients() {
    // Respawn all valid client entities who's health is < 0.
    for (auto& player : game.world->GetClassEntityRange<0, MAX_EDICTS>()
        | cef::Standard | cef::HasClient | cef::IsSubclassOf<SVGBasePlayer>())
    {
        if (player->GetHealth() < 0) {
            RespawnClient(dynamic_cast<SVGBasePlayer*>(player));
        }
    }
}

//===============
// CoopGamemode::ClientDeath
// 
// Does nothing for this game mode.
//===============
void CoopGamemode::ClientDeath(SVGBasePlayer *player) {
    // Ensure the client is valid.
    if (!player) {
        return;
    }
    // Ensure it is SVGBasePlayer or a sub-class thereof.
    if (!player->IsSubclassOf<SVGBasePlayer>()) {
        return;
    }

    // Fetch server client.
    ServerClient* client = player->GetClient();
    if (!client) {
        return;
    }

    // Clear inventory this is kind of ugly, but it's how we want to handle keys in coop
    for (int32_t i = 0; i < MAX_ITEMS; i++) {
        client->persistent.inventory.items[i] = 0;
    }
    //for (int32_t i = 0; i < game.numberOfItems; i++) {
    //    //if (itemlist[i].flags & ItemFlags::IsKey)
    //    //    client->respawn.persistentCoopRespawn.inventory[i] = client->persistent.inventory[i];
    //    client->persistent.inventory.items[i] = 0;
    //}
}

//===============
// CoopGamemode::StorePlayerPersistentData
// 
// Some information that should be persistant, like health,
// is still stored in the edict structure, so it needs to
// be mirrored out to the client structure before all the
// edicts are wiped.
//===============
void CoopGamemode::StorePlayerPersistentData(void) {
    Entity *ent;

    ServerClient* gameClients = game.GetClients();
    Entity*	  serverEntities = game.world->GetServerEntities();

    for (int32_t i = 0 ; i < game.GetMaxClients() ; i++) {
        ent = &serverEntities[1 + i];
        if (!ent->inUse)
            continue;
        if (!ent->classEntity)
            continue;
	    gameClients[i].persistent.stats.health = ent->classEntity->GetHealth();
	    gameClients[i].persistent.stats.maxHealth = ent->classEntity->GetMaxHealth();
	    gameClients[i].persistent.savedFlags = (ent->classEntity->GetFlags() & (EntityFlags::GodMode | EntityFlags::NoTarget | EntityFlags::PowerArmor));
        if (ent->client)
		    gameClients[i].persistent.score = ent->client->respawn.score;
    }
}

//===============
// CoopGamemode::RespawnClient
// 
// // Fetch client data that was stored between previous entity wipe session
//===============
void CoopGamemode::RestorePlayerPersistentData(SVGBaseEntity* player, ServerClient* client) {
    if (!player || !client) {
        return;
    }

    // Restore player entity information.
    player->SetHealth(client->persistent.stats.health);
    player->SetMaxHealth(client->persistent.stats.maxHealth);
    player->SetFlags(player->GetFlags() | client->persistent.savedFlags);

    // Restore the score the player had stored.
    client->respawn.score = client->persistent.score;
}