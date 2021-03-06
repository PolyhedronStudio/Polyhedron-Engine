/*
Copyright (C) 1997-2001 Id Software, Inc.

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
// Core.
#include "../ServerGameLocals.h" // Include SVGame header.

// Entities.
#include "../Entities.h"
// Server Game Base Entity.
#include "../Entities/Base/SVGBaseEntity.h"
#include "../Entities/Base/SVGBaseTrigger.h"
#include "../Entities/Base/SVGBaseItem.h"
#include "../Entities/Base/SVGBaseItemWeapon.h"
#include "../Entities/Base/SVGBasePlayer.h"

// GameModes.
#include "../Gamemodes/IGamemode.h"
//#include "../Gamemodes/DefaultGamemode.h"
#include "../Gamemodes/CoopGamemode.h"
#include "../Gamemodes/DeathMatchGamemode.h"

// World.
#include "../World/ServerGameWorld.h"

// Player Client & Hud Header.
#include "Client.h"     // Include Player Client header.
#include "Hud.h"        // Include HUD header.


/*
======================================================================

INTERMISSION

======================================================================
*/

//
//===============
// HUD_MoveClientToIntermission
// 
// Changes the client's movement type to PlayerMoveType::Freeze while setting its
// origin and viewAngles to the previously fetched intermission entity 
// values.
//================
//
void HUD_MoveClientToIntermission(Entity *ent)
{
    //// Ensure it is a valid client entity.
    //if (!ent) {
    //    return;
    //}
    //if (!ent->client) {
    //    return;
    //}

    //if (deathmatch->value || coop->value)
    //    ent->client->showScores = true;

    //// Copy over the previously fetched map intermission entity origin into
    //// the client player states positions.
    //ent->state.origin = level.intermission.origin;
    //ent->client->playerState.pmove.origin = level.intermission.origin;
    //ent->client->playerState.pmove.viewAngles = level.intermission.viewAngle;
    //// Setup the rest of the client player state.
    //ent->client->playerState.pmove.type = EnginePlayerMoveType::Freeze;
    //ent->client->playerState.gunIndex = 0;
    //ent->client->playerState.blend[3] = 0;
    //ent->client->playerState.rdflags &= ~RDF_UNDERWATER;

    //ent->viewHeight = 0;
    //ent->state.modelIndex = 0;
    //ent->state.modelIndex2 = 0;
    //ent->state.modelIndex3 = 0;
    //ent->state.modelIndex = 0;
    //ent->state.effects = 0;
    //ent->state.sound = 0;
    //ent->solid = Solid::Not;

    //// Add the layout in case of a deathmatch or co-op gamemode.
    //if (deathmatch->value || coop->value) {
    //    SVG_HUD_GenerateDMScoreboardLayout(ent, NULL);
    //    gi.Unicast(ent, true);
    //}

}

