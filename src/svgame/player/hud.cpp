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
#include "../g_local.h" // Include SVGame header.
#include "client.h"     // Include Player Client header.
#include "hud.h"        // Include HUD header.


/*
======================================================================

INTERMISSION

======================================================================
*/

//
//===============
// HUD_MoveClientToIntermission
// 
// Changes the client's movement type to PM_FREEZE while setting its
// origin and viewAngles to the previously fetched intermission entity 
// values.
//================
//
void HUD_MoveClientToIntermission(entity_t *ent)
{
    // Ensure it is a valid client entity.
    if (!ent) {
        return;
    }
    if (!ent->client) {
        return;
    }

    if (deathmatch->value || coop->value)
        ent->client->showscores = true;

    // Copy over the previously fetched map intermission entity origin into
    // the client player states positions.
    ent->s.origin = level.intermission_origin;
    ent->client->playerState.pmove.origin = level.intermission_origin;
    ent->client->playerState.viewAngles = level.intermission_angle;
    // Setup the rest of the client player state.
    ent->client->playerState.pmove.type = PM_FREEZE;
    ent->client->playerState.gunindex = 0;
    ent->client->playerState.blend[3] = 0;
    ent->client->playerState.rdflags &= ~RDF_UNDERWATER;

    ent->viewHeight = 0;
    ent->s.modelindex = 0;
    ent->s.modelindex2 = 0;
    ent->s.modelindex3 = 0;
    ent->s.modelindex = 0;
    ent->s.effects = 0;
    ent->s.sound = 0;
    ent->solid = SOLID_NOT;

    // Add the layout in case of a deathmatch or co-op gamemode.
    if (deathmatch->value || coop->value) {
        HUD_GenerateDMScoreboardLayout(ent, NULL);
        gi.Unicast(ent, true);
    }

}

//
//===============
// HUD_BeginIntermission
// 
// Begins an intermission process for the given target entity.
//================
//
void HUD_BeginIntermission(entity_t *targ)
{
    int     i, n;
    entity_t *client = nullptr;

    // Ensure targ is valid.
    if (!targ) {
        return;
    }

    if (level.intermissiontime) {
        return;     // already activated
    }

    game.autosaved = false;

    // respawn any dead clients
    for (i = 0 ; i < maxclients->value ; i++) {
        client = g_edicts + 1 + i;
        if (!client->inUse) {
            continue;
        }
        if (client->health <= 0) {
            RespawnClient(client);
        }
    }

    level.intermissiontime = level.time;
    level.changemap = targ->map;

    if (strstr(level.changemap, "*")) {
        if (coop->value) {
            for (i = 0 ; i < maxclients->value ; i++) {
                client = g_edicts + 1 + i;
                if (!client->inUse) {
                    continue;
                }
                // strip players of all keys between units
                for (n = 0; n < MAX_ITEMS; n++) {
                    if (itemlist[n].flags & IT_KEY) {
                        client->client->pers.inventory[n] = 0;
                    }
                }
            }
        }
    } else {
        if (!deathmatch->value) {
            level.exitintermission = 1;     // go immediately to the next level
            return;
        }
    }

    level.exitintermission = 0;

    // Fetch an intermission entity.
    entity_t *intermissionEntity = G_Find(NULL, FOFS(classname), "info_player_intermission");
    if (!intermissionEntity) {
        // the map creator forgot to put in an intermission point...
        intermissionEntity = G_Find(NULL, FOFS(classname), "info_player_start");
        if (!intermissionEntity) {
            intermissionEntity = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
        }
    } else {
        // chose one of four spots
        i = rand() & 3;
        while (i--) {
            intermissionEntity = G_Find(intermissionEntity, FOFS(classname), "info_player_intermission");
            if (!intermissionEntity) {  // wrap around the list 
                intermissionEntity = G_Find(intermissionEntity, FOFS(classname), "info_player_intermission");
            }
        }
    }

    level.intermission_origin = intermissionEntity->s.origin, level.intermission_origin;
    level.intermission_angle = intermissionEntity->s.angles;

    // Initiate the client intermission mode for all clients.
    // (MoveType = PM_FREEZE, positioned at intermission entity view values.)
    for (i = 0 ; i < maxclients->value ; i++) {
        // Fetch client.
        client = g_edicts + 1 + i;

        // Ensure a client is in use, otherwise skip it.
        if (!client->inUse)
            continue;

        // Switch to intermission.
        HUD_MoveClientToIntermission(client);
    }
}


