/*
// LICENSE HERE.

//
// DefaultGameMode.cpp
//
//
*/
#include "../g_local.h"          // SVGame.

// Server Game Base Entity.
#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/PlayerClient.h"

// Weapons.h
#include "../player/client.h"
#include "../player/weapons.h"

// Game Mode.
#include "DefaultGameMode.h"

//
// Constructor/Deconstructor.
//
DefaultGameMode::DefaultGameMode() {

}
DefaultGameMode::~DefaultGameMode() {

}

//
// Interface functions. 
//
//
//===============
// DefaultGameMode::OnSameTeam
//
// Returns false either way, because yes, there is no... team in this case.
// PS: ClientTeam <-- weird function, needs C++-fying and oh.. it stinks anyhow.
//===============
//
qboolean DefaultGameMode::OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) {
    //char    ent1Team[512];
    //char    ent2Team[512];

    //if (!((int)(dmflags->value) & (GameModeFlags::ModelTeams | GameModeFlags::SkinTeams)))
    //    return false;

    ////strcpy(ent1Team, ClientTeam(ent1));
    ////strcpy(ent2Team, ClientTeam(ent2));

    //if (strcmp(ent1Team, ent2Team) == 0)
    //    return true;
    return false;
}

//
//===============
// DefaultGameMode::CanDamage
//
//===============
//
qboolean DefaultGameMode::CanDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor) {
	
    vec3_t  dest;
    SVGTrace trace;

    // WID: Admer, why the fuck did they rush hour these comments all the time?
    // bmodels need special checking because their origin is 0,0,0 <-- is bad.
    //
    // Solid entities need a special check, as their origin is usually 0,0,0
    // Exception to the above: the solid entity moves or has an origin brush
    if (target->GetMoveType() == MoveType::Push) {
        // Calculate destination.
        dest = vec3_scale(target->GetAbsoluteMin() + target->GetAbsoluteMax(), 0.5f);
        trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), dest, inflictor, CONTENTS_MASK_SOLID);
        if (trace.fraction == 1.0)
            return true;
        if (trace.ent == target)
            return true;
        return false;
    }

    // From here on we start tracing in various directions. Look at the code yourself to figure that one out...
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), target->GetOrigin(), inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    dest = target->GetOrigin();
    dest[0] += 15.0;
    dest[1] += 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    dest = target->GetOrigin();
    dest[0] += 15.0;
    dest[1] -= 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    dest = target->GetOrigin();
    dest[0] -= 15.0;
    dest[1] += 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    dest = target->GetOrigin();
    dest[0] -= 15.0;
    dest[1] -= 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    // If we reached this point... Well, it is false :)
    return false;
}


//
//===============
// DefaultGameMode::SpawnTempDamageEntity
// 
// Sends a message to all clients in the current PVS, spawning a temp entity for
// displaying damage entities client side. (Sparks, what have ya.)
//===============
//
void DefaultGameMode::SpawnTempDamageEntity(int type, const vec3_t& origin, const vec3_t& normal, int damage) {
    // WID: Ensure the effect can't send more damage. But that is unimplemented for the clients atm to even detect...
    if (damage > 255)
        damage = 255;
    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(type);
    //  gi.WriteByte (damage); // <-- This was legacy crap, might wanna implement it ourselves eventually.
    gi.WriteVector3(origin);
    gi.WriteVector3(normal);
    gi.Multicast(origin, MultiCast::PVS);
}

//
//===============
// DefaultGameMode::CalculateDamageVelocity
// 
// Default implementation for calculating velocity damage.
//===============
//
vec3_t DefaultGameMode::CalculateDamageVelocity(int32_t damage) {
    // Pick random velocities.
    vec3_t velocity = {
        100.0f * crandom(),
        100.0f * crandom(),
        200.0f + 100.0f * random()
    };

    // Scale velocities.
    if (damage < 50)
        velocity = vec3_scale(velocity, 0.7f);
    else
        velocity = vec3_scale(velocity, 1.2f);

    // Return.
    return velocity;
}

//
//===============
// DefaultGameMode::OnLevelExit
// 
// Default implementation for exiting levels.
//===============
//
void DefaultGameMode::OnLevelExit() {
    // Create the command to use for switching to the next game map.
    std::string command = "gamemap \"";
    command += level.intermission.changeMap;
    command += +"\"";

    // Add the gamemap command to the 
    gi.AddCommandString(command.c_str());
    // Reset the changeMap string, intermission time, and regular level time.
    level.intermission.changeMap = NULL;
    level.intermission.exitIntermission = 0;
    level.intermission.time = 0;

    // End the server frames for all clients.
    SVG_ClientEndServerFrames();

    // Fetch the WorldSpawn entity number.
    int32_t stateNumber = g_entities[0].state.number;

    // Fetch the corresponding base entity.
    SVGBaseEntity* entity = g_baseEntities[stateNumber];

    // Loop through the server entities, and run the base entity frame if any exists.
    for (int32_t i = 0; i < globals.numberOfEntities; i++) {
        // Acquire state number.
        stateNumber = g_entities[i].state.number;

        // Fetch the corresponding base entity.
        SVGBaseEntity* entity = g_baseEntities[stateNumber];

        // Is it even valid?
        if (entity == nullptr)
            continue;

        // Don't go on if it isn't in use.
        if (!entity->IsInUse())
            continue;

        // Continue in case... cuz we know...
        if (!entity->GetClient())
            continue;

        // Ensure an entity its health is reset to default.
        if (entity->GetHealth() > entity->GetClient()->persistent.maxHealth)
            entity->SetHealth(entity->GetClient()->persistent.maxHealth);
    }
}

//===
// DefaultGameMode::ClientBeginServerFrame
// 
// Does logic checking for a client's start of a server frame. In case there
// is a "level.intermission.time" set, it'll flat out return.
// 
// This basically allows for the game to disable fetching user input that makes
// our movement tick. And/or shoot weaponry while in intermission time.
//
void DefaultGameMode::ClientBeginServerFrame(PlayerClient* player) {
    // Ensure we aren't in an intermission time.
    if (level.intermission.time)
        return;

    // Is the entity valid?
    if (!player)
        return;

    // Fetch the client.
    GameClient* client = player->GetClient();

    // In case there is no client, return.
    if (!client)
        return;

    // This has to go ofc.... lol. What it simply does though, is determine whether there is 
    // a need to respawn as spectator.
    //if (deathmatch->value &&
    //    client->persistent.isSpectator != client->respawn.isSpectator &&
    //    (level.time - client->respawnTime) >= 5) {
    //    spectator_respawn(ent->GetServerEntity());
    //    return;
    //}

    // Run weapon animations in case this has not been done by user input itself.
    // (Idle animations, and general weapon thinking when a weapon is not in action.)
    if (!client->weaponThunk && !client->respawn.isSpectator)
        SVG_ThinkWeapon(player);
    else
        client->weaponThunk = false;

    // Check if the player is actually dead or not. If he is, we're going to enact on
    // the user input that's been given to us. When fired, we'll respawn.
    int32_t buttonMask = 0;
    if (player->GetDeadFlag()) {
        // Wait for any button just going down
        if (level.time > client->respawnTime) {
            // In old code, the need to hit a key was only set in DM mode.
            // I figured, let's keep it like this instead.
            //if (deathmatch->value)
                buttonMask = BUTTON_ATTACK;
            //else
            //buttonMask = -1;
            
            if ((client->latchedButtons & buttonMask) ||
                (deathmatch->value && ((int)dmflags->value & GameModeFlags::ForceRespawn))) {
                SVG_RespawnClient(player->GetServerEntity());
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