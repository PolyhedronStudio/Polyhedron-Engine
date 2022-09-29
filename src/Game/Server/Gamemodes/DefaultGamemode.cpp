/*
// LICENSE HERE.

//
// DefaultGameMode.cpp
//
//
*/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../Effects.h"     // Effects.
#include "../Entities.h"    // Entities.
#include "../Utilities.h"       // Util funcs.

// Server Game Base Entity.
#include "../Entities/Base/BodyCorpse.h"
#include "../Entities/Base/SVGBasePlayer.h"
#include "../Entities/Info/InfoPlayerStart.h"

// Weapons.h
#include "../Player/Client.h"
#include "../Player/Hud.h"
#include "../Player/Weapons.h"
#include "../Player/Animations.h"

// Game Mode.
#include "DefaultGamemode.h"

// World.
#include "../World/ServerGameWorld.h"

// Shared Game API
#include "Game/Shared/GameBindings/ServerBinding.h"



// Skip Entity for PM tracing.
SVGBasePlayer* DefaultGameMode::pmSkipEntity = nullptr;

//! Constructor/Destructor.
DefaultGameMode::DefaultGameMode() : IGamemode() {

}

//
// Interface functions. 
//

qboolean DefaultGameMode::CanSaveGame(qboolean isDedicatedServer) {
    // For default game mode we'll just assume that dedicated servers are not allowed to save a game.
    if (isDedicatedServer) {
        return false;
    } else {
        return true;
    }
}

//===============
// DefaultGameMode::OnLevelExit
// 
// Default implementation for exiting levels.
//===============
void DefaultGameMode::OnLevelExit() {
    // Acquire pod entities pointer.
    Entity* podEntities = game.world->GetPODEntities();
    
    // Acquire game entities pointer.
    GameEntityVector &gameEntities = game.world->GetGameEntities();

    // Create the command to use for switching to the next game map.
    std::string command = "gamemap \"";
    command += level.intermission.changeMap;
    command += +"\"";

    // Add the gamemap command to the 
    gi.AddCommandString(command.c_str());
    // Reset the changeMap string, intermission time, and regular level time.
    level.intermission.changeMap = NULL;
    level.intermission.exitIntermission = 0;
    level.intermission.time = GameTime::zero();

    // End the server frames for all clients.
    SVG_ClientEndServerFrames();

    // Loop through the server entities, and run the base entity frame if any exists.
    for (int32_t i = 0; i < maximumclients->value; i++) {
        // Fetch the Worldspawn entity number.
        PODEntity *podEntity = &podEntities[i];

        if (!podEntity || !podEntity->inUse) {
            continue;
		}

		// Get state number.
        const uint32_t stateNumber = podEntity->currentState.number;

        // Fetch the corresponding base entity.
        GameEntity* gameEntity = gameEntities[stateNumber];

        // Ensure an entity its health is reset to default.
        if (gameEntity->GetHealth() > gameEntity->GetClient()->persistent.stats.maxHealth) {
            gameEntity->SetHealth(gameEntity->GetClient()->persistent.stats.maxHealth);
		}
    }
} 

qboolean DefaultGameMode::IsDeadEntity(SVGBaseEntity *gameEntity) {
    // Sanity check.
    if (!gameEntity) {
        gi.DPrintf("Warning: IsDeadEntity called with a nullptr.\n");
        return true;
    }

    // In this case it's dead either when health is < 1, or if its DeadFlag has been set.
    if (gameEntity->GetHealth() < 1 || gameEntity->GetDeadFlag()) {
        return true;
    }

    // It's alive.
    return false;
}

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
qboolean DefaultGameMode::OnSameTeam(IServerGameEntity* ent1, IServerGameEntity* ent2) {
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
qboolean DefaultGameMode::CanDamage(IServerGameEntity* target, IServerGameEntity* inflictor) {
    // Destination.
    vec3_t   destination = vec3_zero();
    // Trace.
    SVGTraceResult trace;

    // Solid entities need a special check, as their origin is usually 0,0,0
    // Exception to the above: the solid entity moves or has an origin brush
    if (target->GetMoveType() == MoveType::Push) {
        // Calculate destination.
        destination = target->GetAbsoluteMin() + target->GetAbsoluteMax();
        destination = vec3_scale(destination, 0.5f);
        
        // Execute trace.
        trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, BrushContentsMask::Solid);

        //
        if (trace.fraction == 1.0) {
	        return true;
	    }
	    if (trace.gameEntity == target) {
	        return true;
	    }

        return false;
    }

    // From here on we start tracing in various directions. Look at the code yourself to figure that one out...
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), target->GetOrigin(), inflictor, BrushContentsMask::Solid);

    if (trace.fraction == 1.0) {
        return true;
    }

    destination = target->GetOrigin();
    destination[0] += 15.0;
    destination[1] += 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, BrushContentsMask::Solid);
    if (trace.fraction == 1.0)
        return true;

    destination = target->GetOrigin();
    destination[0] += 15.0;
    destination[1] -= 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, BrushContentsMask::Solid);
    if (trace.fraction == 1.0)
        return true;

    destination = target->GetOrigin();
    destination[0] -= 15.0;
    destination[1] += 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, BrushContentsMask::Solid);
    if (trace.fraction == 1.0)
        return true;

    destination = target->GetOrigin();
    destination[0] -= 15.0;
    destination[1] -= 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_zero(), vec3_zero(), destination, inflictor, BrushContentsMask::Solid);
    if (trace.fraction == 1.0)
        return true;

    // If we reached this point... Well, it is false :)
    return false;
}

