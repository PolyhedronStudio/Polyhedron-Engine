/*
// LICENSE HERE.

//
// DefaultGamemode.cpp
//
//
*/
#include "../ServerGameLocal.h"          // SVGame.
#include "../Effects.h"     // Effects.
#include "../Entities.h"    // Entities.
#include "../Utilities.h"       // Util funcs.

// Shared Game.
#include "SharedGame/SharedGame.h" // Include SG Base.
#include "SharedGame/PMove.h"   // Include SG PMove.

// Server Game Base Entity.
#include "../Entities/Base/BodyCorpse.h"
#include "../Entities/Base/SVGBasePlayer.h"
#include "../Entities/Info/InfoPlayerStart.h"

// Weapons.h
#include "../Player/Client.h"
#include "../Player/Hud.h"
#include "../Player/Weapons.h"
#include "../Player/View.h"
#include "../Player/Animations.h"

// Game Mode.
#include "DefaultGamemode.h"

// World.
#include "../World/Gameworld.h"

//
// Constructor/Deconstructor.
//
DefaultGamemode::DefaultGamemode() {

}
DefaultGamemode::~DefaultGamemode() {

}

//
// Interface functions. 
//

qboolean DefaultGamemode::CanSaveGame(qboolean isDedicatedServer) {
    // For default game mode we'll just assume that dedicated servers are not allowed to save a game.
    if (isDedicatedServer) {
        return false;
    } else {
        return true;
    }
}

//===============
// DefaultGamemode::OnLevelExit
// 
// Default implementation for exiting levels.
//===============
void DefaultGamemode::OnLevelExit() {
    // Acquire server entities pointer.
    Entity* serverEntities = game.world->GetServerEntities();
    
    // Acquire class entities pointer.
    SVGBaseEntity** classEntities = game.world->GetClassEntities();

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
    for (int32_t i = 0; i < maximumclients->value; i++) {
        // Fetch the Worldspawn entity number.
        Entity *serverEntity = &serverEntities[i];

        if (!serverEntity)
            continue;

        if (!serverEntity->inUse)
            continue;

        uint32_t stateNumber = serverEntity->state.number;

        // Fetch the corresponding base entity.
        SVGBaseEntity* entity = classEntities[stateNumber];

        // Ensure an entity its health is reset to default.
        if (entity->GetHealth() > entity->GetClient()->persistent.stats.maxHealth)
            entity->SetHealth(entity->GetClient()->persistent.stats.maxHealth);
    }
} 

qboolean DefaultGamemode::IsDeadEntity(SVGBaseEntity *entity) {
    // Sanity check.
    if (!entity) {
        gi.DPrintf("Warning: IsDeadEntity called with a nullptr.\n");
        return true;
    }

    // In this case it's dead either when health is < 1, or if its DeadFlag has been set.
    if (entity->GetHealth() < 1 || entity->GetDeadFlag()) {
        return true;
    }

    // It's alive.
    return false;
}

//===============
// DefaultGamemode::GetEntityTeamName
//
// Assigns the teamname to the string passed, returns false in case the entity
// is not part of a team at all.
//===============
qboolean DefaultGamemode::GetEntityTeamName(SVGBaseEntity* ent, std::string &teamName) {
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
    //if (gamemodeflags->integer & GamemodeFlags::ModelTeams) {
    //    return clientSkin;
    //}

    //// Otherwise, in case we got skin teams... Return the skin specific part as team name.
    //if (gamemodeflags->integer & GamemodeFlags::SkinTeams) {
    //    return clientSkin.substr(slashPosition);
    //}

    //// We should never reach this point, but... there just in case.
    //return "";
}

//===============
// DefaultGamemode::OnSameTeam
//
// Returns false either way, because yes, there is no... team in this case.
// PS: ClientTeam <-- weird function, needs C++-fying and oh.. it stinks anyhow.
//===============
qboolean DefaultGamemode::OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) {
    //// There is only a reason to check for this in case these specific
    //// game mode flags are set.
    //if (!((int)(gamemodeflags->value) & (GamemodeFlags::ModelTeams | GamemodeFlags::SkinTeams)))
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
// DefaultGamemode::CanDamage
//
//===============
qboolean DefaultGamemode::CanDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor) {
    // Destination.
    vec3_t   destination = vec3_zero();
    // Trace.
    SVGTrace trace;

    // Solid entities need a special check, as their origin is usually 0,0,0
    // Exception to the above: the solid entity moves or has an origin brush
    if (target->GetMoveType() == MoveType::Push) {
        // Calculate destination.
        destination = target->GetAbsoluteMin() + target->GetAbsoluteMax();
        destination = vec3_scale(destination, 0.5f);
        
        // Execute trace.
        trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, CONTENTS_MASK_SOLID);

        //
        if (trace.fraction == 1.0) {
	        return true;
	    }
	    if (trace.ent == target) {
	        return true;
	    }

        return false;
    }

    // From here on we start tracing in various directions. Look at the code yourself to figure that one out...
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), target->GetOrigin(), inflictor, CONTENTS_MASK_SOLID);

    if (trace.fraction == 1.0) {
        return true;
    }

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
// DefaultGamemode::FindWithinRadius
//
// Returns a ClassEntityVector list containing the results of the found
// given entities that reside within the origin to radius. 
// 
// Flags can be set to determine which "solids" to exclude.
//===============
ClassEntityVector DefaultGamemode::FindBaseEnitiesWithinRadius(const vec3_t& origin, float radius, uint32_t excludeSolidFlags) {
    // List of base entities to return.
    ClassEntityVector radiusEntities;

    // Iterate over all entities, see who is nearby, and who is not.
    for (auto* radiusEntity : game.world->GetClassEntityRange<0, MAX_EDICTS>()
         | cef::Standard
         | cef::WithinRadius(origin, radius, excludeSolidFlags)) {

        // Push radiusEntity result item to the list.
        radiusEntities.push_back(radiusEntity);
    }

    // The list might be empty, ensure to check for that ;-)
    return radiusEntities;
}

