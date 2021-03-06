/*
// LICENSE HERE.

//
// DeathmatchGameMode.cpp
//
//
*/
#include "../ServerGameLocals.h"          // SVGame.
#include "../Effects.h"     // Effects.
#include "../Entities.h"    // Entities.
#include "../Utilities.h"       // Util funcs.

// Server Game Base Entity.
#include "../Entities/Base/SVGBasePlayer.h"

// Weapons.h
#include "../Player/Client.h"
#include "../Player/Hud.h"
#include "../Player/Weapons.h"

// Game Mode.
#include "DeathMatchGamemode.h"

// World.
#include "../World/ServerGameWorld.h"

//
// Constructor/Deconstructor.
//
DeathmatchGameMode::DeathmatchGameMode() : DefaultGameMode() {

}

//
// Interface functions. 
//
//

qboolean DeathmatchGameMode::CanSaveGame(qboolean isDedicatedServer) {
    // Can't save deathmatch games.
    return false;
}

//
//===============
// DeathmatchGameMode::CanDamage
//
// Template function serves as an example atm.
//===============
//
qboolean DeathmatchGameMode::CanDamage(IServerGameEntity* target, IServerGameEntity* inflictor) {
    // Let it be to DefaultGameMode. :)
    return DefaultGameMode::CanDamage(target, inflictor);
}

//===============
// DeathmatchGameMode::ClientBegin
// 
// Called when a client is ready to be placed in the game after connecting.
//===============
void DeathmatchGameMode::ClientBegin(PODEntity *svEntity) {
    // Create the player client entity.
    SVGBasePlayer* player = SVGBasePlayer::Create(svEntity);

    // Initialize client respawn data.
    InitializePlayerRespawnData(svEntity->client);
 
    // Put into our server and blast away! (Takes care of spawning gameEntity).
    PlacePlayerInGame(player);

    if (level.intermission.time != GameTime::zero()) {
        HUD_MoveClientToIntermission(svEntity);
    } else {
        gi.MSG_WriteUint8(ServerGameCommand::MuzzleFlash);//WriteByte(ServerGameCommand::MuzzleFlash);
        //gi.WriteShort(serverEntity - g_entities);
        gi.MSG_WriteInt16(player->GetNumber());//WriteShort(player->GetNumber());
        gi.MSG_WriteUint8(MuzzleFlashType::Login);//WriteByte(MuzzleFlashType::Login);
        gi.Multicast(player->GetOrigin(), Multicast::PVS);
    }
    
    // Let it be known, client ... entered the mfkn game!
    gi.BPrintf(PRINT_HIGH, "%s entered the game\n", svEntity->client->persistent.netname);

    // Call ClientEndServerFrame to finish processing for this frame.
    ClientEndServerFrame(player, svEntity->client);
}

