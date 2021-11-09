/*
// LICENSE HERE.

//
// DefaultGameMode.cpp
//
//
*/
#include "../g_local.h"          // SVGame.
#include "../effects.h"     // Effects.
#include "../entities.h"    // Entities.
#include "../utils.h"       // Util funcs.

// Server Game Base Entity.
#include "../entities/base/SVGBaseEntity.h"
#include "../entities/base/BodyCorpse.h"
#include "../entities/base/PlayerClient.h"

// Weapons.h
#include "../player/client.h"
#include "../player/hud.h"
#include "../player/weapons.h"
#include "../player/animations.h"

// Game Mode.
#include "DefaultGameMode.h"

//
// Constructor/Deconstructor.
//
DefaultGameMode::DefaultGameMode() {
    // Defaults.
    meansOfDeath = 0;
}
DefaultGameMode::~DefaultGameMode() {

}

//
// Interface functions. 
//
//===============
// DefaultGameMode::GetEntityTeamName
//
// Assigns the teamname to the string passed, returns false in case the entity
// is not part of a team at all.
//===============
qboolean DefaultGameMode::GetEntityTeamName(SVGBaseEntity* ent, std::string &teamName) {
    // Placeholder.
    teamName = "";

    return false;

    //// We'll assume that this entity needs to have a client.
    //if (!ent->GetClient())
    //    return "";

    //// Fetch the 'skin' info_valueforkey of the given client.
    //std::string clientSkin = Info_ValueForKey(ent->GetClient()->persistent.userinfo, "skin");

    //// Start scanning for a /, in case there is none, we can just return the skin as is.
    //auto slashPosition = clientSkin.find_last_of('/');
    //if (slashPosition == std::string::npos)
    //    return clientSkin;

    //// Since we did find one if we reach this code, we'll check waht our game mode flags demand.
    //if (gamemodeflags->integer & GameModeFlags::ModelTeams) {
    //    return clientSkin;
    //}

    //// Otherwise, in case we got skin teams... Return the skin specific part as team name.
    //if (gamemodeflags->integer & GameModeFlags::SkinTeams) {
    //    return clientSkin.substr(slashPosition);
    //}

    //// We should never reach this point, but... there just in case.
    //return "";
}

//===============
// DefaultGameMode::OnSameTeam
//
// Returns false either way, because yes, there is no... team in this case.
// PS: ClientTeam <-- weird function, needs C++-fying and oh.. it stinks anyhow.
//===============
qboolean DefaultGameMode::OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) {
    //// There is only a reason to check for this in case these specific
    //// game mode flags are set.
    //if (!((int)(gamemodeflags->value) & (GameModeFlags::ModelTeams | GameModeFlags::SkinTeams)))
    //    return false;

    //// Fetch the team names of both entities.
    //std::string teamEntity1 = GetEntityTeamName(ent1);
    //std::string teamEntity2 = GetEntityTeamName(ent2);

    //// In case they are equal, return true.
    //if (!teamEntity1.empty() && !teamEntity2.empty())
    //    if (teamEntity1 != "" && teamEntity2 != "")
    //        if (teamEntity1 == teamEntity2)
    //            return true;

    // If we reached this point, we're done, no going on.
    return false;
}

//===============
// DefaultGameMode::CanDamage
//
//===============
qboolean DefaultGameMode::CanDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor) {
    vec3_t  destination;
    SVGTrace trace;

    // WID: Admer, why the fuck did they rush hour these comments all the time?
    // bmodels need special checking because their origin is 0,0,0 <-- is bad.
    //
    // Solid entities need a special check, as their origin is usually 0,0,0
    // Exception to the above: the solid entity moves or has an origin brush
    if (target->GetMoveType() == MoveType::Push) {
        // Calculate destination.
        destination = target->GetAbsoluteMin() + target->GetAbsoluteMax();
        destination = vec3_scale(destination, 0.5f);
        trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, CONTENTS_MASK_SOLID);
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

    destination = target->GetOrigin();
    destination[0] += 15.0;
    destination[1] += 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    destination = target->GetOrigin();
    destination[0] += 15.0;
    destination[1] -= 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    destination = target->GetOrigin();
    destination[0] -= 15.0;
    destination[1] += 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    destination = target->GetOrigin();
    destination[0] -= 15.0;
    destination[1] -= 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    // If we reached this point... Well, it is false :)
    return false;
}

//===============
// DefaultGameMode::FindWithinRadius
//
// Returns a BaseEntityVector list containing the results of the found
// given entities that reside within the origin to radius. 
// 
// Flags can be set to determine which "solids" to exclude.
//===============
BaseEntityVector DefaultGameMode::FindBaseEnitiesWithinRadius(const vec3_t& origin, float radius, uint32_t excludeSolidFlags) {
    // List of base entities to return.
    std::vector<SVGBaseEntity*> baseEntityList;

    // Iterate over all entities, see who is nearby, and who is not.
    for (auto* radiusBaseEntity : GetBaseEntityRange<0, MAX_EDICTS>()
         | bef::Standard
         | bef::WithinRadius(origin, radius, excludeSolidFlags)) {

        // Push radiusEntity result item to the list.
        baseEntityList.push_back(radiusBaseEntity);
    }

    // The list might be empty, ensure to check for that ;-)
    return baseEntityList;
}

//===============
// DefaultGameMode::EntityKilled
//
// Called when an entity is killed, or at least, about to be.
// Determine how to deal with it, usually resides in a callback to Die.
//===============
void DefaultGameMode::EntityKilled(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int32_t damage, vec3_t point) {
    // Ensure health isn't exceeding limits.
    if (target->GetHealth() < -999)
        target->SetHealth(-999);

    // Set the enemy pointer to the current attacker.
    target->SetEnemy(attacker);

    // Determine whether it is a monster, and if it IS set to being dead....
    if ((target->GetServerFlags() & EntityServerFlags::Monster) && (target->GetDeadFlag() != DEAD_DEAD)) {
        target->SetServerFlags(target->GetServerFlags() | EntityServerFlags::DeadMonster);   // Now treat as a different content type

//        if (!(targ->monsterInfo.aiflags & AI_GOOD_GUY)) {
//            level.killedMonsters++;
//            if (coop->value && attacker->client)
//                attacker->client->respawn.score++;
//            // medics won't heal monsters that they kill themselves
//            if (strcmp(attacker->className, "monster_medic") == 0)
//                targ->owner = attacker;
//        }
    }

    if (target->GetMoveType() == MoveType::Push || target->GetMoveType() == MoveType::Stop || target->GetMoveType() == MoveType::None) {
        // Doors, triggers, etc
        if (target) {
            target->Die(inflictor, attacker, damage, point);
        }

        return;
    }

    if ((target->GetServerFlags() & EntityServerFlags::Monster) && (target->GetDeadFlag() != DEAD_DEAD)) {
        target->SetTouchCallback(nullptr);

        // This can only be done on base monster entities and derivates
        //if (target->IsSubclassOf<BaseMonster>()) {
        //    target->DeathUse();
        //}
    //    monster_death_use(targ);
    }
    if (target) {
        target->Die(inflictor, attacker, damage, point);
    }
    //targ->Die(targ, inflictor, attacker, damage, point);
}