//===============
// HUD_BeginIntermission
// 
// Begins an intermission process for the given target entity.
//================
void SVG_HUD_BeginIntermission(Entity *targ)
{
    int     i;//, n;
    Entity *client = nullptr;

    //
    // 
    // TODO: Move entire intermission stuff into gamemodes.
    // 
    // 
    // Ensure targ is valid.
    if (!targ) {
        return;
    }

    // If it's already activate there is no use in executing this function again.
    if (level.intermission.time != Frametime::zero()) {
        return;
    }

    // Let the game know we didn't autosave.
    game.autoSaved = false;

    // Respawn any dead clients.
    GetGameMode()->RespawnAllClients();

    // Set intermission time and the map to change to.
    level.intermission.time = level.time;
    //level.intermission.changeMap = targ->map;

    // 
    if (strstr(level.intermission.changeMap, "*")) {
        if (!GetGameMode()->IsClass<CoopGameMode>()) {
            for (i = 0 ; i < maximumclients->value ; i++) {
                client = game.world->GetPODEntities() + 1 + i;
                if (!client->inUse) {
                    continue;
                }

                // strip players of all keys between units
                //for (n = 0; n < MAX_ITEMS; n++) {
                //    if (itemlist[n].flags & ItemFlags::IsKey) {
                //        client->client->persistent.inventory[n] = 0;
                //    }
                //}
            }
        }
    } else {
        if (!GetGameMode()->IsClass<DeathmatchGameMode>()) {
            level.intermission.exitIntermission = 1;     // go immediately to the next level
            return;
        }
    }

    level.intermission.exitIntermission = 0;

    //// Fetch an intermission entity.
    //Entity *intermissionEntity = SVG_Find(NULL, FOFS(classname), "info_player_intermission");
    //if (!intermissionEntity) {
    //    // the map creator forgot to put in an intermission point...
    //    intermissionEntity = SVG_Find(NULL, FOFS(classname), "info_player_start");
    //    if (!intermissionEntity) {
    //        intermissionEntity = SVG_Find(NULL, FOFS(classname), "info_player_deathmatch");
    //    }
    //} else {
    //    // chose one of four spots
    //    i = rand() & 3;
    //    while (i--) {
    //        intermissionEntity = SVG_Find(intermissionEntity, FOFS(classname), "info_player_intermission");
    //        if (!intermissionEntity) {  // wrap around the list 
    //            intermissionEntity = SVG_Find(intermissionEntity, FOFS(classname), "info_player_intermission");
    //        }
    //    }
    //}

    // Setup intermission origin and view angle.
    level.intermission.origin = vec3_zero();// intermissionEntity->state.origin, level.intermission.origin;
    level.intermission.viewAngle = vec3_zero(); //intermissionEntity->state.angles;

    // Initiate the client intermission mode for all clients.
    // (MoveType = PM_FREEZE, positioned at intermission entity view values.)
    for (i = 0 ; i < maximumclients->value ; i++) {
        // Fetch client.
        client = game.world->GetPODEntities() + 1 + i;

        // Ensure a client is in use, otherwise skip it.
        if (!client->inUse)
            continue;

        // Switch to intermission.
        HUD_MoveClientToIntermission(client);
    }
}


/*
==================
SVG_HUD_GenerateDMScoreboardLayout

==================
*/
void SVG_HUD_GenerateDMScoreboardLayout(IServerGameEntity *ent, IServerGameEntity *killer)
{
 //   char    entry[1024];
 //   char    string[1400];
 //   int     stringlength;
 //   int     i, j, k;
 //   int     sorted[MAX_CLIENTS];
 //   int     sortedscores[MAX_CLIENTS];
 //   int     score, total;
 //   int     x, y;
 //   ServerClient   *cl;
 //   Entity     *cl_ent;
 //   const char    *tag; // C++20: STRING: Added const to char*

 //   ServerClient* clients = game.GetClients();

 //   // sort the clients by score
 //   total = 0;
 //   for (i = 0 ; i < game.GetMaxClients() ; i++) {
 //       cl_ent = game.world->GetPODEntities() + 1 + i;
 //       if (!cl_ent->inUse || clients[i].respawn.isSpectator)
 //           continue;
 //       score = clients[i].respawn.score;
 //       for (j = 0 ; j < total ; j++) {
 //           if (score > sortedscores[j])
 //               break;
 //       }
 //       for (k = total ; k > j ; k--) {
 //           sorted[k] = sorted[k - 1];
 //           sortedscores[k] = sortedscores[k - 1];
 //       }
 //       sorted[j] = i;
 //       sortedscores[j] = score;
 //       total++;
 //   }

 //   // print level name and exit rules
 //   string[0] = 0;

 //   stringlength = strlen(string);

 //   // add the clients in sorted order
 //   if (total > 12)
 //       total = 12;

 //   for (i = 0 ; i < total ; i++) {
	//cl = &game.GetClients()[sorted[i]];
 //       cl_ent = game.world->GetPODEntities() + 1 + sorted[i];

 //       x = (i >= 6) ? 160 : 0;
 //       y = 32 + 32 * (i % 6);

 //       // add a dogtag
 //       if (ent && cl_ent == ent->GetPODEntity())
 //           tag = "tag1";
 //       else if (killer && cl_ent == killer->GetPODEntity())
 //           tag = "tag2";
 //       else
 //           tag = NULL;
 //       if (tag) {
 //           Q_snprintf(entry, sizeof(entry),
 //                      "xv %i yv %i picn %s ", x + 32, y, tag);
 //           j = strlen(entry);
 //           if (stringlength + j > 1024)
 //               break;
 //           strcpy(string + stringlength, entry);
 //           stringlength += j;
 //       }

 //       // send the layout
 //       Q_snprintf(entry, sizeof(entry),
 //                  "client %i %i %i %i %i %i ",
 //                  x, y, sorted[i], cl->respawn.score, cl->ping, (level.frameNumber - cl->respawn.enterGameTimestamp) / 600);
 //       j = strlen(entry);
 //       if (stringlength + j > 1024)
 //           break;
 //       strcpy(string + stringlength, entry);
 //       stringlength += j;
 //   }

 //   gi.MSG_WriteUint8(ServerGameCommand::Layout);//gi.WriteByte(ServerGameCommand::Layout);
 //   gi.MSG_WriteString(string);//gi.WriteString(string);
}