/*
==================
HUD_GenerateDMScoreboardLayout

==================
*/
void HUD_GenerateDMScoreboardLayout(entity_t *ent, entity_t *killer)
{
    char    entry[1024];
    char    string[1400];
    int     stringlength;
    int     i, j, k;
    int     sorted[MAX_CLIENTS];
    int     sortedscores[MAX_CLIENTS];
    int     score, total;
    int     x, y;
    gclient_t   *cl;
    entity_t     *cl_ent;
    const char    *tag; // C++20: STRING: Added const to char*

    // sort the clients by score
    total = 0;
    for (i = 0 ; i < game.maxclients ; i++) {
        cl_ent = g_edicts + 1 + i;
        if (!cl_ent->inUse || game.clients[i].resp.spectator)
            continue;
        score = game.clients[i].resp.score;
        for (j = 0 ; j < total ; j++) {
            if (score > sortedscores[j])
                break;
        }
        for (k = total ; k > j ; k--) {
            sorted[k] = sorted[k - 1];
            sortedscores[k] = sortedscores[k - 1];
        }
        sorted[j] = i;
        sortedscores[j] = score;
        total++;
    }

    // print level name and exit rules
    string[0] = 0;

    stringlength = strlen(string);

    // add the clients in sorted order
    if (total > 12)
        total = 12;

    for (i = 0 ; i < total ; i++) {
        cl = &game.clients[sorted[i]];
        cl_ent = g_edicts + 1 + sorted[i];

        x = (i >= 6) ? 160 : 0;
        y = 32 + 32 * (i % 6);

        // add a dogtag
        if (cl_ent == ent)
            tag = "tag1";
        else if (cl_ent == killer)
            tag = "tag2";
        else
            tag = NULL;
        if (tag) {
            Q_snprintf(entry, sizeof(entry),
                       "xv %i yv %i picn %s ", x + 32, y, tag);
            j = strlen(entry);
            if (stringlength + j > 1024)
                break;
            strcpy(string + stringlength, entry);
            stringlength += j;
        }

        // send the layout
        Q_snprintf(entry, sizeof(entry),
                   "client %i %i %i %i %i %i ",
                   x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe) / 600);
        j = strlen(entry);
        if (stringlength + j > 1024)
            break;
        strcpy(string + stringlength, entry);
        stringlength += j;
    }

    gi.WriteByte(svg_layout);
    gi.WriteString(string);
}