//===============
// DefaultGameMode::InflictDamage
// 
//===============
void DefaultGameMode::InflictDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, const vec3_t& dmgDir, const vec3_t& point, const vec3_t& normal, int32_t damage, int32_t knockBack, int32_t damageFlags, int32_t mod) {
    int32_t damageTaken = 0;   // Damage taken.
    int32_t damageSaved = 0;   // Damaged saved, from being taken.

                               // Best be save than sorry.
    if (!target || !inflictor || !attacker) {
        return;
    }

    // In case this entity is not taking any damage, all bets are off, don't bother moving on.
    if (!target->GetTakeDamage())
        return;

    // WID: This sticks around, cuz of reference, but truly will be all but this itself was.
    // friendly fire avoidance
    // if enabled you can't hurt teammates (but you can hurt yourself)
    // knockBack still occurs
    //if ((targ != attacker) && ((deathmatch->value && ((int)(gamemodeflags->value) & (GameModeFlags::ModelTeams | GameModeFlags::SkinTeams))) || coop->value)) {
    //    if (game.gameMode->OnSameTeam(targ, attacker)) {
    //        if ((int)(gamemodeflags->value) & GameModeFlags::NoFriendlyFire)
    //            damage = 0;
    //        else
    //            mod |= MeansOfDeath::FriendlyFire;
    //    }
    //}
    // We resort to defaults, but keep the above as mentioned.
    SetCurrentMeansOfDeath(mod);

    // Fetch client.
    GameClient* client = target->GetClient();

    // Lame thing, regarding the sparks to use. Ancient code, keeping it for now.
    int32_t te_sparks = TempEntityEvent::Sparks;
    if (damageFlags & DamageFlags::Bullet)
        te_sparks = TempEntityEvent::BulletSparks;

    // Retrieve normalized direction.
    vec3_t dir = vec3_normalize(dmgDir);

    // Ensure there is no odd knockback issues.
    if (target->GetFlags() & EntityFlags::NoKnockBack)
        knockBack = 0;

    // Figure out the momentum to add in case KnockBacks are off. 
    if (!(damageFlags & DamageFlags::NoKnockBack)) {
        if ((knockBack) && (target->GetMoveType() != MoveType::None) && (target->GetMoveType() != MoveType::Bounce) && (target->GetMoveType() != MoveType::Push) && (target->GetMoveType() != MoveType::Stop)) {
            vec3_t  kvel = { 0.f, 0.f, 0.f };
            float   mass = 50; // Defaults to 50, otherwise... issues, this is the OG code style btw.

                               // Based on mass, if it is below 50, we wanna hook it to being 50. Otherwise...
            if (target->GetMass() > 50)
                mass = target->GetMass();

            // Determine whether attacker == target, and the client itself, that means we gotta jump back hard.
            if (target->GetClient() && attacker == target)
                kvel = vec3_scale(dir, 1600.0 * (float)knockBack / mass); // ROCKET JUMP HACK IS HERE BRUH <--
            else
                kvel = vec3_scale(dir, 500 * (float)knockBack / mass);

            // Assign the new velocity, since yeah, it's bound to knock the fuck out of us.
            target->SetVelocity(target->GetVelocity() + kvel);
        }
    }

    // Setup damages, so we can maths with them, yay. Misses code cuz we got no armors no more :P
    damageTaken = damage;       // Damage taken.
    damageSaved = 0;            // Damaged saved, from being taken.

                                // check for godmode
    if ((target->GetFlags() & EntityFlags::GodMode) && !(damageFlags & DamageFlags::IgnoreProtection)) {
        damageTaken = 0;
        damageSaved = damage;

        // Leave it for the game mode to move on and spawn this temp entity (if allowed.)
        SpawnTempDamageEntity(te_sparks, point, normal, damageSaved);
    }

    // Team damage avoidance
    if (!(damageFlags & DamageFlags::IgnoreProtection) && game.gameMode->OnSameTeam(target, attacker))
        return;

    // Inflict the actual damage, in case we got to deciding to do so based on the above.
    if (damageTaken) {
        // Check if monster, or client, in which case, we spawn blood.
        // If not... :)... Do not.
        if ((target->GetServerFlags() & EntityServerFlags::Monster) || (client)) {
            // SpawnTempDamageEntity(TempEntityEvent::Blood, point, normal, take);
            // Leave it for the game mode to move on and spawn this temp entity (if allowed.)
            SpawnTempDamageEntity(TempEntityEvent::Blood, point, dir, damageTaken);
        } else {
            // Leave it for the game mode to move on and spawn this temp entity (if allowed.)
            SpawnTempDamageEntity(te_sparks, point, normal, damageTaken);
        }

        // Adjust health based on calculated damage to take.
        target->SetHealth(target->GetHealth() - damageTaken);

        // In case health is/was below 0.
        if (target->GetHealth() <= 0) {
            // Check if monster, or client, in which case, we execute no knockbacks :)
            if ((target->GetServerFlags() & EntityServerFlags::Monster) || (client))
                target->SetFlags(target->GetFlags() | EntityFlags::NoKnockBack);

            // It's dead though, or at least we assume so... Call on to: EntityKilled.
            EntityKilled(target, inflictor, attacker, damageTaken, point);
            return;
        }
    }

    // Special damage handling for monsters.
    if (target->GetServerFlags() & EntityServerFlags::Monster) {
        // WID: Maybe do some check for monster entities here sooner or later? Who knows...
        // Gotta have them cunts react to it. But we'll see, might as well be on TakeDamage :)
        //M_ReactToDamage(targ, attacker);

        //if (!(targ->monsterInfo.aiflags & AI_DUCKED) && (take)) {
        target->TakeDamage(attacker, knockBack, damageTaken);
        //// nightmare mode monsters don't go into pain frames often
        //if (skill->value == 3)
        //    targ->debouncePainTime = level.time + 5;
        //}
    } else {
        if (client) {
            //if (!(targ->flags & EntityFlags::GodMode) && (take))
            //    targ->Pain(targ, attacker, knockBack, take);
            if (!(target->GetFlags() & EntityFlags::GodMode) && (damageTaken)) {
                target->TakeDamage(attacker, knockBack, damageTaken);
            }
        } else if (damageTaken) {
            target->TakeDamage(attacker, knockBack, damageTaken);
        }
    }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    if (client) {
        client->damages.blood += damageTaken;
        client->damages.knockBack += knockBack;
        client->damages.from = point;
    }
}

//===============
// DefaultGameMode::InflictDamage
// 
//===============
void DefaultGameMode::InflictRadiusDamage(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, float damage, SVGBaseEntity* ignore, float radius, int32_t mod) {
    // Damage point counter for radius sum ups.
    float   points = 0.f;

    // Actual entity loop pointer.
    SVGBaseEntity* ent = nullptr;

    // N&C: From Yamagi Q2, to prevent issues.
    if (!inflictor || !attacker) {
        return;
    }

    // Find entities within radius.
    BaseEntityVector radiusEntities = FindBaseEnitiesWithinRadius(inflictor->GetOrigin(), radius, Solid::Not);

    //while ((ent = SVG_FindEntitiesWithinRadius(ent, inflictor->GetOrigin(), radius)) != NULL) {
    for (auto& baseEntity : radiusEntities) {
        //// Continue in case this entity has to be ignored from applying damage.
        if (baseEntity == ignore)
            continue;

        // Continue in case this entity CAN'T take any damage.
        if (!baseEntity->GetTakeDamage())
            continue;

        // Calculate damage points.
        vec3_t velocity = baseEntity->GetMins() + baseEntity->GetMaxs();
        velocity = vec3_fmaf(baseEntity->GetOrigin(), 0.5f, velocity);
        velocity -= inflictor->GetOrigin();
        points = damage - 0.5f * vec3_length(velocity);

        // In case the attacker is the own entity, half damage.
        if (ent == attacker)
            points = points * 0.5f;

        // Apply damage points.
        if (points > 0) {
            // Ensure whether we CAN actually apply damage.
            if (CanDamage(baseEntity, inflictor)) {
                // Calculate direcion.
                vec3_t dir = baseEntity->GetOrigin() - inflictor->GetOrigin();

                // Apply damages.
                InflictDamage(baseEntity, inflictor, attacker, dir, inflictor->GetOrigin(), vec3_zero(), (int)points, (int)points, DamageFlags::IndirectFromRadius, mod);
            }
        }
    }
}

//===============
// DefaultGameMode::SetCurrentMeansOfDeath
// 
//===============
void DefaultGameMode::SetCurrentMeansOfDeath(int32_t meansOfDeath) {
    this->meansOfDeath = meansOfDeath;
}

//===============
// DefaultGameMode::GetCurrentMeansOfDeath
// 
//===============
const int32_t& DefaultGameMode::GetCurrentMeansOfDeath() {
    return this->meansOfDeath;
}