//===============
// DefaultGamemode::EntityKilled
//
// Called when an entity is killed, or at least, about to be.
// Determine how to deal with it, usually resides in a callback to Die.
//===============
void DefaultGamemode::EntityKilled(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int32_t damage, vec3_t point) {
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
//            if (strcmp(attacker->classname, "monster_medic") == 0)
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
// DefaultGamemode::InflictDamage
// 
//===============
void DefaultGamemode::InflictDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, const vec3_t& dmgDir, const vec3_t& point, const vec3_t& normal, int32_t damage, int32_t knockBack, int32_t damageFlags, int32_t mod) {
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
    //if ((targ != attacker) && ((deathmatch->value && ((int)(gamemodeflags->value) & (GamemodeFlags::ModelTeams | GamemodeFlags::SkinTeams))) || coop->value)) {
    //    if (game.GetGamemode()->OnSameTeam(targ, attacker)) {
    //        if ((int)(gamemodeflags->value) & GamemodeFlags::NoFriendlyFire)
    //            damage = 0;
    //        else
    //            mod |= MeansOfDeath::FriendlyFire;
    //    }
    //}
    // We resort to defaults, but keep the above as mentioned.
    SetCurrentMeansOfDeath(mod);

    // Fetch client.
    ServerClient* client = target->GetClient();

    // Determine which temp entity event to use by default.
    int32_t tempEntityEvent = TempEntityEvent::Sparks;
    if (damageFlags & DamageFlags::Bullet)
        tempEntityEvent = TempEntityEvent::BulletSparks;

    // Retrieve normalized direction.
    vec3_t dir = vec3_normalize(dmgDir);

    // Ensure there is no odd knockback issues.
    if (target->GetFlags() & EntityFlags::NoKnockBack)
        knockBack = 0;

    // Figure out the momentum to add in case KnockBacks are off. 
    if (!(damageFlags & DamageFlags::NoKnockBack)) {
        if ((knockBack) && (target->GetMoveType() != MoveType::None) && (target->GetMoveType() != MoveType::Bounce) && (target->GetMoveType() != MoveType::Push) && (target->GetMoveType() != MoveType::Stop)) {
            vec3_t  kickbackVelocity = { 0.f, 0.f, 0.f };
            float   targetMass = 50; // Defaults to 50, otherwise... issues, this is the OG code style btw.

            // Based on mass, if it is below 50, we wanna hook it to being 50. Otherwise...
            if (target->GetMass() > 50)
                targetMass = target->GetMass();

            // Determine whether attacker == target, and the client itself, that means we gotta jump back hard.
            if (target->GetClient() && attacker == target)
                kickbackVelocity = vec3_scale(dir, 1600.0 * (float)knockBack / targetMass); // ROCKET JUMP HACK IS HERE BRUH <--
            else
                kickbackVelocity = vec3_scale(dir, 500 * (float)knockBack / targetMass);

            // Assign the new velocity, since yeah, it's bound to knock the fuck out of us.
            target->SetVelocity(target->GetVelocity() + kickbackVelocity);
        }
    }

    // Setup damages, so we can maths with them, yay. Misses code cuz we got no armors no more :P
    damageTaken = damage;       // Damage taken.
    damageSaved = 0;            // Damaged saved, from being taken.

    // Check for godmode.
    if ((target->GetFlags() & EntityFlags::GodMode) && !(damageFlags & DamageFlags::IgnoreProtection)) {
        damageTaken = 0;
        damageSaved = damage;

        // Leave it for the game mode to move on and spawn this temp entity (if allowed.)
        SpawnTempDamageEntity(tempEntityEvent, point, normal, damageSaved);
    }

    // Team damage avoidance
    if (!(damageFlags & DamageFlags::IgnoreProtection) && game.GetGamemode()->OnSameTeam(target, attacker))
        return;

    // Inflict the actual damage, in case we got to deciding to do so based on the above.
    if (damageTaken) {
        // Check if monster, or client, in which case, we spawn blood.
        // If not... :)... Do not.
        if (target->GetServerFlags() & EntityServerFlags::Monster || client) {
            // SpawnTempDamageEntity(TempEntityEvent::Blood, point, normal, take);
            // Leave it for the game mode to move on and spawn this temp entity (if allowed.)
	        SpawnTempDamageEntity(TempEntityEvent::Blood, point, dir, damageTaken);
        } else {
            // Leave it for the game mode to move on and spawn this temp entity (if allowed.)
            SpawnTempDamageEntity(tempEntityEvent, point, normal, damageTaken);
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
// DefaultGamemode::InflictDamage
// 
//===============
void DefaultGamemode::InflictRadiusDamage(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, float damage, SVGBaseEntity* ignore, float radius, int32_t mod) {
    // Damage point counter for radius sum ups.
    float   points = 0.f;

    // Actual entity loop pointer.
    SVGBaseEntity* ent = nullptr;

    // PH: From Yamagi Q2, to prevent issues.
    if (!inflictor || !attacker) {
        return;
    }

    // Find entities within radius.
    ClassEntityVector radiusEntities = FindBaseEnitiesWithinRadius(inflictor->GetOrigin(), radius, Solid::Not);

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
// DefaultGamemode::SetCurrentMeansOfDeath
// 
//===============
void DefaultGamemode::SetCurrentMeansOfDeath(int32_t meansOfDeath) {
    this->meansOfDeath = meansOfDeath;
}

//===============
// DefaultGamemode::GetCurrentMeansOfDeath
// 
//===============
const int32_t& DefaultGamemode::GetCurrentMeansOfDeath() {
    return this->meansOfDeath;
}

//===============
// DefaultGamemode::SpawnClientCorpse
// 
// Spawns a dead body entity for the given client.
//===============
void DefaultGamemode::SpawnClientCorpse(SVGBaseEntity* ent) {
    // Ensure it is an entity.
    if (!ent)
        return;

    // Ensure it is a client.
    if (!ent->GetClient())
        return;

    // Acquire pointer to server entities array.
    Entity *serverEntities = game.world->GetServerEntities();

    // Unlink the player client entity.
    ent->UnlinkEntity();

    // Grab a body from the queue, and cycle to the next one.
    Entity* bodyEntity = &serverEntities[game.GetMaxClients() + level.bodyQue + 1];
    level.bodyQue = (level.bodyQue + 1) % BODY_QUEUE_SIZE;

    // Send an effect on this body, in case it already has a model index.
    // This'll cause a body not to just "disappear", but actually play some
    // bloody particles over there.
    if (bodyEntity->state.modelIndex) {
        gi.MSG_WriteUint8(ServerGameCommand::TempEntity);//WriteByte(ServerGameCommand::TempEntity);
        gi.MSG_WriteUint8(TempEntityEvent::Blood);//WriteByte(TempEntityEvent::Blood);
        gi.MSG_WriteVector3(bodyEntity->state.origin, false);
        gi.MSG_WriteVector3(vec3_zero(), false);
        gi.Multicast(bodyEntity->state.origin, Multicast::PVS);
    }

    // Create the class entity for this queued bodyEntity.
    SVGBaseEntity *bodyClassEntity = game.world->CreateClassEntity<BodyCorpse>(bodyEntity, false);

    // Unlink the body entity, in case it was linked before.
    bodyClassEntity->UnlinkEntity();

    // Copy over the bodies state of the current entity into the body entity.
    bodyClassEntity->SetState(ent->GetState());
    // Change its number so it is accurately set to the one belonging to bodyEntity.
    // (Has to happen since we first copied over an entire entity state.)
    bodyClassEntity->SetNumber(bodyEntity - serverEntities);
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
    //bodyClassEntity->SetGroundEntity(ent->GetGroundEntity());

    // Set the die callback, and set its take damage.
    bodyClassEntity->SetDieCallback(&BodyCorpse::BodyCorpseDie);
    bodyClassEntity->SetTakeDamage(TakeDamage::Yes);

    // Link it in for collision etc.
    bodyClassEntity->LinkEntity();
}

//===============
// DefaultGamemode::SpawnTempDamageEntity
// 
// Sends a message to all clients in the current PVS, spawning a temp entity for
// displaying damage entities client side. (Sparks, what have ya.)
//===============
void DefaultGamemode::SpawnTempDamageEntity(int32_t type, const vec3_t& origin, const vec3_t& normal, int32_t damage) {
    // WID: Ensure the effect can't send more damage. But that is unimplemented for the clients atm to even detect...
    if (damage > 255)
        damage = 255;

    // Write away.
    gi.MSG_WriteUint8(ServerGameCommand::TempEntity);//WriteByte(ServerGameCommand::TempEntity);
    gi.MSG_WriteUint8(type);//WriteByte(type);
    //  gi.WriteByte (damage); // <-- This was legacy crap, might wanna implement it ourselves eventually.
    gi.MSG_WriteVector3(origin, false);
    gi.MSG_WriteVector3(normal, false);
    gi.Multicast(origin, Multicast::PVS);
}

//===============
// DefaultGamemode::CalculateDamageVelocity
// 
// Default implementation for calculating velocity damage.
//===============
vec3_t DefaultGamemode::CalculateDamageVelocity(int32_t damage) {
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
// DefaultGamemode::ClientBeginServerFrame
// 
// Does logic checking for a client's start of a server frame. In case there
// is a "level.intermission.time" set, it'll flat out return.
// 
// This basically allows for the game to disable fetching user input that makes
// our movement tick. And/or shoot weaponry while in intermission time.
//===============
void DefaultGamemode::ClientBeginServerFrame(SVGBasePlayer* player, ServerClient *client) {
    // Ensure valid pointers.
    if (!player || !client) {
        return;
    }

    // Ensure we aren't in intermission mode.
    if (level.intermission.time)
        return;

    // Run weapon animations in case this has not been done by user input itself.
    // (Idle animations, and general weapon thinking when a weapon is not in action.)
    if (!client->respawn.isSpectator) { //(!client->weaponState.shouldThink && !client->respawn.isSpectator)
        player->WeaponThink();
    } /*else {
        client->weaponState.shouldThink = false;
    }*/

    // Check if the player is actually dead or not. If he is, we're going to enact on
    // the user input that's been given to us. When fired, we'll respawn.
    if (player->GetDeadFlag()) {
        // Wait for any button just going down
        if (level.time > client->respawnTime) {
            // In old code, the need to hit a key was only set in DM mode.
            // I figured, let's keep it like this instead.
            //if (deathmatch->value)
            int32_t buttonMask = ButtonBits::Attack;
            //else
            //buttonMask = -1;

            if (client->latchedButtons & buttonMask) 
                // || (deathmatch->value && ((int)gamemodeflags->value & GamemodeFlags::ForceRespawn))) {
            {
                game.GetGamemode()->RespawnClient(player);
                client->latchedButtons = 0;
            }
        }
        return;
    }

    // add player trail so monsters can follow
    //if (!deathmatch->value)
    //    if (!visible(ent, SVG_PlayerTrail_LastSpot()))
    //        SVG_PlayerTrail_Add(ent->state.oldOrigin);

    // Reset the latched buttons.
    client->latchedButtons = 0;
}


//===============
// DefaultGamemode::ClientEndServerFrame
// 
// Called for each player at the end of the server frame and right 
// after spawning.
// 
// Used to set the latest view offsets
//===============
void DefaultGamemode::ClientEndServerFrame(SVGBasePlayer* player, ServerClient* client) {
    // Acquire server entity.
    Entity* serverEntity = player->GetServerEntity();
    // Fetch the bobMove state.
    SVGBasePlayer::BobMoveCycle& bobMoveCycle = player->GetBobMoveCycle();

    //
    // If the origin or velocity have changed since ClientThink(),
    // update the pmove values.  This will happen when the client
    // is pushed by a bmodel or kicked by an explosion.
    //
    // If it wasn't updated here, the view position would lag a frame
    // behind the body position when pushed -- "sinking into plats"
    //
    client->playerState.pmove.origin = player->GetOrigin();
    client->playerState.pmove.velocity = player->GetVelocity();

    //
    // If the end of unit layout is displayed, don't give
    // the player any normal movement attributes
    //
    if (level.intermission.time) {
        // FIXME: add view drifting here?
        client->playerState.blend[3] = 0;
        client->playerState.fov = 90;
        SVG_HUD_SetClientStats(player, client);
        return;
    }

    vec3_vectors(client->aimAngles, &player->GetBobMoveCycle().forward, &player->GetBobMoveCycle().right, &player->GetBobMoveCycle().up);

    // Burn from lava, etc
    player->CheckWorldEffects();

    //
    // Set model angles from view angles so other things in
    // the world can tell which direction you are looking
    //
    vec3_t newAngles = player->GetAngles();

    if (client->aimAngles[vec3_t::Pitch] > 180)
        newAngles[vec3_t::Pitch] = (-360 + client->aimAngles[vec3_t::Pitch]) / 3;
    else
        newAngles[vec3_t::Pitch] = client->aimAngles[vec3_t::Pitch] / 3;
    newAngles[vec3_t::Yaw] = client->aimAngles[vec3_t::Yaw];
    newAngles[vec3_t::Roll] = 0;
    newAngles[vec3_t::Roll] = player->CalculateRoll(newAngles, player->GetVelocity()) * 4;

    // Last but not least, after having calculated the Pitch, Yaw, and Roll, set the new angles.
    player->SetAngles(newAngles);

    //
    // Calculate the player its X Y axis' speed and calculate the cycle for
    // bobbing based on that.
    //
    vec3_t playerVelocity = player->GetVelocity();
    // Without * FRAMETIME = XYSpeed = std::sqrtf(playerVelocity[0] * playerVelocity[0] + playerVelocity[1] * playerVelocity[1]);
    bobMoveCycle.XYSpeed = std::sqrtf(playerVelocity[0] * playerVelocity[0] + playerVelocity[1] * playerVelocity[1]);

    if (bobMoveCycle.XYSpeed < 5 || !(client->playerState.pmove.flags & PMF_ON_GROUND)) {
        // Special handling for when not on ground.
        bobMoveCycle.move = 0;

        // Start at beginning of cycle again (See the else if statement.)
        client->bobTime = 0;
    } else if (player->GetGroundEntity() || player->GetWaterLevel() == 2) {
        // So bobbing only cycles when on ground.
        if (bobMoveCycle.XYSpeed > 450)
            bobMoveCycle.move = 0.25;
        else if (bobMoveCycle.XYSpeed > 210)
            bobMoveCycle.move = 0.125;
        else if (!player->GetGroundEntity() && player->GetWaterLevel() == 2 && bobMoveCycle.XYSpeed > 100)
            bobMoveCycle.move = 0.225;
        else if (bobMoveCycle.XYSpeed > 100)
            bobMoveCycle.move = 0.0825;
        else if (!player->GetGroundEntity() && player->GetWaterLevel() == 2)
            bobMoveCycle.move = 0.1625;
        else
            bobMoveCycle.move = 0.03125;
    }

    // Generate bob time.
    bobMoveCycle.move /= 3.5;
    float bobTime = (client->bobTime += bobMoveCycle.move);

    if (client->playerState.pmove.flags & PMF_DUCKED)
        bobTime *= 1.5;

    bobMoveCycle.cycle = (int)bobTime;
    bobMoveCycle.fracSin = fabs(sin(bobTime * M_PI));

    // Detect hitting the floor, and apply damage appropriately.
    player->CheckFallingDamage();

    // Apply all other the damage taken this frame
    player->ApplyDamageFeedback();

    // Determine the new frame's view offsets
    player->CalculateViewOffset();

    // Determine the gun offsets
    player->CalculateGunOffset();

    // Determine the full screen color blend
    // must be after viewOffset, so eye contents can be
    // accurately determined
    // FIXME: with client prediction, the contents
    // should be determined by the client
    player->CalculateScreenBlend();

    // Set the stats to display for this client (one of the chase isSpectator stats or...)
    if (client->respawn.isSpectator)
        SVG_HUD_SetSpectatorStats(player, client);
    else
	    SVG_HUD_SetClientStats(player, client);

    SVG_HUD_CheckChaseStats(serverEntity);

    player->UpdateEvent();

    player->UpdateEffects();

    player->UpdateSound();

    player->UpdateAnimationFrame();

    // Store velocity and view angles.
    client->oldVelocity = player->GetVelocity();
    client->oldViewAngles = client->playerState.pmove.viewAngles;

    // Reset weapon kicks to zer0.
    client->kickOrigin = vec3_zero();
    client->kickAngles = vec3_zero();

    // if the scoreboard is up, update it
    if (client->showScores && !(level.frameNumber & 31)) {
        SVG_HUD_GenerateDMScoreboardLayout(player, player->GetEnemy());
        gi.Unicast(serverEntity, false);
    }
}

// TODO: Obvious to see what to do here lol.
//
// Create a static PM_Trace in Gameworld perhaps?
static SVGBasePlayer* pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t q_gameabi PM_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end)
{
    if (pm_passent && pm_passent->GetHealth() > 0) {
        return gi.Trace(start, mins, maxs, end, pm_passent->GetServerEntity(), CONTENTS_MASK_PLAYERSOLID);
    } else {
        return gi.Trace(start, mins, maxs, end, pm_passent->GetServerEntity(), CONTENTS_MASK_DEADSOLID);
    }
}
void DefaultGamemode::ClientThink(SVGBasePlayer* player, ServerClient* client, ClientMoveCommand* moveCommand) {
    // Store the current entity to be run from SVG_RunFrame.
    level.currentEntity = player;

    // Set move type to freeze in case intermission has a waiting time set on it.
    if (level.intermission.time) {
        player->SetPlayerMoveType(EnginePlayerMoveType::Freeze);
        
        // Can exit intermission after five seconds
        if (level.time > level.intermission.time + 5.0 && (moveCommand->input.buttons & ButtonBits::Any)) {
            level.intermission.exitIntermission = true;
        }
    }

    // Special behavior for view angles in case of chasing a client.
    if (client->chaseTarget) {
        client->respawn.commandViewAngles = moveCommand->input.viewAngles;
    } else {
        // When our model index isn't 255 (player model), we are gibbed.
        if (player->GetModelIndex() != 255) {
            player->SetPlayerMoveType(EnginePlayerMoveType::Gib);
        // Ensure movetype changes to Dead if we got a DeadFlag set.
        } else if (player->GetDeadFlag()) {
            player->SetPlayerMoveType(EnginePlayerMoveType::Dead);
        } else {
            uint32_t moveType = player->GetMoveType();

            // Check entity move types, and set player move type based on that.
            switch (moveType) {
            case MoveType::NoClip:
                player->SetPlayerMoveType(PlayerMoveType::Noclip);
                break;
            case MoveType::Spectator:
                player->SetPlayerMoveType(PlayerMoveType::Spectator);
                break;
            default:
                player->SetPlayerMoveType(PlayerMoveType::Normal);
                break;
            }
        }

        // Store pass entity.
        pm_passent = player;

        // Update player move's gravity state.
        client->playerState.pmove.gravity = sv_gravity->value;

        // Copy over the pmove state from the latest player state.
        PlayerMove pm       = {};
        pm.moveCommand      = *moveCommand;
        pm.groundEntityPtr  = player->GetGroundEntity().Get();
        pm.state            = client->playerState.pmove;
        pm.state.origin     = player->GetOrigin();
        pm.state.velocity   = player->GetVelocity();
        
        // Set trace callbacks.
        pm.Trace            = PM_Trace;
        pm.PointContents    = gi.PointContents;

        // Simulate player movement for the current frame.
        PMove(&pm);

        // Store results back into the client's player state.
        client->playerState.pmove = pm.state;

        // Update entity properties based on results of the player move simulation.
        player->SetOrigin(pm.state.origin);
        player->SetVelocity(pm.state.velocity);
        player->SetMins(pm.mins);
        player->SetMaxs(pm.maxs);
        player->SetViewHeight(pm.state.viewOffset[2]);
        player->SetWaterLevel(pm.waterLevel);
        player->SetWaterType(pm.waterType);

        // Check for jumping sound.
        if (player->GetGroundEntity() && !pm.groundEntityPtr && (pm.moveCommand.input.upMove >= 10) && (pm.waterLevel == 0)) {
            SVG_Sound(player, CHAN_VOICE, gi.SoundIndex("*jump1.wav"), 1, ATTN_NORM, 0);
            SVG_PlayerNoise(player, player->GetOrigin(), PNOISE_SELF);
        }
        
        // Use an entity handle to validate and store the new ground entity after pmove.
        SVGEntityHandle groundEntityHandle = pm.groundEntityPtr;
        if (*groundEntityHandle && groundEntityHandle.Get()) {
            player->SetGroundEntity(*groundEntityHandle);
            player->SetGroundEntityLinkCount(groundEntityHandle->GetLinkCount());
        } else {
            player->SetGroundEntity(nullptr);
        }

        // Copy over the user command angles so they are stored for respawns.
        // (Used when going into a new map etc.)
        client->respawn.commandViewAngles[0] = moveCommand->input.viewAngles[0];
        client->respawn.commandViewAngles[1] = moveCommand->input.viewAngles[1];
        client->respawn.commandViewAngles[2] = moveCommand->input.viewAngles[2];

        // Special treatment for angles in case we are dead. Target the killer entity yaw angle.
        if (player->GetDeadFlag() != DEAD_NO) {
            client->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
            client->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
            client->playerState.pmove.viewAngles[vec3_t::Yaw] = client->killerYaw;
        } else {
            // Otherwise, store the resulting view angles accordingly.
            client->aimAngles = pm.viewAngles;
            client->playerState.pmove.viewAngles = pm.viewAngles;
        }

        // Link player entity back in for collision testing.
        player->LinkEntity();

        // Get player move type.
        int32_t playerMoveType = player->GetMoveType();

        // Execute touch callbacks as long as movetype isn't noclip, or spectator.
        if (playerMoveType != MoveType::NoClip && playerMoveType  != MoveType::Spectator) {
            // Trigger touch logic. 
            UTIL_TouchTriggers(player);

            // Solid touch logic.
            int32_t i = 0;
            int32_t j = 0;
            
            for (i = 0 ; i < pm.numTouchedEntities; i++) {
                for (j = 0 ; j < i ; j++) {
                    if (pm.touchedEntities[j] == pm.touchedEntities[i]) {
                        break;
                    }
                }
                if (j != i) {
                    continue;   // duplicated
                }

                SVGEntityHandle other(pm.touchedEntities[i]);
                if (!other || !*other) {
                    continue;
                }

                other->Touch(*other, player, NULL, NULL);
            }

        }
    }

    // Update client button bits.
    SetClientButtonBits(client, moveCommand);

    // save light level the player is standing on for
    // monster sighting AI
    //ent->lightLevel = moveCommand->input.lightLevel;

    // Fire weapon from final position if needed
    if (client->latchedButtons & ButtonBits::Attack) {
        if (client->respawn.isSpectator) {

            client->latchedButtons = 0;

            if (client->chaseTarget) {
                client->chaseTarget = NULL;
                client->playerState.pmove.flags &= ~PMF_NO_PREDICTION;
            } else {
//                SVG_GetChaseTarget(player);
            }

        } else {// if (!client->weaponState.shouldThink) {
            //client->weaponState.shouldThink = true;
            //player->WeaponThink();
        }
    } else {
	    if (!client->respawn.isSpectator) {
	        //player->WeaponThink();
	    }
    }

    // Act on the jump key(which sets upMove), used to change spectator targets.
    if (client->respawn.isSpectator) {
        if (moveCommand->input.upMove >= 10) {
            // When jump isn't held yet in the player move flags..
            if (!(client->playerState.pmove.flags & PMF_JUMP_HELD)) {
                // We add the jump held bit.
                client->playerState.pmove.flags |= PMF_JUMP_HELD;

                // So we can change chase target.
                if (client->chaseTarget) {
    //                SVG_ChaseNext(player);
                } else {
  //                  SVG_GetChaseTarget(player);
                }
            }
        } else {
            // Undo jump button after having let go of the jump key..
            client->playerState.pmove.flags &= ~PMF_JUMP_HELD;
        }
    }

    // update chase cam if being followed
    //for (int i = 1; i <= maximumclients->value; i++) {
    //    other = game.world->GetServerEntities() + i;
    //    if (other->inUse && other->client->chaseTarget == serverEntity)
    //        SVG_UpdateChaseCam(playerEntity);
    //}
}

//===============
// DefaultGamemode::ClientConnect
// 
// Client is connecting, what do? :)
//===============
qboolean DefaultGamemode::ClientConnect(Entity* svEntity, char *userinfo) {
    if (!svEntity) {
        gi.DPrintf("ClientConnect executed with invalid (nullptr) serverEntity");
        return false;
    }

    // Store keyValue.
    char *keyValue = nullptr;

    // check to see if they are on the banned IP list
    keyValue = Info_ValueForKey(userinfo, "ip");
    if (SVG_FilterPacket(keyValue)) {
        Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
        return false;
    }

    // check for a password
    keyValue = Info_ValueForKey(userinfo, "keyValue");
    if (*password->string && strcmp(password->string, "none") && strcmp(password->string, keyValue)) {
        Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
        return false;
    }

    // they can connect
    ServerClient* clients = game.GetClients();
    svEntity->client = &clients[svEntity->state.number - 1];//game.clients + (serverEntity - g_entities - 1);

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (svEntity->inUse == false) {
        // clear the respawning variables
        InitializePlayerRespawnData(svEntity->client);
        if (!game.autoSaved || !svEntity->client->persistent.inventory.activeWeaponID)
            InitializePlayerPersistentData(svEntity->client);
    }

    ClientUserinfoChanged(svEntity, userinfo);

    // This is default behaviour for this function.
    if (game.GetMaxClients() > 1) {
        gi.DPrintf("%s connected\n", svEntity->client->persistent.netname);
    }

    // Make sure we start with clean serverFlags.
    svEntity->serverFlags = 0;
    svEntity->client->persistent.isConnected = true;

    return true;
}

//===============
// DefaultGamemode::ClientBegin
// 
// Called when a client is ready to be placed in the game after connecting.
//===============
void DefaultGamemode::ClientBegin(Entity* svEntity) {
    if (!svEntity) {
        gi.DPrintf("ClientBegin executed with invalid (nullptr) serverEntity");
        return;
    }

    if (!svEntity->client) {
        gi.DPrintf("ClientBegin executed with invalid (nullptr) serverEntity->client");
        return;
    }

    // Fetch client.
    ServerClient* clients = game.GetClients();
    ServerClient* client = &clients[svEntity->state.number - 1];  //(serverEntity - g_entities - 1);

    // Assign  this client to the server entity.
    svEntity->client = client;

    // Player entity, to be assigned next.
    SVGBasePlayer* player = nullptr;

    // If there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (!!svEntity->inUse == true) {
        // Only pull through if it is a base player.
        if (svEntity->classEntity->IsSubclassOf<SVGBasePlayer>()) {
	        player = dynamic_cast<SVGBasePlayer*>(svEntity->classEntity);
        } else {
	        gi.Error("ClientBegin called with an inUse entity that is not of type or derived from SVGBasePlayer\n");
        }

        // The client has cleared the client side viewAngles upon
        // connecting to the server, which is different than the
        // state when the game is saved, so we need to compensate
        // with deltaangles
        for (int32_t i = 0; i < 3; i++) {
            svEntity->client->playerState.pmove.deltaAngles[i] = svEntity->client->playerState.pmove.viewAngles[i];
        }
    } else {
        // Assign  this client to the server entity.
        svEntity->client  = client;

        // Create the player client entity.
        player = SVGBasePlayer::Create(svEntity);

        // Initialize client respawn data.
        InitializePlayerRespawnData(client);
 
        // Put into our server and blast away! (Takes care of spawning classEntity).
        PlacePlayerInGame(player);
    }

    if (level.intermission.time) {
        HUD_MoveClientToIntermission(svEntity);
    } else {
        // send effect if in a multiplayer game
        if (game.GetMaxClients() > 1) {
	        gi.MSG_WriteUint8(ServerGameCommand::MuzzleFlash);//WriteByte(ServerGameCommand::MuzzleFlash);
	        //gi.WriteShort(serverEntity - g_entities);
	        gi.MSG_WriteInt16(player->GetNumber());//WriteShort(player->GetNumber());
	        gi.MSG_WriteUint8(MuzzleFlashType::Login);//WriteByte(MuzzleFlashType::Login);
	        gi.Multicast(player->GetOrigin(), Multicast::PVS);

            gi.BPrintf(PRINT_HIGH, "%s entered the game\n", client->persistent.netname);
        }
    }

    // Call ClientEndServerFrame to update him through the beginning frame.
    ClientEndServerFrame(player, client);
}

//===============
// DefaultGamemode::ClientDisconnect.
// 
// Does logic checking for a client's start of a server frame. In case there
// is a "level.intermission.time" set, it'll flat out return.
// 
// This basically allows for the game to disable fetching user input that makes
// our movement tick. And/or shoot weaponry while in intermission time.
//===============
void DefaultGamemode::ClientDisconnect(SVGBasePlayer* player, ServerClient *client) {
    // Print who disconnected.
    gi.BPrintf(PRINT_HIGH, "%s disconnected\n", client->persistent.netname);

    // Send effect
    if (player->IsInUse()) {
        gi.MSG_WriteUint8(ServerGameCommand::MuzzleFlash);
        gi.MSG_WriteInt16(player->GetNumber());
        gi.MSG_WriteUint8(MuzzleFlashType::Logout);
        gi.Multicast(player->GetOrigin(), Multicast::PVS);
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
    player->SetClassname("disconnected");

    // Ensure a state is stored for that this client is not connected anymore.
    client->persistent.isConnected = false;

    // FIXME: don't break skins on corpses, etc
    //playernum = ent-g_entities-1;
    //gi.configstring (ConfigStrings::PlayerSkins+playernum, "");
}

//===============
// DefaultGamemode::ClientUserinfoChanged
// 
//===============
void DefaultGamemode::ClientUserinfoChanged(Entity* ent, char* userinfo) {
    char    *s;
    int     playernum;

    // check for malformed or illegal info strings
    if (!Info_Validate(userinfo)) {
        strcpy(userinfo, "\\name\\badinfo\\skin\\male/grunt");
    }

    // set name
    s = Info_ValueForKey(userinfo, "name");
    strncpy(ent->client->persistent.netname, s, sizeof(ent->client->persistent.netname) - 1);

    // Set spectator to false.
    ent->client->persistent.isSpectator = false;

    // set skin
    s = Info_ValueForKey(userinfo, "skin");

    playernum = ent - game.world->GetServerEntities() - 1;

    // combine name and skin into a configstring
    gi.configstring(ConfigStrings::PlayerSkins + playernum, va("%s\\%s", ent->client->persistent.netname, s));

    // fov
    if (((int)gamemodeflags->value & GamemodeFlags::FixedFOV)) {
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
// DefaultGamemode::ClientUpdateObituary
// 
//===============
void DefaultGamemode::ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) {
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
        if (!message.empty()) {
            gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClassname(), messageAddition.c_str());
        }
    }

    // 
    gi.BPrintf(PRINT_MEDIUM, "%s died.\n", self->GetClient()->persistent.netname);
}


void DefaultGamemode::InitializePlayerPersistentData(ServerClient* client) {
    // Can't go on without a valid client.
    if (!client) {
        return;
    }

    /**
    *   Main Persistent Data.
    **/
    // Reset its persistent data, we're initializing it for this client.
    client->persistent = { 
        .isConnected = true 
    };

    /**
    *   Stats.
    **/
    client->persistent.stats.health       = 100;
    client->persistent.stats.maxHealth    = 100;

    /**
    *   Inventory.
    **/
    // Give the client a default starter weapon to spawn with.
    //client->persistent.selectedItem = 0;//ITEM_INDEX(item);
    //client->persistent.inventory[client->persistent.selectedItem] = 1;

    // Reset active weapon to nothing.
    client->persistent.inventory.activeWeaponID = 0;

    // Reset maximum carrying values of inventory.
    client->persistent.inventory.maxAmmo9mm   = 150;

}

void DefaultGamemode::InitializePlayerRespawnData(ServerClient* client) {
    if (!client) { 
        return;
    }

    client->respawn = {};
    client->respawn.enterGameFrameNumber = level.frameNumber;
    client->respawn.persistentCoopRespawn = client->persistent;
}

//===============
// DefaultGamemode::SelectClientSpawnPoint
//
// Choose any info_player_start or its derivates, it'll do a subclassof check, so the only valid classnames are
// those who have inherited from info_player_start. (info_player_deathmatch, etc).
//===============
void DefaultGamemode::SelectPlayerSpawnPoint(SVGBasePlayer* player, vec3_t& origin, vec3_t& angles) {
    // Spawn point entity pointer.
    SVGBaseEntity *spawnPoint = nullptr;

    // Find a spawn point that has a targetname matching game.spawnpoint.
    auto targetSpawnPoints = game.world->GetClassEntityRange<0, MAX_EDICTS>() | cef::Standard | cef::IsSubclassOf<InfoPlayerStart>() | cef::HasKeyValue("targetname", game.spawnpoint);

    // First try and find the one with the targetname.
    for (auto& entity : targetSpawnPoints) {
        // If the targetname matches, we've found our spawn point.
        if (entity->GetTargetName() == game.spawnpoint) {
            spawnPoint = entity;
            break;
        }
    }

    // If we haven't found one, we'll push the spanned results to a vector and randomly select one from it.
    if (!spawnPoint) {
        // TODO: Improve this, find a method to select random from a range. (Or wrap something similar up.)
	    std::vector<SVGBaseEntity*> spawnVector;
        auto spawnPoints = game.world->GetClassEntityRange<0, MAX_EDICTS>() | cef::Standard | cef::IsSubclassOf<InfoPlayerStart>();
        for (auto& entity : spawnPoints) { spawnVector.push_back(entity); }

        // Select random spawn point.
    	if (spawnVector.size() > 0) {
		    spawnPoint = spawnVector[RandomRangeui(0, spawnVector.size())];
	    }
    }

    // Assign origin and raise us up 9 on the z axis to ensure we are never stuck on something.
    if (spawnPoint) {
        origin = spawnPoint->GetOrigin();
        origin.z += 9;
        angles = spawnPoint->GetAngles();
    } else {
        // We might as well error out at this point.
        gi.Error("Couldn't find spawn point %s", game.spawnpoint);
    }
}

//===============
// DefaultGamemode::PlacePlayerInGame
// 
// Called when a player connects to a single and multiplayer. 
// 
// #1. In the case of a SP mode death, the loadmenu pops up and selecting a load game
// will restart the server.
// #2. In thecase of a MP mode death however, after a small intermission time, it'll
// call this function again to respawn our player.
//===============
void DefaultGamemode::PlacePlayerInGame(SVGBasePlayer *player) {
    // Acquire pointer to game clients.
    ServerClient* gameClients = game.GetClients();
    
    // Find a spawn point
    vec3_t  spawnOrigin = vec3_zero();
    vec3_t  spawnAngles = vec3_zero();

    SelectPlayerSpawnPoint(player, spawnOrigin, spawnAngles);
    player->SetOrigin(spawnOrigin);
    player->SetAngles(spawnAngles);

    // Acquire the new client index belonging to this entity.
    int32_t clientIndex = player->GetNumber() - 1; //ent - g_entities - 1;

    // Acquire pointer to the current client.
    ServerClient* client = player->GetClient();

    // Client user info.
    char userinfo[MAX_INFO_STRING];
    // Store a copy of our respawn data for later use. 
    ClientRespawnData respawnData = client->respawn;
    // Copy over client's user info into our userinfo buffer.
    memcpy(userinfo, client->persistent.userinfo, sizeof(userinfo));

    // If we had persistent coop respawn data, be sure to back it up right now.
    client->persistent = respawnData.persistentCoopRespawn;
    // Inform of a client user info change.
    ClientUserinfoChanged(player->GetServerEntity(), userinfo);
    // In case the score was higher, be sure to update it.
    if (respawnData.score > client->persistent.score) {
	    client->persistent.score = respawnData.score;
    }

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
    //SVGBasePlayer* player = dynamic_cast<SVGBasePlayer*>(ent->classEntity);
    player->Spawn();

    // Copy some data from the client to the entity
    RestorePlayerPersistentData(player, client);

    // Update the client pointer this entity belongs to.
    client = &gameClients[clientIndex];
    player->SetClient(client);
 
    // Clear playerstate values.
    client->playerState = {};

    // Setup player move origin to spawnpoint origin.
    client->playerState.pmove.origin = spawnOrigin;

    if (((int)gamemodeflags->value & GamemodeFlags::FixedFOV)) {
        client->playerState.fov = 90;
    } else {
        client->playerState.fov = atoi(Info_ValueForKey(client->persistent.userinfo, "fov"));
        if (client->playerState.fov < 1) {
            client->playerState.fov = 90;
        } else if (client->playerState.fov > 160) {
            client->playerState.fov = 160;
        }
    }

    // Set entity state origins and angles.
    player->SetOrigin(spawnOrigin + vec3_t { 0.f, 0.f, 1.f });
    player->SetOldOrigin(player->GetOrigin());
    player->SetAngles(vec3_t { 0.f, spawnAngles[vec3_t::Yaw], 0.f });

    // Set client and player move state angles.
    client->playerState.pmove.deltaAngles = spawnAngles - client->respawn.commandViewAngles;
    client->playerState.pmove.viewAngles = player->GetAngles();
    client->aimAngles = player->GetAngles();

    // spawn a spectator in case the client was/is one.
    if (client->persistent.isSpectator) {
        // Nodefault chase target.
        client->chaseTarget = nullptr;

        // Well we knew this but store it in respawn data too.
        client->respawn.isSpectator = true;

        // Movement type is the obvious spectator.
        player->SetMoveType(MoveType::Spectator);

        // No solid.
        player->SetSolid(Solid::BoundingBox);

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

    // Set player state gun index to whichever was persistent in the previous map (if there was one).
    client->playerState.gunIndex = gi.ModelIndex("models/weapons/v_mark23/tris.iqm");  //gi.ModelIndex(client->persistent.activeWeapon->viewModel);

    // Set its current new weapon to the one that was stored in persistent and activate it.
    //client->newWeapon = client->persistent.activeWeapon;
    player->ChangeWeapon(client->persistent.inventory.activeWeaponID, false);
}

//===============
// DefaultGamemode::RespawnClient
// 
// Since the default game mode is intended to be a single player mode,
// there is no respawning and we show a loadgame menu instead.
//===============
void DefaultGamemode::RespawnClient(SVGBasePlayer* ent) {
    // Kept around here to port later to other gamemodes.
    //if (deathmatch->value || coop->value) {
    //    // Spectator's don't leave bodies
    //    if (self->classEntity->GetMoveType() != MoveType::NoClip && self->classEntity->GetMoveType() != MoveType::Spectator)
    //        game.GetGamemode()->SpawnClientCorpse(self->classEntity);

    //    self->serverFlags &= ~EntityServerFlags::NoClient;
    //    game.GetGamemode()->PlacePlayerInGame((SVGBasePlayer*)self->classEntity);

    //    // add a teleportation effect
    //    self->state.eventID = EntityEvent::PlayerTeleport;

    //    // hold in place briefly
    //    self->client->playerState.pmove.flags = PMF_TIME_TELEPORT;
    //    self->client->playerState.pmove.time = 14;

    //    self->client->respawnTime = level.time;

    //    return;
    //}

    // Restart the entire server by letting them pick a loadgame.
    // This is for singleplayer mode.
    gi.AddCommandString("pushmenu loadgame\n");
}

//===============
// DefaultGamemode::RespawnAllClients
//
//===============
void DefaultGamemode::RespawnAllClients() {
    // Do nothing for default game mode.
}

//===============
// DefaultGamemode::ClientDeath
// 
// Does nothing for this game mode.
//===============
void DefaultGamemode::ClientDeath(SVGBasePlayer *player) {

}

//===============
// DefaultGamemode::StorePlayerPersistentData
// 
// Some information that should be persistant, like health,
// is still stored in the edict structure, so it needs to
// be mirrored out to the client structure before all the
// edicts are wiped.
//===============
void DefaultGamemode::StorePlayerPersistentData(void) {
    // Acquire server entity pointer.
    Entity* serverEntities = game.world->GetServerEntities();

    // Acquire a pointer to the game's clients.
    ServerClient* gameClients = game.GetClients();
    
    for (int32_t i = 0 ; i < game.GetMaxClients(); i++) {
        Entity *entity = &serverEntities[1 + i];
        if (!entity->inUse)
            continue;
        if (!entity->classEntity)
            continue;

        gameClients[i].persistent.stats.health = entity->classEntity->GetHealth();
        gameClients[i].persistent.stats.maxHealth = entity->classEntity->GetMaxHealth();
        gameClients[i].persistent.savedFlags = (entity->classEntity->GetFlags() & (EntityFlags::GodMode | EntityFlags::NoTarget | EntityFlags::PowerArmor));
    }
}

//===============
// DefaultGamemode::RespawnClient
// 
// // Fetch client data that was stored between previous entity wipe session
//===============
void DefaultGamemode::RestorePlayerPersistentData(SVGBaseEntity* player, ServerClient* client) {
    if (!player || !client)
        return;
        
    player->SetHealth(client->persistent.stats.health);
    player->SetMaxHealth(client->persistent.stats.maxHealth);
    player->SetFlags(player->GetFlags() | client->persistent.savedFlags);
}

/**
*   @brief  Sets a client's button, oldButton, and latched button bits.
**/
void DefaultGamemode::SetClientButtonBits(ServerClient *client, ClientMoveCommand* moveCommand) {
    if (!client || !moveCommand) {
        return;
    }

    // Store the buttons that were still set as current frame buttons in the oldButtons.
    client->oldButtons = client->buttons;

    // Update current buttons with those acquired from the move command.
    client->buttons = moveCommand->input.buttons;

    // Figure out the latched buttons by bit fun. (latched buttons is used for single button press logic.)
    client->latchedButtons |= client->buttons & ~client->oldButtons;
}

/**
*   @brief  Sets client into intermission mode by setting movetype to freeze
*           and positioning the client at the intermission point.
**/
void DefaultGamemode::StartClientIntermission(SVGBasePlayer* player, ServerClient *client) {
    // Only continue if valid.
    if (!player || !client) {
        return;
    }

    //if (deathmatch->value || coop->value)
    //    ent->client->showScores = true;

    // Copy over the previously fetched map intermission entity origin into
    // the client player states positions.
    player->SetOrigin(level.intermission.origin);
    
    // Update the client's player state.
    client->playerState.pmove.origin        = level.intermission.origin;
    client->playerState.pmove.viewAngles    = level.intermission.viewAngle;

    // Disable movement, remove weapon, blend factor, and underwater render flag if any.
    client->playerState.pmove.type = EnginePlayerMoveType::Freeze;
    client->playerState.gunIndex = 0;
    client->playerState.blend[3] = 0;
    client->playerState.rdflags &= ~RDF_UNDERWATER;

    // Reset specific entity state settings.
    player->SetViewHeight(0);
    player->SetModelIndex(0);
    player->SetModelIndex2(0);
    player->SetModelIndex3(0);
    player->SetModelIndex4(0);
    player->SetEffects(0);
    player->SetSound(0);
    player->SetSolid(0);

    // Add the layout in case of a deathmatch or co-op gamemode.
    //if (deathmatch->value || coop->value) {
    //    SVG_HUD_GenerateDMScoreboardLayout(ent, NULL);
    //    gi.Unicast(ent, true);
    //}
}