/*
==================
HUD_SendDMScoreboardMessage

Sends the deatchmatch scoreboard svc_layout message.
==================
*/
void HUD_SendDMScoreboardMessage(entity_t *ent)
{
    HUD_GenerateDMScoreboardLayout(ent, ent->enemy);
    gi.Unicast(ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f(entity_t *ent)
{
    ent->client->showinventory = false;
    ent->client->showhelp = false;

    if (!deathmatch->value && !coop->value)
        return;

    if (ent->client->showscores) {
        ent->client->showscores = false;
        return;
    }

    ent->client->showscores = true;
    HUD_SendDMScoreboardMessage(ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer(entity_t *ent)
{
    char    string[1024];
    const char    *sk; // C++20: STRING: Added const to char*

    if (skill->value == 0)
        sk = "easy";
    else if (skill->value == 1)
        sk = "medium";
    else if (skill->value == 2)
        sk = "hard";
    else
        sk = "hard+";

    // send the layout
    Q_snprintf(string, sizeof(string),
               "xv 32 yv 8 picn help "         // background
               "xv 202 yv 12 string2 \"%s\" "      // skill
               "xv 0 yv 24 cstring2 \"%s\" "       // level name
               "xv 0 yv 54 cstring2 \"%s\" "       // help 1
               "xv 0 yv 110 cstring2 \"%s\" "      // help 2
               "xv 50 yv 164 string2 \" kills     goals    secrets\" "
               "xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ",
               sk,
               level.level_name,
               game.helpmessage1,
               game.helpmessage2,
               level.killed_monsters, level.total_monsters,
               level.found_goals, level.total_goals,
               0, 0);

    gi.WriteByte(svg_layout);
    gi.WriteString(string);
    gi.Unicast(ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f(entity_t *ent)
{
    // this is for backwards compatability
    if (deathmatch->value) {
        Cmd_Score_f(ent);
        return;
    }

    ent->client->showinventory = false;
    ent->client->showscores = false;

    if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged)) {
        ent->client->showhelp = false;
        return;
    }

    ent->client->showhelp = true;
    ent->client->pers.helpchanged = 0;
    HelpComputer(ent);
}


//=======================================================================

//
//===============
// HUD_SetClientStats
// 
// Sets the entities client, player state, status array with the current
// frame game state data. Such as ammo, etc, it also index/precaches images
// and audio if required.
//================
//
void HUD_SetClientStats(entity_t* ent)
{
    gitem_t* item;
    int         index, cells;
    int         power_armor_type;

    // Ensure ent is valid.
    if (!ent) {
        return;
    }

    //
    // health
    //
    ent->client->playerState.stats[STAT_HEALTH_ICON] = level.pic_health;
    ent->client->playerState.stats[STAT_HEALTH] = ent->health;

    //
    // ammo
    //
    if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */) {
        ent->client->playerState.stats[STAT_AMMO_ICON] = 0;
        ent->client->playerState.stats[STAT_AMMO] = 0;
    }
    else {
        item = &itemlist[ent->client->ammo_index];
        ent->client->playerState.stats[STAT_AMMO_ICON] = gi.ImageIndex(item->icon);
        ent->client->playerState.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
    }

    //
    // armor
    //
    ent->client->playerState.stats[STAT_ARMOR_ICON] = 0;
    ent->client->playerState.stats[STAT_ARMOR] = 0;

    //
    // pickup message
    //
    if (level.time > ent->client->pickup_msg_time) {
        ent->client->playerState.stats[STAT_PICKUP_ICON] = 0;
        ent->client->playerState.stats[STAT_PICKUP_STRING] = 0;
    }

    //
    // timers
    //
    ent->client->playerState.stats[STAT_TIMER_ICON] = 0;
    ent->client->playerState.stats[STAT_TIMER] = 0;

    //
    // selected item
    //
    if (ent->client->pers.selected_item == -1)
        ent->client->playerState.stats[STAT_SELECTED_ICON] = 0;
    else
        ent->client->playerState.stats[STAT_SELECTED_ICON] = gi.ImageIndex(itemlist[ent->client->pers.selected_item].icon);

    ent->client->playerState.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

    //
    // layouts
    //
    ent->client->playerState.stats[STAT_LAYOUTS] = 0;

    if (deathmatch->value) {
        if (ent->client->pers.health <= 0 || level.intermissiontime
            || ent->client->showscores)
            ent->client->playerState.stats[STAT_LAYOUTS] |= 1;
        if (ent->client->showinventory && ent->client->pers.health > 0)
            ent->client->playerState.stats[STAT_LAYOUTS] |= 2;
    }
    else {
        if (ent->client->showscores || ent->client->showhelp)
            ent->client->playerState.stats[STAT_LAYOUTS] |= 1;
        if (ent->client->showinventory && ent->client->pers.health > 0)
            ent->client->playerState.stats[STAT_LAYOUTS] |= 2;
    }

    //
    // frags
    //
    ent->client->playerState.stats[STAT_FRAGS] = ent->client->resp.score;

    //
    // help icon / current weapon if not shown
    //
    if (ent->client->pers.helpchanged && (level.framenum & 8)) {
        ent->client->playerState.stats[STAT_HELPICON] = gi.ImageIndex("i_help");
    } else if ((ent->client->pers.hand == CENTER_HANDED
                || ent->client->playerState.fov > 91)
                && ent->client->pers.weapon) {

        ent->client->playerState.stats[STAT_HELPICON] = gi.ImageIndex(ent->client->pers.weapon->icon);
    } else {
        ent->client->playerState.stats[STAT_HELPICON] = 0;
    }

    ent->client->playerState.stats[STAT_SPECTATOR] = 0;
}

/*
===============
HUD_CheckChaseStats
===============
*/
void HUD_CheckChaseStats(entity_t *ent)
{
    int i;

    if (!ent)    {
        return;
    }

    for (i = 1; i <= maxclients->value; i++) {
        gclient_t* cl;

        cl = g_edicts[i].client;

        if (!g_edicts[i].inUse || (cl->chase_target != ent)) {
            continue;
        }

        memcpy(cl->playerState.stats, ent->client->playerState.stats, sizeof(cl->playerState.stats));
        HUD_SetSpectatorStats(g_edicts + i);
    }
}

/*
===============
HUD_SetSpectatorStats
===============
*/
void HUD_SetSpectatorStats(entity_t *ent)
{
    if (!ent) {
        return;
    }

    gclient_t* cl = ent->client;

    if (!cl->chase_target) {
        HUD_SetClientStats(ent);
    }

    cl->playerState.stats[STAT_SPECTATOR] = 1;

    /* layouts are independant in spectator */
    cl->playerState.stats[STAT_LAYOUTS] = 0;

    if ((cl->pers.health <= 0) || level.intermissiontime || cl->showscores)
    {
        cl->playerState.stats[STAT_LAYOUTS] |= 1;
    }

    if (cl->showinventory && (cl->pers.health > 0))
    {
        cl->playerState.stats[STAT_LAYOUTS] |= 2;
    }

    if (cl->chase_target && cl->chase_target->inUse)
    {
        cl->playerState.stats[STAT_CHASE] = CS_PLAYERSKINS +
            (cl->chase_target - g_edicts) - 1;
    }
    else
    {
        cl->playerState.stats[STAT_CHASE] = 0;
    }
}