//===============
// DefaultGameMode::SpawnClientCorpse
// 
// Spawns a dead body entity for the given client.
//===============
void DefaultGameMode::SpawnClientCorpse(SVGBaseEntity* ent) {
    // Ensure it is an entity.
    if (!ent)
        return;

    // Ensure it is a client.
    if (!ent->GetClient())
        return;

    // Unlink the player client entity.
    ent->UnlinkEntity();

    // Grab a body from the queue, and cycle to the next one.
    Entity *bodyEntity = &g_entities[game.maximumClients + level.bodyQue + 1];
    level.bodyQue = (level.bodyQue + 1) % BODY_QUEUE_SIZE;

    // Send an effect on this body, in case it already has a model index.
    // This'll cause a body not to just "disappear", but actually play some
    // bloody particles over there.
    if (bodyEntity->state.modelIndex) {
        gi.WriteByte(SVG_CMD_TEMP_ENTITY);
        gi.WriteByte(TempEntityEvent::Blood);
        gi.WriteVector3(bodyEntity->state.origin);
        gi.WriteVector3(vec3_zero());
        gi.Multicast(bodyEntity->state.origin, MultiCast::PVS);
    }

    // Create the class entity for this queued bodyEntity.
    SVGBaseEntity *bodyClassEntity = SVG_CreateClassEntity<BodyCorpse>(bodyEntity, false);

    // Unlink the body entity, in case it was linked before.
    bodyClassEntity->UnlinkEntity();

    // Copy over the bodies state of the current entity into the body entity.
    bodyClassEntity->SetState(ent->GetState());
    // Change its number so it is accurately set to the one belonging to bodyEntity.
    // (Has to happen since we first copied over an entire entity state.)
    bodyClassEntity->SetNumber(bodyEntity - g_entities);
    // Set the event ID for this frame to OtherTeleport.
    bodyClassEntity->SetEventID(EntityEvent::OtherTeleport);

    // Copy over the serverflags from ent.
    bodyClassEntity->SetServerFlags(ent->GetServerFlags());
    bodyClassEntity->SetMins(ent->GetMins());
    bodyClassEntity->SetMaxs(ent->GetMaxs());
    bodyClassEntity->SetAbsoluteMin(ent->GetAbsoluteMin());
    bodyClassEntity->SetAbsoluteMax(ent->GetAbsoluteMax());
    bodyClassEntity->SetSize(ent->GetSize());
    bodyClassEntity->SetVelocity(ent->GetVelocity());
    bodyClassEntity->SetAngularVelocity(ent->GetAngularVelocity());
    bodyClassEntity->SetSolid(ent->GetSolid());
    bodyClassEntity->SetClipMask(ent->GetClipMask());
    bodyClassEntity->SetOwner(ent->GetOwner());
    bodyClassEntity->SetMoveType(ent->GetMoveType());
    bodyClassEntity->SetGroundEntity(ent->GetGroundEntity());

    // Set the die callback, and set its take damage.
    bodyClassEntity->SetDieCallback(&BodyCorpse::BodyCorpseDie);
    bodyClassEntity->SetTakeDamage(TakeDamage::Yes);

    // Link it in for collision etc.
    bodyClassEntity->LinkEntity();
}

//===============
// DefaultGameMode::SpawnTempDamageEntity
// 
// Sends a message to all clients in the current PVS, spawning a temp entity for
// displaying damage entities client side. (Sparks, what have ya.)
//===============
void DefaultGameMode::SpawnTempDamageEntity(int32_t type, const vec3_t& origin, const vec3_t& normal, int32_t damage) {
    // WID: Ensure the effect can't send more damage. But that is unimplemented for the clients atm to even detect...
    if (damage > 255)
        damage = 255;

    // Write away.
    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(type);
    //  gi.WriteByte (damage); // <-- This was legacy crap, might wanna implement it ourselves eventually.
    gi.WriteVector3(origin);
    gi.WriteVector3(normal);
    gi.Multicast(origin, MultiCast::PVS);
}

//===============
// DefaultGameMode::CalculateDamageVelocity
// 
// Default implementation for calculating velocity damage.
//===============
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

