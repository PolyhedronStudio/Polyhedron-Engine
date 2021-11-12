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
#include "g_local.h"
#include "chasecamera.h"
#include "entities.h"
#include "player/animations.h"

// Class Entities.
#include "entities/base/SVGBaseEntity.h"
#include "entities/base/PlayerClient.h"

// Game Modes.
#include "gamemodes/IGameMode.h"

char *ClientTeam(SVGBaseEntity *ent)
{
    char        *p;
    static char value[512];

    value[0] = 0;

    if (!ent->GetClient())
        return value;

    strcpy(value, Info_ValueForKey(ent->GetClient()->persistent.userinfo, "skin"));
    p = strchr(value, '/');
    if (!p)
        return value;

    if ((int)(gamemodeflags->value) & GameModeFlags::ModelTeams) {
        *p = 0;
        return value;
    }

    // if ((int)(gamemodeflags->value) & DF_SKINTEAMS)
    return ++p;
}

qboolean SVG_OnSameTeam(SVGBaseEntity *ent1, SVGBaseEntity *ent2)
{
    char    ent1Team [512];
    char    ent2Team [512];

    if (!((int)(gamemodeflags->value) & (GameModeFlags::ModelTeams | GameModeFlags::SkinTeams)))
        return false;

    strcpy(ent1Team, ClientTeam(ent1));
    strcpy(ent2Team, ClientTeam(ent2));

    if (strcmp(ent1Team, ent2Team) == 0)
        return true;
    return false;
}


void SelectNextItem(PlayerClient *ent, int itflags)
{
    ServersClient   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->GetClient();

    if (cl->chaseTarget) {
        SVG_ChaseNext(ent);
        return;
    }

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (cl->persistent.selectedItem + i) % MAX_ITEMS;
        if (!cl->persistent.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->Use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->persistent.selectedItem = index;
        return;
    }

    cl->persistent.selectedItem = -1;
}

void SelectPrevItem(Entity *ent, int itflags)
{
    ServersClient   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->client;

    if (cl->chaseTarget) {
        SVG_ChasePrev((PlayerClient*)ent->classEntity);
        return;
    }

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (cl->persistent.selectedItem + MAX_ITEMS - i) % MAX_ITEMS;
        if (!cl->persistent.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->Use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->persistent.selectedItem = index;
        return;
    }

    cl->persistent.selectedItem = -1;
}

