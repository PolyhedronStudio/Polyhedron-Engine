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
#include "ServerGameLocal.h"
#include "ChaseCamera.h"
#include "Entities.h"
#include "Player/Animations.h"

// Class Entities.
#include "Entities/Base/SVGBasePlayer.h"

// Game Modes.
#include "Gamemodes/IGamemode.h"


    //char *ClientTeam(SVGBaseEntity *ent)
//{
//    char        *p;
//    static char value[512];
//
//    value[0] = 0;
//
//    if (!ent->GetClient())
//        return value;
//
//    strcpy(value, Info_ValueForKey(ent->GetClient()->persistent.userinfo, "skin"));
//    p = strchr(value, '/');
//    if (!p)
//        return value;
//
//    if ((int)(gamemodeflags->value) & GamemodeFlags::ModelTeams) {
//        *p = 0;
//        return value;
//    }
//
//    // if ((int)(gamemodeflags->value) & DF_SKINTEAMS)
//    return ++p;
//}

static inline const std::string ClientTeam(SVGBaseEntity* ent) {
    // No team name to return.
    if (!ent->GetClient())
	    "";

    // Check for skin info key.
    std::string teamName = Info_ValueForKey(ent->GetClient()->persistent.userinfo, "skin");

    // Check to see if we can find a '/', in which case we had a positive match
    // on our info key value check.
    if (teamName.find_first_of('/') == std::string::npos) {
        return teamName;
    }

    // Return empty team.
    return "";
    //if ((int)(gamemodeflags->value) & GamemodeFlags::ModelTeams) {
    //	*p = 0;
	   // return value;
    //}

    //// if ((int)(gamemodeflags->value) & DF_SKINTEAMS)
    //return ++p;
}

qboolean SVG_OnSameTeam(SVGBaseEntity *entityA, SVGBaseEntity *entityB)
{
    // TODO: Check in gameworld whether 
    if (!((int)(gamemodeflags->value) & (GamemodeFlags::ModelTeams | GamemodeFlags::SkinTeams)))
        return false;

    // Fetch teamname for each entity and compare if they're the same.
    if (ClientTeam(entityB) == ClientTeam(entityA)) {
        // They are the same, return true.
        return true;
    }

    // Not the same, return false.
    return false;
}


void SelectNextItem(SVGBasePlayer *ent, int itflags)
{
    ServerClient   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->GetClient();

    if (cl->chaseTarget) {
        SVG_ChaseNext(ent);
        return;
    }

    // scan  for the next valid one
    //for (i = 1 ; i <= MAX_ITEMS ; i++) {
    //    index = (cl->persistent.selectedItem + i) % MAX_ITEMS;
    //    if (!cl->persistent.inventory[index])
    //        continue;
    //    it = &itemlist[index];
    //    if (!it->Use)
    //        continue;
    //    if (!(it->flags & itflags))
    //        continue;

    //    cl->persistent.selectedItem = index;
    //    return;
    //}

    cl->persistent.selectedItem = -1;
}

void SelectPrevItem(Entity *ent, int itflags)
{
    ServerClient   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->client;

    if (cl->chaseTarget) {
        SVG_ChasePrev((SVGBasePlayer*)ent->classEntity);
        return;
    }

    // scan  for the next valid one
    //for (i = 1 ; i <= MAX_ITEMS ; i++) {
    //    index = (cl->persistent.selectedItem + MAX_ITEMS - i) % MAX_ITEMS;
    //    if (!cl->persistent.inventory[index])
    //        continue;
    //    it = &itemlist[index];
    //    if (!it->Use)
    //        continue;
    //    if (!(it->flags & itflags))
    //        continue;

    //    cl->persistent.selectedItem = index;
    //    return;
    //}

    cl->persistent.selectedItem = -1;
}