//===============
// DefaultGameMode::OnLevelExit
// 
// Default implementation for exiting levels.
//===============
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

    // Loop through the server entities, and run the base entity frame if any exists.
    for (int32_t i = 0; i < maximumClients->value; i++) {
        // Fetch the WorldSpawn entity number.
        Entity *serverEntity = &g_entities[i];

        if (!serverEntity)
            continue;

        if (!serverEntity->inUse)
            continue;

        uint32_t stateNumber = serverEntity->state.number;

        // Fetch the corresponding base entity.
        SVGBaseEntity* entity = g_baseEntities[stateNumber];

        // Ensure an entity its health is reset to default.
        if (entity->GetHealth() > entity->GetClient()->persistent.maxHealth)
            entity->SetHealth(entity->GetClient()->persistent.maxHealth);
    }
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
void DefaultGameMode::ClientBeginServerFrame(Entity* serverEntity) {
    // Ensure we aren't in an intermission time.
    if (level.intermission.time)
        return;

    // Fetch the client.
    GameClient* client = serverEntity->client;
    PlayerClient* player = (PlayerClient*)serverEntity->classEntity;
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
                (deathmatch->value && ((int)gamemodeflags->value & GameModeFlags::ForceRespawn))) {
                game.gameMode->RespawnClient((PlayerClient*)player->GetServerEntity());
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

// The actual current player entity that this C++ unit is acting on.
static PlayerClient *currentProcessingPlayer;

// The current client belonging to the player (class-)entity
static GameClient   *currentProcessingClient;

// Direction and speed vectors.
static vec3_t  forward, right, up;
static float   XYSpeed;

// Bobbing for movement.
static float   bobMove;
static int     bobCycle;       // odd cycles are right foot going forward
static float   bobFracsin;     // sin(bobfrac*M_PI)

//
//===============
// SVG_CalcRoll
// 
//
//===============
//
static float SVG_CalcRoll(const vec3_t &angles, const vec3_t &velocity)
{
    float   sign;
    float   side;
    float   value;

    side = vec3_dot(velocity, right);
    sign = side < 0 ? -1 : 1;
    side = fabs(side);

    value = sv_rollangle->value;

    if (side < sv_rollspeed->value)
        side = side * value / sv_rollspeed->value;
    else
        side = value;

    return side * sign;

}

//
//===============
// SVG_Player_ApplyDamageFeedback
// 
// Handles color blends and view kicks
//===============
//
static void SVG_Player_ApplyDamageFeedback(PlayerClient *ent)
{
    float   side;
    float   realcount, count, kick;
    vec3_t  v;
    int     r, l;
    static  vec3_t  power_color = {0.0f, 1.0f, 0.0f};
    static  vec3_t  acolor = {1.0f, 1.0f, 1.0f};
    static  vec3_t  bcolor = {1.0f, 0.0f, 0.0f};

    //client = player->client;

    // flash the backgrounds behind the status numbers
    currentProcessingClient->playerState.stats[STAT_FLASHES] = 0;
    if (currentProcessingClient->damages.blood)
        currentProcessingClient->playerState.stats[STAT_FLASHES] |= 1;
    if (currentProcessingClient->damages.armor && !(ent->GetFlags() & EntityFlags::GodMode))
        currentProcessingClient->playerState.stats[STAT_FLASHES] |= 2;

    // total points of damage shot at the player this frame
    count = (currentProcessingClient->damages.blood + currentProcessingClient->damages.armor + currentProcessingClient->damages.powerArmor);
    if (count == 0)
        return;     // didn't take any damage

                    // start a pain animation if still in the player model
    if (currentProcessingClient->animation.priorityAnimation < PlayerAnimation::Pain && ent->GetModelIndex() == 255) {
        static int      i;

        currentProcessingClient->animation.priorityAnimation = PlayerAnimation::Pain;
        if (currentProcessingClient->playerState.pmove.flags & PMF_DUCKED) {
            ent->SetFrame(FRAME_crpain1 - 1);
            currentProcessingClient->animation.endFrame = FRAME_crpain4;
        } else {
            i = (i + 1) % 3;
            switch (i) {
            case 0:
                ent->SetFrame(FRAME_pain101 - 1);
                currentProcessingClient->animation.endFrame = FRAME_pain104;
                break;
            case 1:
                ent->SetFrame(FRAME_pain201 - 1);
                currentProcessingClient->animation.endFrame = FRAME_pain204;
                break;
            case 2:
                ent->SetFrame(FRAME_pain301 - 1);
                currentProcessingClient->animation.endFrame = FRAME_pain304;
                break;
            }
        }
    }

    realcount = count;
    if (count < 10)
        count = 10; // always make a visible effect

                    // Play an apropriate pain sound
    if ((level.time > ent->GetDebouncePainTime()) && !(ent->GetFlags() & EntityFlags::GodMode)) {
        r = 1 + (rand() & 1);
        ent->SetDebouncePainTime(level.time + 0.7f);
        if (ent->GetHealth() < 25)
            l = 25;
        else if (ent->GetHealth() < 50)
            l = 50;
        else if (ent->GetHealth() < 75)
            l = 75;
        else
            l = 100;
        SVG_Sound(ent, CHAN_VOICE, gi.SoundIndex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
    }

    // The total alpha of the blend is always proportional to count.
    if (currentProcessingClient->damageAlpha < 0.f)
        currentProcessingClient->damageAlpha = 0.f;
    currentProcessingClient->damageAlpha += count * 0.01f;
    if (currentProcessingClient->damageAlpha < 0.2f)
        currentProcessingClient->damageAlpha = 0.2f;
    if (currentProcessingClient->damageAlpha > 0.6f)
        currentProcessingClient->damageAlpha = 0.6f;     // don't go too saturated

                                                         // The color of the blend will vary based on how much was absorbed
                                                         // by different armors.
    vec3_t blendColor = vec3_zero();
    if (currentProcessingClient->damages.powerArmor)
        blendColor = vec3_fmaf(blendColor, (float)currentProcessingClient->damages.powerArmor / realcount, power_color);
    if (currentProcessingClient->damages.armor)
        blendColor = vec3_fmaf(blendColor, (float)currentProcessingClient->damages.armor / realcount, acolor);
    if (currentProcessingClient->damages.blood)
        blendColor = vec3_fmaf(blendColor, (float)currentProcessingClient->damages.blood / realcount, bcolor);
    currentProcessingClient->damageBlend = blendColor;


    //
    // Calculate view angle kicks
    //
    kick = abs(currentProcessingClient->damages.knockBack);
    if (kick && ent->GetHealth() > 0) { // kick of 0 means no view adjust at all
        kick = kick * 100 / ent->GetHealth();

        if (kick < count * 0.5f)
            kick = count * 0.5f;
        if (kick > 50)
            kick = 50;

        vec3_t kickVec = currentProcessingClient->damages.from - ent->GetOrigin();
        kickVec = vec3_normalize(kickVec);

        side = vec3_dot(kickVec, right);
        currentProcessingClient->viewDamage.roll = kick * side * 0.3f;

        side = -vec3_dot(kickVec, forward);
        currentProcessingClient->viewDamage.pitch = kick * side * 0.3f;

        currentProcessingClient->viewDamage.time = level.time + DAMAGE_TIME;
    }

    //
    // clear totals
    //
    currentProcessingClient->damages.blood = 0;
    currentProcessingClient->damages.armor = 0;
    currentProcessingClient->damages.powerArmor = 0;
    currentProcessingClient->damages.knockBack = 0;
}

//
//===============
// SVG_CalculateViewOffset
// 
// Calculates t
//
// fall from 128 : 400 = 160000
// fall from 256 : 580 = 336400
// fall from 384 : 720 = 518400
// fall from 512 : 800 = 640000
// fall from 640 : 960 =
//
// damage = deltavelocity * deltavelocity * 0.0001
// 
//===============
//
static void SVG_CalculateViewOffset(PlayerClient *ent)
{
    float       bob;
    float       ratio;
    float       delta;

    //
    // Calculate new kick angle vales. (
    // 
    // If dead, set a fixed angle and don't add any kick
    if (ent->GetDeadFlag()) {
        currentProcessingClient->playerState.kickAngles = vec3_zero();

        currentProcessingClient->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
        currentProcessingClient->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
        currentProcessingClient->playerState.pmove.viewAngles[vec3_t::Yaw] = currentProcessingClient->killerYaw;
    } else {
        // Fetch client kick angles.
        vec3_t newKickAngles = currentProcessingClient->playerState.kickAngles = currentProcessingClient->kickAngles; //ent->client->playerState.kickAngles;

                                                                                                                      // Add pitch(X) and roll(Z) angles based on damage kick
        ratio = ((currentProcessingClient->viewDamage.time - level.time) / DAMAGE_TIME) * FRAMETIME;
        if (ratio < 0) {
            ratio = currentProcessingClient->viewDamage.pitch = currentProcessingClient->viewDamage.roll = 0;
        }
        newKickAngles[vec3_t::Pitch] += ratio * currentProcessingClient->viewDamage.pitch;
        newKickAngles[vec3_t::Roll] += ratio * currentProcessingClient->viewDamage.roll;

        // Add pitch based on fall kick
        ratio = ((currentProcessingClient->fallTime - level.time) / FALL_TIME) * FRAMETIME;;
        if (ratio < 0)
            ratio = 0;
        newKickAngles[vec3_t::Pitch] += ratio * currentProcessingClient->fallValue;

        // Add angles based on velocity
        delta = vec3_dot(ent->GetVelocity(), forward) * FRAMETIME;;
        newKickAngles[vec3_t::Pitch] += delta * run_pitch->value;

        delta = vec3_dot(ent->GetVelocity(), right) * FRAMETIME;;
        newKickAngles[vec3_t::Roll] += delta * run_roll->value;

        // Add angles based on bob
        delta = bobFracsin * bob_pitch->value * XYSpeed * FRAMETIME;;
        if (currentProcessingClient->playerState.pmove.flags & PMF_DUCKED)
            delta *= 6;     // crouching
        newKickAngles[vec3_t::Pitch] += delta;
        delta = bobFracsin * bob_roll->value * XYSpeed;
        if (currentProcessingClient->playerState.pmove.flags & PMF_DUCKED)
            delta *= 6;     // crouching
        if (bobCycle & 1)
            delta = -delta;
        newKickAngles[vec3_t::Roll] += delta;

        // Last but not least, assign new kickangles to player state.
        currentProcessingClient->playerState.kickAngles = newKickAngles;
    }

    //
    // Calculate new view offset.
    //
    // Start off with the base entity viewheight. (Set by Player Move code.)
    vec3_t newViewOffset = {
        0.f,
        0.f,
        (float)ent->GetViewHeight()
    };

    // Add fall impact view punch height.
    ratio = (currentProcessingClient->fallTime - level.time) / FALL_TIME;
    if (ratio < 0)
        ratio = 0;
    newViewOffset.z -= ratio * currentProcessingClient->fallValue * 0.4f;

    // Add bob height.
    bob = bobFracsin * XYSpeed * bob_up->value * FRAMETIME;;
    if (bob > 6)
        bob = 6;
    newViewOffset.z += bob;

    // Add kick offset
    newViewOffset += currentProcessingClient->kickOrigin;

    // Clamp the new view offsets, and finally assign them to the player state.
    // Clamping ensures that they never exceed the non visible, but physically 
    // there, player bounding box.
    currentProcessingClient->playerState.pmove.viewOffset = vec3_clamp(newViewOffset,
                                                                       //{ -14, -14, -22 },
                                                                       //{ 14,  14, 30 }
                                                                       ent->GetMins(),
                                                                       ent->GetMaxs()
    );
}

//
//===============
// SVG_CalculateGunOffset
// 
//===============
//
static void SVG_CalculateGunOffset(PlayerClient *ent)
{
    int     i;
    float   delta;

    // gun angles from bobbing
    currentProcessingClient->playerState.gunAngles[vec3_t::Roll] = XYSpeed * bobFracsin * 0.005;
    currentProcessingClient->playerState.gunAngles[vec3_t::Yaw]  = XYSpeed * bobFracsin * 0.01;
    if (bobCycle & 1) {
        currentProcessingClient->playerState.gunAngles[vec3_t::Roll] = -currentProcessingClient->playerState.gunAngles[vec3_t::Roll];
        currentProcessingClient->playerState.gunAngles[vec3_t::Yaw]  = -currentProcessingClient->playerState.gunAngles[vec3_t::Yaw];
    }

    currentProcessingClient->playerState.gunAngles[vec3_t::Pitch] = XYSpeed * bobFracsin * 0.005;

    // gun angles from delta movement
    for (i = 0 ; i < 3 ; i++) {
        delta = currentProcessingClient->oldViewAngles[i] - currentProcessingClient->playerState.pmove.viewAngles[i];
        if (delta > 180)
            delta -= 360;
        if (delta < -180)
            delta += 360;
        if (delta > 45)
            delta = 45;
        if (delta < -45)
            delta = -45;
        if (i == vec3_t::Yaw)
            currentProcessingClient->playerState.gunAngles[vec3_t::Roll] += 0.1 * delta;
        currentProcessingClient->playerState.gunAngles[i] += 0.2 * delta;
    }

    // gun height
    currentProcessingClient->playerState.gunOffset = vec3_zero();
    //  ent->playerState->gunorigin[2] += bob;

    // gun_x / gun_y / gun_z are development tools
    for (i = 0 ; i < 3 ; i++) {
        currentProcessingClient->playerState.gunOffset[i] += forward[i] * (gun_y->value);
        currentProcessingClient->playerState.gunOffset[i] += right[i] * gun_x->value;
        currentProcessingClient->playerState.gunOffset[i] += up[i] * (-gun_z->value);
    }
}

//
//===============
// SV_AddBlend
// 
//===============
//
static void SV_AddBlend(float r, float g, float b, float a, float *v_blend)
{
    float   a2, a3;

    if (a <= 0)
        return;
    a2 = v_blend[3] + (1 - v_blend[3]) * a; // new total alpha
    a3 = v_blend[3] / a2;   // fraction of color from old

    v_blend[0] = v_blend[0] * a3 + r * (1 - a3);
    v_blend[1] = v_blend[1] * a3 + g * (1 - a3);
    v_blend[2] = v_blend[2] * a3 + b * (1 - a3);
    v_blend[3] = a2;
}

//
//===============
// SVG_CalculateBlend
// 
//===============
//
static void SVG_CalculateBlend(PlayerClient *ent)
{
    // Clear blend values.
    currentProcessingClient->playerState.blend[0] = currentProcessingClient->playerState.blend[1] =
        currentProcessingClient->playerState.blend[2] = currentProcessingClient->playerState.blend[3] = 0;

    // Calculate view origin to use for PointContents.
    vec3_t viewOrigin = ent->GetOrigin() + currentProcessingClient->playerState.pmove.viewOffset;
    int32_t contents = gi.PointContents(viewOrigin);

    if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
        currentProcessingClient->playerState.rdflags |= RDF_UNDERWATER;
    else
        currentProcessingClient->playerState.rdflags &= ~RDF_UNDERWATER;

    if (contents & (CONTENTS_SOLID | CONTENTS_LAVA))
        SV_AddBlend(1.0, 0.3, 0.0, 0.6, currentProcessingClient->playerState.blend);
    else if (contents & CONTENTS_SLIME)
        SV_AddBlend(0.0, 0.1, 0.05, 0.6, currentProcessingClient->playerState.blend);
    else if (contents & CONTENTS_WATER)
        SV_AddBlend(0.5, 0.3, 0.2, 0.4, currentProcessingClient->playerState.blend);

    // add for damage
    if (currentProcessingClient->damageAlpha > 0)
        SV_AddBlend(currentProcessingClient->damageBlend[0], currentProcessingClient->damageBlend[1]
                    , currentProcessingClient->damageBlend[2], currentProcessingClient->damageAlpha, currentProcessingClient->playerState.blend);

    if (currentProcessingClient->bonusAlpha > 0)
        SV_AddBlend(0.85, 0.7, 0.3, currentProcessingClient->bonusAlpha, currentProcessingClient->playerState.blend);

    // drop the damage value
    currentProcessingClient->damageAlpha -= 0.06;
    if (currentProcessingClient->damageAlpha < 0)
        currentProcessingClient->damageAlpha = 0;

    // drop the bonus value
    currentProcessingClient->bonusAlpha -= 0.1;
    if (currentProcessingClient->bonusAlpha < 0)
        currentProcessingClient->bonusAlpha = 0;
}

//
//===============
// SVG_Player_CheckFallingDamage
// 
//===============
//
static void SVG_Player_CheckFallingDamage(PlayerClient *ent)
{
    float   delta;
    int     damage;
    vec3_t  dir;

    if (ent->GetModelIndex() != 255)
        return;     // not in the player model

    if (ent->GetMoveType() == MoveType::NoClip || ent->GetMoveType() == MoveType::Spectator)
        return;

    // Calculate delta velocity.
    vec3_t velocity = ent->GetVelocity();

    if ((currentProcessingClient->oldVelocity[2] < 0) && (velocity[2] > currentProcessingClient->oldVelocity[2]) && (!ent->GetGroundEntity())) {
        delta = currentProcessingClient->oldVelocity[2];
    } else {
        if (!ent->GetGroundEntity())
            return;
        delta = velocity[2] - currentProcessingClient->oldVelocity[2];
    }
    delta = delta * delta * 0.0001;

    // never take falling damage if completely underwater
    if (ent->GetWaterLevel() == 3)
        return;
    if (ent->GetWaterLevel() == 2)
        delta *= 0.25;
    if (ent->GetWaterLevel() == 1)
        delta *= 0.5;

    if (delta < 1)
        return;

    if (delta < 15) {
        ent->SetEventID(EntityEvent::Footstep);
        return;
    }

    currentProcessingClient->fallValue = delta * 0.5;
    if (currentProcessingClient->fallValue > 40)
        currentProcessingClient->fallValue = 40;
    currentProcessingClient->fallTime = level.time + FALL_TIME;

    if (delta > 30) {
        if (ent->GetHealth() > 0) {
            if (delta >= 55)
                ent->SetEventID(EntityEvent::FallFar);
            else
                ent->SetEventID(EntityEvent::Fall);
        }
        ent->SetDebouncePainTime(level.time);   // no normal pain sound
        damage = (delta - 30) / 2;
        if (damage < 1)
            damage = 1;
        dir = { 0.f, 0.f, 1.f };

        if (!deathmatch->value || !((int)gamemodeflags->value & GameModeFlags::NoFalling))
            SVG_InflictDamage(ent, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), dir, ent->GetOrigin(), vec3_zero(), damage, 0, 0, MeansOfDeath::Falling);
    } else {
        ent->SetEventID(EntityEvent::FallShort);
        return;
    }
}

//
//===============
// SVG_Player_CheckWorldEffects
// 
//===============
//
static void SVG_Player_CheckWorldEffects(void)
{
    int         waterlevel, oldWaterLevel;

    if (!currentProcessingPlayer)
        return;

    if (currentProcessingPlayer->GetMoveType() == MoveType::NoClip || currentProcessingPlayer->GetMoveType() == MoveType::Spectator) {
        currentProcessingPlayer->SetAirFinishedTime(level.time + 12 * FRAMETIME); // don't need air
        return;
    }

    // Retreive waterlevel.
    waterlevel = currentProcessingPlayer->GetWaterLevel();
    oldWaterLevel = currentProcessingClient->oldWaterLevel;
    currentProcessingClient->oldWaterLevel = waterlevel;

    //
    // if just entered a water volume, play a sound
    //
    if (!oldWaterLevel && waterlevel) {
        SVG_PlayerNoise(currentProcessingPlayer, currentProcessingPlayer->GetOrigin(), PNOISE_SELF);
        if (currentProcessingPlayer->GetWaterType() & CONTENTS_LAVA)
            SVG_Sound(currentProcessingPlayer, CHAN_BODY, gi.SoundIndex("player/lava_in.wav"), 1, ATTN_NORM, 0);
        else if (currentProcessingPlayer->GetWaterType() & CONTENTS_SLIME)
            SVG_Sound(currentProcessingPlayer, CHAN_BODY, gi.SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
        else if (currentProcessingPlayer->GetWaterType() & CONTENTS_WATER)
            SVG_Sound(currentProcessingPlayer, CHAN_BODY, gi.SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
        currentProcessingPlayer->SetFlags(currentProcessingPlayer->GetFlags() | EntityFlags::InWater);

        // clear damage_debounce, so the pain sound will play immediately
        currentProcessingPlayer->SetDebounceDamageTime(level.time - 1 * FRAMETIME);
    }

    //
    // if just completely exited a water volume, play a sound
    //
    if (oldWaterLevel && ! waterlevel) {
        SVG_PlayerNoise(currentProcessingPlayer, currentProcessingPlayer->GetOrigin(), PNOISE_SELF);
        SVG_Sound(currentProcessingPlayer, CHAN_BODY, gi.SoundIndex("player/watr_out.wav"), 1, ATTN_NORM, 0);
        currentProcessingPlayer->SetFlags(currentProcessingPlayer->GetFlags() & ~EntityFlags::InWater);
    }

    //
    // check for head just going under water
    //
    if (oldWaterLevel != 3 && waterlevel == 3) {
        SVG_Sound(currentProcessingPlayer, CHAN_BODY, gi.SoundIndex("player/watr_un.wav"), 1, ATTN_NORM, 0);
    }

    //
    // check for head just coming out of water
    //
    if (oldWaterLevel == 3 && waterlevel != 3) {
        if (currentProcessingPlayer->GetAirFinishedTime() < level.time) {
            // gasp for air
            SVG_Sound(currentProcessingPlayer, CHAN_VOICE, gi.SoundIndex("player/gasp1.wav"), 1, ATTN_NORM, 0);
            SVG_PlayerNoise(currentProcessingPlayer, currentProcessingPlayer->GetOrigin(), PNOISE_SELF);
        } else  if (currentProcessingPlayer->GetAirFinishedTime() < level.time + 11 * FRAMETIME) {
            // just break surface
            SVG_Sound(currentProcessingPlayer, CHAN_VOICE, gi.SoundIndex("player/gasp2.wav"), 1, ATTN_NORM, 0);
        }
    }

    //
    // check for drowning
    //
    if (waterlevel == 3) {
        // if out of air, start drowning
        if (currentProcessingPlayer->GetAirFinishedTime() < level.time) {
            // drown!
            if (currentProcessingPlayer->GetNextDrownTime() < level.time
                && currentProcessingPlayer->GetHealth() > 0) {
                currentProcessingPlayer->SetNextDrownTime(level.time + 1);

                // take more damage the longer underwater
                currentProcessingPlayer->SetDamage(currentProcessingPlayer->GetDamage() + 2);
                if (currentProcessingPlayer->GetDamage() > 15)
                    currentProcessingPlayer->SetDamage(15);

                // play a gurp sound instead of a normal pain sound
                if (currentProcessingPlayer->GetHealth() <= currentProcessingPlayer->GetDamage())
                    SVG_Sound(currentProcessingPlayer, CHAN_VOICE, gi.SoundIndex("player/drown1.wav"), 1, ATTN_NORM, 0);
                else if (rand() & 1)
                    SVG_Sound(currentProcessingPlayer, CHAN_VOICE, gi.SoundIndex("*gurp1.wav"), 1, ATTN_NORM, 0);
                else
                    SVG_Sound(currentProcessingPlayer, CHAN_VOICE, gi.SoundIndex("*gurp2.wav"), 1, ATTN_NORM, 0);

                currentProcessingPlayer->SetDebouncePainTime(level.time);

                SVG_InflictDamage(currentProcessingPlayer, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), vec3_zero(), currentProcessingPlayer->GetOrigin(), vec3_zero(), currentProcessingPlayer->GetDamage(), 0, DamageFlags::NoArmorProtection, MeansOfDeath::Water);
            }
        }
    } else {
        currentProcessingPlayer->SetAirFinishedTime(level.time + 12 * FRAMETIME);
        currentProcessingPlayer->SetDamage(2);
    }

    //
    // check for sizzle damage
    //
    if (waterlevel && (currentProcessingPlayer->GetWaterType() & (CONTENTS_LAVA | CONTENTS_SLIME))) {
        if (currentProcessingPlayer->GetWaterType() & CONTENTS_LAVA) {
            if (currentProcessingPlayer->GetHealth() > 0
                && currentProcessingPlayer->GetDebouncePainTime() <= level.time) {
                if (rand() & 1)
                    SVG_Sound(currentProcessingPlayer, CHAN_VOICE, gi.SoundIndex("player/burn1.wav"), 1, ATTN_NORM, 0);
                else
                    SVG_Sound(currentProcessingPlayer, CHAN_VOICE, gi.SoundIndex("player/burn2.wav"), 1, ATTN_NORM, 0);
                currentProcessingPlayer->SetDebouncePainTime(level.time + 1 * FRAMETIME);
            }

            SVG_InflictDamage(currentProcessingPlayer, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), vec3_zero(), currentProcessingPlayer->GetOrigin(), vec3_zero(), 3 * waterlevel, 0, 0, MeansOfDeath::Lava);
        }

        if (currentProcessingPlayer->GetWaterType() & CONTENTS_SLIME) {
            SVG_InflictDamage(currentProcessingPlayer, SVG_GetWorldClassEntity(), SVG_GetWorldClassEntity(), vec3_zero(), currentProcessingPlayer->GetOrigin(), vec3_zero(), 1 * waterlevel, 0, 0, MeansOfDeath::Slime);
        }
    }
}

//
//===============
// SVG_SetClientEffects
// 
//===============
//
static void SVG_SetClientEffects(PlayerClient *ent)
{
    ent->SetEffects(0);
    ent->SetRenderEffects(0);

    if (ent->GetHealth() <= 0 || level.intermission.time)
        return;

    // show cheaters!!!
    if (ent->GetFlags() & EntityFlags::GodMode) {
        ent->SetRenderEffects(ent->GetRenderEffects() | (RenderEffects::RedShell | RenderEffects::GreenShell | RenderEffects::BlueShell));
    }
}

//
//===============
// SVG_SetClientEvent
// 
//===============
//
static void SVG_SetClientEvent(PlayerClient *ent)
{
    if (ent->GetEventID())
        return;

    if (ent->GetGroundEntity() && XYSpeed > 225) {
        if ((int)(currentProcessingClient->bobTime + bobMove) != bobCycle)
            ent->SetEventID(EntityEvent::Footstep);
    }
}

//
//===============
// SVG_SetClientSound
// 
//===============
//
static void SVG_SetClientSound(PlayerClient *ent)
{
    const char    *weap; // C++20: STRING: Added const to char*

    if (currentProcessingClient->persistent.activeWeapon)
        weap = currentProcessingClient->persistent.activeWeapon->className;
    else
        weap = "";

    if (ent->GetWaterLevel() && (ent->GetWaterType() & (CONTENTS_LAVA | CONTENTS_SLIME)))
        ent->SetSound(snd_fry);
    else if (strcmp(weap, "weapon_railgun") == 0)
        ent->SetSound(gi.SoundIndex("weapons/rg_hum.wav"));
    else if (strcmp(weap, "weapon_bfg") == 0)
        ent->SetSound(gi.SoundIndex("weapons/bfg_hum.wav"));
    else if (currentProcessingClient->weaponSound)
        ent->SetSound(currentProcessingClient->weaponSound);
    else
        ent->SetSound(0);
}

//
//===============
// SVG_SetClientAnimationFrame
// 
//===============
//
static void SVG_SetClientAnimationFrame(PlayerClient *ent)
{
    qboolean isDucking = false;
    qboolean isRunning = false;

    if (!ent)
        return;

    if (ent->GetModelIndex() != 255)
        return;     // not in the player model

                    //client = ent->client;

    if (currentProcessingClient->playerState.pmove.flags & PMF_DUCKED)
        isDucking = true;
    else
        isDucking = false;
    if (XYSpeed)
        isRunning = true;
    else
        isRunning = false;

    // check for stand/duck and stop/go transitions
    if (isDucking != currentProcessingClient->animation.isDucking && currentProcessingClient->animation.priorityAnimation < PlayerAnimation::Death)
        goto newanim;
    if (isRunning != currentProcessingClient->animation.isRunning && currentProcessingClient->animation.priorityAnimation == PlayerAnimation::Basic)
        goto newanim;
    if (!ent->GetGroundEntity() && currentProcessingClient->animation.priorityAnimation <= PlayerAnimation::Wave)
        goto newanim;

    if (currentProcessingClient->animation.priorityAnimation == PlayerAnimation::Reverse) {
        if (ent->GetFrame() > currentProcessingClient->animation.endFrame) {
            ent->SetFrame(ent->GetFrame() - 1);
            return;
        }
    } else if (ent->GetFrame() < currentProcessingClient->animation.endFrame) {
        // continue an animation
        ent->SetFrame(ent->GetFrame() + 1);
        return;
    }

    if (currentProcessingClient->animation.priorityAnimation == PlayerAnimation::Death)
        return;     // stay there
    if (currentProcessingClient->animation.priorityAnimation == PlayerAnimation::Jump) {
        if (!ent->GetGroundEntity())
            return;     // stay there
        currentProcessingClient->animation.priorityAnimation = PlayerAnimation::Wave;
        ent->SetFrame(FRAME_jump3);
        currentProcessingClient->animation.endFrame = FRAME_jump6;
        return;
    }

newanim:
    // return to either a running or standing frame
    currentProcessingClient->animation.priorityAnimation = PlayerAnimation::Basic;
    currentProcessingClient->animation.isDucking = isDucking;
    currentProcessingClient->animation.isRunning = isRunning;

    if (!ent->GetGroundEntity()) {
        currentProcessingClient->animation.priorityAnimation = PlayerAnimation::Jump;
        if (ent->GetFrame() != FRAME_jump2)
            ent->SetFrame(FRAME_jump1);
        currentProcessingClient->animation.endFrame = FRAME_jump2;
    } else if (isRunning) {
        // running
        if (isDucking) {
            ent->SetFrame(FRAME_crwalk1);
            currentProcessingClient->animation.endFrame = FRAME_crwalk6;
        } else {
            ent->SetFrame(FRAME_run1);
            currentProcessingClient->animation.endFrame = FRAME_run6;
        }
    } else {
        // standing
        if (isDucking) {
            ent->SetFrame(FRAME_crstnd01);
            currentProcessingClient->animation.endFrame = FRAME_crstnd19;
        } else {
            ent->SetFrame(FRAME_stand01);
            currentProcessingClient->animation.endFrame = FRAME_stand40;
        }
    }
}


//===============
// DefaultGameMode::ClientEndServerFrame
// 
// Called for each player at the end of the server frame and right 
// after spawning.
// 
// Used to set the latest view offsets
//===============
void DefaultGameMode::ClientEndServerFrame(Entity *serverEntity) {
    float   bobTime;

    if (!serverEntity || !serverEntity->client || !serverEntity->classEntity) {
        return;
    }

    // Setup the current player and entity being processed.
    currentProcessingPlayer = (PlayerClient*)serverEntity->classEntity;
    currentProcessingClient = serverEntity->client;

    //
    // If the origin or velocity have changed since ClientThink(),
    // update the pmove values.  This will happen when the client
    // is pushed by a bmodel or kicked by an explosion.
    //
    // If it wasn't updated here, the view position would lag a frame
    // behind the body position when pushed -- "sinking into plats"
    //
    currentProcessingClient->playerState.pmove.origin = serverEntity->classEntity->GetOrigin();
    currentProcessingClient->playerState.pmove.velocity = serverEntity->classEntity->GetVelocity();

    //
    // If the end of unit layout is displayed, don't give
    // the player any normal movement attributes
    //
    if (level.intermission.time) {
        // FIXME: add view drifting here?
        currentProcessingClient->playerState.blend[3] = 0;
        currentProcessingClient->playerState.fov = 90;
        SVG_HUD_SetClientStats(serverEntity);
        return;
    }

    vec3_vectors(currentProcessingClient->aimAngles, &forward, &right, &up);

    // Burn from lava, etc
    CheckClientWorldEffects((PlayerClient*)serverEntity->classEntity);

    //
    // Set model angles from view angles so other things in
    // the world can tell which direction you are looking
    //
    vec3_t newPlayerAngles = serverEntity->state.angles;

    if (currentProcessingClient->aimAngles[vec3_t::Pitch] > 180)
        newPlayerAngles[vec3_t::Pitch] = (-360 + currentProcessingClient->aimAngles[vec3_t::Pitch]) / 3;
    else
        newPlayerAngles[vec3_t::Pitch] = currentProcessingClient->aimAngles[vec3_t::Pitch] / 3;
    newPlayerAngles[vec3_t::Yaw] = currentProcessingClient->aimAngles[vec3_t::Yaw];
    newPlayerAngles[vec3_t::Roll] = 0;
    newPlayerAngles[vec3_t::Roll] = SVG_CalcRoll(newPlayerAngles, serverEntity->classEntity->GetVelocity()) * 4;

    // Last but not least, after having calculated the Pitch, Yaw, and Roll, set the new angles.
    serverEntity->classEntity->SetAngles(newPlayerAngles);

    //
    // Calculate the player its X Y axis' speed and calculate the cycle for
    // bobbing based on that.
    //
    vec3_t playerVelocity = serverEntity->classEntity->GetVelocity();
    // Without * FRAMETIME = XYSpeed = std::sqrtf(playerVelocity[0] * playerVelocity[0] + playerVelocity[1] * playerVelocity[1]);
    XYSpeed = std::sqrtf(playerVelocity[0] * playerVelocity[0] + playerVelocity[1] * playerVelocity[1]) * FRAMETIME;

    if (XYSpeed < 5. * FRAMETIME || !(currentProcessingClient->playerState.pmove.flags & PMF_ON_GROUND)) {
        // Special handling for when not on ground.
        bobMove = 0;

        // Start at beginning of cycle again (See the else if statement.)
        currentProcessingClient->bobTime = 0;
    } else if (serverEntity->classEntity->GetGroundEntity() || serverEntity->classEntity->GetWaterLevel() == 2) {
        // So bobbing only cycles when on ground.
        if (XYSpeed > 450 * FRAMETIME)
            bobMove = 0.25;
        else if (XYSpeed > 210 * FRAMETIME)
            bobMove = 0.125;
        else if (!serverEntity->classEntity->GetGroundEntity() && serverEntity->classEntity->GetWaterLevel() == 2 && XYSpeed > 100 * FRAMETIME)
            bobMove = 0.225;
        else if (XYSpeed > 100 * FRAMETIME)
            bobMove = 0.0825;
        else if (!serverEntity->classEntity->GetGroundEntity() && serverEntity->classEntity->GetWaterLevel() == 2)
            bobMove = 0.1625;
        else
            bobMove = 0.03125;
    }

    // Generate bob time.
    currentProcessingClient->bobTime += bobMove * FRAMETIME;
    bobTime = currentProcessingClient->bobTime;

    if (currentProcessingClient->playerState.pmove.flags & PMF_DUCKED)
        bobTime *= 2* FRAMETIME;   // N&C: Footstep tweak.

    bobCycle = (int)bobTime;
    bobFracsin = std::fabsf(std::sinf(bobTime * M_PI)) * FRAMETIME;

    // Detect hitting the floor, and apply damage appropriately.
    SVG_Player_CheckFallingDamage((PlayerClient*)serverEntity->classEntity);

    // Apply all other the damage taken this frame
    SVG_Player_ApplyDamageFeedback((PlayerClient*)serverEntity->classEntity);

    // Determine the new frame's view offsets
    SVG_CalculateViewOffset((PlayerClient*)serverEntity->classEntity);

    // Determine the gun offsets
    SVG_CalculateGunOffset((PlayerClient*)serverEntity->classEntity);

    // Determine the full screen color blend
    // must be after viewOffset, so eye contents can be
    // accurately determined
    // FIXME: with client prediction, the contents
    // should be determined by the client
    SVG_CalculateBlend((PlayerClient*)serverEntity->classEntity);

    // Set the stats to display for this client (one of the chase isSpectator stats or...)
    if (currentProcessingClient->respawn.isSpectator)
        SVG_HUD_SetSpectatorStats(serverEntity);
    else
        SVG_HUD_SetClientStats(serverEntity);

    SVG_HUD_CheckChaseStats(serverEntity);

    SVG_SetClientEvent((PlayerClient*)serverEntity->classEntity);

    SVG_SetClientEffects((PlayerClient*)serverEntity->classEntity);

    SVG_SetClientSound((PlayerClient*)serverEntity->classEntity);

    SVG_SetClientAnimationFrame((PlayerClient*)serverEntity->classEntity);

    // Store velocity and view angles.
    currentProcessingClient->oldVelocity = serverEntity->classEntity->GetVelocity();
    currentProcessingClient->oldViewAngles = currentProcessingClient->playerState.pmove.viewAngles;

    // Reset weapon kicks to zer0.
    currentProcessingClient->kickOrigin = vec3_zero();
    currentProcessingClient->kickAngles = vec3_zero();

    // if the scoreboard is up, update it
    /*if (currentProcessingClient->showScores && !(level.frameNumber & 31)) {
    SVG_HUD_GenerateDMScoreboardLayout(ent, ent->GetEnemy());
    gi.Unicast(ent->GetServerEntity(), false);
    }*/
}

//===============
// DefaultGameMode::ClientCanConnect
// 
// Checks for whether it is OK for a client to connect go here.
//===============
qboolean DefaultGameMode::ClientCanConnect(Entity* serverEntity, char* userInfo) {
    // Check to see if they are on the banned IP list
    char *value = Info_ValueForKey(userInfo, "ip");
    if (SVG_FilterPacket(value)) {
        Info_SetValueForKey(userInfo, "rejmsg", "Banned.");
        return false;
    }

    // Check for a password, and if there is one, does it match?
    value = Info_ValueForKey(userInfo, "password");
    if (*password->string && strcmp(password->string, "none") &&
        strcmp(password->string, value)) {
        Info_SetValueForKey(userInfo, "rejmsg", "Password required or incorrect.");
        return false;
    }

    // Return true, client is allowed to connect.
    return true;
}

//===============
// DefaultGameMode::ClientConnect
// 
// Client is connecting, what do? :)
//===============
void DefaultGameMode::ClientConnect(Entity* serverEntity) {
    if (!serverEntity) {
        gi.DPrintf("ClientConnect executed with invalid (nullptr) serverEntity");
        return;
    }

    // This is default behaviour for this function.
    if (game.maximumClients > 1)
        gi.DPrintf("%s connected\n", serverEntity->client->persistent.netname);
}

//===============
// DefaultGameMode::ClientBegin
// 
// Called when a client is ready to be placed in the game after connecting.
//===============
void DefaultGameMode::ClientBegin(Entity* serverEntity) {
    if (!serverEntity) {
        gi.DPrintf("ClientBegin executed with invalid (nullptr) serverEntity");
        return;
    }

    // Setup the client for the server entity.
    serverEntity->client = game.clients + (serverEntity - g_entities - 1);

    // If there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (serverEntity->inUse == true) {
        // The client has cleared the client side viewAngles upon
        // connecting to the server, which is different than the
        // state when the game is saved, so we need to compensate
        // with deltaangles
        for (int32_t i = 0; i < 3; i++)
            serverEntity->client->playerState.pmove.deltaAngles[i] = serverEntity->client->playerState.pmove.viewAngles[i];
    } else {
        InitializeClientRespawnData(serverEntity->client);
        PutClientInServer((PlayerClient*)serverEntity);
        //serverEntity->classEntity->Respawn();
    }

    if (level.intermission.time) {
        HUD_MoveClientToIntermission(serverEntity);
    } else {
        // send effect if in a multiplayer game
        if (game.maximumClients > 1) {
            gi.WriteByte(SVG_CMD_MUZZLEFLASH);
            //gi.WriteShort(ent - g_entities);
            gi.WriteShort(serverEntity->state.number);
            gi.WriteByte(MuzzleFlashType::Login);
            gi.Multicast(serverEntity->state.origin, MultiCast::PVS);

            gi.BPrintf(PRINT_HIGH, "%s entered the game\n", serverEntity->client->persistent.netname);
        }
    }

    // Call ClientEndServerFrame to update him through the beginning frame.
    ClientEndServerFrame(serverEntity);
}