void DeathmatchGameMode::PlacePlayerInGame(SVGBasePlayer *player) {
    // Acquire a pointer to the game's clients.
    ServerClient* gameClients = game.GetClients();

    // Find a spawn point
    vec3_t  spawnOrigin = vec3_zero();
    vec3_t  spawnAngles = vec3_zero();


    // Acquire the new client index belonging to this entity.
    int32_t clientIndex = player->GetNumber() - 1;  //ent - g_entities - 1;

    // Acquire pointer to the current client.
    ServerClient* client = player->GetClient();

    // Client user info.
    char userinfo[MAX_INFO_STRING];
    // Store a copy of our respawn data for later use. 
    ClientRespawnData respawnData = client->respawn;
    // Copy over client's user info into our userinfo buffer.
    memcpy(userinfo, client->persistent.userinfo, sizeof(userinfo));
    // Reinitialize persistent data since we are in a fresh spawn.
    InitializePlayerPersistentData(client);
    // Inform of a client user info change.
    ClientUserinfoChanged(player->GetPODEntity(), userinfo);

    // Backup the current client persistent data.
    ClientPersistentData persistentData = client->persistent;
    // Reset the client's information.
    *client = {};
    // Now move its persistent data back into the client's information.
    client->persistent = persistentData;
    // In case the persistent data consists of a dead client, reinitialize it.
    if (client->persistent.stats.health <= 0) {
	    InitializePlayerPersistentData(client);
    }
    // Last but not least, set its respawn data.
    client->respawn = respawnData;

    // Spawn the client again using spawn instead of respawn. (Respawn serves a different use.)
    player->Spawn();

    // Copy some data from the client to the entity
    RestorePlayerPersistentData(player, client);

    // Update the client pointer to match with the client index.
    client = &gameClients[clientIndex];
    // Ready to roll, let's assign it.
    player->SetClient(client);
 
    // Clear playerstate values.
    client->playerState = {};

    if (((int)gamemodeflags->value & GameModeFlags::FixedFOV)) {
        client->playerState.fov = 90;
    } else {
        client->playerState.fov = atoi(Info_ValueForKey(client->persistent.userinfo, "fov"));
        if (client->playerState.fov < 1) {
            client->playerState.fov = 90;
        } else if (client->playerState.fov > 160) {
            client->playerState.fov = 160;
        }
    }

    // Set gun index to whichever was persistent in the previous map (if there was one).
    client->playerState.gunIndex = 0;  //gi.PrecacheModel(client->persistent.activeWeapon->viewModel);
	
    // Select the clients spawn point.
    SelectPlayerSpawnPoint(player, spawnOrigin, spawnAngles);

	// Set the actual player spawn origin. Offset of 1 unit off the ground.
    player->SetOrigin(spawnOrigin);
	// We set the oldOrigin too, since we're spawning at and NOT lerping from a location.
    player->SetOldOrigin(spawnOrigin);
	// Set the entity rotation to spawn angle yaw.
	const vec3_t spawnViewAngles = { 
		0.f, 
		AngleMod( spawnAngles[vec3_t::Yaw] ), 
		0.f 
	};
	player->SetAngles( spawnViewAngles );

	// Setup player move origin to spawnpoint origin.	
    client->playerState.pmove.origin = spawnOrigin;
	// Set the player move state's directional view angles.
	client->playerState.pmove.viewAngles = spawnViewAngles;
	// Calculate the delta angles of the client.
	client->playerState.pmove.deltaAngles = client->playerState.pmove.viewAngles - client->respawn.commandViewAngles;
	client->oldViewAngles = spawnViewAngles;
	client->aimAngles = spawnViewAngles;

    // spawn a spectator in case the client was/is one.
    if (client->persistent.isSpectator) {
        // Nodefault chase target.
        client->chaseTarget = nullptr;

        // Well we knew this but store it in respawn data too.
        client->respawn.isSpectator = true;

        // Movement type is the obvious spectator.
        player->SetMoveType(MoveType::Spectator);

        // No solid.
        player->SetSolid(Solid::OctagonBox);

        // NoClient flag, aka, do not send this entity to other clients. It is invisible to them.
    	player->SetServerFlags(player->GetServerFlags() | EntityServerFlags::NoClient);

        // Ensure it has no gun index, spectators can't shoot after all.
        client->playerState.gunIndex = 0;

        // Last but not least link our entity.
        player->LinkEntity();
        
        // We're done in case of spawning a spectator.
        return;
    } else {
        // Let it be known to respawn that we are not in spectator mode.
        client->respawn.isSpectator = false;
    }

    // Make sure we can spawn.
    if (!SVG_KillBox(player)) {
        // could't spawn in?
    }

    // Link our entity.
    player->LinkEntity();

    // Ensure we change to whichever active weaponID we had.
    player->ChangeWeapon(ItemID::Barehands, false);
}

