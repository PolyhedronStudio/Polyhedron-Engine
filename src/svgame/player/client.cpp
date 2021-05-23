/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
// General.
#include "../g_local.h"     // SVGame.
#include "../chasecamera.h" // Chase Camera.
#include "../effects.h"     // Effects.
#include "../entities.h"    // Entities.
#include "../utils.h"       // Util funcs.
#include "client.h"         // Include Player Client header.
#include "hud.h"            // Include HUD header.
#include "view.h"           // View header.
#include "weapons.h"

// ClassEntities.
#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/PlayerClient.h"

// Shared Game.
#include "sharedgame/sharedgame.h" // Include SG Base.
#include "sharedgame/pmove.h"   // Include SG PMove.
#include "animations.h"         // Include Player Client Animations.

void SVG_ClientUserinfoChanged(Entity *ent, char *userinfo);


//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetName as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

void SP_FixCoopSpots(Entity *self)
{
    Entity *spot;
    vec3_t  d;

    spot = NULL;

    while (1) {
        spot = SVG_Find(spot, FOFS(className), "info_player_start");
        if (!spot)
            return;
        if (!spot->targetName)
            continue;
        VectorSubtract(self->state.origin, spot->state.origin, d);
        if (VectorLength(d) < 384) {
            if ((!self->targetName) || Q_stricmp(self->targetName, spot->targetName) != 0) {
//              gi.DPrintf("FixCoopSpots changed %s at %s targetName from %s to %s\n", self->className, Vec3ToString(self->state.origin), self->targetName, spot->targetName);
                self->targetName = spot->targetName;
            }
            return;
        }
    }
}

//=======================================================================





qboolean IsFemale(SVGBaseEntity *ent)
{
    char        *info;

    if (!ent->GetClient())
        return false;

    info = Info_ValueForKey(ent->GetClient()->persistent.userinfo, "gender");
    if (info[0] == 'f' || info[0] == 'F')
        return true;
    return false;
}

qboolean IsNeutral(SVGBaseEntity *ent)
{
    char        *info;

    if (!ent->GetClient())
        return false;

    info = Info_ValueForKey(ent->GetClient()->persistent.userinfo, "gender");
    if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
        return true;
    return false;
}

void SVG_ClientUpdateObituary(SVGBaseEntity *self, SVGBaseEntity *inflictor, SVGBaseEntity *attacker)
{
    int         mod;
    const char        *message; // C++20: STRING: Added const to char*
    const char        *message2; // C++20: STRING: Added const to char*
    qboolean    ff;

    if (coop->value && attacker->GetClient())
        meansOfDeath |= MeansOfDeath::FriendlyFire;

    if (deathmatch->value || coop->value) {
        ff = meansOfDeath & MeansOfDeath::FriendlyFire;
        mod = meansOfDeath & ~MeansOfDeath::FriendlyFire;
        message = NULL;
        message2 = "";

        switch (mod) {
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
        if (attacker == self) {
            switch (mod) {
            case MeansOfDeath::GrenadeSplash:
                if (IsNeutral(self))
                    message = "tripped on its own grenade";
                else if (IsFemale(self))
                    message = "tripped on her own grenade";
                else
                    message = "tripped on his own grenade";
                break;
            case MeansOfDeath::RocketSplash:
                if (IsNeutral(self))
                    message = "blew itself up";
                else if (IsFemale(self))
                    message = "blew herself up";
                else
                    message = "blew himself up";
                break;
            default:
                if (IsNeutral(self))
                    message = "killed itself";
                else if (IsFemale(self))
                    message = "killed herself";
                else
                    message = "killed himself";
                break;
            }
        }
        if (message) {
            gi.BPrintf(PRINT_MEDIUM, "%s %s.\n", self->GetClient()->persistent.netname, message);
            if (deathmatch->value)
                self->GetClient()->respawn.score--;
            self->SetEnemy(NULL);
            return;
        }

        self->SetEnemy(attacker);
        if (attacker && attacker->GetClient()) {
            switch (mod) {
            case MeansOfDeath::Blaster:
                message = "was blasted by";
                break;
            case MeansOfDeath::Shotgun:
                message = "was gunned down by";
                break;
            case MeansOfDeath::SuperShotgun:
                message = "was blown away by";
                message2 = "'s super shotgun";
                break;
            case MeansOfDeath::Machinegun:
                message = "was machinegunned by";
                break;
            case MeansOfDeath::Grenade:
                message = "was popped by";
                message2 = "'s grenade";
                break;
            case MeansOfDeath::GrenadeSplash:
                message = "was shredded by";
                message2 = "'s shrapnel";
                break;
            case MeansOfDeath::Rocket:
                message = "ate";
                message2 = "'s rocket";
                break;
            case MeansOfDeath::RocketSplash:
                message = "almost dodged";
                message2 = "'s rocket";
                break;
            case MeansOfDeath::TeleFrag:
                message = "tried to invade";
                message2 = "'s personal space";
                break;
            }
            if (message) {
                gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message, attacker->GetClient()->persistent.netname, message2);
                if (deathmatch->value) {
                    if (ff)
                        attacker->GetClient()->respawn.score--;
                    else
                        attacker->GetClient()->respawn.score++;
                }
                return;
            }
        }
    }

    gi.BPrintf(PRINT_MEDIUM, "%s died.\n", self->GetClient()->persistent.netname);
    if (deathmatch->value)
        self->GetClient()->respawn.score--;
}