//===============
// DefaultGameMode::ClientDisconnect.
// 
// Does logic checking for a client's start of a server frame. In case there
// is a "level.intermission.time" set, it'll flat out return.
// 
// This basically allows for the game to disable fetching user input that makes
// our movement tick. And/or shoot weaponry while in intermission time.
//===============
void DefaultGameMode::ClientDisconnect(PlayerClient* player) {
    // Fetch the client.
    GameClient* client = player->GetClient();

    // Print who disconnected.
    gi.BPrintf(PRINT_HIGH, "%s disconnected\n", client->persistent.netname);

    // Send effect
    if (player->IsInUse()) {
        gi.WriteByte(SVG_CMD_MUZZLEFLASH);
        //gi.WriteShort(ent - g_entities);
        gi.WriteShort(player->GetNumber());
        gi.WriteByte(MuzzleFlashType::Logout);
        gi.Multicast(player->GetOrigin(), MultiCast::PVS);
    }

    // Unset this entity, after all, it's about to disconnect so.
    // We don't want it having any model, collision, sound, event, effects...
    // and ensure it is not in use anymore, also change its classname.
    player->UnlinkEntity();
    player->SetModelIndex(0);
    player->SetSound(0);
    player->SetEventID(0);
    player->SetEffects(0);
    player->SetSolid(Solid::Not);
    player->SetInUse(false);
    player->SetClassName("disconnected");

    // Ensure a state is stored for that this client is not connected anymore.
    client->persistent.isConnected = false;

    // FIXME: don't break skins on corpses, etc
    //playernum = ent-g_entities-1;
    //gi.configstring (ConfigStrings::PlayerSkins+playernum, "");
}