//===============
// DeathmatchGameMode::ClientUserinfoChanged
// 
//===============
void DeathmatchGameMode::ClientUserinfoChanged(Entity* ent, char* userinfo) {
    char    *s;
    int     playernum;

    // check for malformed or illegal info strings
    if (!Info_Validate(userinfo)) {
        strcpy(userinfo, "\\name\\badinfo\\skin\\male/grunt");
    }

    // set name
    s = Info_ValueForKey(userinfo, "name");
    strncpy(ent->client->persistent.netname, s, sizeof(ent->client->persistent.netname) - 1);

    // set spectator
    s = Info_ValueForKey(userinfo, "spectator");
    // spectators are only supported in deathmatch
    if (*s && strcmp(s, "0"))
        ent->client->persistent.isSpectator = true;
    else
        ent->client->persistent.isSpectator = false;

    // set skin
    s = Info_ValueForKey(userinfo, "skin");

    playernum = ent - game.world->GetPODEntities() - 1;

    // combine name and skin into a configstring
    gi.configstring(ConfigStrings::PlayerSkins + playernum, va("%s\\%s", ent->client->persistent.netname, s));

    // fov
    if (((int)gamemodeflags->value & GameModeFlags::FixedFOV)) {
        ent->client->playerState.fov = 90;
    } else {
        ent->client->playerState.fov = atoi(Info_ValueForKey(userinfo, "fov"));
        if (ent->client->playerState.fov < 1)
            ent->client->playerState.fov = 90;
        else if (ent->client->playerState.fov > 160)
            ent->client->playerState.fov = 160;
    }

    // handedness
    s = Info_ValueForKey(userinfo, "hand");
    if (strlen(s)) {
        ent->client->persistent.hand = atoi(s);
    }

    // save off the userinfo in case we want to check something later
    strncpy(ent->client->persistent.userinfo, userinfo, sizeof(ent->client->persistent.userinfo));
}

//===============
// DefaultGameMode::ClientBeginServerFrame
// 
// Does logic checking for a client's start of a server frame. In case there
// is a "level.intermission.time" set, it'll flat out return.
// 
// This basically allows for the game to disable fetching user input that makes
// our movement tick. And/or shoot weaponry while in intermission time.
//===============
void DeathmatchGameMode::ClientBeginServerFrame(SVGBasePlayer* player, ServerClient* client) {
    // Ensure valid pointers.
    if (!player || !client) {
        return;
    }

    // Ensure we aren't in an intermission time.
    if (level.intermission.time != GameTime::zero())
        return;

    // This has to go ofc.... lol. What it simply does though, is determine whether there is 
    // a need to respawn as spectator.
    if (client->persistent.isSpectator != client->respawn.isSpectator && (level.time - client->respawnTime) >= 5s) {
        RespawnSpectator(player, client);
        return;
    }

    // Run weapon animations in case this has not been done by user input itself.
    // (Idle animations, and general weapon thinking when a weapon is not in action.)
    if (!client->respawn.isSpectator) {
        player->WeaponThink();
    }

    // Check if the player is actually dead or not. If he is, we're going to enact on
    // the user input that's been given to us. When fired, we'll respawn.
    if (player->GetDeadFlag()) {
        // Wait for any button just going down
        if (level.time > client->respawnTime) {
            // In old code, the need to hit a key was only set in DM mode.
            // I figured, let's keep it like this instead.
            //if (deathmatch->value)
            int32_t buttonMask = ButtonBits::PrimaryFire;
            //else
            //buttonMask = -1;

            if ((client->latchedButtons & buttonMask) || ((int32_t)gamemodeflags->value & GameModeFlags::ForceRespawn)) {
                RespawnClient(player);
                client->latchedButtons = 0;
            }
        }
        return;
    }

    //// add player trail so monsters can follow
    //if (!deathmatch->value)
    //    if (!visible(ent, SVG_PlayerTrail_LastSpot()))
    //        SVG_PlayerTrail_Add(ent->state.oldOrigin);

    // Reset the latched buttons.
    client->latchedButtons = 0;
}

