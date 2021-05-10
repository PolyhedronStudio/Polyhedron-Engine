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
#include "../g_local.h"     // SVGame.
#include "../effects.h"     // Effects.
#include "../utils.h"       // Util funcs.
#include "client.h"         // Include Player Client header.
#include "hud.h"            // Include HUD header.

#include "sharedgame/sharedgame.h" // Include SG Base.
#include "sharedgame/pmove.h"   // Include SG PMove.
#include "animations.h"         // Include Player Client Animations.

void ClientUserinfoChanged(entity_t *ent, char *userinfo);

void SP_misc_teleporter_dest(entity_t *ent);

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

void SP_FixCoopSpots(entity_t *self)
{
    entity_t *spot;
    vec3_t  d;

    spot = NULL;

    while (1) {
        spot = G_Find(spot, FOFS(classname), "info_player_start");
        if (!spot)
            return;
        if (!spot->targetName)
            continue;
        VectorSubtract(self->state.origin, spot->state.origin, d);
        if (VectorLength(d) < 384) {
            if ((!self->targetName) || Q_stricmp(self->targetName, spot->targetName) != 0) {
//              gi.DPrintf("FixCoopSpots changed %s at %s targetName from %s to %s\n", self->classname, Vec3ToString(self->state.origin), self->targetName, spot->targetName);
                self->targetName = spot->targetName;
            }
            return;
        }
    }
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

void SP_CreateCoopSpots(entity_t *self)
{
    entity_t *spot;

    if (Q_stricmp(level.mapname, "security") == 0) {
        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->state.origin[0] = 188 - 64;
        spot->state.origin[1] = -164;
        spot->state.origin[2] = 80;
        spot->targetName = "jail3";
        spot->state.angles[1] = 90;

        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->state.origin[0] = 188 + 64;
        spot->state.origin[1] = -164;
        spot->state.origin[2] = 80;
        spot->targetName = "jail3";
        spot->state.angles[1] = 90;

        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->state.origin[0] = 188 + 128;
        spot->state.origin[1] = -164;
        spot->state.origin[2] = 80;
        spot->targetName = "jail3";
        spot->state.angles[1] = 90;

        return;
    }
}


//=======================================================================





qboolean IsFemale(entity_t *ent)
{
    char        *info;

    if (!ent->client)
        return false;

    info = Info_ValueForKey(ent->client->persistent.userinfo, "gender");
    if (info[0] == 'f' || info[0] == 'F')
        return true;
    return false;
}

qboolean IsNeutral(entity_t *ent)
{
    char        *info;

    if (!ent->client)
        return false;

    info = Info_ValueForKey(ent->client->persistent.userinfo, "gender");
    if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
        return true;
    return false;
}

void ClientUpdateObituary(entity_t *self, entity_t *inflictor, entity_t *attacker)
{
    int         mod;
    const char        *message; // C++20: STRING: Added const to char*
    const char        *message2; // C++20: STRING: Added const to char*
    qboolean    ff;

    if (coop->value && attacker->client)
        meansOfDeath |= MOD_FRIENDLY_FIRE;

    if (deathmatch->value || coop->value) {
        ff = meansOfDeath & MOD_FRIENDLY_FIRE;
        mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
        message = NULL;
        message2 = "";

        switch (mod) {
        case MOD_SUICIDE:
            message = "suicides";
            break;
        case MOD_FALLING:
            message = "cratered";
            break;
        case MOD_CRUSH:
            message = "was squished";
            break;
        case MOD_WATER:
            message = "sank like a rock";
            break;
        case MOD_SLIME:
            message = "melted";
            break;
        case MOD_LAVA:
            message = "does a back flip into the lava";
            break;
        case MOD_EXPLOSIVE:
        case MOD_BARREL:
            message = "blew up";
            break;
        case MOD_EXIT:
            message = "found a way out";
            break;
        case MOD_TARGET_BLASTER:
            message = "got blasted";
            break;
        case MOD_BOMB:
        case MOD_SPLASH:
        case MOD_TRIGGER_HURT:
            message = "was in the wrong place";
            break;
        }
        if (attacker == self) {
            switch (mod) {
            case MOD_HELD_GRENADE:
                message = "tried to put the pin back in";
                break;
            case MOD_HG_SPLASH:
            case MOD_G_SPLASH:
                if (IsNeutral(self))
                    message = "tripped on its own grenade";
                else if (IsFemale(self))
                    message = "tripped on her own grenade";
                else
                    message = "tripped on his own grenade";
                break;
            case MOD_R_SPLASH:
                if (IsNeutral(self))
                    message = "blew itself up";
                else if (IsFemale(self))
                    message = "blew herself up";
                else
                    message = "blew himself up";
                break;
            case MOD_BFG_BLAST:
                message = "should have used a smaller gun";
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
            gi.BPrintf(PRINT_MEDIUM, "%s %s.\n", self->client->persistent.netname, message);
            if (deathmatch->value)
                self->client->respawn.score--;
            self->enemy = NULL;
            return;
        }

        self->enemy = attacker;
        if (attacker && attacker->client) {
            switch (mod) {
            case MOD_BLASTER:
                message = "was blasted by";
                break;
            case MOD_SHOTGUN:
                message = "was gunned down by";
                break;
            case MOD_SSHOTGUN:
                message = "was blown away by";
                message2 = "'s super shotgun";
                break;
            case MOD_MACHINEGUN:
                message = "was machinegunned by";
                break;
            case MOD_CHAINGUN:
                message = "was cut in half by";
                message2 = "'s chaingun";
                break;
            case MOD_GRENADE:
                message = "was popped by";
                message2 = "'s grenade";
                break;
            case MOD_G_SPLASH:
                message = "was shredded by";
                message2 = "'s shrapnel";
                break;
            case MOD_ROCKET:
                message = "ate";
                message2 = "'s rocket";
                break;
            case MOD_R_SPLASH:
                message = "almost dodged";
                message2 = "'s rocket";
                break;
            case MOD_HYPERBLASTER:
                message = "was melted by";
                message2 = "'s hyperblaster";
                break;
            case MOD_RAILGUN:
                message = "was railed by";
                break;
            case MOD_BFG_LASER:
                message = "saw the pretty lights from";
                message2 = "'s BFG";
                break;
            case MOD_BFG_BLAST:
                message = "was disintegrated by";
                message2 = "'s BFG blast";
                break;
            case MOD_BFG_EFFECT:
                message = "couldn't hide from";
                message2 = "'s BFG";
                break;
            case MOD_HANDGRENADE:
                message = "caught";
                message2 = "'s handgrenade";
                break;
            case MOD_HG_SPLASH:
                message = "didn't see";
                message2 = "'s handgrenade";
                break;
            case MOD_HELD_GRENADE:
                message = "feels";
                message2 = "'s pain";
                break;
            case MOD_TELEFRAG:
                message = "tried to invade";
                message2 = "'s personal space";
                break;
            }
            if (message) {
                gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->client->persistent.netname, message, attacker->client->persistent.netname, message2);
                if (deathmatch->value) {
                    if (ff)
                        attacker->client->respawn.score--;
                    else
                        attacker->client->respawn.score++;
                }
                return;
            }
        }
    }

    gi.BPrintf(PRINT_MEDIUM, "%s died.\n", self->client->persistent.netname);
    if (deathmatch->value)
        self->client->respawn.score--;
}


void Touch_Item(entity_t *ent, entity_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon(entity_t *self)
{
    gitem_t     *item;
    entity_t     *drop;
    qboolean    quad;
    float       spread = 1.5f;

    if (!deathmatch->value)
        return;

    item = self->client->persistent.weapon;
    if (! self->client->persistent.inventory[self->client->ammoIndex])
        item = NULL;
    if (item && (strcmp(item->pickupName, "Blaster") == 0))
        item = NULL;

    if (item) {
        self->client->aimAngles[vec3_t::Yaw] -= spread;
        drop = Drop_Item(self, item);
        self->client->aimAngles[vec3_t::Yaw] += spread;
        drop->spawnFlags = DROPPED_PLAYER_ITEM;
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
void InitClientPersistant(gclient_t *client)
{
	gitem_t     *item;

	memset(&client->persistent, 0, sizeof(client->persistent));

	item = FindItem("Blaster");
	client->persistent.selected_item = ITEM_INDEX(item);
	client->persistent.inventory[client->persistent.selected_item] = 1;

	client->persistent.weapon = item;

    client->persistent.health         = 100;
    client->persistent.maxHealth     = 100;

    client->persistent.max_bullets    = 200;
    client->persistent.max_shells     = 100;
    client->persistent.max_rockets    = 50;
    client->persistent.max_grenades   = 50;
    client->persistent.max_cells      = 200;
    client->persistent.max_slugs      = 50;

    client->persistent.connected = true;
}


void InitClientResp(gclient_t *client)
{
    memset(&client->respawn, 0, sizeof(client->respawn));
    client->respawn.enterframe = level.frameNumber;
    client->respawn.coop_respawn = client->persistent;
}

/*
==================
SaveClientData

Some information that should be persistant, like health,
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData(void)
{
    int     i;
    entity_t *ent;

    for (i = 0 ; i < game.maxClients ; i++) {
        ent = &g_edicts[1 + i];
        if (!ent->inUse)
            continue;
        game.clients[i].persistent.health = ent->health;
        game.clients[i].persistent.maxHealth = ent->maxHealth;
        game.clients[i].persistent.savedFlags = (ent->flags & (EntityFlags::GodMode | EntityFlags::NoTarget | EntityFlags::PowerArmor));
        if (coop->value && ent->client)
            game.clients[i].persistent.score = ent->client->respawn.score;
    }
}

void FetchClientEntData(entity_t *ent)
{
    ent->health = ent->client->persistent.health;
    ent->maxHealth = ent->client->persistent.maxHealth;
    ent->flags |= ent->client->persistent.savedFlags;
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
float   PlayersRangeFromSpot(entity_t *spot)
{
    entity_t *player;
    float   bestplayerdistance;
    vec3_t  v;
    int     n;
    float   playerdistance;


    bestplayerdistance = 9999999;

    for (n = 1; n <= maxClients->value; n++) {
        player = &g_edicts[n];

        if (!player->inUse)
            continue;

        if (player->health <= 0)
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
entity_t *SelectRandomDeathmatchSpawnPoint(void)
{
    entity_t *spot, *spot1, *spot2;
    int     count = 0;
    int     selection;
    float   range, range1, range2;

    spot = NULL;
    range1 = range2 = 99999;
    spot1 = spot2 = NULL;

    while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
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
        spot = G_Find(spot, FOFS(classname), "info_player_deathmatch");
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
entity_t *SelectFarthestDeathmatchSpawnPoint(void)
{
    entity_t *bestspot;
    float   bestdistance, bestplayerdistance;
    entity_t *spot;


    spot = NULL;
    bestspot = NULL;
    bestdistance = 0;
    while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
        bestplayerdistance = PlayersRangeFromSpot(spot);

        if (bestplayerdistance > bestdistance) {
            bestspot = spot;
            bestdistance = bestplayerdistance;
        }
    }

    if (bestspot) {
        return bestspot;
    }

    // if there is a player just spawned on each and every start spot
    // we have no choice to turn one into a telefrag meltdown
    spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");

    return spot;
}

entity_t *SelectDeathmatchSpawnPoint(void)
{
    if ((int)(dmflags->value) & DeathMatchFlags::SpawnFarthest)
        return SelectFarthestDeathmatchSpawnPoint();
    else
        return SelectRandomDeathmatchSpawnPoint();
}


entity_t *SelectCoopSpawnPoint(entity_t *ent)
{
    int     index;
    entity_t *spot = NULL;
    const char    *target; // C++20: STRING: Added const to char*

    index = ent->client - game.clients;

    // player 0 starts in normal player spawn point
    if (!index)
        return NULL;

    spot = NULL;

    // assume there are four coop spots at each spawnpoint
    while (1) {
        spot = G_Find(spot, FOFS(classname), "info_player_coop");
        if (!spot)
            return NULL;    // we didn't have enough...

        target = spot->targetName;
        if (!target)
            target = "";
        if (Q_stricmp(game.spawnpoint, target) == 0) {
            // this is a coop spawn point for one of the clients here
            index--;
            if (!index)
                return spot;        // this is it
        }
    }


    return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void    SelectSpawnPoint(entity_t *ent, vec3_t &origin, vec3_t &angles)
{
    entity_t *spot = NULL;

    if (deathmatch->value)
        spot = SelectDeathmatchSpawnPoint();
    else if (coop->value)
        spot = SelectCoopSpawnPoint(ent);

    // find a single player start spot
    if (!spot) {
        while ((spot = G_Find(spot, FOFS(classname), "info_player_start")) != NULL) {
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
                spot = G_Find(spot, FOFS(classname), "info_player_start");
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

void body_die(entity_t *self, entity_t *inflictor, entity_t *attacker, int damage, const vec3_t& point)
{
    int n;

    if (self->health < -40) {
        gi.Sound(self, CHAN_BODY, gi.SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n = 0; n < 4; n++)
            ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        self->state.origin[2] -= 48;
        ThrowClientHead(self, damage);
        self->takeDamage = TakeDamage::No;
    }
}

void CopyToBodyQue(entity_t *ent)
{
    entity_t     *body;

    gi.UnlinkEntity(ent);

    // grab a body que and cycle to the next one
    body = &g_edicts[game.maxClients + level.body_que + 1];
    level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

    // send an effect on the removed body
    if (body->state.modelIndex) {
        gi.WriteByte(svg_temp_entity);
        gi.WriteByte(TempEntityEvent::Blood);
        gi.WritePosition(body->state.origin);
        gi.WriteDirection(vec3_origin);
        gi.Multicast(&body->state.origin, MultiCast::PVS);
    }

    gi.UnlinkEntity(body);
    body->state = ent->state;
    body->state.number = body - g_edicts;
    body->state.event = EntityEvent::OtherTeleport;

    body->serverFlags = ent->serverFlags;
    VectorCopy(ent->mins, body->mins);
    VectorCopy(ent->maxs, body->maxs);
    VectorCopy(ent->absMin, body->absMin);
    VectorCopy(ent->absMax, body->absMax);

    body->size = ent->size; // VectorCopy(ent->size, body->size);
    body->velocity = ent->velocity; // VectorCopy(ent->velocity, body->velocity);
    body->avelocity = ent->avelocity; //  VectorCopy(ent->avelocity, body->avelocity);
    body->solid = ent->solid;
    body->clipMask = ent->clipMask;
    body->owner = ent->owner;
    body->moveType = ent->moveType;
    body->groundEntityPtr = ent->groundEntityPtr;

    body->Die = body_die;
    body->takeDamage = TakeDamage::Yes;

    gi.LinkEntity(body);
}

void RespawnClient(entity_t *self)
{
    if (deathmatch->value || coop->value) {
        // spectator's don't leave bodies
        if (self->moveType != MoveType::NoClip && self->moveType != MoveType::Spectator)
            CopyToBodyQue(self);
        self->serverFlags &= ~EntityServerFlags::NoClient;
        PutClientInServer(self);

        // add a teleportation effect
        self->state.event = EntityEvent::PlayerTeleport;

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
 * only called when persistent.spectator changes
 * note that resp.spectator should be the opposite of persistent.spectator here
 */
void spectator_respawn(entity_t *ent)
{
    int i, numspec;

    // If the user wants to become a spectator, make sure he doesn't
    // exceed max_spectators
    if (ent->client->persistent.spectator) {
        // Test if the spectator password was correct, if not, error and return.
        char *value = Info_ValueForKey(ent->client->persistent.userinfo, "spectator");
        if (*spectator_password->string &&
            strcmp(spectator_password->string, "none") &&
            strcmp(spectator_password->string, value)) {
            // Report error message by centerprinting it to client.
            gi.CPrintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");

            // Enable spectator state.
            ent->client->persistent.spectator = false;

            // Let the client go out of its spectator mode by using a StuffCmd.
            gi.StuffCmd(ent, "spectator 0\n");
            return;
        }

        // Count actual active spectators
        for (i = 1, numspec = 0; i <= maxClients->value; i++)
            if (g_edicts[i].inUse && g_edicts[i].client->persistent.spectator)
                numspec++;

        if (numspec >= maxspectators->value) {
            // Report error message by centerprinting it to client.
            gi.CPrintf(ent, PRINT_HIGH, "Server spectator limit is full.\n");

            // Enable spectator state.
            ent->client->persistent.spectator = false;

            // Let the client go out of its spectator mode by using a StuffCmd.
            gi.StuffCmd(ent, "spectator 0\n");
            return;
        }
    } else {
        // He was a spectator and wants to join the game 
        // He must have the right password
        // Test if the spectator password was correct, if not, error and return.
        char *value = Info_ValueForKey(ent->client->persistent.userinfo, "password");
        if (*password->string && strcmp(password->string, "none") &&
            strcmp(password->string, value)) {
            // Report error message by centerprinting it to client.
            gi.CPrintf(ent, PRINT_HIGH, "Password incorrect.\n");

            // Enable spectator state.
            ent->client->persistent.spectator = true;

            // Let the client go in its spectator mode by using a StuffCmd.
            gi.StuffCmd(ent, "spectator 1\n");
            return;
        }
    }

    // clear client on respawn
    ent->client->respawn.score = ent->client->persistent.score = 0;

    ent->serverFlags &= ~EntityServerFlags::NoClient;
    PutClientInServer(ent);

    // add a teleportation effect
    if (!ent->client->persistent.spectator)  {
        // send effect
        gi.WriteByte(svg_muzzleflash);
        gi.WriteShort(ent - g_edicts);
        gi.WriteByte(MuzzleFlashType::Login);
        gi.Multicast(&ent->state.origin, MultiCast::PVS);

        // hold in place briefly
        ent->client->playerState.pmove.flags = PMF_TIME_TELEPORT;
        ent->client->playerState.pmove.time = 14;
    }

    ent->client->respawnTime = level.time;

    if (ent->client->persistent.spectator)
        gi.BPrintf(PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->persistent.netname);
    else
        gi.BPrintf(PRINT_HIGH, "%s joined the game\n", ent->client->persistent.netname);
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer(entity_t *ent)
{
    int     index;
    vec3_t  spawn_origin, spawn_angles;
    gclient_t   *client;
    int     i;
    client_persistant_t saved;
    client_respawn_t    resp;

    // find a spawn point
    // do it before setting health back up, so farthest
    // ranging doesn't count this client
    SelectSpawnPoint(ent, spawn_origin, spawn_angles);

    index = ent - g_edicts - 1;
    client = ent->client;

    // deathmatch wipes most client data every spawn
    if (deathmatch->value) {
        char        userinfo[MAX_INFO_STRING];

        resp = client->respawn;
        memcpy(userinfo, client->persistent.userinfo, sizeof(userinfo));
        InitClientPersistant(client);
        ClientUserinfoChanged(ent, userinfo);
    } else if (coop->value) {
//      int         n;
        char        userinfo[MAX_INFO_STRING];

        resp = client->respawn;
        memcpy(userinfo, client->persistent.userinfo, sizeof(userinfo));
        // this is kind of ugly, but it's how we want to handle keys in coop
//      for (n = 0; n < game.num_items; n++)
//      {
//          if (itemlist[n].flags & IT_KEY)
//              resp.coop_respawn.inventory[n] = client->persistent.inventory[n];
//      }
        client->persistent = resp.coop_respawn;
        ClientUserinfoChanged(ent, userinfo);
        if (resp.score > client->persistent.score)
            client->persistent.score = resp.score;
    } else {
        resp = {};
    }

    // clear everything but the persistant data
    saved = client->persistent;
    memset(client, 0, sizeof(*client));
    client->persistent = saved;
    if (client->persistent.health <= 0)
        InitClientPersistant(client);
    client->respawn = resp;

    // copy some data from the client to the entity
    FetchClientEntData(ent);

    // clear entity values
    ent->groundEntityPtr = NULL;
    ent->client = &game.clients[index];
    ent->takeDamage = TakeDamage::Aim;
    ent->moveType = MoveType::Walk;
    ent->viewHeight = 22;
    ent->inUse = true;
    ent->classname = "player";
    ent->mass = 200;
    ent->solid = Solid::BoundingBox;
    ent->deadFlag = DEAD_NO;
    ent->air_finished = level.time + 12;
    ent->clipMask = CONTENTS_MASK_PLAYERSOLID;
    ent->model = "players/male/tris.md2";
    ent->Pain = Player_Pain;
    ent->Die = Player_Die;
    ent->waterLevel = 0;
    ent->waterType = 0;
    ent->flags &= ~EntityFlags::NoKnockBack;
    ent->serverFlags &= ~EntityServerFlags::DeadMonster;

    ent->mins = vec3_scale(PM_MINS, PM_SCALE);
    ent->maxs = vec3_scale(PM_MAXS, PM_SCALE);
    ent->velocity = vec3_zero();

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

    client->playerState.gunIndex = gi.ModelIndex(client->persistent.weapon->viewModel);

    // Clear certain entity state values
    ent->state.effects = 0;
    ent->state.modelIndex = 255;        // Will use the skin specified model
    ent->state.modelIndex2 = 255;       // Custom gun model
    // sknum is player num and weapon number
    // weapon number will be added in changeweapon
    ent->state.skinNumber = ent - g_edicts - 1;

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

    // spawn a spectator
    if (client->persistent.spectator) {
        client->chaseTarget = NULL;

        client->respawn.spectator = true;

        ent->moveType = MoveType::Spectator;
        ent->solid = Solid::Not;
        ent->serverFlags |= EntityServerFlags::NoClient;
        ent->client->playerState.gunIndex = 0;
        gi.LinkEntity(ent);
        return;
    } else
        client->respawn.spectator = false;

    if (!KillBox(ent)) {
        // could't spawn in?
    }

    gi.LinkEntity(ent);

    // force the current weapon up
    client->newweapon = client->persistent.weapon;
    ChangeWeapon(ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch(entity_t *ent)
{
    G_InitEntity(ent);

    InitClientResp(ent->client);

    // locate ent at a spawn point
    PutClientInServer(ent);

    if (level.intermissiontime) {
        HUD_MoveClientToIntermission(ent);
    } else {
        // send effect
        gi.WriteByte(svg_muzzleflash);
        gi.WriteShort(ent - g_edicts);
        gi.WriteByte(MuzzleFlashType::Login);
        gi.Multicast(&ent->state.origin, MultiCast::PVS);
    }

    gi.BPrintf(PRINT_HIGH, "%s entered the game\n", ent->client->persistent.netname);

    // make sure all view stuff is valid
    ClientEndServerFrame(ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin(entity_t *ent)
{
    int     i;

    ent->client = game.clients + (ent - g_edicts - 1);

    if (deathmatch->value) {
        ClientBeginDeathmatch(ent);
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
    } else {
        // a spawn point will completely reinitialize the entity
        // except for the persistant data that was initialized at
        // ClientConnect() time
        G_InitEntity(ent);
        ent->classname = "player";
        InitClientResp(ent->client);
        PutClientInServer(ent);
    }

    if (level.intermissiontime) {
        HUD_MoveClientToIntermission(ent);
    } else {
        // send effect if in a multiplayer game
        if (game.maxClients > 1) {
            gi.WriteByte(svg_muzzleflash);
            gi.WriteShort(ent - g_edicts);
            gi.WriteByte(MuzzleFlashType::Login);
            gi.Multicast(&ent->state.origin, MultiCast::PVS);

            gi.BPrintf(PRINT_HIGH, "%s entered the game\n", ent->client->persistent.netname);
        }
    }

    // Called to make sure all view stuff is valid
    ClientEndServerFrame(ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged(entity_t *ent, char *userinfo)
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

    // set spectator
    s = Info_ValueForKey(userinfo, "spectator");
    // spectators are only supported in deathmatch
    if (deathmatch->value && *s && strcmp(s, "0"))
        ent->client->persistent.spectator = true;
    else
        ent->client->persistent.spectator = false;

    // set skin
    s = Info_ValueForKey(userinfo, "skin");

    playernum = ent - g_edicts - 1;

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
qboolean ClientConnect(entity_t *ent, char *userinfo)
{
    char    *value;

    // check to see if they are on the banned IP list
    value = Info_ValueForKey(userinfo, "ip");
    if (SV_FilterPacket(value)) {
        Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
        return false;
    }

    // check for a spectator
    value = Info_ValueForKey(userinfo, "spectator");
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
            if (g_edicts[i + 1].inUse && g_edicts[i + 1].client->persistent.spectator)
                numspec++;

        if (numspec >= maxspectators->value) {
            Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
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
    ent->client = game.clients + (ent - g_edicts - 1);

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (ent->inUse == false) {
        // clear the respawning variables
        InitClientResp(ent->client);
        if (!game.autosaved || !ent->client->persistent.weapon)
            InitClientPersistant(ent->client);
    }

    ClientUserinfoChanged(ent, userinfo);

    if (game.maxClients > 1)
        gi.DPrintf("%s connected\n", ent->client->persistent.netname);

    ent->serverFlags = 0; // make sure we start with known default
    ent->client->persistent.connected = true;
    return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect(entity_t *ent)
{
    //int     playernum;

    if (!ent->client)
        return;

    gi.BPrintf(PRINT_HIGH, "%s disconnected\n", ent->client->persistent.netname);

    // send effect
    if (ent->inUse) {
        gi.WriteByte(svg_muzzleflash);
        gi.WriteShort(ent - g_edicts);
        gi.WriteByte(MuzzleFlashType::Logout);
        gi.Multicast(&ent->state.origin, MultiCast::PVS);
    }

    gi.UnlinkEntity(ent);
    ent->state.modelIndex = 0;
    ent->state.sound = 0;
    ent->state.event = 0;
    ent->state.effects = 0;
    ent->solid = Solid::Not;
    ent->inUse = false;
    ent->classname = "disconnected";
    ent->client->persistent.connected = false;

    // FIXME: don't break skins on corpses, etc
    //playernum = ent-g_edicts-1;
    //gi.configstring (ConfigStrings::PlayerSkins+playernum, "");
}


//==============================================================


entity_t *pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t q_gameabi PM_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end)
{
    if (pm_passent->health > 0)
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
void ClientThink(entity_t *ent, ClientUserCommand *clientUserCommand)
{
    gclient_t* client = nullptr;
    entity_t* other = nullptr;

    PlayerMove pm = {};
    
    // Sanity checks.
    if (!ent) {
        Com_Error(ErrorType::ERR_DROP, "%s: has a NULL *ent!\n", __FUNCTION__);
    }
    if (!ent->client)
        Com_Error(ErrorType::ERR_DROP, "%s: *ent has no client to think with!\n", __FUNCTION__);

    // Store the current entity to be run from G_RunFrame.
    level.current_entity = ent;

    // Fetch the entity client.
    client = ent->client;


    if (level.intermissiontime) {
        client->playerState.pmove.type = EnginePlayerMoveType::Freeze;
        // can exit intermission after five seconds
        if (level.time > level.intermissiontime + 5.0
            && (clientUserCommand->moveCommand.buttons & BUTTON_ANY))
            level.exitintermission = true;
        return;
    }

    pm_passent = ent;

    if (ent->client->chaseTarget) {
        // Angles are fetched from the client we are chasing.
        client->respawn.commandViewAngles[0] = clientUserCommand->moveCommand.viewAngles[0];
        client->respawn.commandViewAngles[1] = clientUserCommand->moveCommand.viewAngles[1];
        client->respawn.commandViewAngles[2] = clientUserCommand->moveCommand.viewAngles[2];
    } else {

        // set up for pmove
        memset(&pm, 0, sizeof(pm));

        if ( ent->moveType == MoveType::NoClip )
            client->playerState.pmove.type = PlayerMoveType::Noclip;
        else if ( ent->moveType == MoveType::Spectator )
            client->playerState.pmove.type = PlayerMoveType::Spectator;
        else if ( ent->state.modelIndex != 255 )
            client->playerState.pmove.type = EnginePlayerMoveType::Gib;
        else if ( ent->deadFlag )
            client->playerState.pmove.type = EnginePlayerMoveType::Dead;
        else
            client->playerState.pmove.type = PlayerMoveType::Normal;

        client->playerState.pmove.gravity = sv_gravity->value;
        
        // Copy over the latest playerstate its pmove state.
        pm.state = client->playerState.pmove;

        // Move over entity state values into the player move state so it is up to date.
        pm.state.origin = ent->state.origin;
        pm.state.velocity = ent->velocity;
        pm.clientUserCommand = *clientUserCommand;
        pm.groundEntityPtr = ent->groundEntityPtr;
        pm.Trace = PM_Trace;
        pm.PointContents = gi.PointContents;

        // perform a pmove
        PMove(&pm);

        // Save client pmove results.
        client->playerState.pmove = pm.state;

        // Move over needed results to the entity and its state.
        ent->state.origin = pm.state.origin;
        ent->velocity = pm.state.velocity;
        ent->mins = pm.mins;
        ent->maxs = pm.maxs;
        ent->viewHeight = pm.state.viewOffset[2];
        ent->waterLevel = pm.waterLevel;
        ent->waterType = pm.waterType;

        // Check for jumping sound.
        if (ent->groundEntityPtr && !pm.groundEntityPtr && (pm.clientUserCommand.moveCommand.upMove >= 10) && (pm.waterLevel == 0)) {
            gi.Sound(ent, CHAN_VOICE, gi.SoundIndex("*jump1.wav"), 1, ATTN_NORM, 0);
            PlayerNoise(ent, ent->state.origin, PNOISE_SELF);
        }

        ent->groundEntityPtr = pm.groundEntityPtr;

        // Copy over the user command angles so they are stored for respawns.
        // (Used when going into a new map etc.)
        client->respawn.commandViewAngles[0] = clientUserCommand->moveCommand.viewAngles[0];
        client->respawn.commandViewAngles[1] = clientUserCommand->moveCommand.viewAngles[1];
        client->respawn.commandViewAngles[2] = clientUserCommand->moveCommand.viewAngles[2];

        // Store entity link count in case we have a ground entity pointer.
        if (pm.groundEntityPtr)
            ent->groundEntityLinkCount = pm.groundEntityPtr->linkCount;

        // Special treatment for angles in case we are dead. Target the killer entity yaw angle.
        if (ent->deadFlag) {
            client->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
            client->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
            client->playerState.pmove.viewAngles[vec3_t::Yaw] = client->killerYaw;
        } else {
            // Otherwise, store the resulting view angles accordingly.
            client->aimAngles = pm.viewAngles;
            client->playerState.pmove.viewAngles = pm.viewAngles;
        }

        gi.LinkEntity(ent);

        // Only check for trigger and object touches if not one of these movetypes.
        if (ent->moveType != MoveType::NoClip && ent->moveType != MoveType::Spectator)
            UTIL_TouchTriggers(ent);

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
            if (!other->Touch)
                continue;
            other->Touch(other, ent, NULL, NULL);
        }

    }

    client->oldButtons = client->buttons;
    client->buttons = clientUserCommand->moveCommand.buttons;
    client->latchedButtons |= client->buttons & ~client->oldButtons;

    // save light level the player is standing on for
    // monster sighting AI
    ent->lightLevel = clientUserCommand->moveCommand.lightLevel;

    // fire weapon from final position if needed
    if (client->latchedButtons & BUTTON_ATTACK) {
        if (client->respawn.spectator) {

            client->latchedButtons = 0;

            if (client->chaseTarget) {
                client->chaseTarget = NULL;
                client->playerState.pmove.flags &= ~PMF_NO_PREDICTION;
            } else
                GetChaseTarget(ent);

        } else if (!client->weaponThunk) {
            client->weaponThunk = true;
            Think_Weapon(ent);
        }
    }

    if (client->respawn.spectator) {
        if (clientUserCommand->moveCommand.upMove >= 10) {
            if (!(client->playerState.pmove.flags & PMF_JUMP_HELD)) {
                client->playerState.pmove.flags |= PMF_JUMP_HELD;
                if (client->chaseTarget)
                    ChaseNext(ent);
                else
                    GetChaseTarget(ent);
            }
        } else
            client->playerState.pmove.flags &= ~PMF_JUMP_HELD;
    }

    // update chase cam if being followed
    for (int i = 1; i <= maxClients->value; i++) {
        other = g_edicts + i;
        if (other->inUse && other->client->chaseTarget == ent)
            UpdateChaseCam(other);
    }
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame(entity_t *ent)
{
    gclient_t   *client;
    int         buttonMask;

    if (level.intermissiontime)
        return;

    client = ent->client;

    if (deathmatch->value &&
        client->persistent.spectator != client->respawn.spectator &&
        (level.time - client->respawnTime) >= 5) {
        spectator_respawn(ent);
        return;
    }

    // run weapon animations if it hasn't been done by a ucmd_t
    if (!client->weaponThunk && !client->respawn.spectator)
        Think_Weapon(ent);
    else
        client->weaponThunk = false;

    if (ent->deadFlag) {
        // wait for any button just going down
        if (level.time > client->respawnTime) {
            // in deathmatch, only wait for attack button
            if (deathmatch->value)
                buttonMask = BUTTON_ATTACK;
            else
                buttonMask = -1;

            if ((client->latchedButtons & buttonMask) ||
                (deathmatch->value && ((int)dmflags->value & DeathMatchFlags::ForceRespawn))) {
                RespawnClient(ent);
                client->latchedButtons = 0;
            }
        }
        return;
    }

    // add player trail so monsters can follow
    if (!deathmatch->value)
        if (!visible(ent, PlayerTrail_LastSpot()))
            PlayerTrail_Add(ent->state.oldOrigin);

    client->latchedButtons = 0;
}