//===============
// DefaultGameMode::ClientUserinfoChanged
// 
//===============
void DefaultGameMode::ClientUserinfoChanged(Entity* ent, char* userinfo) {
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
        ent->client->persistent.isSpectator = true;
    else
        ent->client->persistent.isSpectator = false;

    // set skin
    s = Info_ValueForKey(userinfo, "skin");

    playernum = ent - g_entities - 1;

    // combine name and skin into a configstring
    gi.configstring(ConfigStrings::PlayerSkins + playernum, va("%s\\%s", ent->client->persistent.netname, s));

    // fov
    if (deathmatch->value && ((int)gamemodeflags->value & GameModeFlags::FixedFOV)) {
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
// DefaultGameMode::ClientUpdateObituary
// 
//===============
void DefaultGameMode::ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) {
    std::string message = ""; // String stating what happened to whichever entity. "suicides", "was squished" etc.
    std::string messageAddition = ""; // String stating what is additioned to it, "'s shrapnell" etc. Funny stuff.

                                      // Goes to COOP GAME MODE.
                                      //if (coop->value && attacker->GetClient())
                                      //    meansOfDeath |= MeansOfDeath::FriendlyFire;

    qboolean friendlyFire = meansOfDeath & MeansOfDeath::FriendlyFire;
    int32_t finalMeansOfDeath = meansOfDeath & ~MeansOfDeath::FriendlyFire;

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
    if (message != "") {
        gi.BPrintf(PRINT_MEDIUM, "%s %s.\n", self->GetClient()->persistent.netname, message.c_str());
        // WID: We can uncomment these in case we end up making a SinglePlayerMode after all.
        //if (deathmatch->value)
        //    self->GetClient()->respawn.score--;
        self->SetEnemy(NULL);
        return;
    }

    // Set the attacker to self.
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
        if (message != "") {
            gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClient()->persistent.netname, messageAddition.c_str());
            // WID: We can uncomment these in case we end up making a SinglePlayerMode after all.
            //if (deathmatch->value) {
            //    if (friendlyFire)
            //        attacker->GetClient()->respawn.score--;
            //    else
            //        attacker->GetClient()->respawn.score++;
            //}
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
        //    gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClassName(), messageAddition.c_str());
        //    if (deathmatch->value) {
        //        if (friendlyFire)
        //            attacker->GetClient()->respawn.score--;
        //        else
        //            attacker->GetClient()->respawn.score++;
        //    }
        //    return;
        //}
    }

    // 
    gi.BPrintf(PRINT_MEDIUM, "%s died.\n", self->GetClient()->persistent.netname);

    // WID: We can uncomment these in case we end up making a SinglePlayerMode after all.
    //if (deathmatch->value)
    //    self->GetClient()->respawn.score--;
}