/*
==================
HUD_SendDMScoreboardMessage

Sends the deatchmatch scoreboard svc_layout message.
==================
*/
void HUD_SendDMScoreboardMessage(SVGBaseEntity *ent)
{
    // WID: Putting this check here for future issue preventing.
    // Truth is, this stuff has to go when we got RMLUI :)
    if (!ent)
        return;

    SVG_HUD_GenerateDMScoreboardLayout(ent, ent->GetEnemy());
    gi.Unicast(ent->GetPODEntity(), true);
}


/*
==================
SVG_Command_Score_f

Display the scoreboard
==================
*/
void SVG_Command_Score_f(SVGBasePlayer *player, ServerClient *client) {
    // Hide inventory display.
    client->showInventory = false;

    // Don't show scores if not in one of the following game modes.
    if (!GetGameMode()->IsClass<DeathmatchGameMode>() && !GetGameMode()->IsClass<CoopGameMode>()) {
        return;
    }

    // If score display was up, hide it and terminate this function to prevent another display of scores.
    if (client->showScores) {
        client->showScores = false;
        return;
    }

    // Show score display.
    client->showScores = true;
    HUD_SendDMScoreboardMessage(player);
}


//=======================================================================

//
//===============
// SVG_HUD_SetClientStats
// 
// Sets the entities client, player state, status array with the current
// frame game state data. Such as ammo, etc, it also index/precaches images
// and audio if required.
//================
//
void SVG_HUD_SetClientStats(SVGBasePlayer* player, ServerClient* client) {
//    gitem_t* item;

    // Ensure ent is valid.
    if (!player || !player->IsSubclassOf<SVGBasePlayer>() || !client) {
        return;
    }

    /**
    *   Health.
    **/    
    client->playerState.stats[ PlayerStats::HealthIcon ]	= 0;
    client->playerState.stats[ PlayerStats::Health ]		= player->GetHealth();

    /**
    *   Primary Ammo.
    **/
    if (!client->primaryAmmoIndex /* || !ent->client->persistent.inventory[ent->client->ammoIndex] */) {
        client->playerState.stats[ PlayerStats::PrimaryAmmoIcon ] = 0;
        client->playerState.stats[ PlayerStats::PrimaryAmmo ]     = 0;
    } else {
        client->playerState.stats[ PlayerStats::PrimaryAmmoIcon ] = 1;//gi.PrecacheImage(item->icon);
        client->playerState.stats[ PlayerStats::PrimaryAmmo ]     = client->persistent.inventory.items[client->primaryAmmoIndex];
    }

    /**
    *   Secondary Ammo.
    **/
    if ( !client->secondaryAmmoIndex /* || !ent->client->persistent.inventory[ent->client->ammoIndex] */) {
        client->playerState.stats[ PlayerStats::SecondaryAmmoIcon ] = 0;
        client->playerState.stats[ PlayerStats::SecondaryAmmo ]     = 0;
    } else {
        client->playerState.stats[ PlayerStats::SecondaryAmmoIcon ] = 1;//gi.PrecacheImage(item->icon);
        client->playerState.stats[ PlayerStats::SecondaryAmmo ]     = client->persistent.inventory.items[client->secondaryAmmoIndex];
    }

    /**
    *   Clip Ammo.
    **/
    if ( !client->clipAmmoIndex /* || !ent->client->persistent.inventory[ent->client->ammoIndex] */) {
        client->playerState.stats[ PlayerStats::ClipAmmoIcon ] = 0;
        client->playerState.stats[ PlayerStats::ClipAmmo ]     = 0;
    } else {
        client->playerState.stats[ PlayerStats::ClipAmmoIcon ] = 1;
        client->playerState.stats[ PlayerStats::ClipAmmo ]     = client->persistent.inventory.clipAmmo[client->clipAmmoIndex];
    }

	/**
	*	Pending Weapon.
	**/
	if ( !client->persistent.inventory.nextWeaponID ) {
		client->playerState.stats[ PlayerStats::PendingWeapon ] = 0;
	} else {
		client->playerState.stats[ PlayerStats::PendingWeapon ] = !client->persistent.inventory.nextWeaponID;
	}

	/**
	*	Weapon.
	**/
	if ( !client->persistent.inventory.activeWeaponID ) {
		client->playerState.stats[ PlayerStats::Weapon ] = 0;
	} else {
		client->playerState.stats[ PlayerStats::Weapon ] = !client->persistent.inventory.activeWeaponID;
	}

	/**
	*	Weapon Time.
	**/	
	if ( client->weaponState.timeStamp == GameTime::zero() ) {
		client->playerState.stats[ PlayerStats::Weapon ] = 0;
	} else {
		client->playerState.stats[ PlayerStats::Weapon ] = !client->persistent.inventory.activeWeaponID;
	}


    // Get active weapon.
    //SVGBaseItemWeapon* activeWeapon = player->GetActiveWeapon();

    //// Get primary ammo identifier.
    //uint32_t primaryAmmoIdentifier = (activeWeapon ? activeWeapon->GetPrimaryAmmoIdentifier() : 0);
    //if (primaryAmmoIdentifier != 0) {
	   // client->playerState.stats[STAT_AMMO_ICON] = 1;
    //    client->playerState.stats[STAT_AMMO_PRIMARY] = client->persistent.inventory[primaryAmmoIdentifier];
    //} else {
	   // client->playerState.stats[STAT_AMMO_ICON] = 0;
    //    client->playerState.stats[STAT_AMMO_PRIMARY] = 0;
    //}

    //
    // armor
    //
    client->playerState.stats[PlayerStats::ArmorIcon]   = 0;
    client->playerState.stats[PlayerStats::Armor]       = 0;

    //
    // pickup message
    //
    if (level.time > client->pickupMessageTime) {
        client->playerState.stats[PlayerStats::PickupIcon]     = 0;
        client->playerState.stats[PlayerStats::PickupString]   = 0;
    }

    //
    // timers
    //
    client->playerState.stats[PlayerStats::TimerIcon]   = 0;
    client->playerState.stats[PlayerStats::Timer]       = 0;

    //
    // selected item
    //
    //if (client->persistent.selectedItem == -1) {
	   // client->playerState.stats[STAT_SELECTED_ICON] = 0;
    //} else {
    //	client->playerState.stats[STAT_SELECTED_ICON] = 0;  //gi.PrecacheImage(itemlist[ent->client->persistent.selectedItem].icon);
    //}
    //
    client->playerState.stats[PlayerStats::SelectedItemIcon]    = 0;
    client->playerState.stats[PlayerStats::SelectedItem]        = 0; //client->persistent.selectedItem;

    //
    // layouts
    //
    client->playerState.stats[PlayerStats::Layouts] = 0;

    // Special layout for deathmatch.
    if (GetGameMode()->IsClass<DeathmatchGameMode>()) {
	    if (client->persistent.stats.health <= 0 || level.intermission.time != GameTime::zero() || client->showScores) {
	        client->playerState.stats[PlayerStats::Layouts] |= 1;
	    }
	    if (client->showInventory && client->persistent.stats.health > 0) { 
            client->playerState.stats[PlayerStats::Layouts] |= 2;
        }
    } else {
        if (client->showScores) {
	        client->playerState.stats[PlayerStats::Layouts] |= 1;
        }
	    if (client->showInventory && client->persistent.stats.health > 0) {
	        client->playerState.stats[PlayerStats::Layouts] |= 2;
	    }
    }

    //
    // frags
    //
    client->playerState.stats[PlayerStats::Frags] = client->respawn.score;

    //
    // help icon / current weapon if not shown
    //
    if ((client->persistent.hand == CENTER_HANDED || client->playerState.fov > 91) && client->persistent.inventory.activeWeaponID) {
        //ent->client->playerState.stats[STAT_HELPICON] = gi.PrecacheImage(ent->client->persistent.activeWeapon->GetItemIcon());
	    client->playerState.stats[PlayerStats::HelpIcon] = 0;
    } else {
        client->playerState.stats[PlayerStats::HelpIcon] = 0;
    }

    client->playerState.stats[PlayerStats::IsSpectator] = 0;
}

