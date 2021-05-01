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
#include "player/animations.h"


char *ClientTeam(entity_t *ent)
{
    char        *p;
    static char value[512];

    value[0] = 0;

    if (!ent->client)
        return value;

    strcpy(value, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
    p = strchr(value, '/');
    if (!p)
        return value;

    if ((int)(dmflags->value) & DeathMatchFlags::ModelTeams) {
        *p = 0;
        return value;
    }

    // if ((int)(dmflags->value) & DF_SKINTEAMS)
    return ++p;
}

qboolean OnSameTeam(entity_t *ent1, entity_t *ent2)
{
    char    ent1Team [512];
    char    ent2Team [512];

    if (!((int)(dmflags->value) & (DeathMatchFlags::ModelTeams | DeathMatchFlags::SkinTeams)))
        return false;

    strcpy(ent1Team, ClientTeam(ent1));
    strcpy(ent2Team, ClientTeam(ent2));

    if (strcmp(ent1Team, ent2Team) == 0)
        return true;
    return false;
}


void SelectNextItem(entity_t *ent, int itflags)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->client;

    if (cl->chase_target) {
        ChaseNext(ent);
        return;
    }

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (cl->pers.selected_item + i) % MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->Use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->pers.selected_item = index;
        return;
    }

    cl->pers.selected_item = -1;
}

void SelectPrevItem(entity_t *ent, int itflags)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->client;

    if (cl->chase_target) {
        ChasePrev(ent);
        return;
    }

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (cl->pers.selected_item + MAX_ITEMS - i) % MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->Use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->pers.selected_item = index;
        return;
    }

    cl->pers.selected_item = -1;
}