void DefaultGameMode::InitializeClientPersistentData(Entity* ent) {
    gitem_t     *item = NULL;

    if (!ent || !ent->client)
        return;

    GameClient* client = ent->client;

    //memset(&client->persistent, 0, sizeof(client->persistent));
    client->persistent = {};

    item = SVG_FindItemByPickupName("Blaster");
    client->persistent.selectedItem = ITEM_INDEX(item);
    client->persistent.inventory[client->persistent.selectedItem] = 1;

    client->persistent.activeWeapon = item;

    client->persistent.health       = 100;
    client->persistent.maxHealth    = 100;

    client->persistent.maxBullets   = 200;
    client->persistent.maxShells    = 100;
    client->persistent.maxRockets   = 50;
    client->persistent.maxGrenades  = 50;
    client->persistent.maxCells     = 200;
    client->persistent.maxSlugs     = 50;

    client->persistent.isConnected = true;
}

void DefaultGameMode::InitializeClientRespawnData(GameClient* client) {
    if (!client)
        return;

    client->respawn = {};
    client->respawn.enterGameFrameNumber = level.frameNumber;
    client->respawn.persistentCoopRespawn = client->persistent;
}

//===============
// DefaultGameMode::PutClientInServer
// 
// Can be used to legit respawn a client at a spawn point.
// For SinglePlayer you want to take it a bit easy with this function.
// For Multiplayer games however, you definitely want to use this function.
//
// SP games: Use it once... (or at load time)
// MP games: Use it every respawn.
//===============
void DefaultGameMode::PutClientInServer(PlayerClient *ent) {
    // Find a spawn point for this client to be "placed"/"put" at.

}