/**
*   @brief  Enlists an obituary event.
**/
void DeathmatchGameMode::ClientUpdateObituary(IServerGameEntity* self, IServerGameEntity* inflictor, IServerGameEntity* attacker) {
    std::string message = ""; // String stating what happened to whichever entity. "suicides", "was squished" etc.
    std::string messageAddition = ""; // String stating what is additioned to it, "'s shrapnell" etc. Funny stuff.

    // No friendly fire in DeathMatch.
    //if (coop->value && attacker->client)
    //    meansOfDeath |= MOD_FRIENDLY_FIRE;

    //// Set a bool for whether we got friendly fire.
    //qboolean friendlyFire = meansOfDeath & MeansOfDeath::FriendlyFire;
    qboolean friendlyFire = false;
    // Quickly remove it from meansOfDeath again, our bool is set. This prevents it from 
    // sticking around when we process the next entity/client.
    int32_t finalMeansOfDeath = meansOfDeath;// &~MeansOfDeath::FriendlyFire; // Sum of things, final means of death.

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
        if (!message.empty()) {
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
        if (!message.empty()) {
            gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClassname(), messageAddition.c_str());
            if (friendlyFire) {
                attacker->GetClient()->respawn.score--;
            } else {
                attacker->GetClient()->respawn.score++;
            }

            return;
        }
    }

    //// Inform the client died.
    //gi.BPrintf(PRINT_MEDIUM, "%s died.\n", self->GetClient()->persistent.netname);

    //// WID: This was an old piece of code, keeping it so people know what..// if (deathmatch->value)
    //// Get the client, and change its current score.
    //self->GetClient()->respawn.score--;
}

/**
*   @brief  Respawns a client after intermission and hitting a button.
**/
void DeathmatchGameMode::RespawnClient(SVGBasePlayer* player) {
    // Spectator's don't leave bodies
    if (player->GetMoveType() != MoveType::Spectator) {
        SpawnClientCorpse(player);
    }

    // Remove no client flag.
    player->SetServerFlags(player->GetServerFlags() & ~EntityServerFlags::NoClient);

    // Attach client to player.
    PlacePlayerInGame(player);

    // Give player "Barehands".
    player->GiveWeapon(ItemID::Barehands, 1);

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
}

/**
*   @brief  Respawn all valid client entities who's health is < 0.
**/
void DeathmatchGameMode::RespawnAllClients() {
    for (auto& player : game.world->GetGameEntityRange(0, MAX_WIRED_POD_ENTITIES) | cef::Standard | cef::HasClient | cef::IsSubclassOf<SVGBasePlayer>()) {
        if (player->GetHealth() < 0) {
            RespawnClient(dynamic_cast<SVGBasePlayer*>(player));
        }
    }
}

/**
*   @brief  Does nothing for this game mode.
**/
void DeathmatchGameMode::ClientDeath(SVGBasePlayer *player) {

}

/**
*   @brief  Respawn given player as a spectator.
**/
void DeathmatchGameMode::RespawnSpectator(SVGBasePlayer* player, ServerClient *client) {
    // Ensure this player is valid.
    if (!player || !client) {
        return;
    }

    // Spectator's don't leave bodies
    if (player->GetMoveType() != MoveType::NoClip)
        SpawnClientCorpse(player);

    // Add NoClient to serverflags so other clients won't see this player anymore.
    player->SetServerFlags(player->GetServerFlags() & ~EntityServerFlags::NoClient);

    // Enplace player in world.
    PlacePlayerInGame(player);

    // Add a teleportation effect to the player.
    player->SetEventID(EntityEvent::PlayerTeleport);

    // Hold player in place briefly.
    client->playerState.pmove.flags = PMF_TIME_TELEPORT;
    client->playerState.pmove.time = 14;

    // Set respawn time to current time.
    client->respawnTime = level.time;
}

/**
*   @brief  Sets client into intermission mode like default game mode but
*           also generates a scoreboard display.
**/
void DeathmatchGameMode::StartClientIntermission(SVGBasePlayer* player, ServerClient* client) {
    // Only continue if valid.
    if (!player || !client) {
	    return;
    }

    // Execute default game mode logic.
    DefaultGameMode::StartClientIntermission(player, client);

    // Tell this client to show scores.
    client->showScores = true;

    // Generate scoreboard layout.
    SVG_HUD_GenerateDMScoreboardLayout(player, NULL);

    // Do a reliable unicast.
    gi.Unicast(player->GetPODEntity(), true);
}