void SVG_TossClientWeapon(PlayerClient *playerClient)
{
    gitem_t     *item;
    Entity     *drop;
    float       spread = 1.5f;

    if (!deathmatch->value)
        return;

    item = playerClient->GetActiveWeapon();
    if (!playerClient->GetClient()->persistent.inventory[playerClient->GetClient()->ammoIndex])
        item = NULL;
    if (item && (strcmp(item->pickupName, "Blaster") == 0))
        item = NULL;

    if (item) {
        playerClient->GetClient()->aimAngles[vec3_t::Yaw] -= spread;
        drop = SVG_DropItem(playerClient->GetServerEntity(), item);
        playerClient->GetClient()->aimAngles[vec3_t::Yaw] += spread;
        drop->spawnFlags = ItemSpawnFlags::DroppedPlayerItem;
    }
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant(GameClient *client)
{
	gitem_t     *item = NULL;

    if (!client)
        return;

	//memset(&client->persistent, 0, sizeof(client->persistent));
    client->persistent = {};

	item = SVG_FindItemByPickupName("Blaster");
	client->persistent.selectedItem = ITEM_INDEX(item);
	client->persistent.inventory[client->persistent.selectedItem] = 1;

	client->persistent.activeWeapon = item;

    client->persistent.health         = 100;
    client->persistent.maxHealth     = 100;

    client->persistent.maxBullets    = 200;
    client->persistent.maxShells     = 100;
    client->persistent.maxRockets    = 50;
    client->persistent.maxGrenades   = 50;
    client->persistent.maxCells      = 200;
    client->persistent.maxSlugs      = 50;

    client->persistent.isConnected = true;
}


void InitClientResp(GameClient *client)
{
    if (!client)
        return;

    client->respawn = {};
    client->respawn.enterFrame = level.frameNumber;
    client->respawn.persistentCoopRespawn = client->persistent;
}

/*
==================
SVG_SaveClientData

Some information that should be persistant, like health,
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SVG_SaveClientData(void)
{
    int     i;
    Entity *ent;

    for (i = 0 ; i < game.maxClients ; i++) {
        ent = &g_entities[1 + i];
        if (!ent->inUse)
            continue;
        game.clients[i].persistent.health = ent->classEntity->GetHealth();
        game.clients[i].persistent.maxHealth = ent->classEntity->GetMaxHealth();
        game.clients[i].persistent.savedFlags = (ent->flags & (EntityFlags::GodMode | EntityFlags::NoTarget | EntityFlags::PowerArmor));
        if (coop->value && ent->client)
            game.clients[i].persistent.score = ent->client->respawn.score;
    }
}

void SVG_FetchClientData(Entity *ent)
{
    ent->classEntity->SetHealth(ent->client->persistent.health);
    ent->classEntity->SetMaxHealth(ent->client->persistent.maxHealth);
    ent->classEntity->SetFlags(ent->classEntity->GetFlags() | ent->client->persistent.savedFlags);
    if (coop->value && ent->client)
        ent->client->respawn.score = ent->client->persistent.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float   PlayersRangeFromSpot(Entity *spot)
{
    Entity *player;
    float   bestplayerdistance;
    vec3_t  v;
    int     n;
    float   playerdistance;


    bestplayerdistance = 9999999;

    for (n = 1; n <= maxClients->value; n++) {
        player = &g_entities[n];

        if (!player->inUse)
            continue;

        if (player->classEntity && player->classEntity->GetHealth() <= 0)
            continue;

        VectorSubtract(spot->state.origin, player->state.origin, v);
        playerdistance = VectorLength(v);

        if (playerdistance < bestplayerdistance)
            bestplayerdistance = playerdistance;
    }

    return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
Entity *SelectRandomDeathmatchSpawnPoint(void)
{
    Entity *spot, *spot1, *spot2;
    int     count = 0;
    int     selection;
    float   range, range1, range2;

    spot = NULL;
    range1 = range2 = 99999;
    spot1 = spot2 = NULL;

    while ((spot = SVG_Find(spot, FOFS(className), "info_player_deathmatch")) != NULL) {
        count++;
        range = PlayersRangeFromSpot(spot);
        if (range < range1) {
            range1 = range;
            spot1 = spot;
        } else if (range < range2) {
            range2 = range;
            spot2 = spot;
        }
    }

    if (!count)
        return NULL;

    if (count <= 2) {
        spot1 = spot2 = NULL;
    } else
        count -= 2;

    selection = rand() % count;

    spot = NULL;
    do {
        spot = SVG_Find(spot, FOFS(className), "info_player_deathmatch");
        if (spot == spot1 || spot == spot2)
            selection++;
    } while (selection--);

    return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
Entity *SelectFarthestDeathmatchSpawnPoint(void)
{
    Entity *bestSpawnLocationEntity = NULL;
    float   bestdistance = 0;
    float  bestplayerdistance = 0;
    Entity *spawnLocationEntity = NULL;

    while ((spawnLocationEntity = SVG_Find(spawnLocationEntity, FOFS(className), "info_player_deathmatch")) != NULL) {
        bestplayerdistance = PlayersRangeFromSpot(spawnLocationEntity);

        if (bestplayerdistance > bestdistance) {
            bestSpawnLocationEntity = spawnLocationEntity;
            bestdistance = bestplayerdistance;
        }
    }

    if (bestSpawnLocationEntity) {
        return bestSpawnLocationEntity;
    }

    // if there is a player just spawned on each and every start spot
    // we have no choice to turn one into a telefrag meltdown
    spawnLocationEntity = SVG_Find(NULL, FOFS(className), "info_player_deathmatch");

    return spawnLocationEntity;
}

Entity *SelectDeathmatchSpawnPoint(void)
{
    if ((int)(dmflags->value) & DeathMatchFlags::SpawnFarthest)
        return SelectFarthestDeathmatchSpawnPoint();
    else
        return SelectRandomDeathmatchSpawnPoint();
}


Entity *SelectCoopSpawnPoint(Entity *ent)
{
    int clientIndex = 0;
    Entity *spawnPointEntity = NULL;
    const char *target; // C++20: STRING: Added const to char*

    clientIndex = ent->client - game.clients;

    // Player 0 starts in normal player spawn point
    if (!clientIndex)
        return NULL;

    spawnPointEntity = NULL;

    // Assume there are four coop spots at each spawnpoint
    while (1) {
        spawnPointEntity = SVG_Find(spawnPointEntity, FOFS(className), "info_player_coop");
        if (!spawnPointEntity)
            return NULL;    // we didn't have enough...

        target = spawnPointEntity->targetName;
        if (!target)
            target = "";
        if (Q_stricmp(game.spawnpoint, target) == 0) {
            // this is a coop spawn point for one of the clients here
            clientIndex--;
            if (!clientIndex)
                return spawnPointEntity;        // this is it
        }
    }

    return spawnPointEntity;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void    SelectSpawnPoint(Entity *ent, vec3_t &origin, vec3_t &angles)
{
    Entity *spot = NULL;

    if (deathmatch->value)
        spot = SelectDeathmatchSpawnPoint();
    else if (coop->value)
        spot = SelectCoopSpawnPoint(ent);

    // find a single player start spot
    if (!spot) {
        while ((spot = SVG_Find(spot, FOFS(className), "info_player_start")) != NULL) {
            if (!game.spawnpoint[0] && !spot->targetName)
                break;

            if (!game.spawnpoint[0] || !spot->targetName)
                continue;

            if (Q_stricmp(game.spawnpoint, spot->targetName) == 0)
                break;
        }

        if (!spot) {
            if (!game.spawnpoint[0]) {
                // there wasn't a spawnpoint without a target, so use any
                spot = SVG_Find(spot, FOFS(className), "info_player_start");
            }
            if (!spot)
                gi.Error("Couldn't find spawn point %s", game.spawnpoint);
        }
    }

    if (spot) {
        origin = spot->state.origin;
        origin.z += 9;
        angles = spot->state.angles;
    }
}

//======================================================================

void body_die(Entity *self, Entity *inflictor, Entity *attacker, int damage, const vec3_t& point)
{
    int n;

    if (self->classEntity && self->classEntity->GetHealth() < -40) {
        gi.Sound(self, CHAN_BODY, gi.SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n = 0; n < 4; n++)
            ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        self->state.origin.z -= 48;
        ThrowClientHead(self, damage);
        self->takeDamage = TakeDamage::No;
    }
}

void CopyToBodyQue(Entity *ent)
{
    Entity     *body;

    gi.UnlinkEntity(ent);

    // grab a body que and cycle to the next one
    body = &g_entities[game.maxClients + level.bodyQue + 1];
    level.bodyQue = (level.bodyQue + 1) % BODY_QUEUE_SIZE;

    // send an effect on the removed body
    if (body->state.modelIndex) {
        gi.WriteByte(SVG_CMD_TEMP_ENTITY);
        gi.WriteByte(TempEntityEvent::Blood);
        gi.WritePosition(body->state.origin);
        gi.WriteDirection(vec3_zero());
        gi.Multicast(&body->state.origin, MultiCast::PVS);
    }

    gi.UnlinkEntity(body);
    body->state = ent->state;
    body->state.number = body - g_entities;
    body->state.eventID = EntityEvent::OtherTeleport;

    body->serverFlags = ent->serverFlags;
    VectorCopy(ent->mins, body->mins);
    VectorCopy(ent->maxs, body->maxs);
    VectorCopy(ent->absMin, body->absMin);
    VectorCopy(ent->absMax, body->absMax);

    body->size = ent->size; // VectorCopy(ent->size, body->size);
    //body->velocity = ent->classEntity->GetVelocity(); // VectorCopy(ent->velocity, body->velocity);
    //body->angularVelocity = ent->classEntity->GetAngularVelocity(); //  VectorCopy(ent->angularVelocity, body->angularVelocity);
    body->solid = ent->solid;
    body->clipMask = ent->clipMask;
    body->owner = ent->owner;
    body->classEntity->SetMoveType(ent->classEntity->GetMoveType());
    body->classEntity->SetGroundEntity(ent->classEntity->GetGroundEntity());

    //body->Die = body_die;
    body->takeDamage = TakeDamage::Yes;

    gi.LinkEntity(body);
}

void SVG_RespawnClient(Entity *self)
{
    if (deathmatch->value || coop->value) {
        // isSpectator's don't leave bodies
        if (self->classEntity->GetMoveType() != MoveType::NoClip && self->classEntity->GetMoveType() != MoveType::Spectator)
            CopyToBodyQue(self);
        self->serverFlags &= ~EntityServerFlags::NoClient;
        SVG_PutClientInServer(self);

        // add a teleportation effect
        self->state.eventID = EntityEvent::PlayerTeleport;

        // hold in place briefly
        self->client->playerState.pmove.flags = PMF_TIME_TELEPORT;
        self->client->playerState.pmove.time = 14;

        self->client->respawnTime = level.time;

        return;
    }

    // restart the entire server
    gi.AddCommandString("pushmenu loadgame\n");
}

/*
 * only called when persistent.isSpectator changes
 * note that resp.isSpectator should be the opposite of persistent.isSpectator here
 */
void spectator_respawn(Entity *ent)
{
    int i, numspec;

    // If the user wants to become a isSpectator, make sure he doesn't
    // exceed max_spectators
    if (ent->client->persistent.isSpectator) {
        // Test if the isSpectator password was correct, if not, error and return.
        char *value = Info_ValueForKey(ent->client->persistent.userinfo, "isSpectator");
        if (*spectator_password->string &&
            strcmp(spectator_password->string, "none") &&
            strcmp(spectator_password->string, value)) {
            // Report error message by centerprinting it to client.
            gi.CPrintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");

            // Enable isSpectator state.
            ent->client->persistent.isSpectator = false;

            // Let the client go out of its isSpectator mode by using a StuffCmd.
            gi.StuffCmd(ent, "isSpectator 0\n");
            return;
        }

        // Count actual active spectators
        for (i = 1, numspec = 0; i <= maxClients->value; i++)
            if (g_entities[i].inUse && g_entities[i].client->persistent.isSpectator)
                numspec++;

        if (numspec >= maxspectators->value) {
            // Report error message by centerprinting it to client.
            gi.CPrintf(ent, PRINT_HIGH, "Server isSpectator limit is full.\n");

            // Enable isSpectator state.
            ent->client->persistent.isSpectator = false;

            // Let the client go out of its isSpectator mode by using a StuffCmd.
            gi.StuffCmd(ent, "isSpectator 0\n");
            return;
        }
    } else {
        // He was a isSpectator and wants to join the game 
        // He must have the right password
        // Test if the isSpectator password was correct, if not, error and return.
        char *value = Info_ValueForKey(ent->client->persistent.userinfo, "password");
        if (*password->string && strcmp(password->string, "none") &&
            strcmp(password->string, value)) {
            // Report error message by centerprinting it to client.
            gi.CPrintf(ent, PRINT_HIGH, "Password incorrect.\n");

            // Enable isSpectator state.
            ent->client->persistent.isSpectator = true;

            // Let the client go in its isSpectator mode by using a StuffCmd.
            gi.StuffCmd(ent, "isSpectator 1\n");
            return;
        }
    }

    // clear client on respawn
    ent->client->respawn.score = ent->client->persistent.score = 0;

    ent->serverFlags &= ~EntityServerFlags::NoClient;
    SVG_PutClientInServer(ent);

    // add a teleportation effect
    if (!ent->client->persistent.isSpectator)  {
        // send effect
        gi.WriteByte(SVG_CMD_MUZZLEFLASH);
        gi.WriteShort(ent - g_entities);
        gi.WriteByte(MuzzleFlashType::Login);
        gi.Multicast(&ent->state.origin, MultiCast::PVS);

        // hold in place briefly
        ent->client->playerState.pmove.flags = PMF_TIME_TELEPORT;
        ent->client->playerState.pmove.time = 14;
    }

    ent->client->respawnTime = level.time;

    if (ent->client->persistent.isSpectator)
        gi.BPrintf(PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->persistent.netname);
    else
        gi.BPrintf(PRINT_HIGH, "%s joined the game\n", ent->client->persistent.netname);
}

//==============================================================


/*
===========
SVG_PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void SVG_PutClientInServer(Entity *ent)
{
    int     index;
    vec3_t  spawn_origin, spawn_angles;
    GameClient   *client;
    int     i;

    ClientRespawnData    resp;

    // find a spawn point
    // do it before setting health back up, so farthest
    // ranging doesn't count this client
    SelectSpawnPoint(ent, spawn_origin, spawn_angles);

    index = ent - g_entities - 1;
    client = ent->client;

    // deathmatch wipes most client data every spawn
    if (deathmatch->value) {
        char        userinfo[MAX_INFO_STRING];

        // Store the respawn values.
        resp = client->respawn;

        // Store user info.
        memcpy(userinfo, client->persistent.userinfo, sizeof(userinfo));

        // Initialize a fresh client persistent state.
        InitClientPersistant(client);

        // Check for changed user info.
        SVG_ClientUserinfoChanged(ent, userinfo);
//    } else if (coop->value) {
////      int         n;
//        char        userinfo[MAX_INFO_STRING];
//
//        resp = client->respawn;
//        memcpy(userinfo, client->persistent.userinfo, sizeof(userinfo));
//        // this is kind of ugly, but it's how we want to handle keys in coop
////      for (n = 0; n < game.numberOfItems; n++)
////      {
////          if (itemlist[n].flags & ItemFlags::IsKey)
////              resp.persistentCoopRespawn.inventory[n] = client->persistent.inventory[n];
////      }
//        client->persistent = resp.persistentCoopRespawn;
//        SVG_ClientUserinfoChanged(ent, userinfo);
//        if (resp.score > client->persistent.score)
//            client->persistent.score = resp.score;
    } else {
        resp = {};
    }

    // Store persistent client data.
    ClientPersistantData saved = client->persistent;

    // Reset the client data.
    *client = {};

    // Restore persistent client data.
    client->persistent = saved;

    // In case of death, initialize a fresh client persistent data.
    if (client->persistent.health <= 0)
        InitClientPersistant(client);

    // Last but not least, respawn.
    client->respawn = resp;

    //
    // Spawn client class entity.
    //
    SVG_FreeClassEntity(ent);

    ent->className = "PlayerClient";
    PlayerClient *playerClientEntity = (PlayerClient*)(ent->classEntity = SVG_SpawnClassEntity(ent, ent->className));
    playerClientEntity->SetClient(&game.clients[index]);
    playerClientEntity->Precache();
    playerClientEntity->Spawn();
    playerClientEntity->PostSpawn();

    // copy some data from the client to the entity
    SVG_FetchClientData(ent);

    //
    // clear entity values
    //
    //ent->groundEntityPtr = NULL;
    //ent->client = &game.clients[index];
    //ent->takeDamage = TakeDamage::Aim;
    //ent->classEntity->SetMoveType(MoveType::Walk);
    //ent->viewHeight = 22;
    //ent->inUse = true;
    //ent->className = "PlayerClient";
    //ent->mass = 200;
    //ent->solid = Solid::BoundingBox;
    //ent->deadFlag = DEAD_NO;
    //ent->airFinishedTime = level.time + 12;
    //ent->clipMask = CONTENTS_MASK_PLAYERSOLID;
    //ent->model = "players/male/tris.md2";
    ////ent->Pain = SVG_Player_Pain;
    ////ent->Die = SVG_Player_Die;
    //ent->waterLevel = 0;
    //ent->waterType = 0;
    //ent->flags &= ~EntityFlags::NoKnockBack;
    //ent->serverFlags &= ~EntityServerFlags::DeadMonster;

    //ent->mins = vec3_scale(PM_MINS, PM_SCALE);
    //ent->maxs = vec3_scale(PM_MAXS, PM_SCALE);
    //ent->velocity = vec3_zero();

    // Clear playerstate values
    //memset(&ent->client->playerState, 0, sizeof(client->playerState));
    client->playerState = {};

    // Assign spawn origin to player state origin.
    client->playerState.pmove.origin = spawn_origin;
    
    // Assign spawn origin to the entity state origin, ensure that it is off-ground.
    ent->state.origin = ent->state.oldOrigin = spawn_origin + vec3_t{ 0.f, 0.f, 1.f };

    // Set FOV, fixed, or custom.
    if (deathmatch->value && ((int)dmflags->value & DeathMatchFlags::FixedFOV)) {
        client->playerState.fov = 90;
    } else {
        client->playerState.fov = atoi(Info_ValueForKey(client->persistent.userinfo, "fov"));
        if (client->playerState.fov < 1)
            client->playerState.fov = 90;
        else if (client->playerState.fov > 160)
            client->playerState.fov = 160;
    }

    client->playerState.gunIndex = gi.ModelIndex(client->persistent.activeWeapon->viewModel);

    // Clear certain entity state values
    ent->state.effects = 0;
    ent->state.modelIndex = 255;        // Will use the skin specified model
    ent->state.modelIndex2 = 255;       // Custom gun model
    // sknum is player num and weapon number
    // weapon number will be added in changeweapon
    ent->state.skinNumber = ent - g_entities - 1;

    ent->state.frame = 0;

    // set the delta angle
    for (i = 0 ; i < 3 ; i++) {
        client->playerState.pmove.deltaAngles[i] = spawn_angles[i] - client->respawn.commandViewAngles[i];
    }

    ent->state.angles[vec3_t::Pitch] = 0;
    ent->state.angles[vec3_t::Yaw] = spawn_angles[vec3_t::Yaw];
    ent->state.angles[vec3_t::Roll] = 0;
    VectorCopy(ent->state.angles, client->playerState.pmove.viewAngles);
    VectorCopy(ent->state.angles, client->aimAngles);

    // spawn a isSpectator
    if (client->persistent.isSpectator) {
        client->chaseTarget = NULL;

        client->respawn.isSpectator = true;

        ent->classEntity->SetMoveType(MoveType::Spectator);
        ent->solid = Solid::Not;
        ent->serverFlags |= EntityServerFlags::NoClient;
        ent->client->playerState.gunIndex = 0;
        gi.LinkEntity(ent);
        return;
    } else
        client->respawn.isSpectator = false;

    if (!SVG_KillBox(ent->classEntity)) {
        // could't spawn in?
    }

    gi.LinkEntity(ent);

    // force the current weapon up
    client->newWeapon = client->persistent.activeWeapon;
    SVG_ChangeWeapon((PlayerClient*)ent->classEntity);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
=====================
*/
void SVG_ClientBeginDeathmatch(Entity *ent)
{
    SVG_InitEntity(ent);

    InitClientResp(ent->client);

    // locate ent at a spawn point
    SVG_PutClientInServer(ent);

    if (level.intermission.time) {
        HUD_MoveClientToIntermission(ent);
    } else {
        // send effect
        gi.WriteByte(SVG_CMD_MUZZLEFLASH);
        gi.WriteShort(ent - g_entities);
        gi.WriteByte(MuzzleFlashType::Login);
        gi.Multicast(&ent->state.origin, MultiCast::PVS);
    }

    gi.BPrintf(PRINT_HIGH, "%s entered the game\n", ent->client->persistent.netname);

    // make sure all view stuff is valid
    SVG_ClientEndServerFrame((PlayerClient*)ent->classEntity);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void SVG_ClientBegin(Entity *ent)
{
    int     i;

    ent->client = game.clients + (ent - g_entities - 1);

    // Spawn client class entity.
    //ent->className = "PlayerClient";
    //
    //// If the client already has an entity class, ditch it.
    //if (ent->classEntity)
    //    delete ent->classEntity;

    //ent->classEntity = new PlayerClient(ent);
    //ent->classEntity->Spawn();

    if (deathmatch->value) {
        SVG_ClientBeginDeathmatch(ent);
        return;
    }

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (ent->inUse == true) { // warning C4805: '==': unsafe mix of type 'qboolean' and type 'bool' in operation
        // the client has cleared the client side viewAngles upon
        // connecting to the server, which is different than the
        // state when the game is saved, so we need to compensate
        // with deltaangles
        for (i = 0 ; i < 3 ; i++)
            ent->client->playerState.pmove.deltaAngles[i] = ent->client->playerState.pmove.viewAngles[i];

        // 
        // If the client already has an entity class, ditch it.
        SVG_FreeClassEntity(ent);

        ent->className = "PlayerClient";
        ent->classEntity = SVG_SpawnClassEntity(ent, ent->className);
        ent->classEntity->Precache();
        ent->classEntity->Spawn();
        ent->classEntity->PostSpawn();
    } else {
        // a spawn point will completely reinitialize the entity
        // except for the persistant data that was initialized at
        // ClientConnect() time
        SVG_InitEntity(ent);
        ent->className = "PlayerClient";
        InitClientResp(ent->client);
        SVG_PutClientInServer(ent);
    }

    if (level.intermission.time) {
        HUD_MoveClientToIntermission(ent);
    } else {
        // send effect if in a multiplayer game
        if (game.maxClients > 1) {
            gi.WriteByte(SVG_CMD_MUZZLEFLASH);
            gi.WriteShort(ent - g_entities);
            gi.WriteByte(MuzzleFlashType::Login);
            gi.Multicast(&ent->state.origin, MultiCast::PVS);

            gi.BPrintf(PRINT_HIGH, "%s entered the game\n", ent->client->persistent.netname);
        }
    }

    // Called to make sure all view stuff is valid
    SVG_ClientEndServerFrame((PlayerClient*)ent->classEntity);
}

/*
===========
SVG_ClientUserinfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void SVG_ClientUserinfoChanged(Entity *ent, char *userinfo)
{
    char    *s;
    int     playernum;

    // check for malformed or illegal info strings
    if (!Info_Validate(userinfo)) {
        strcpy(userinfo, "\\name\\badinfo\\skin\\male/grunt");
    }

    // set name
    s = Info_ValueForKey(userinfo, "name");
    strncpy(ent->client->persistent.netname, s, sizeof(ent->client->persistent.netname) - 1);

    // set isSpectator
    s = Info_ValueForKey(userinfo, "isSpectator");
    // spectators are only supported in deathmatch
    if (deathmatch->value && *s && strcmp(s, "0"))
        ent->client->persistent.isSpectator = true;
    else
        ent->client->persistent.isSpectator = false;

    // set skin
    s = Info_ValueForKey(userinfo, "skin");

    playernum = ent - g_entities - 1;

    // combine name and skin into a configstring
    gi.configstring(ConfigStrings::PlayerSkins + playernum, va("%s\\%s", ent->client->persistent.netname, s));

    // fov
    if (deathmatch->value && ((int)dmflags->value & DeathMatchFlags::FixedFOV)) {
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
    strncpy(ent->client->persistent.userinfo, userinfo, sizeof(ent->client->persistent.userinfo) - 1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean SVG_ClientConnect(Entity *ent, char *userinfo)
{
    char    *value;

    // check to see if they are on the banned IP list
    value = Info_ValueForKey(userinfo, "ip");
    if (SVG_FilterPacket(value)) {
        Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
        return false;
    }

    // check for a isSpectator
    value = Info_ValueForKey(userinfo, "isSpectator");
    if (deathmatch->value && *value && strcmp(value, "0")) {
        int i, numspec;

        if (*spectator_password->string &&
            strcmp(spectator_password->string, "none") &&
            strcmp(spectator_password->string, value)) {
            Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
            return false;
        }

        // count spectators
        for (i = numspec = 0; i < maxClients->value; i++)
            if (g_entities[i + 1].inUse && g_entities[i + 1].client->persistent.isSpectator)
                numspec++;

        if (numspec >= maxspectators->value) {
            Info_SetValueForKey(userinfo, "rejmsg", "Server isSpectator limit is full.");
            return false;
        }
    } else {
        // check for a password
        value = Info_ValueForKey(userinfo, "password");
        if (*password->string && strcmp(password->string, "none") &&
            strcmp(password->string, value)) {
            Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
            return false;
        }
    }


    // they can connect
    ent->client = game.clients + (ent - g_entities - 1);

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (ent->inUse == false) {
        // clear the respawning variables
        InitClientResp(ent->client);
        if (!game.autoSaved || !ent->client->persistent.activeWeapon)
            InitClientPersistant(ent->client);
    }

    SVG_ClientUserinfoChanged(ent, userinfo);

    if (game.maxClients > 1)
        gi.DPrintf("%s connected\n", ent->client->persistent.netname);

    ent->serverFlags = 0; // make sure we start with known default
    ent->client->persistent.isConnected = true;
    return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void SVG_ClientDisconnect(Entity *ent)
{
    //int     playernum;

    if (!ent->client)
        return;

    gi.BPrintf(PRINT_HIGH, "%s disconnected\n", ent->client->persistent.netname);

    // send effect
    if (ent->inUse) {
        gi.WriteByte(SVG_CMD_MUZZLEFLASH);
        gi.WriteShort(ent - g_entities);
        gi.WriteByte(MuzzleFlashType::Logout);
        gi.Multicast(&ent->state.origin, MultiCast::PVS);
    }

    gi.UnlinkEntity(ent);
    ent->state.modelIndex = 0;
    ent->state.sound = 0;
    ent->state.eventID = 0;
    ent->state.effects = 0;
    ent->solid = Solid::Not;
    ent->inUse = false;
    ent->className = "disconnected";
    ent->client->persistent.isConnected = false;

    // FIXME: don't break skins on corpses, etc
    //playernum = ent-g_entities-1;
    //gi.configstring (ConfigStrings::PlayerSkins+playernum, "");
}


//==============================================================


Entity *pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t q_gameabi PM_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end)
{
    if (pm_passent->classEntity && pm_passent->classEntity->GetHealth() > 0)
        return gi.Trace(start, mins, maxs, end, pm_passent, CONTENTS_MASK_PLAYERSOLID);
    else
        return gi.Trace(start, mins, maxs, end, pm_passent, CONTENTS_MASK_DEADSOLID);
}

unsigned CheckBlock(void *b, int c)
{
    int v, i;
    v = 0;
    for (i = 0 ; i < c ; i++)
        v += ((byte *)b)[i];
    return v;
}
void PrintPMove(PlayerMove *pm)
{
    unsigned    c1, c2;

    c1 = CheckBlock(&pm->state, sizeof(pm->state));
    c2 = CheckBlock(&pm->clientUserCommand, sizeof(pm->clientUserCommand));
    Com_Printf("sv %3i:%i %i\n", pm->clientUserCommand.moveCommand.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void SVG_ClientThink(Entity *serverEntity, ClientUserCommand *clientUserCommand)
{
    GameClient* client = nullptr;
    PlayerClient *classEntity = nullptr;
    Entity* other = nullptr;


    PlayerMove pm = {};
    
    // Sanity checks.
    if (!serverEntity) {
        Com_Error(ErrorType::ERR_DROP, "%s: has a NULL *ent!\n", __FUNCTION__);
    }
    if (!serverEntity->client)
        Com_Error(ErrorType::ERR_DROP, "%s: *ent has no client to think with!\n", __FUNCTION__);

    if (!serverEntity->classEntity)
        return;

    // Store the current entity to be run from SVG_RunFrame.
    level.currentEntity = serverEntity->classEntity;

    // Fetch the entity client.
    client = serverEntity->client;

    // Fetch the class entity.
    classEntity = (PlayerClient*)serverEntity->classEntity;

    if (level.intermission.time) {
        client->playerState.pmove.type = EnginePlayerMoveType::Freeze;
        // can exit intermission after five seconds
        if (level.time > level.intermission.time + 5.0
            && (clientUserCommand->moveCommand.buttons & BUTTON_ANY))
            level.intermission.exitIntermission = true;
        return;
    }

    pm_passent = serverEntity;

    if (client->chaseTarget) {
        // Angles are fetched from the client we are chasing.
        client->respawn.commandViewAngles[0] = clientUserCommand->moveCommand.viewAngles[0];
        client->respawn.commandViewAngles[1] = clientUserCommand->moveCommand.viewAngles[1];
        client->respawn.commandViewAngles[2] = clientUserCommand->moveCommand.viewAngles[2];
    } else {

        // set up for pmove
        memset(&pm, 0, sizeof(pm));

        if ( classEntity->GetMoveType() == MoveType::NoClip )
            client->playerState.pmove.type = PlayerMoveType::Noclip;
        else if ( classEntity->GetMoveType() == MoveType::Spectator )
            client->playerState.pmove.type = PlayerMoveType::Spectator;
        else if (classEntity->GetModelIndex() != 255 )
            client->playerState.pmove.type = EnginePlayerMoveType::Gib;
        else if ( classEntity->GetDeadFlag() )
            client->playerState.pmove.type = EnginePlayerMoveType::Dead;
        else
            client->playerState.pmove.type = PlayerMoveType::Normal;

        client->playerState.pmove.gravity = sv_gravity->value;
        
        // Copy over the latest playerstate its pmove state.
        pm.state = client->playerState.pmove;

        // Move over entity state values into the player move state so it is up to date.
        pm.state.origin = classEntity->GetOrigin();
        pm.state.velocity = classEntity->GetVelocity();
        pm.clientUserCommand = *clientUserCommand;
        if (classEntity->GetGroundEntity())
            pm.groundEntityPtr = classEntity->GetGroundEntity()->GetServerEntity();
        else
            pm.groundEntityPtr = NULL;
        pm.Trace = PM_Trace;
        pm.PointContents = gi.PointContents;

        // perform a pmove
        PMove(&pm);

        // Save client pmove results.
        client->playerState.pmove = pm.state;

        // Move over needed results to the entity and its state.
        classEntity->SetOrigin(pm.state.origin);
        classEntity->SetVelocity(pm.state.velocity);
        classEntity->SetMins(pm.mins);
        classEntity->SetMaxs(pm.maxs);
        classEntity->SetViewHeight(pm.state.viewOffset[2]);
        classEntity->SetWaterLevel(pm.waterLevel);
        classEntity->SetWaterType(pm.waterType);

        // Check for jumping sound.
        if (classEntity->GetGroundEntity() && !pm.groundEntityPtr && (pm.clientUserCommand.moveCommand.upMove >= 10) && (pm.waterLevel == 0)) {
            gi.Sound(serverEntity, CHAN_VOICE, gi.SoundIndex("*jump1.wav"), 1, ATTN_NORM, 0);
            SVG_PlayerNoise(classEntity, classEntity->GetOrigin(), PNOISE_SELF);
        }

        if (pm.groundEntityPtr != NULL)
            classEntity->SetGroundEntity(pm.groundEntityPtr->classEntity);
        else
            classEntity->SetGroundEntity(NULL);

        // Copy over the user command angles so they are stored for respawns.
        // (Used when going into a new map etc.)
        client->respawn.commandViewAngles[0] = clientUserCommand->moveCommand.viewAngles[0];
        client->respawn.commandViewAngles[1] = clientUserCommand->moveCommand.viewAngles[1];
        client->respawn.commandViewAngles[2] = clientUserCommand->moveCommand.viewAngles[2];

        // Store entity link count in case we have a ground entity pointer.
        if (pm.groundEntityPtr)
            classEntity->SetGroundEntityLinkCount(pm.groundEntityPtr->linkCount);

        // Special treatment for angles in case we are dead. Target the killer entity yaw angle.
        if (classEntity->GetDeadFlag()) {
            client->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
            client->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
            client->playerState.pmove.viewAngles[vec3_t::Yaw] = client->killerYaw;
        } else {
            // Otherwise, store the resulting view angles accordingly.
            client->aimAngles = pm.viewAngles;
            client->playerState.pmove.viewAngles = pm.viewAngles;
        }

        // Link it back in for collision testing.
        classEntity->LinkEntity();

        // Only check for trigger and object touches if not one of these movetypes.
        if (classEntity->GetMoveType() != MoveType::NoClip && classEntity->GetMoveType() != MoveType::Spectator)
            UTIL_TouchTriggers(classEntity);

        // touch other objects
        int i = 0;
        int j = 0;
        for (i = 0 ; i < pm.numTouchedEntities; i++) {
            other = pm.touchedEntities[i];
            for (j = 0 ; j < i ; j++)
                if (pm.touchedEntities[j] == other)
                    break;
            if (j != i)
                continue;   // duplicated
            //if (!other->Touch)
            //    continue;
            //other->Touch(other, ent, NULL, NULL);
            if (!other->classEntity)
                continue;
            other->classEntity->Touch(other->classEntity, classEntity, NULL, NULL);
        }

    }

    client->oldButtons = client->buttons;
    client->buttons = clientUserCommand->moveCommand.buttons;
    client->latchedButtons |= client->buttons & ~client->oldButtons;

    // save light level the player is standing on for
    // monster sighting AI
    //ent->lightLevel = clientUserCommand->moveCommand.lightLevel;

    // fire weapon from final position if needed
    if (client->latchedButtons & BUTTON_ATTACK) {
        if (client->respawn.isSpectator) {

            client->latchedButtons = 0;

            if (client->chaseTarget) {
                client->chaseTarget = NULL;
                client->playerState.pmove.flags &= ~PMF_NO_PREDICTION;
            } else
                SVG_GetChaseTarget(classEntity);

        } else if (!client->weaponThunk) {
            client->weaponThunk = true;
            SVG_ThinkWeapon(classEntity);
        }
    }

    if (client->respawn.isSpectator) {
        if (clientUserCommand->moveCommand.upMove >= 10) {
            if (!(client->playerState.pmove.flags & PMF_JUMP_HELD)) {
                client->playerState.pmove.flags |= PMF_JUMP_HELD;
                if (client->chaseTarget)
                    SVG_ChaseNext(classEntity);
                else
                    SVG_GetChaseTarget(classEntity);
            }
        } else
            client->playerState.pmove.flags &= ~PMF_JUMP_HELD;
    }

    // update chase cam if being followed
    for (int i = 1; i <= maxClients->value; i++) {
        other = g_entities + i;
        if (other->inUse && other->client->chaseTarget == serverEntity)
            SVG_UpdateChaseCam(classEntity);
    }
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void SVG_ClientBeginServerFrame(Entity *ent)
{
    GameClient   *client;
    int         buttonMask;

    if (level.intermission.time)
        return;

    client = ent->client;

    if (deathmatch->value &&
        client->persistent.isSpectator != client->respawn.isSpectator &&
        (level.time - client->respawnTime) >= 5) {
        spectator_respawn(ent);
        return;
    }

    // run weapon animations if it hasn't been done by a ucmd_t
    if (!client->weaponThunk && !client->respawn.isSpectator)
        SVG_ThinkWeapon((PlayerClient*)ent->classEntity);
    else
        client->weaponThunk = false;

    if (ent->classEntity->GetDeadFlag()) {
        // wait for any button just going down
        if (level.time > client->respawnTime) {
            // in deathmatch, only wait for attack button
            if (deathmatch->value)
                buttonMask = BUTTON_ATTACK;
            else
                buttonMask = -1;

            if ((client->latchedButtons & buttonMask) ||
                (deathmatch->value && ((int)dmflags->value & DeathMatchFlags::ForceRespawn))) {
                SVG_RespawnClient(ent);
                client->latchedButtons = 0;
            }
        }
        return;
    }

    //// add player trail so monsters can follow
    //if (!deathmatch->value)
    //    if (!visible(ent, SVG_PlayerTrail_LastSpot()))
    //        SVG_PlayerTrail_Add(ent->state.oldOrigin);

    client->latchedButtons = 0;
}