//===============
// DefaultGameMode::RespawnClient
// 
// Respawns a client (if that is what the game mode wants).
//===============
void DefaultGameMode::RespawnClient(PlayerClient* ent) {
    // Kept around here to port later to other gamemodes.
    //if (deathmatch->value || coop->value) {
    //    // Spectator's don't leave bodies
    //    if (self->classEntity->GetMoveType() != MoveType::NoClip && self->classEntity->GetMoveType() != MoveType::Spectator)
    //        game.gameMode->SpawnClientCorpse(self->classEntity);

    //    self->serverFlags &= ~EntityServerFlags::NoClient;
    //    game.gameMode->PutClientInServer((PlayerClient*)self->classEntity);

    //    // add a teleportation effect
    //    self->state.eventID = EntityEvent::PlayerTeleport;

    //    // hold in place briefly
    //    self->client->playerState.pmove.flags = PMF_TIME_TELEPORT;
    //    self->client->playerState.pmove.time = 14;

    //    self->client->respawnTime = level.time;

    //    return;
    //}

    //// restart the entire server
    //gi.AddCommandString("pushmenu loadgame\n");

    // Restart the entire server by letting them pick a loadgame.
    // This is for singleplayer mode.
    gi.AddCommandString("pushmenu loadgame\n");
}

void DefaultGameMode::CheckClientWorldEffects(PlayerClient* ent) {

}