//===============
// DefaultGameMode::FindWithinRadius
//
// Returns a GameEntityVector list containing the results of the found
// given entities that reside within the origin to radius. 
// 
// Flags can be set to determine which "solids" to exclude.
//===============
GameEntityVector DefaultGameMode::FindBaseEnitiesWithinRadius(const vec3_t& origin, float radius, uint32_t excludeSolidFlags) {
    // List of base entities to return.
    GameEntityVector radiusEntities;

    // Iterate over all entities, see who is nearby, and who is not.
    for (auto* radiusEntity : game.world->GetGameEntityRange<0, MAX_WIRED_POD_ENTITIES>()
         | cef::Standard
         | cef::WithinRadius(origin, radius, excludeSolidFlags)) {

        // Push radiusEntity result item to the list.
        radiusEntities.push_back(radiusEntity);
    }

    // The list might be empty, ensure to check for that ;-)
    return radiusEntities;
}

//===============
// DefaultGameMode::EntityKilled
//
// Called when an entity is killed, or at least, about to be.
// Determine how to deal with it, usually resides in a callback to Die.
//===============
void DefaultGameMode::EntityKilled(IServerGameEntity* target, IServerGameEntity* inflictor, IServerGameEntity* attacker, int32_t damage, vec3_t point) {
    // Ensure health isn't exceeding limits.
    if (target->GetHealth() < -999)
        target->SetHealth(-999);

    // Set the enemy pointer to the current attacker.
    target->SetEnemy(attacker);

    // Determine whether it is a monster, and if it IS set to being dead....
    if ((target->GetServerFlags() & EntityServerFlags::Monster) && (target->GetDeadFlag() != DeadFlags::Dead)) {
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
            target->DispatchDieCallback(inflictor, attacker, damage, point);
        }

        return;
    }

    if ((target->GetServerFlags() & EntityServerFlags::Monster) && (target->GetDeadFlag() != DeadFlags::Dead)) {
        target->SetTouchCallback(nullptr);

        // This can only be done on base monster entities and derivates
        //if (target->IsSubclassOf<BaseMonster>()) {
        //    target->DeathUse();
        //}
    //    monster_death_use(targ);
    }
    if (target) {
        target->DispatchDieCallback(inflictor, attacker, damage, point);
    }
    //targ->DispatchDieCallback(targ, inflictor, attacker, damage, point);
}

/**
*   @details    Inflicts damage on the targeted entity if it passes certain demands based on the
*               game mode.
*
*               An example of how this method operates using the following operators listed below:
*               Example : target = monster, inflictor = rocket, attacker = player
* 
*   @param  target      The entity that is about to take damage.
*   @param  inflictor   The entity that is causing the damage.
*   @param  attacker    The entity that caused the inflictor to damage target.
*
*   @param  dmgDir      Direction of the attack.
*   @param  point       Point at which the damage is being inflicted.
*   @param  normal      Normal vector from that point.
*   @param  damage      Amount of damage being inflicted.
*   @param  knockBack   Force to be applied against targ as a result of the damage.
*   @param  damageFlags The following flags can be passed in order to affect the outcome:
*                       DamageFlags::IndirectFromRadius Damage was indirect(from a nearby explosion).
*                       DamageFlags::NoArmorProtection  Armor does not protect from this damage.
*                       DamageFlags::EnergyBasedWeapon  Damage is from an energy based weapon.
*                       DamageFlags::NoKnockBack        Do not affect velocity, just view angles.
*                       DamageFlags::Bullet             Damage is from a bullet(used for ricochets).
*                       DamageFlags::IgnoreProtection   Kills the entity, even if it reside in godmode, has specific armor, etc.
**/
void DefaultGameMode::InflictDamage(IServerGameEntity* target, IServerGameEntity* inflictor, IServerGameEntity* attacker, const vec3_t& dmgDir, const vec3_t& point, const vec3_t& normal, int32_t damage, int32_t knockBack, int32_t damageFlags, int32_t mod) {
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
    //    if (GetGameMode()->OnSameTeam(targ, attacker)) {
    //        if ((int)(gamemodeflags->value) & GameModeFlags::NoFriendlyFire)
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
    if (!(damageFlags & DamageFlags::IgnoreProtection) && GetGameMode()->OnSameTeam(target, attacker))
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
        target->DispatchTakeDamageCallback(attacker, knockBack, damageTaken);
        //// nightmare mode monsters don't go into pain frames often
        //if (skill->value == 3)
        //    targ->debouncePainTime = level.time + 5;
        //}
    } else {
        if (client) {
            //if (!(targ->flags & EntityFlags::GodMode) && (take))
            //    targ->Pain(targ, attacker, knockBack, take);
            if (!(target->GetFlags() & EntityFlags::GodMode) && (damageTaken)) {
                target->DispatchTakeDamageCallback(attacker, knockBack, damageTaken);
            }
        } else if (damageTaken) {
            target->DispatchTakeDamageCallback(attacker, knockBack, damageTaken);
        }
    }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    if (client) {
        client->damages.blood += damageTaken;
        client->damages.knockBack += knockBack;
        client->damages.fromOrigin = point;
    }
}