void HUD_ValidateSelectedItem(entity_t *ent)
{
    // Ensure these are valid.
    if (!ent || !ent->client) {
        return;
    }

    gclient_t   *cl;

    cl = ent->client;

    if (cl->pers.inventory[cl->pers.selected_item])
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
void Cmd_Give_f(entity_t *ent)
{
    const char        *name;
    gitem_t     *it;
    int         index;
    int         i;
    qboolean    give_all;
    entity_t     *it_ent;

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
            ent->health = atoi(gi.argv(2));
        else
            ent->health = ent->maxHealth;
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "weapons") == 0) {
        for (i = 0 ; i < game.num_items ; i++) {
            it = itemlist + i;
            if (!it->Pickup)
                continue;
            if (!(it->flags & IT_WEAPON))
                continue;
            ent->client->pers.inventory[i] += 1;
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "ammo") == 0) {
        for (i = 0 ; i < game.num_items ; i++) {
            it = itemlist + i;
            if (!it->Pickup)
                continue;
            if (!(it->flags & IT_AMMO))
                continue;
            Add_Ammo(ent, it, 1000);
        }
        if (!give_all)
            return;
    }

    if (give_all) {
        for (i = 0 ; i < game.num_items ; i++) {
            it = itemlist + i;
            if (!it->Pickup)
                continue;
            if (it->flags & (IT_WEAPON | IT_AMMO))
                continue;
            ent->client->pers.inventory[i] = 1;
        }
        return;
    }

    it = FindItem(name);
    if (!it) {
        name = gi.argv(1); // C++20: Added cast.
        it = FindItem(name);
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

    if (it->flags & IT_AMMO) {
        if (gi.argc() == 3)
            ent->client->pers.inventory[index] = atoi(gi.argv(2));
        else
            ent->client->pers.inventory[index] += it->quantity;
    } else {
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem(it_ent, it);
        Touch_Item(it_ent, ent, NULL, NULL);
        if (it_ent->inUse)
            G_FreeEntity(it_ent);
    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f(entity_t *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.CPrintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->flags ^= FL_GODMODE;
    if (!(ent->flags & FL_GODMODE))
        gi.CPrintf(ent, PRINT_HIGH, "godmode OFF\n");
    else
        gi.CPrintf(ent, PRINT_HIGH, "godmode ON\n");
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f(entity_t *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.CPrintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->flags ^= FL_NOTARGET;
    if (!(ent->flags & FL_NOTARGET))
        gi.CPrintf(ent, PRINT_HIGH, "notarget OFF\n");
    else
        gi.CPrintf(ent, PRINT_HIGH, "notarget ON\n");
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f(entity_t *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.CPrintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    if (ent->moveType == MOVETYPE_NOCLIP) {
        ent->moveType = MOVETYPE_WALK;
        gi.CPrintf(ent, PRINT_HIGH, "noclip OFF\n");
    } else {
        ent->moveType = MOVETYPE_NOCLIP;
        gi.CPrintf(ent, PRINT_HIGH, "noclip ON\n");
    }
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f(entity_t *ent)
{
    int         index;
    gitem_t     *it;
    const char        *s;

    s = gi.args(); // C++20: Added casts.
    it = FindItem(s);
    if (!it) {
        gi.CPrintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
        return;
    }
    if (!it->Use) {
        gi.CPrintf(ent, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    index = ITEM_INDEX(it);
    if (!ent->client->pers.inventory[index]) {
        gi.CPrintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
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
void Cmd_Drop_f(entity_t *ent)
{
    int         index;
    gitem_t     *it;
    const char        *s;

    s = (const char*)gi.args(); // C++20: Added casts.
    it = FindItem(s);
    if (!it) {
        gi.CPrintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
        return;
    }
    if (!it->Drop) {
        gi.CPrintf(ent, PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    index = ITEM_INDEX(it);
    if (!ent->client->pers.inventory[index]) {
        gi.CPrintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
        return;
    }

    it->Drop(ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f(entity_t *ent)
{
    int         i;
    gclient_t   *cl;

    cl = ent->client;

    cl->showscores = false;
    cl->showhelp = false;

    if (cl->showinventory) {
        cl->showinventory = false;
        return;
    }

    cl->showinventory = true;

    gi.WriteByte(svg_inventory);
    for (i = 0 ; i < MAX_ITEMS ; i++) {
        gi.WriteShort(cl->pers.inventory[i]);
    }
    gi.Unicast(ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f(entity_t *ent)
{
    gitem_t     *it;

    HUD_ValidateSelectedItem(ent);

    if (ent->client->pers.selected_item == -1) {
        gi.CPrintf(ent, PRINT_HIGH, "No item to use.\n");
        return;
    }

    it = &itemlist[ent->client->pers.selected_item];
    if (!it->Use) {
        gi.CPrintf(ent, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    it->Use(ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f(entity_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    if (!cl->pers.weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->pers.weapon);

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (selected_weapon + i) % MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->Use)
            continue;
        if (!(it->flags & IT_WEAPON))
            continue;
        it->Use(ent, it);
        if (cl->pers.weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f(entity_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    if (!cl->pers.weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->pers.weapon);

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (selected_weapon + MAX_ITEMS - i) % MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->Use)
            continue;
        if (!(it->flags & IT_WEAPON))
            continue;
        it->Use(ent, it);
        if (cl->pers.weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f(entity_t *ent)
{
    gclient_t   *cl;
    int         index;
    gitem_t     *it;

    cl = ent->client;

    if (!cl->pers.weapon || !cl->pers.lastweapon)
        return;

    index = ITEM_INDEX(cl->pers.lastweapon);
    if (!cl->pers.inventory[index])
        return;
    it = &itemlist[index];
    if (!it->Use)
        return;
    if (!(it->flags & IT_WEAPON))
        return;
    it->Use(ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f(entity_t *ent)
{
    gitem_t     *it;

    HUD_ValidateSelectedItem(ent);

    if (ent->client->pers.selected_item == -1) {
        gi.CPrintf(ent, PRINT_HIGH, "No item to drop.\n");
        return;
    }

    it = &itemlist[ent->client->pers.selected_item];
    if (!it->Drop) {
        gi.CPrintf(ent, PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    it->Drop(ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f(entity_t *ent)
{
    if ((level.time - ent->client->respawn_time) < 5)
        return;
    ent->flags &= ~FL_GODMODE;
    ent->health = 0;
    meansOfDeath = MOD_SUICIDE;
    player_die(ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f(entity_t *ent)
{
    ent->client->showscores = false;
    ent->client->showhelp = false;
    ent->client->showinventory = false;
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
void Cmd_Players_f(entity_t *ent)
{
    int     i;
    int     count;
    char    small[64];
    char    large[1280];
    int     index[256];

    count = 0;
    for (i = 0 ; i < maxclients->value ; i++)
        if (game.clients[i].pers.connected) {
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
                   game.clients[index[i]].pers.netname);
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
void Cmd_Wave_f(entity_t *ent)
{
    int     i;

    i = atoi(gi.argv(1));

    // can't wave when ducked
    if (ent->client->playerState.pmove.flags & PMF_DUCKED)
        return;

    if (ent->client->anim_priority > ANIM_WAVE)
        return;

    ent->client->anim_priority = ANIM_WAVE;

    switch (i) {
    case 0:
        gi.CPrintf(ent, PRINT_HIGH, "flipoff\n");
        ent->s.frame = FRAME_flip01 - 1;
        ent->client->anim_end = FRAME_flip12;
        break;
    case 1:
        gi.CPrintf(ent, PRINT_HIGH, "salute\n");
        ent->s.frame = FRAME_salute01 - 1;
        ent->client->anim_end = FRAME_salute11;
        break;
    case 2:
        gi.CPrintf(ent, PRINT_HIGH, "taunt\n");
        ent->s.frame = FRAME_taunt01 - 1;
        ent->client->anim_end = FRAME_taunt17;
        break;
    case 3:
        gi.CPrintf(ent, PRINT_HIGH, "wave\n");
        ent->s.frame = FRAME_wave01 - 1;
        ent->client->anim_end = FRAME_wave11;
        break;
    case 4:
    default:
        gi.CPrintf(ent, PRINT_HIGH, "point\n");
        ent->s.frame = FRAME_point01 - 1;
        ent->client->anim_end = FRAME_point12;
        break;
    }
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f(entity_t *ent, qboolean team, qboolean arg0)
{
    int     i, j;
    entity_t *other;
    char    *p; // C++20: Removed const.
    char    text[2048];
    gclient_t *cl;

    if (gi.argc() < 2 && !arg0)
        return;

    if (!((int)(dmflags->value) & (DeathMatchFlags::ModelTeams | DeathMatchFlags::SkinTeams)))
        team = false;

    if (team)
        Q_snprintf(text, sizeof(text), "(%s): ", ent->client->pers.netname);
    else
        Q_snprintf(text, sizeof(text), "%s: ", ent->client->pers.netname);

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

        if (level.time < cl->flood_locktill) {
            gi.CPrintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
                       (int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when) / sizeof(cl->flood_when[0])) + i;
        if (cl->flood_when[i] &&
            level.time - cl->flood_when[i] < flood_persecond->value) {
            cl->flood_locktill = level.time + flood_waitdelay->value;
            gi.CPrintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
                       (int)flood_waitdelay->value);
            return;
        }
        cl->flood_whenhead = (cl->flood_whenhead + 1) %
                             (sizeof(cl->flood_when) / sizeof(cl->flood_when[0]));
        cl->flood_when[cl->flood_whenhead] = level.time;
    }

    if (dedicated->value)
        gi.CPrintf(NULL, PRINT_CHAT, "%s", text);

    for (j = 1; j <= game.maxclients; j++) {
        other = &g_edicts[j];
        if (!other->inUse)
            continue;
        if (!other->client)
            continue;
        if (team) {
            if (!OnSameTeam(ent, other))
                continue;
        }
        gi.CPrintf(other, PRINT_CHAT, "%s", text);
    }
}

void Cmd_PlayerList_f(entity_t *ent)
{
    int i;
    char st[80];
    char text[1400];
    entity_t *e2;

    // connect time, ping, score, name
    *text = 0;
    for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
        if (!e2->inUse)
            continue;

        Q_snprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
                   (level.framenum - e2->client->resp.enterframe) / 600,
                   ((level.framenum - e2->client->resp.enterframe) % 600) / 10,
                   e2->client->ping,
                   e2->client->resp.score,
                   e2->client->pers.netname,
                   e2->client->resp.spectator ? " (spectator)" : "");
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
void ClientCommand(entity_t *ent)
{
    const char    *cmd;

    if (!ent->client)
        return;     // not fully in game yet

    cmd = gi.argv(0);

    if (Q_stricmp(cmd, "players") == 0) {
        Cmd_Players_f(ent);
        return;
    }
    if (Q_stricmp(cmd, "say") == 0) {
        Cmd_Say_f(ent, false, false);
        return;
    }
    if (Q_stricmp(cmd, "say_team") == 0) {
        Cmd_Say_f(ent, true, false);
        return;
    }
    if (Q_stricmp(cmd, "score") == 0) {
        Cmd_Score_f(ent);
        return;
    }

    if (level.intermissiontime)
        return;

    if (Q_stricmp(cmd, "use") == 0)
        Cmd_Use_f(ent);
    else if (Q_stricmp(cmd, "drop") == 0)
        Cmd_Drop_f(ent);
    else if (Q_stricmp(cmd, "give") == 0)
        Cmd_Give_f(ent);
    else if (Q_stricmp(cmd, "god") == 0)
        Cmd_God_f(ent);
    else if (Q_stricmp(cmd, "notarget") == 0)
        Cmd_Notarget_f(ent);
    else if (Q_stricmp(cmd, "noclip") == 0)
        Cmd_Noclip_f(ent);
    else if (Q_stricmp(cmd, "inven") == 0)
        Cmd_Inven_f(ent);
    else if (Q_stricmp(cmd, "invnext") == 0)
        SelectNextItem(ent, -1);
    else if (Q_stricmp(cmd, "invprev") == 0)
        SelectPrevItem(ent, -1);
    else if (Q_stricmp(cmd, "invnextw") == 0)
        SelectNextItem(ent, IT_WEAPON);
    else if (Q_stricmp(cmd, "invprevw") == 0)
        SelectPrevItem(ent, IT_WEAPON);
    else if (Q_stricmp(cmd, "invnextp") == 0)
        SelectNextItem(ent, IT_POWERUP);
    else if (Q_stricmp(cmd, "invprevp") == 0)
        SelectPrevItem(ent, IT_POWERUP);
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
        Cmd_PutAway_f(ent);
    else if (Q_stricmp(cmd, "wave") == 0)
        Cmd_Wave_f(ent);
    else if (Q_stricmp(cmd, "playerlist") == 0)
        Cmd_PlayerList_f(ent);
    else    // anything that doesn't match a command will be a chat
        Cmd_Say_f(ent, false, true);
}