/*
===============
SVG_HUD_CheckChaseStats
===============
*/
void SVG_HUD_CheckChaseStats(Entity *ent)
{
    // TODO: Iterate over class entities(client range) and use that information instead.
    //int i;

    //if (!ent)    {
    //    return;
    //}

    //for (i = 1; i <= maximumclients->value; i++) {
    //    ServerClient* cl;

    //    cl = game.world->GetPODEntities()[i].client;

    //    if (!game.world->GetPODEntities()[i].inUse || (cl->chaseTarget != ent)) {
    //        continue;
    //    }

    //    //memcpy(cl->playerState.stats, ent->client->playerState.stats, sizeof(cl->playerState.stats));

    //    // Copy the server entities client into the current spectator.
    //    SVG_HUD_SetSpectatorStats(game.world->GetPODEntities() + i);
    //}
}

/*
===============
SVG_HUD_SetSpectatorStats
===============
*/
void SVG_HUD_SetSpectatorStats(SVGBasePlayer *player, ServerClient *client) {
    if (!player || !client) {
        return;
    }

    if (!client->chaseTarget) {
        SVG_HUD_SetClientStats(player, client);
    }

    client->playerState.stats[PlayerStats::IsSpectator] = 1;

    /* layouts are independant in isSpectator */
    client->playerState.stats[PlayerStats::Layouts] = 0;

    if ((client->persistent.stats.health <= 0) || level.intermission.time != GameTime::zero() || client->showScores)
    {
	    client->playerState.stats[PlayerStats::Layouts] |= 1;
    }

    if (client->showInventory && (client->persistent.stats.health > 0))
    {
        client->playerState.stats[PlayerStats::Layouts] |= 2;
    }

    if (client->chaseTarget && client->chaseTarget->inUse)
    {
    	client->playerState.stats[PlayerStats::ChaseClientID] = ConfigStrings::PlayerSkins + (client->chaseTarget - game.world->GetPODEntities()) - 1;
    }
    else
    {
	    client->playerState.stats[PlayerStats::ChaseClientID] = 0;
    }
}