//===============
// DefaultGameMode::InflictDamage
// 
//===============
void DefaultGameMode::InflictRadiusDamage(IServerGameEntity* inflictor, IServerGameEntity* attacker, float damage, IServerGameEntity* ignore, float radius, int32_t mod) {
    // Damage point counter for radius sum ups.
    float   points = 0.f;

    // Actual entity loop pointer.
    SVGBaseEntity* ent = nullptr;

    // PH: From Yamagi Q2, to prevent issues.
    if (!inflictor || !attacker) {
        return;
    }

    // Find entities within radius.
    GameEntityVector radiusEntities = FindBaseEnitiesWithinRadius(inflictor->GetOrigin(), radius, Solid::Not);

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

    // Acquire pointer to server entities array.
    Entity *serverEntities = game.world->GetPODEntities();

    // Unlink the player client entity.
    ent->UnlinkEntity();

    // Grab a body from the queue, and cycle to the next one.
    Entity* bodyEntity = &serverEntities[game.GetMaxClients() + level.bodyQue + 1];
    level.bodyQue = (level.bodyQue + 1) % BODY_QUEUE_SIZE;

    // Send an effect on this body, in case it already has a model index.
    // This'll cause a body not to just "disappear", but actually play some
    // bloody particles over there.
    if (bodyEntity->currentState.modelIndex) {
        gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
        gi.MSG_WriteUint8(TempEntityEvent::Blood);//WriteByte(TempEntityEvent::Blood);
        gi.MSG_WriteVector3(bodyEntity->currentState.origin, false);
        //gi.MSG_WriteVector3(vec3_zero(), false);
		gi.MSG_WriteUint8(DirectionToByte(vec3_zero()));
        gi.Multicast(bodyEntity->currentState.origin, Multicast::PVS);
    }

    // Create the game entity for this queued bodyEntity.
    SVGBaseEntity *bodyGameEntity = game.world->CreateGameEntity<BodyCorpse>(bodyEntity, false);

    // Unlink the body entity, in case it was linked before.
    bodyGameEntity->UnlinkEntity();

    // Copy over the bodies state of the current entity into the body entity.
    bodyGameEntity->SetState(ent->GetState());
    // Change its number so it is accurately set to the one belonging to bodyEntity.
    // (Has to happen since we first copied over an entire entity state.)
    bodyGameEntity->SetNumber(bodyEntity - serverEntities);
    // Set the event ID for this frame to OtherTeleport.
    bodyGameEntity->SetEventID(EntityEvent::OtherTeleport);

    // Copy over the serverflags from ent.
    bodyGameEntity->SetServerFlags(ent->GetServerFlags());
    bodyGameEntity->SetMins(ent->GetMins());
    bodyGameEntity->SetMaxs(ent->GetMaxs());
    bodyGameEntity->SetAbsoluteMin(ent->GetAbsoluteMin());
    bodyGameEntity->SetAbsoluteMax(ent->GetAbsoluteMax());
    bodyGameEntity->SetSize(ent->GetSize());
    bodyGameEntity->SetVelocity(ent->GetVelocity());
    bodyGameEntity->SetAngularVelocity(ent->GetAngularVelocity());
    bodyGameEntity->SetSolid(ent->GetSolid());
    bodyGameEntity->SetClipMask(ent->GetClipMask());
    bodyGameEntity->SetOwner(ent->GetOwner());
    bodyGameEntity->SetMoveType(ent->GetMoveType());
    //bodyGameEntity->SetGroundEntity(ent->GetGroundEntityHandle());

    // Set the die callback, and set its take damage.
    bodyGameEntity->SetDieCallback(&BodyCorpse::BodyCorpseDie);
    bodyGameEntity->SetTakeDamage(TakeDamage::Yes);

    // Link it in for collision etc.
    bodyGameEntity->LinkEntity();
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
    gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
    gi.MSG_WriteUint8(type);//WriteByte(type);
    //  gi.WriteByte (damage); // <-- This was legacy crap, might wanna implement it ourselves eventually.
    gi.MSG_WriteVector3(origin, false);
	gi.MSG_WriteUint8(DirectionToByte(normal));
    //gi.MSG_WriteVector3(normal, false);
    gi.Multicast(origin, Multicast::PVS);
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
// DefaultGameMode::ClientBeginServerFrame
// 
// Does logic checking for a client's start of a server frame. In case there
// is a "level.intermission.time" set, it'll flat out return.
// 
// This basically allows for the game to disable fetching user input that makes
// our movement tick. And/or shoot weaponry while in intermission time.
//===============
void DefaultGameMode::ClientBeginServerFrame(SVGBasePlayer* player, ServerClient *client) {
    // Ensure valid pointers.
    if (!player || !client) {
        return;
    }

    // Ensure we aren't in intermission mode.
    if (level.intermission.time != GameTime::zero()) {
        return;
    }

    // Run weapon animations in case this has not been done by user input itself.
    // (Idle animations, and general weapon thinking when a weapon is not in action.)
    if (!client->respawn.isSpectator) {
        player->WeaponThink();
    }

    // Check if the player is actually dead or not. If he is, we're going to enact on
    // the user input that's been given to us. When fired, we'll respawn.
    if (player->GetDeadFlag()) {
        // Wait for any button just going down
        if (level.time > client->respawnTime) {
            // In old code, the need to hit a key was only set in DM mode.
            // I figured, let's keep it like this instead.
            //if (deathmatch->value)
            int32_t buttonMask = ButtonBits::PrimaryFire;
            //else
            //buttonMask = -1;

            if (client->latchedButtons & buttonMask) 
                // || (deathmatch->value && ((int)gamemodeflags->value & GameModeFlags::ForceRespawn))) {
            {
                GetGameMode()->RespawnClient(player);
                client->latchedButtons = 0;
            }
        }
        return;
    }

    // add player trail so monsters can follow
    //if (!deathmatch->value)
    //    if (!visible(ent, SVG_PlayerTrail_LastSpot()))
    //        SVG_PlayerTrail_Add(ent->currentState.oldOrigin);

    // Reset the latched buttons.
    client->latchedButtons = 0;
}


//===============
// DefaultGameMode::ClientEndServerFrame
// 
// Called for each player at the end of the server frame and right 
// after spawning.
// 
// Used to set the latest view offsets
//===============
void DefaultGameMode::ClientEndServerFrame(SVGBasePlayer* player, ServerClient* client) {
    // Acquire server entity.
    Entity* serverEntity = player->GetPODEntity();
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
    if (level.intermission.time != GameTime::zero()) {
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

	if (client->playerState.pmove.viewAngles[vec3_t::Pitch] > 180)
        newAngles[vec3_t::Pitch] = (-360 + client->playerState.pmove.viewAngles[vec3_t::Pitch]) / 3;
    else
        newAngles[vec3_t::Pitch] = client->playerState.pmove.viewAngles[vec3_t::Pitch] / 3;
    newAngles[vec3_t::Yaw] = client->playerState.pmove.viewAngles[vec3_t::Yaw];
    newAngles[vec3_t::Roll] = 0;
    newAngles[vec3_t::Roll] = player->CalculateRoll(newAngles, player->GetVelocity()) * 4;

    //if (client->aimAngles[vec3_t::Pitch] > 180)
    //    newAngles[vec3_t::Pitch] = (-360 + client->aimAngles[vec3_t::Pitch]) / 3;
    //else
    //    newAngles[vec3_t::Pitch] = client->aimAngles[vec3_t::Pitch] / 3;
    //newAngles[vec3_t::Yaw] = client->aimAngles[vec3_t::Yaw];
    //newAngles[vec3_t::Roll] = 0;
    //newAngles[vec3_t::Roll] = player->CalculateRoll(newAngles, player->GetVelocity()) * 4;

    // Last but not least, after having calculated the Pitch, Yaw, and Roll, set the new angles.
    player->SetAngles(newAngles);

    //
    // Calculate the player its X Y axis' speed and calculate the cycle for
    // bobbing based on that.
    //
    vec3_t playerVelocity = player->GetVelocity();
    // Without * FRAMETIME_S = XYSpeed = sqrtf(playerVelocity[0] * playerVelocity[0] + playerVelocity[1] * playerVelocity[1]);
    bobMoveCycle.XYSpeed = sqrtf(playerVelocity[0] * playerVelocity[0] + playerVelocity[1] * playerVelocity[1]);

	// Do we have a valid ground entity?
	bool isValidGroundEntity = ServerGameWorld::ValidateEntity(player->GetGroundEntityHandle());

    if ( bobMoveCycle.XYSpeed < 5 ) {
        // Special handling for when not on ground.
        bobMoveCycle.move = 0;

        // Start at beginning of cycle again (See the else if statement.)
        client->bobTime = 0;
    } else if ( isValidGroundEntity || player->GetWaterLevel() == 2 ) {
        // So bobbing only cycles when on ground.
        if ( bobMoveCycle.XYSpeed > 450) {
            bobMoveCycle.move = 0.25;
		} else if ( bobMoveCycle.XYSpeed > 210) {
            bobMoveCycle.move = 0.125;
		} else if ( !isValidGroundEntity && player->GetWaterLevel() == 2 && bobMoveCycle.XYSpeed > 100 ) {
            bobMoveCycle.move = 0.225;
		} else if ( bobMoveCycle.XYSpeed > 100 ) {
            bobMoveCycle.move = 0.0825;
		} else if ( !isValidGroundEntity && player->GetWaterLevel() == 2 ) {
            bobMoveCycle.move = 0.1625;
		} else {
            bobMoveCycle.move = 0.03125;
		}
    }

    // Calculate bob time, cycle, and sin fraction.
    bobMoveCycle.move /= 3.5;

	// Local frame bobtime set to total bobtime after adding the move of our frame.
    float bobTime = ( client->bobTime += bobMoveCycle.move );

	// Multipl by 1.5 in case of crouching.
    if ( client->playerState.pmove.flags & PMF_DUCKED ) {
        bobTime *= 1.5;
	}

	// Calculate cycle and frac sin.
    bobMoveCycle.cycle = static_cast<int64_t>( bobTime );
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
    if (client->showScores && ((level.time % 3200) == GameTime::zero())) {
        SVG_HUD_GenerateDMScoreboardLayout(player, player->GetEnemy());
        gi.Unicast(serverEntity, false);
    }
}

/**
*	@brief	Wrapper PM_Trace to interscept and adjust tracing needs if desired.
*			( PMove doesn't need to know about skipEntity and ContentMask. )
**/
TraceResult DefaultGameMode::PM_Trace( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end ) {
	if (!pmSkipEntity) {
		Com_Error( ErrorType::Drop, "DefaultGameMode::PM_Trace called without a valid skipEntity\n" );
	}

	if ( pmSkipEntity->GetHealth() > 0 ) {
        return gi.Trace(start, mins, maxs, end, pmSkipEntity->GetPODEntity(), BrushContentsMask::PlayerSolid);
    } else {
        return gi.Trace(start, mins, maxs, end, pmSkipEntity->GetPODEntity(), BrushContentsMask::DeadSolid);
    }
}


void DefaultGameMode::ClientThink( SVGBasePlayer* player, ServerClient* client, ClientMoveCommand* moveCommand ) {
    // Store the current entity to be run from SVG_RunFrame.
    level.currentEntity = player;

    // Set move type to freeze in case intermission has a waiting time set on it.
    if ( level.intermission.time != GameTime::zero() ) {
        player->SetPlayerMoveType( EnginePlayerMoveType::Freeze );
        
        // Can exit intermission after five seconds
        if ( level.time > level.intermission.time + 5s && ( moveCommand->input.buttons & ButtonBits::Any ) ) {
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
        pmSkipEntity = player;

        // Update player move's gravity state.
        client->playerState.pmove.gravity = sv_gravity->value;

		// Validate the ground entity we acquire based on the groundEntityNumber.
		GameEntity *validGroundEntity = ServerGameWorld::ValidateEntity(player->GetGroundPODEntity());
		const int32_t groundEntityNumber = (validGroundEntity ? validGroundEntity->GetNumber() : -1);

        // Copy over the pmove state from the latest player state.
        PlayerMove pm         = {};
        pm.moveCommand        = *moveCommand;
        pm.groundEntityNumber = groundEntityNumber;
        pm.state              = client->playerState.pmove;
        pm.state.origin       = player->GetOrigin();
        pm.state.velocity     = player->GetVelocity();
        
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
		GameEntity *gePlayerGroundEntity = ServerGameWorld::ValidateEntity( player->GetGroundEntityHandle() );

		// If the player had a ground entity stored coming from its previous player move frame.
		if (gePlayerGroundEntity) {
			// And we lost ground in the current move, player our jump sound.
			if (pm.groundEntityNumber == -1 && (pm.moveCommand.input.upMove >= 10) && (pm.waterLevel == 0)) {
				SVG_Sound(player, SoundChannel::Voice, SVG_PrecacheSound("player/jump1.wav"), 1, Attenuation::Normal, 0);
				player->PlayerNoise(player, player->GetOrigin(), PlayerNoiseType::Self);
			}
        }

		// Resolve the perhaps new Ground Entity.
		ServerGameWorld *gameWorld = GetGameWorld();
		// Resolve the perhaps new Ground Entity.
		if ( gameWorld ) {
			GameEntity *geGround = gameWorld->GetGameEntityByIndex( pm.groundEntityNumber );

			// Is the ground a valid pointer?
			if ( geGround ) {
				player->SetGroundEntity( geGround );
				player->SetGroundEntityLinkCount( geGround->GetLinkCount() );
			} else {
				player->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
			}
		}

        // Copy over the user command angles so they are stored for respawns.
        // (Used when going into a new map etc.)
        client->respawn.commandViewAngles[0] = moveCommand->input.viewAngles[0];
        client->respawn.commandViewAngles[1] = moveCommand->input.viewAngles[1];
        client->respawn.commandViewAngles[2] = moveCommand->input.viewAngles[2];

        // Special treatment for angles in case we are dead. Target the killer entity yaw angle.
        if ( player->GetDeadFlag() != DeadFlags::Alive ) {
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
        const int32_t playerMoveType = player->GetMoveType();

        // Execute touch callbacks as long as movetype isn't noclip, or spectator.
        if ( playerMoveType != MoveType::NoClip && playerMoveType  != MoveType::Spectator ) {
            // Trigger touch logic. 
            SG_TouchTriggers(player);
		
            // Solid touch logic.
            int32_t i = 0;
            int32_t j = 0;
            
            for ( i = 0; i < pm.numTouchedEntities; i++ ) {
				// Skip touch events on the same entities.
                for ( j = 0; j < i ; j++ ) {
					const int32_t touchEntityNumberA = SG_GetEntityNumber( pm.touchedEntityTraces[j].podEntity );
					const int32_t touchEntityNumberB = SG_GetEntityNumber( pm.touchedEntityTraces[i].podEntity );
                    if ( touchEntityNumberA == touchEntityNumberB ) {
                        break;
                    }
                }
				// 
                if ( j != i ) {
                    continue; // Duplicated.
                }

				// Get Touched Entity number.
				const int32_t touchedEntityNumber = SG_GetEntityNumber( pm.touchedEntityTraces[i].podEntity );

				// Get the POD Entity.
				PODEntity *podTouchEntity = gameWorld->GetPODEntityByIndex( touchedEntityNumber );

				// Create our Entity Handle.
				SGEntityHandle ehOther(podTouchEntity);

				GameEntity *geOther = ServerGameWorld::ValidateEntity( ehOther );
				if (!geOther) {
					continue;
				}

				geOther->DispatchTouchCallback( geOther, player, NULL, NULL );
            } // for (i = 0 ; i < pm.numTouchedEntities; i++) {
       } // if ( playerMoveType != MoveType::NoClip && playerMoveType  != MoveType::Spectator ) {
    }

    // Update client button bits.
    SetClientButtonBits( client, moveCommand );


	/**
	*	Use Functionality.
	*
	*	If the player sent us a Use button action, scan for which entity he
	*	has targetted and dispatch its use callback.
	**/
	if ( client->latchedButtons & ButtonBits::Use ) {
		// Get Aim Angles.
		const vec3_t aimAngles = client->aimAngles + client->kickAngles;

		// Calculate forward and right vectors.
        vec3_t forward = vec3_zero(), right = vec3_zero();
        AngleVectors( aimAngles, &forward, &right, NULL );

		// Calculate traceStart by projecting an offset vector from player origin.
		const vec3_t originOffset = { 0, 8, static_cast<float>(player->GetViewHeight() - 8) };
        const vec3_t useTraceStart = SVG_ProjectSource( player->GetOrigin(), originOffset, forward, right );

		// Calculate traceEnd starting from the traceStart .
		const vec3_t useTraceEnd	= vec3_fmaf( useTraceStart, 64, forward );

		// Trace Mask.
		const int32_t useTraceMask	= BrushContentsMask::PlayerSolid | BrushContentsMask::MonsterSolid;

		// Perform trace.
		const SGTraceResult useTraceResult = SG_Trace( useTraceStart, vec3_zero(), vec3_zero(), useTraceEnd, player, useTraceMask );
		
		// Fetch entity.
		const int32_t useEntityNumber = SG_GetEntityNumber(useTraceResult.gameEntity);

		// If we got an entity, get a pointer to it for dispatching.
		if ( useEntityNumber > 0 ) {
			// Get GameWorld.
			SGGameWorld *gameWorld = GetGameWorld();
			// Get 'Use' entity.
			GameEntity *geUse = gameWorld->GetGameEntityByIndex( useEntityNumber );

			// The geUse pointer is valid, inspect whether it can truly be "Used".
			if ( geUse ) {
				// Figure out if this entity is active( in use ).
				const bool isInUse = geUse->IsInUse();
				// Get the entity use flags to determine how to operate from here on.
				const int32_t useEntityFlags = geUse->GetUseEntityFlags();

				// Use it once, requiring the '+use' action to be released again.
				if ( useEntityFlags & UseEntityFlags::Toggle ) {
					// Get 
					geUse->DispatchUseCallback( player, player );

					// Remove 'Use' button bit.
					client->latchedButtons &= ~ButtonBits::Use;
				}
				// Hold usage simply means we don't release the '+use' action.
				if ( useEntityFlags & UseEntityFlags::Hold ) {
					// Get 
					geUse->DispatchUseCallback( player, player );

					// Remove 'Use' button bit.
					//client->latchedButtons &= ~ButtonBits::Use;
				}
			} // if ( geUse )
		} // (useEntityNumber > 0)
	} // if ( client->latchedButtons & ButtonBits::Use )

	/**
	*	Fire weapon from final position if needed
	**/
    if ( client->latchedButtons & ButtonBits::PrimaryFire ) {
        if ( client->respawn.isSpectator ) {

            client->latchedButtons = 0;

            if ( client->chaseTarget ) {
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
	    if ( !client->respawn.isSpectator ) {
	        //player->WeaponThink();
	    }
    }

    // Act on the jump key(which sets upMove), used to change spectator targets.
    if ( client->respawn.isSpectator ) {
        if ( moveCommand->input.upMove >= 10 ) {
            // When jump isn't held yet in the player move flags..
            if ( !(client->playerState.pmove.flags & PMF_JUMP_HELD) ) {
                // We add the jump held bit.
                client->playerState.pmove.flags |= PMF_JUMP_HELD;

                // So we can change chase target.
                if ( client->chaseTarget ) {
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
    //    other = game.world->GetPODEntities() + i;
    //    if (other->inUse && other->client->chaseTarget == serverEntity)
    //        SVG_UpdateChaseCam(playerEntity);
    //}
}

//===============
// DefaultGameMode::ClientConnect
// 
// Client is connecting, what do? :)
//===============
qboolean DefaultGameMode::ClientConnect(PODEntity *svEntity, char *userinfo) {
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
    svEntity->client = &clients[svEntity->currentState.number - 1];//game.clients + (serverEntity - g_entities - 1);

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
// DefaultGameMode::ClientBegin
// 
// Called when a client is ready to be placed in the game after connecting.
//===============
void DefaultGameMode::ClientBegin(PODEntity *svEntity) {
    // Player entity, to be assigned next.
    SVGBasePlayer* player = nullptr;

    // If there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (static_cast<bool>(svEntity->inUse) == true) {
        // Only pull through if it is a base player.
        if (svEntity->gameEntity->IsSubclassOf<SVGBasePlayer>()) {
	        player = dynamic_cast<SVGBasePlayer*>(svEntity->gameEntity);
        } else {
	        gi.Error(ErrorType::Drop, "SVGame Error: ClientBegin called with an inUse entity that is not of type or derived from SVGBasePlayer\n");
        }

        // The client has cleared the client side viewAngles upon
        // connecting to the server, which is different than the
        // state when the game is saved, so we need to compensate
        // with deltaangles
        for (int32_t i = 0; i < 3; i++) {
            svEntity->client->playerState.pmove.deltaAngles[i] = svEntity->client->playerState.pmove.viewAngles[i];
        }
    } else {
        //// Assign  this client to the server entity.
        //svEntity->client  = client;

        // Create the player client entity.
        player = SVGBasePlayer::Create(svEntity);

        // Initialize client respawn data.
        InitializePlayerRespawnData(svEntity->client);
 
        // Put into our server and blast away! (Takes care of spawning gameEntity).
        PlacePlayerInGame(player);
    }

    if (level.intermission.time != GameTime::zero()) {
        HUD_MoveClientToIntermission(svEntity);
    } else {
        // send effect if in a multiplayer game
        if (game.GetMaxClients() > 1) {
	        gi.MSG_WriteUint8(ServerGameCommand::MuzzleFlash);//WriteByte(ServerGameCommand::MuzzleFlash);
	        //gi.WriteShort(serverEntity - g_entities);
	        gi.MSG_WriteInt16(player->GetNumber());//WriteShort(player->GetNumber());
	        gi.MSG_WriteUint8(MuzzleFlashType::Login);//WriteByte(MuzzleFlashType::Login);
	        gi.Multicast(player->GetOrigin(), Multicast::PVS);

            gi.BPrintf(PRINT_HIGH, "%s entered the game\n", svEntity->client->persistent.netname);
        }
    }

    // Call ClientEndServerFrame to update him through the beginning frame.
    ClientEndServerFrame(player, svEntity->client);
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
void DefaultGameMode::ClientDisconnect(SVGBasePlayer* player, ServerClient *client) {
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
    //player->SetClassname("disconnected");

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

    // Set spectator to false.
    ent->client->persistent.isSpectator = false;

    // set skin
    s = Info_ValueForKey(userinfo, "skin");

    playernum = ent - game.world->GetPODEntities() - 1;

    // combine name and skin into a configstring
    gi.configstring(ConfigStrings::PlayerSkins + playernum, va("%s\\%s", ent->client->persistent.netname, s));

    // fov
    if (((int)gamemodeflags->value & GameModeFlags::FixedFOV)) {
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
void DefaultGameMode::ClientUpdateObituary(IServerGameEntity* self, IServerGameEntity* inflictor, IServerGameEntity* attacker) {
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


void DefaultGameMode::InitializePlayerPersistentData(ServerClient* client) {
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

void DefaultGameMode::InitializePlayerRespawnData(ServerClient* client) {
    if (!client) { 
        return;
    }

    client->respawn = {};
    client->respawn.enterGameTimestamp = level.time;
    client->respawn.persistentCoopRespawn = client->persistent;
}

//===============
// DefaultGameMode::SelectClientSpawnPoint
//
// Choose any info_player_start or its derivates, it'll do a subclassof check, so the only valid classnames are
// those who have inherited from info_player_start. (info_player_deathmatch, etc).
//===============
void DefaultGameMode::SelectPlayerSpawnPoint(SVGBasePlayer* player, vec3_t& origin, vec3_t& angles) {
    // Spawn point entity pointer.
    IServerGameEntity *spawnPoint = nullptr;

    // Find a spawn point that has a targetname matching game.spawnpoint.
    auto targetSpawnPoints = game.world->GetGameEntityRange<0, MAX_WIRED_POD_ENTITIES>() | cef::Standard | cef::IsSubclassOf<InfoPlayerStart>() | cef::HasKeyValue("targetname", game.spawnpoint);

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
	    std::vector<IServerGameEntity*> spawnVector;
        auto spawnPoints = game.world->GetGameEntityRange<0, MAX_WIRED_POD_ENTITIES>() | cef::Standard | cef::IsSubclassOf<InfoPlayerStart>();
        for (auto& entity : spawnPoints) { spawnVector.push_back(entity); }

        // Select random spawn point.
    	if (spawnVector.size() > 0) {
		    spawnPoint = spawnVector[RandomRangeui(0, spawnVector.size())];
	    }
    }

    // Assign origin and raise us up 9 on the z axis to ensure we are never stuck on something.
    if (spawnPoint) {
        origin = spawnPoint->GetOrigin();
        origin.z += 1;
        angles = spawnPoint->GetAngles();
    } else {
        // We might as well error out at this point.
        gi.Error(ErrorType::Drop, "SVGame Error: Couldn't find spawn point %s", game.spawnpoint);
    }
}

//===============
// DefaultGameMode::PlacePlayerInGame
// 
// Called when a player connects to a single and multiplayer. 
// 
// #1. In the case of a SP mode death, the loadmenu pops up and selecting a load game
// will restart the server.
// #2. In thecase of a MP mode death however, after a small intermission time, it'll
// call this function again to respawn our player.
//===============
void DefaultGameMode::PlacePlayerInGame(SVGBasePlayer *player) {
    // Acquire pointer to game clients.
    ServerClient* gameClients = game.GetClients();

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
    ClientUserinfoChanged(player->GetPODEntity(), userinfo);
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
    player->Spawn();

    // Copy some data from the client to the entity
    RestorePlayerPersistentData(player, client);

    // Update the client pointer to match with the client index.
    client = &gameClients[clientIndex];
    // Ready to roll, let's assign it.
    player->SetClient(client);
 
    // Clear playerstate values.
    client->playerState = {};

    if (((int)gamemodeflags->value & GameModeFlags::FixedFOV)) {
        client->playerState.fov = 90;
    } else {
        client->playerState.fov = atoi(Info_ValueForKey(client->persistent.userinfo, "fov"));
        if (client->playerState.fov < 1) {
            client->playerState.fov = 90;
        } else if (client->playerState.fov > 160) {
            client->playerState.fov = 160;
        }
    }

	// Find and select a random spawn point.
    vec3_t  spawnOrigin = vec3_zero();
    vec3_t  spawnAngles = vec3_zero();
    SelectPlayerSpawnPoint(player, spawnOrigin, spawnAngles);

	// Set the actual player spawn origin. Offset of 1 unit off the ground.
    player->SetOrigin(spawnOrigin);
	// We set the oldOrigin too, since we're spawning at and NOT lerping from a location.
    player->SetOldOrigin(spawnOrigin);
	// Set the entity rotation to spawn angle yaw.
	const vec3_t spawnViewAngles = { 
		0.f, 
		AngleMod( spawnAngles[vec3_t::Yaw] ), 
		0.f 
	};
	player->SetAngles( spawnViewAngles );

	// Setup player move origin to spawnpoint origin.	
    client->playerState.pmove.origin = spawnOrigin;
	// Set the player move state's directional view angles.
	client->playerState.pmove.viewAngles = spawnViewAngles;
	// Calculate the delta angles of the client.
	client->playerState.pmove.deltaAngles = client->playerState.pmove.viewAngles - client->respawn.commandViewAngles;
	client->oldViewAngles = spawnViewAngles;
	client->aimAngles = spawnViewAngles;

	//self->s.angles[PITCH] = 0;
	//self->s.angles[YAW] = anglemod( spawn_angles[YAW] );
	//self->s.angles[ROLL] = 0;
	//VectorCopy( self->s.angles, client->ps.viewangles );

	//// set the delta angle
	//for( i = 0; i < 3; i++ )
	//	client->ps.pmove.delta_angles[i] = ANGLE2SHORT( client->ps.viewangles[i] ) - client->ucmd.angles[i];

    // spawn a spectator in case the client was/is one.
    if (client->persistent.isSpectator) {
        // Nodefault chase target.
        client->chaseTarget = nullptr;

        // Well we knew this but store it in respawn data too.
        client->respawn.isSpectator = true;

        // Movement type is the obvious spectator.
        player->SetMoveType(MoveType::Spectator);

        // No solid.
        player->SetSolid(Solid::OctagonBox);

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

    // Unset gun and ammo indices.
    client->playerState.gunIndex    = 0;
    client->primaryAmmoIndex        = 0;
    client->secondaryAmmoIndex      = 0;
    client->clipAmmoIndex           = 0;

    // Ensure we change to whichever active weaponID we had.
    player->ChangeWeapon(client->persistent.inventory.activeWeaponID, false);
}

//===============
// DefaultGameMode::RespawnClient
// 
// Since the default game mode is intended to be a single player mode,
// there is no respawning and we show a loadgame menu instead.
//===============
void DefaultGameMode::RespawnClient(SVGBasePlayer* ent) {
    // Kept around here to port later to other gamemodes.
    //if (deathmatch->value || coop->value) {
    //    // Spectator's don't leave bodies
    //    if (self->gameEntity->GetMoveType() != MoveType::NoClip && self->gameEntity->GetMoveType() != MoveType::Spectator)
    //        GetGameMode()->SpawnClientCorpse(self->gameEntity);

    //    self->serverFlags &= ~EntityServerFlags::NoClient;
    //    GetGameMode()->PlacePlayerInGame((SVGBasePlayer*)self->gameEntity);

    //    // add a teleportation effect
    //    self->currentState.eventID = EntityEvent::PlayerTeleport;

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
// DefaultGameMode::RespawnAllClients
//
//===============
void DefaultGameMode::RespawnAllClients() {
    // Do nothing for default game mode.
}

//===============
// DefaultGameMode::ClientDeath
// 
// Does nothing for this game mode.
//===============
void DefaultGameMode::ClientDeath(SVGBasePlayer *player) {

}

//===============
// DefaultGameMode::StorePlayerPersistentData
// 
// Some information that should be persistant, like health,
// is still stored in the edict structure, so it needs to
// be mirrored out to the client structure before all the
// edicts are wiped.
//===============
void DefaultGameMode::StorePlayerPersistentData(void) {
    // Acquire server entity pointer.
    Entity* serverEntities = game.world->GetPODEntities();

    // Acquire a pointer to the game's clients.
    ServerClient* gameClients = game.GetClients();
    
    for (int32_t i = 0 ; i < game.GetMaxClients(); i++) {
        Entity *entity = &serverEntities[1 + i];
        if (!entity->inUse)
            continue;
        if (!entity->gameEntity)
            continue;

        gameClients[i].persistent.stats.health = entity->gameEntity->GetHealth();
        gameClients[i].persistent.stats.maxHealth = entity->gameEntity->GetMaxHealth();
        gameClients[i].persistent.savedFlags = (entity->gameEntity->GetFlags() & (EntityFlags::GodMode | EntityFlags::NoTarget | EntityFlags::PowerArmor));
    }
}

//===============
// DefaultGameMode::RespawnClient
// 
// // Fetch client data that was stored between previous entity wipe session
//===============
void DefaultGameMode::RestorePlayerPersistentData(SVGBaseEntity* player, ServerClient* client) {
    if (!player || !client)
        return;
    
    // Set health back to what was stored in the persistent data.
    player->SetHealth(client->persistent.stats.health);
    // Set maximum health back to what was stored in the persistent data.
    player->SetMaxHealth(client->persistent.stats.maxHealth);
    // Add saved persistent flags to the player.
    player->SetFlags(player->GetFlags() | client->persistent.savedFlags);
}

/**
*   @brief  Sets a client's button, oldButton, and latched button bits.
**/
void DefaultGameMode::SetClientButtonBits(ServerClient *client, ClientMoveCommand* moveCommand) {
    if (!client || !moveCommand) {
        return;
    }

    // Store the buttons that were still set as current frame buttons in the oldButtons.
    client->oldButtons = client->buttons;

    // Update current buttons with those acquired from the move command.
    client->buttons = moveCommand->input.buttons;

    // Figure out the latched buttons by bit fun. (latched buttons are used for single button press logic.)
    client->latchedButtons |= client->buttons & ~client->oldButtons;
}

/**
*   @brief  Sets client into intermission mode by setting movetype to freeze
*           and positioning the client at the intermission point.
**/
void DefaultGameMode::StartClientIntermission(SVGBasePlayer* player, ServerClient *client) {
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