void HUD_ValidateSelectedItem(SVGBasePlayer *ent)
{
    // Ensure these are valid.
    if (!ent || !ent->GetClient()) {
        return;
    }

    ServerClient* cl = ent->GetClient();

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

    //
    //if (deathmatch->value && !sv_cheats->value) {
    //    gi.CPrintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
    //    return;
    //}

    name = gi.args(); // C++20: Added cast.

//    if (PH_StringCompare(name, "all") == 0)
//        give_all = true;
//    else
//        give_all = false;
//
//    if (give_all || PH_StringCompare(gi.argv(1), "health") == 0) {
//        if (gi.argc() == 3)
//            ent->classEntity->SetHealth(atoi(gi.argv(2)));
//        else
//            ent->classEntity->SetHealth(ent->classEntity->GetMaxHealth());
//        if (!give_all)
//            return;
//    }
//
//    if (give_all || PH_StringCompare(name, "weapons") == 0) {
//        for (i = 0 ; i < game.numberOfItems ; i++) {
//            it = itemlist + i;
//            if (!it->Pickup)
//                continue;
//            if (!(it->flags & ItemFlags::IsWeapon))
//                continue;
//            ent->client->persistent.inventory[i] += 1;
//        }
//        if (!give_all)
//            return;
//    }
//
//    if (give_all || PH_StringCompare(name, "ammo") == 0) {
//        for (i = 0 ; i < game.numberOfItems ; i++) {
//            it = itemlist + i;
//            if (!it->Pickup)
//                continue;
//            if (!(it->flags & ItemFlags::IsAmmo))
//                continue;
//            SVG_AddAmmo(ent, it, 1000);
//        }
//        if (!give_all)
//            return;
//    }
//
//    if (give_all) {
//        for (i = 0 ; i < game.numberOfItems ; i++) {
//            it = itemlist + i;
//            if (!it->Pickup)
//                continue;
//            if (it->flags & (ItemFlags::IsWeapon | ItemFlags::IsAmmo))
//                continue;
//            ent->client->persistent.inventory[i] = 1;
//        }
//        return;
//    }
//
//    it = SVG_FindItemByPickupName(name);
//    if (!it) {
//        name = gi.argv(1); // C++20: Added cast.
//        it = SVG_FindItemByPickupName(name);
//        if (!it) {
//            gi.CPrintf(ent, PRINT_HIGH, "unknown item\n");
//            return;
//        }
//    }
//
//    if (!it->Pickup) {
//        gi.CPrintf(ent, PRINT_HIGH, "non-pickup item\n");
//        return;
//    }
//
//    index = ITEM_INDEX(it);
//
//    if (it->flags & ItemFlags::IsAmmo) {
//        if (gi.argc() == 3)
//            ent->client->persistent.inventory[index] = atoi(gi.argv(2));
//        else
//            ent->client->persistent.inventory[index] += it->quantity;
//    } else {
////        it_ent = SVG_Spawn();
////        it_ent->classname = it->classname;
////        SVG_SpawnItem(it_ent, it);
//////        SVG_TouchItem(it_ent, ent, NULL, NULL); Items..
////        if (it_ent->inUse)
////            SVG_FreeEntity(it_ent);
//    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f(SVGBasePlayer *clientEntity, ServerClient *client) {
    //if (deathmatch->value && !sv_cheats->value) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
    //    return;
    //}

    clientEntity->SetFlags(clientEntity->GetFlags() ^ EntityFlags::GodMode);
    if (!(clientEntity->GetFlags() & EntityFlags::GodMode))
        SVG_CPrintf(clientEntity, PRINT_HIGH, "godmode OFF\n");
    else
        SVG_CPrintf(clientEntity, PRINT_HIGH, "godmode ON\n");
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    //if (deathmatch->value && !sv_cheats->value) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
    //    return;
    //}

    clientEntity->SetFlags(clientEntity->GetFlags() ^ EntityFlags::NoTarget);
    if (!(clientEntity->GetFlags() & EntityFlags::NoTarget))
        SVG_CPrintf(clientEntity, PRINT_HIGH, "notarget OFF\n");
    else
        SVG_CPrintf(clientEntity, PRINT_HIGH, "notarget ON\n");
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f(SVGBasePlayer *clientEntity, ServerClient *client) {
    //if (deathmatch->value && !sv_cheats->value) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
    //    return;
    //}

    if (clientEntity->GetMoveType() == MoveType::NoClip) {
        clientEntity->SetMoveType(MoveType::Walk);
        SVG_CPrintf(clientEntity, PRINT_HIGH, "noclip OFF\n");
    } else {
        clientEntity->SetMoveType(MoveType::NoClip);
        SVG_CPrintf(clientEntity, PRINT_HIGH, "noclip ON\n");
    }
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f(SVGBasePlayer *clientEntity, ServerClient *client) {
    int         index;
    gitem_t     *it = nullptr;
    const char        *s;

    s = gi.args(); // C++20: Added casts.
    //it = SVG_FindItemByPickupName(s);
    if (!it) {
	    SVG_CPrintf(clientEntity, PRINT_HIGH, "unknown item: " + std::string(s) + "\n");
        return;
    }
    if (!it->Use) {
        SVG_CPrintf(clientEntity, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
//    index = ITEM_INDEX(it);
    if (!client->persistent.inventory[index]) {
    	SVG_CPrintf(clientEntity, PRINT_HIGH, "Out of item: " + std::string(s) + "\n");
        return;
    }

    //it->Use(ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f(SVGBasePlayer*ent)
{
    int         index;
    gitem_t     *it;
    const char        *s;

    s = (const char*)gi.args(); // C++20: Added casts.
    //it = SVG_FindItemByPickupName(s);
    //if (!it) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "unknown item: %s\n", s);
    //    return;
    //}
    //if (!it->Drop) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Item is not dropable.\n");
    //    return;
    //}
//    index = ITEM_INDEX(it);
    //if (!ent->GetClient()->persistent.inventory[index]) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Out of item: %s\n", s);
    //    return;
    //}

    //it->Drop(ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f(Entity *ent)
{
    int         i;
    ServerClient   *cl;

    cl = ent->client;

    cl->showScores = false;

    if (cl->showInventory) {
        cl->showInventory = false;
        return;
    }

    cl->showInventory = true;

    gi.WriteByte(ServerGameCommands::Inventory);
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
void Cmd_InvUse_f(SVGBasePlayer *clientEntity, ServerClient *client) {
    gitem_t     *it;

    //HUD_ValidateSelectedItem(ent);

    if (client->persistent.selectedItem == -1) {
        SVG_CPrintf(clientEntity, PRINT_HIGH, "No item to use.\n");
        return;
    }

    //it = &itemlist[ent->GetClient()->persistent.selectedItem];
    //if (!it->Use) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Item is not usable.\n");
    //    return;
    //}
    //it->Use(ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    ServerClient   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    //cl = ent->GetClient();

    //if (!cl->persistent.activeWeapon)
    //    return;

    //selected_weapon = -0;// ITEM_INDEX(cl->persistent.activeWeapon);

    //// scan  for the next valid one
    //for (i = 1 ; i <= MAX_ITEMS ; i++) {
    //    index = (selected_weapon + i) % MAX_ITEMS;
    //    if (!cl->persistent.inventory[index])
    //        continue;
    //    it = &itemlist[index];
    //    if (!it->Use)
    //        continue;
    //    if (!(it->flags & ItemFlags::IsWeapon))
    //        continue;
    //    it->Use(ent, it);
    //    if (cl->persistent.activeWeapon == it)
    //        return; // successful
    //}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    //ServerClient   *cl;
    //int         i, index;
    //gitem_t     *it;
    //int         selected_weapon;

    //cl = ent->GetClient();

    //if (!cl->persistent.activeWeapon)
    //    return;

    //selected_weapon = ITEM_INDEX(cl->persistent.activeWeapon);

    //// scan  for the next valid one
    //for (i = 1 ; i <= MAX_ITEMS ; i++) {
    //    index = (selected_weapon + MAX_ITEMS - i) % MAX_ITEMS;
    //    if (!cl->persistent.inventory[index])
    //        continue;
    //    it = &itemlist[index];
    //    if (!it->Use)
    //        continue;
    //    if (!(it->flags & ItemFlags::IsWeapon))
    //        continue;
    //    it->Use(ent, it);
    //    if (cl->persistent.activeWeapon == it)
    //        return; // successful
    //}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    //ServerClient   *cl;
    //int         index;
    //gitem_t     *it;

    //cl = ent->GetClient();

    //if (!cl->persistent.activeWeapon || !cl->persistent.lastWeapon)
    //    return;

    //index = ITEM_INDEX(cl->persistent.lastWeapon);
    //if (!cl->persistent.inventory[index])
    //    return;
    //it = &itemlist[index];
    //if (!it->Use)
    //    return;
    //if (!(it->flags & ItemFlags::IsWeapon))
    //    return;
    //it->Use(ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    gitem_t     *it;

    //HUD_ValidateSelectedItem(ent);

    //if (ent->GetClient()->persistent.selectedItem == -1) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "No item to drop.\n");
    //    return;
    //}

    //it = &itemlist[ent->GetClient()->persistent.selectedItem];
    //if (!it->Drop) {
    //    gi.CPrintf(ent->GetServerEntity(), PRINT_HIGH, "Item is not dropable.\n");
    //    return;
    //}
    //it->Drop(ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    if ((level.time - client->respawnTime) < 5)
        return;

    clientEntity->SetFlags(clientEntity->GetFlags() & ~EntityFlags::GodMode);
    clientEntity->SetHealth(0);
    game.GetCurrentGamemode()->SetCurrentMeansOfDeath(MeansOfDeath::Suicide);
    clientEntity->Die(clientEntity, clientEntity, 100000, vec3_zero());
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    client->showScores = false;
    client->showInventory = false;
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
void Cmd_Players_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    int32_t numConnectedClients = 0;
    char    small[64];
    char    large[1280];
    int     index[256];

    // Store indices of the currently connected clients.
    for (int32_t i = 0; i < maximumclients->value; i++) { 
        if (game.clients[i].persistent.isConnected) {
	        index[numConnectedClients] = i;
	        numConnectedClients++;
        }
    }

    // Sort connected clients by frags
    qsort(index, numConnectedClients, sizeof(index[0]), PlayerSort);

    // 0 string print information
    large[0] = 0;

    for (int32_t i = 0; i < numConnectedClients; i++) {
        // Generate score string.
        Q_snprintf(small, sizeof(small), "%3i %s\n",
                   game.clients[index[i]].playerState.stats[STAT_FRAGS],
                   game.clients[index[i]].persistent.netname);

        // Ensure buffer doesn't overflow.
        if (strlen(small) + strlen(large) > sizeof(large) - 100) {
            // can't print all of them in one packet
            strcat(large, "...\n");
            break;
        }

        // Cattenate
        strcat(large, small);
    }

    SVG_CPrintf(clientEntity, PRINT_HIGH, std::string(large) + std::string("\n") + std::to_string(numConnectedClients) + std::string(" players\n"));
}

///*
//=================
//Cmd_Wave_f
//=================
//*/
//void Cmd_Wave_f(Entity *ent)
//{
//    int     i;
//
//    i = atoi(gi.argv(1));
//
//    // can't wave when ducked
//    if (ent->client->playerState.pmove.flags & PMF_DUCKED)
//        return;
//
//    if (ent->client->animation.priorityAnimation > PlayerAnimation::Wave)
//        return;
//
//    ent->client->animation.priorityAnimation = PlayerAnimation::Wave;
//
//    switch (i) {
//    case 0:
//        gi.CPrintf(ent, PRINT_HIGH, "flipoff\n");
//        ent->state.frame = FRAME_flip01 - 1;
//        ent->client->animation.endFrame = FRAME_flip12;
//        break;
//    case 1:
//        gi.CPrintf(ent, PRINT_HIGH, "salute\n");
//        ent->state.frame = FRAME_salute01 - 1;
//        ent->client->animation.endFrame = FRAME_salute11;
//        break;
//    case 2:
//        gi.CPrintf(ent, PRINT_HIGH, "taunt\n");
//        ent->state.frame = FRAME_taunt01 - 1;
//        ent->client->animation.endFrame = FRAME_taunt17;
//        break;
//    case 3:
//        gi.CPrintf(ent, PRINT_HIGH, "wave\n");
//        ent->state.frame = FRAME_wave01 - 1;
//        ent->client->animation.endFrame = FRAME_wave11;
//        break;
//    case 4:
//    default:
//        gi.CPrintf(ent, PRINT_HIGH, "point\n");
//        ent->state.frame = FRAME_point01 - 1;
//        ent->client->animation.endFrame = FRAME_point12;
//        break;
//    }
//}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f(SVGBasePlayer *clientEntity, ServerClient *client, qboolean team, qboolean arg0)
{
    int     i, j;

    char    *p; // C++20: Removed const.
    char    text[2048];

    // Buffer for text to "say".
    std::string sayBuffer = "";

    // Ensure we at least got 2 arguments with this server command.
    if (gi.argc() < 2 && !arg0)
        return;

    // Check whether we are in a teamplay game.
    if (!((int)(gamemodeflags->value) & (GamemodeFlags::ModelTeams | GamemodeFlags::SkinTeams))) {
        team = false;
    }

    // Should we print it as a team member or not.
    if (team) {
	    sayBuffer = "(" + std::string(client->persistent.netname) + "): "; // Q_snprintf(text, sizeof(text), "(%s): ", ent->client->persistent.netname);
    } else {
	    sayBuffer = std::string(client->persistent.netname) + ": "; //Q_snprintf(text, sizeof(text), "%s: ", ent->client->persistent.netname);
    }

    if (arg0) {
	    sayBuffer += gi.argv(0);
	    sayBuffer += " ";
	    sayBuffer += gi.args();
        //strcat(text, gi.argv(0));
        //strcat(text, " ");
        //strcat(text, gi.args());
    } else {
        // This is ugly but will do for now.
        p = (char*)gi.args();  // C++20: Added casts.

        if (*p == '"') {
            p++;
            p[strlen(p) - 1] = 0;
        }
//        strcat(text, p);
	    sayBuffer += p;
    }

    // don't let text be too long for malicious reasons
    if (sayBuffer.length() > 150) {
	    sayBuffer = sayBuffer.substr(0, 150);
	}

    // Append newline.
	sayBuffer += "\n";

    //strcat(text, "\n");

    // Flood msg protection.
    if (flood_msgs->value) {
        // Notify client of his spamming behaviors.
        if (level.time < client->flood.lockTill) {
	        SVG_CPrintf(clientEntity, PRINT_HIGH, "You can't talk for " + std::to_string((int)(client->flood.lockTill - level.time)) + " more seconds\n ");
            return;
        }

        i = client->flood.whenHead - flood_msgs->value + 1;
	    if (i < 0) {
	        i = (sizeof(client->flood.when) / sizeof(client->flood.when[0])) + i;
	    }

        if (client->flood.when[i] && level.time - client->flood.when[i] < flood_persecond->value) {
            client->flood.lockTill = level.time + flood_waitdelay->value;
    	    SVG_CPrintf(clientEntity, PRINT_CHAT, "Flood protection:  You can't talk for " + std::to_string(static_cast<int>(flood_waitdelay->value)) + " seconds.\n ");
            return;
        }

        // Circle around our buffer.
        client->flood.whenHead = (client->flood.whenHead + 1) % (sizeof(client->flood.when) / sizeof(client->flood.when[0]));
        client->flood.when[client->flood.whenHead] = level.time;
    }

    if (dedicated->value)
        SVG_CPrintf(NULL, PRINT_CHAT, sayBuffer);

    // Loop over client entities.
    for (auto& otherClientEntity : GetBaseEntityRange(1, game.GetMaxClients()) | bef::Standard | bef::HasClient) {
        if (team) {
            if (!SVG_OnSameTeam(clientEntity, otherClientEntity))
                continue;
        }

        SVG_CPrintf(otherClientEntity, PRINT_CHAT, sayBuffer);
    }
}

void Cmd_PlayerList_f(SVGBasePlayer* clientEntity, ServerClient* client) {
    int i;
    char st[80];
    char text[1400];
    Entity *e2;

    // connect time, ping, score, name
    *text = 0;
    for (i = 0, e2 = g_entities + 1; i < maximumclients->value; i++, e2++) {
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
            SVG_CPrintf(clientEntity, PRINT_HIGH, text);
            return;
        }
        strcat(text, st);
    }
    SVG_CPrintf(clientEntity, PRINT_HIGH, text);
}


/*
=================
ClientCommand
=================
*/
void SVG_ClientCommand(Entity* serverEntity) {
    //
    // TODO In the future the contents of this function will move along
    // into the gi interface implementation. Leaving this function for just
    // checking if the entity is valid to work with.
    //
    //

    // Fetch client entity.
    SVGBasePlayer* clientEntity = GetPlayerClientClassentity(serverEntity);

    // Fetch its client pointer.
    ServerClient *client = clientEntity->GetClient();

    // Fetch cmd.
    std::string command = gi.argv(0);

    if (command == "players") {
        Cmd_Players_f(clientEntity, client);
        return;
    }
    if (command == "say") {
        Cmd_Say_f(clientEntity, client, false, false);
        return;
    }
    if (command == "say_team") {
        Cmd_Say_f(clientEntity, client, true, false);
        return;
    }
    if (command == "score") {
        SVG_Command_Score_f(clientEntity, client);
        return;
    }

    if (level.intermission.time)
        return;

/*    if (command == "use")
        Cmd_Use_f(ent);
    else if (command == "drop")
        Cmd_Drop_f(ent);
    else if (command == "give")
        Cmd_Give_f(ent->GetServerEntity());
    else */if (command == "god")
        Cmd_God_f(clientEntity, client);
    else if (command == "notarget")
        Cmd_Notarget_f(clientEntity, client);
    else if (command == "noclip")
        Cmd_Noclip_f(clientEntity, client);
    //else if (PH_StringCompare(cmd, "inven") == 0)
    //    Cmd_Inven_f(ent->GetServerEntity());
    //else if (PH_StringCompare(cmd, "invnext") == 0)
    //    SelectNextItem(ent, -1);
    //else if (PH_StringCompare(cmd, "invprev") == 0)
    //    SelectPrevItem(serverEntity, -1);
    //else if (PH_StringCompare(cmd, "invnextw") == 0)
    //    SelectNextItem(ent, ItemFlags::IsWeapon);
    //else if (PH_StringCompare(cmd, "invprevw") == 0)
    //    SelectPrevItem(serverEntity, ItemFlags::IsWeapon);
    //else if (PH_StringCompare(cmd, "invnextp") == 0)
    //    SelectNextItem(ent, ItemFlags::IsPowerUp);
    //else if (PH_StringCompare(cmd, "invprevp") == 0)
    //    SelectPrevItem(serverEntity, ItemFlags::IsPowerUp);
    else if (command == "invuse")
        Cmd_InvUse_f(clientEntity, client);
    else if (command == "invdrop")
        Cmd_InvDrop_f(clientEntity, client);
    //else if (PH_StringCompare(cmd, "weapprev") == 0)
    //    Cmd_WeapPrev_f(ent);
    //else if (PH_StringCompare(cmd, "weapnext") == 0)
    //    Cmd_WeapNext_f(ent);
    else if (command == "weaplast")
        Cmd_WeapLast_f(clientEntity, client);
    else if (command == "kill")
        Cmd_Kill_f(clientEntity, client);
    else if (command == "putaway")
        Cmd_PutAway_f(clientEntity, client);
    //else if (PH_StringCompare(cmd, "wave") == 0)
    //    Cmd_Wave_f(serverEntity);
    else if (command == "playerlist")
        Cmd_PlayerList_f(clientEntity, client);
    else    // anything that doesn't match a command will be a chat
        Cmd_Say_f(clientEntity, client, false, true);
}