void HUD_ValidateSelectedItem(PlayerClient *ent)
{
    // Ensure these are valid.
    if (!ent || !ent->GetClient()) {
        return;
    }

    ServersClient* cl = ent->GetClient();

    if (cl->persistent.inventory[cl->persistent.selectedItem])
        return;     // valid

    SelectNextItem(ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f(Entity *ent)
{
    const char        *name;
    gitem_t     *it;
    int         index;
    int         i;
    qboolean    give_all;
    Entity     *it_ent;

    // Ensure these are valid.
    if (!ent) {
        return;
    }

    if (deathmatch->value && !sv_cheats->value) {
        gi.CPrintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    name = gi.args(); // C++20: Added cast.

    if (Q_stricmp(name, "all") == 0)
        give_all = true;
    else
        give_all = false;

    if (give_all || Q_stricmp(gi.argv(1), "health") == 0) {
        if (gi.argc() == 3)
            ent->classEntity->SetHealth(atoi(gi.argv(2)));
        else
            ent->classEntity->SetHealth(ent->classEntity->GetMaxHealth());
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "weapons") == 0) {
        for (i = 0 ; i < game.numberOfItems ; i++) {
            it = itemlist + i;
            if (!it->Pickup)
                continue;
            if (!(it->flags & ItemFlags::IsWeapon))
                continue;
            ent->client->persistent.inventory[i] += 1;
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "ammo") == 0) {
        for (i = 0 ; i < game.numberOfItems ; i++) {
            it = itemlist + i;
            if (!it->Pickup)
                continue;
            if (!(it->flags & ItemFlags::IsAmmo))
                continue;
            SVG_AddAmmo(ent, it, 1000);
        }
        if (!give_all)
            return;
    }

    if (give_all) {
        for (i = 0 ; i < game.numberOfItems ; i++) {
            it = itemlist + i;
            if (!it->Pickup)
                continue;
            if (it->flags & (ItemFlags::IsWeapon | ItemFlags::IsAmmo))
                continue;
            ent->client->persistent.inventory[i] = 1;
        }
        return;
    }

    it = SVG_FindItemByPickupName(name);
    if (!it) {
        name = gi.argv(1); // C++20: Added cast.
        it = SVG_FindItemByPickupName(name);
        if (!it) {
            gi.CPrintf(ent, PRINT_HIGH, "unknown item\n");
            return;
        }
    }

    if (!it->Pickup) {
        gi.CPrintf(ent, PRINT_HIGH, "non-pickup item\n");
        return;
    }

    index = ITEM_INDEX(it);

    if (it->flags & ItemFlags::IsAmmo) {
        if (gi.argc() == 3)
            ent->client->persistent.inventory[index] = atoi(gi.argv(2));
        else
            ent->client->persistent.inventory[index] += it->quantity;
    } else {
        it_ent = SVG_Spawn();
        it_ent->className = it->className;
        SVG_SpawnItem(it_ent, it);
//        SVG_TouchItem(it_ent, ent, NULL, NULL); Items..
        if (it_ent->inUse)
            SVG_FreeEntity(it_ent);
    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f(SVGBaseEntity *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->SetFlags(ent->GetFlags() ^ EntityFlags::GodMode);
    if (!(ent->GetFlags() & EntityFlags::GodMode))
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "godmode OFF\n");
    else
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "godmode ON\n");
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f(SVGBaseEntity *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->SetFlags(ent->GetFlags() ^ EntityFlags::NoTarget);
    if (!(ent->GetFlags() & EntityFlags::NoTarget))
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "notarget OFF\n");
    else
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "notarget ON\n");
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f(SVGBaseEntity *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    if (ent->GetMoveType() == MoveType::NoClip) {
        ent->SetMoveType(MoveType::Walk);
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "noclip OFF\n");
    } else {
        ent->SetMoveType(MoveType::NoClip);
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "noclip ON\n");
    }
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f(PlayerClient *ent)
{
    int         index;
    gitem_t     *it;
    const char        *s;

    s = gi.args(); // C++20: Added casts.
    it = SVG_FindItemByPickupName(s);
    if (!it) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "unknown item: %s\n", s);
        return;
    }
    if (!it->Use) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    index = ITEM_INDEX(it);
    if (!ent->GetClient()->persistent.inventory[index]) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Out of item: %s\n", s);
        return;
    }

    it->Use(ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f(PlayerClient*ent)
{
    int         index;
    gitem_t     *it;
    const char        *s;

    s = (const char*)gi.args(); // C++20: Added casts.
    it = SVG_FindItemByPickupName(s);
    if (!it) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "unknown item: %s\n", s);
        return;
    }
    if (!it->Drop) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    index = ITEM_INDEX(it);
    if (!ent->GetClient()->persistent.inventory[index]) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Out of item: %s\n", s);
        return;
    }

    it->Drop(ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f(Entity *ent)
{
    int         i;
    ServersClient   *cl;

    cl = ent->client;

    cl->showScores = false;

    if (cl->showInventory) {
        cl->showInventory = false;
        return;
    }

    cl->showInventory = true;

    gi.WriteByte(SVG_CMD_INVENTORY);
    for (i = 0 ; i < MAX_ITEMS ; i++) {
        gi.WriteShort(cl->persistent.inventory[i]);
    }
    gi.Unicast(ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f(PlayerClient *ent)
{
    gitem_t     *it;

    HUD_ValidateSelectedItem(ent);

    if (ent->GetClient()->persistent.selectedItem == -1) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "No item to use.\n");
        return;
    }

    it = &itemlist[ent->GetClient()->persistent.selectedItem];
    if (!it->Use) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    it->Use(ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f(PlayerClient *ent)
{
    ServersClient   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->GetClient();

    if (!cl->persistent.activeWeapon)
        return;

    selected_weapon = ITEM_INDEX(cl->persistent.activeWeapon);

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (selected_weapon + i) % MAX_ITEMS;
        if (!cl->persistent.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->Use)
            continue;
        if (!(it->flags & ItemFlags::IsWeapon))
            continue;
        it->Use(ent, it);
        if (cl->persistent.activeWeapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f(PlayerClient *ent)
{
    ServersClient   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->GetClient();

    if (!cl->persistent.activeWeapon)
        return;

    selected_weapon = ITEM_INDEX(cl->persistent.activeWeapon);

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (selected_weapon + MAX_ITEMS - i) % MAX_ITEMS;
        if (!cl->persistent.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->Use)
            continue;
        if (!(it->flags & ItemFlags::IsWeapon))
            continue;
        it->Use(ent, it);
        if (cl->persistent.activeWeapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f(PlayerClient *ent)
{
    ServersClient   *cl;
    int         index;
    gitem_t     *it;

    cl = ent->GetClient();

    if (!cl->persistent.activeWeapon || !cl->persistent.lastWeapon)
        return;

    index = ITEM_INDEX(cl->persistent.lastWeapon);
    if (!cl->persistent.inventory[index])
        return;
    it = &itemlist[index];
    if (!it->Use)
        return;
    if (!(it->flags & ItemFlags::IsWeapon))
        return;
    it->Use(ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f(PlayerClient *ent)
{
    gitem_t     *it;

    HUD_ValidateSelectedItem(ent);

    if (ent->GetClient()->persistent.selectedItem == -1) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "No item to drop.\n");
        return;
    }

    it = &itemlist[ent->GetClient()->persistent.selectedItem];
    if (!it->Drop) {
        gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    it->Drop(ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f(PlayerClient *ent)
{
    if ((level.time - ent->GetClient()->respawnTime) < 5)
        return;

    ent->SetFlags(ent->GetFlags() & ~EntityFlags::GodMode);
    ent->SetHealth(0);
    game.gameMode->SetCurrentMeansOfDeath(MeansOfDeath::Suicide);
    ent->Die(ent, ent, 100000, vec3_zero());
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f(Entity *ent)
{
    ent->client->showScores = false;
    ent->client->showInventory = false;
}


int PlayerSort(void const *a, void const *b)
{
    int     anum, bnum;

    anum = *(int *)a;
    bnum = *(int *)b;

    anum = game.clients[anum].playerState.stats[STAT_FRAGS];
    bnum = game.clients[bnum].playerState.stats[STAT_FRAGS];

    if (anum < bnum)
        return -1;
    if (anum > bnum)
        return 1;
    return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f(Entity *ent)
{
    int     i;
    int     count;
    char    small[64];
    char    large[1280];
    int     index[256];

    count = 0;
    for (i = 0 ; i < maximumClients->value ; i++)
        if (game.clients[i].persistent.isConnected) {
            index[count] = i;
            count++;
        }

    // sort by frags
    qsort(index, count, sizeof(index[0]), PlayerSort);

    // print information
    large[0] = 0;

    for (i = 0 ; i < count ; i++) {
        Q_snprintf(small, sizeof(small), "%3i %s\n",
                   game.clients[index[i]].playerState.stats[STAT_FRAGS],
                   game.clients[index[i]].persistent.netname);
        if (strlen(small) + strlen(large) > sizeof(large) - 100) {
            // can't print all of them in one packet
            strcat(large, "...\n");
            break;
        }
        strcat(large, small);
    }

    gi.CPrintf(ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f(Entity *ent)
{
    int     i;

    i = atoi(gi.argv(1));

    // can't wave when ducked
    if (ent->client->playerState.pmove.flags & PMF_DUCKED)
        return;

    if (ent->client->animation.priorityAnimation > PlayerAnimation::Wave)
        return;

    ent->client->animation.priorityAnimation = PlayerAnimation::Wave;

    switch (i) {
    case 0:
        gi.CPrintf(ent, PRINT_HIGH, "flipoff\n");
        ent->state.frame = FRAME_flip01 - 1;
        ent->client->animation.endFrame = FRAME_flip12;
        break;
    case 1:
        gi.CPrintf(ent, PRINT_HIGH, "salute\n");
        ent->state.frame = FRAME_salute01 - 1;
        ent->client->animation.endFrame = FRAME_salute11;
        break;
    case 2:
        gi.CPrintf(ent, PRINT_HIGH, "taunt\n");
        ent->state.frame = FRAME_taunt01 - 1;
        ent->client->animation.endFrame = FRAME_taunt17;
        break;
    case 3:
        gi.CPrintf(ent, PRINT_HIGH, "wave\n");
        ent->state.frame = FRAME_wave01 - 1;
        ent->client->animation.endFrame = FRAME_wave11;
        break;
    case 4:
    default:
        gi.CPrintf(ent, PRINT_HIGH, "point\n");
        ent->state.frame = FRAME_point01 - 1;
        ent->client->animation.endFrame = FRAME_point12;
        break;
    }
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f(Entity *ent, qboolean team, qboolean arg0)
{
    int     i, j;
    Entity *other;
    char    *p; // C++20: Removed const.
    char    text[2048];
    ServersClient *cl;

    if (gi.argc() < 2 && !arg0)
        return;

    if (!((int)(gamemodeflags->value) & (GameModeFlags::ModelTeams | GameModeFlags::SkinTeams)))
        team = false;

    if (team)
        Q_snprintf(text, sizeof(text), "(%s): ", ent->client->persistent.netname);
    else
        Q_snprintf(text, sizeof(text), "%s: ", ent->client->persistent.netname);

    if (arg0) {
        strcat(text, gi.argv(0));
        strcat(text, " ");
        strcat(text, gi.args());
    } else {
        p = (char*)gi.args();  // C++20: Added casts.

        if (*p == '"') {
            p++;
            p[strlen(p) - 1] = 0;
        }
        strcat(text, p);
    }

    // don't let text be too long for malicious reasons
    if (strlen(text) > 150)
        text[150] = 0;

    strcat(text, "\n");

    if (flood_msgs->value) {
        cl = ent->client;

        if (level.time < cl->flood.lockTill) {
            gi.CPrintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
                       (int)(cl->flood.lockTill - level.time));
            return;
        }
        i = cl->flood.whenHead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood.when) / sizeof(cl->flood.when[0])) + i;
        if (cl->flood.when[i] &&
            level.time - cl->flood.when[i] < flood_persecond->value) {
            cl->flood.lockTill = level.time + flood_waitdelay->value;
            gi.CPrintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
                       (int)flood_waitdelay->value);
            return;
        }
        cl->flood.whenHead = (cl->flood.whenHead + 1) %
                             (sizeof(cl->flood.when) / sizeof(cl->flood.when[0]));
        cl->flood.when[cl->flood.whenHead] = level.time;
    }

    if (dedicated->value)
        gi.CPrintf(NULL, PRINT_CHAT, "%s", text);

    for (j = 1; j <= game.maximumClients; j++) {
        other = &g_entities[j];
        if (!other->inUse)
            continue;
        if (!other->client)
            continue;
        if (team) {
            if (!SVG_OnSameTeam(ent->classEntity, other->classEntity))
                continue;
        }
        gi.CPrintf(other, PRINT_CHAT, "%s", text);
    }
}

void Cmd_PlayerList_f(Entity *ent)
{
    int i;
    char st[80];
    char text[1400];
    Entity *e2;

    // connect time, ping, score, name
    *text = 0;
    for (i = 0, e2 = g_entities + 1; i < maximumClients->value; i++, e2++) {
        if (!e2->inUse)
            continue;

        Q_snprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
                   (level.frameNumber - e2->client->respawn.enterGameFrameNumber) / 600,
                   ((level.frameNumber - e2->client->respawn.enterGameFrameNumber) % 600) / 10,
                   e2->client->ping,
                   e2->client->respawn.score,
                   e2->client->persistent.netname,
                   e2->client->respawn.isSpectator ? " (isSpectator)" : "");
        if (strlen(text) + strlen(st) > sizeof(text) - 50) {
            sprintf(text + strlen(text), "And more...\n");
            gi.CPrintf(ent, PRINT_HIGH, "%s", text);
            return;
        }
        strcat(text, st);
    }
    gi.CPrintf(ent, PRINT_HIGH, "%s", text);
}


/*
=================
ClientCommand
=================
*/
void SVG_ClientCommand(Entity *serverEntity)
{
    const char    *cmd;

    // Ensure it is an entity with active client.
    if (!serverEntity->client)
        return; // Not fully in game yet

    // We can safely cast to PlayerClient now.
    PlayerClient* ent = (PlayerClient*)serverEntity->classEntity;

    // Fetch cmd.
    cmd = gi.argv(0);

    if (Q_stricmp(cmd, "players") == 0) {
        Cmd_Players_f(serverEntity);
        return;
    }
    if (Q_stricmp(cmd, "say") == 0) {
        Cmd_Say_f(serverEntity, false, false);
        return;
    }
    if (Q_stricmp(cmd, "say_team") == 0) {
        Cmd_Say_f(serverEntity, true, false);
        return;
    }
    if (Q_stricmp(cmd, "score") == 0) {
            gi.DPrintf("YO YOU ARE DEBUGGING SCORES");
        SVG_Command_Score_f(ent);
        return;
    }

    if (level.intermission.time)
        return;

    if (Q_stricmp(cmd, "use") == 0)
        Cmd_Use_f(ent);
    else if (Q_stricmp(cmd, "drop") == 0)
        Cmd_Drop_f(ent);
    else if (Q_stricmp(cmd, "give") == 0)
        Cmd_Give_f(ent->GetServerEntity());
    else if (Q_stricmp(cmd, "god") == 0)
        Cmd_God_f(ent);
    else if (Q_stricmp(cmd, "notarget") == 0)
        Cmd_Notarget_f(ent);
    else if (Q_stricmp(cmd, "noclip") == 0)
        Cmd_Noclip_f(ent);
    else if (Q_stricmp(cmd, "inven") == 0)
        Cmd_Inven_f(ent->GetServerEntity());
    else if (Q_stricmp(cmd, "invnext") == 0)
        SelectNextItem(ent, -1);
    else if (Q_stricmp(cmd, "invprev") == 0)
        SelectPrevItem(serverEntity, -1);
    else if (Q_stricmp(cmd, "invnextw") == 0)
        SelectNextItem(ent, ItemFlags::IsWeapon);
    else if (Q_stricmp(cmd, "invprevw") == 0)
        SelectPrevItem(serverEntity, ItemFlags::IsWeapon);
    else if (Q_stricmp(cmd, "invnextp") == 0)
        SelectNextItem(ent, ItemFlags::IsPowerUp);
    else if (Q_stricmp(cmd, "invprevp") == 0)
        SelectPrevItem(serverEntity, ItemFlags::IsPowerUp);
    else if (Q_stricmp(cmd, "invuse") == 0)
        Cmd_InvUse_f(ent);
    else if (Q_stricmp(cmd, "invdrop") == 0)
        Cmd_InvDrop_f(ent);
    else if (Q_stricmp(cmd, "weapprev") == 0)
        Cmd_WeapPrev_f(ent);
    else if (Q_stricmp(cmd, "weapnext") == 0)
        Cmd_WeapNext_f(ent);
    else if (Q_stricmp(cmd, "weaplast") == 0)
        Cmd_WeapLast_f(ent);
    else if (Q_stricmp(cmd, "kill") == 0)
        Cmd_Kill_f(ent);
    else if (Q_stricmp(cmd, "putaway") == 0)
        Cmd_PutAway_f(serverEntity);
    else if (Q_stricmp(cmd, "wave") == 0)
        Cmd_Wave_f(serverEntity);
    else if (Q_stricmp(cmd, "playerlist") == 0)
        Cmd_PlayerList_f(serverEntity);
    else    // anything that doesn't match a command will be a chat
        Cmd_Say_f(serverEntity, false, true);
}
