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
// g_combat.c

#include "g_local.h"         // Include SVGame funcs.
#include "entities.h"
#include "utils.h"           // Include Utilities funcs.

// Game Mode interface.
#include "gamemodes/IGameMode.h"

// Class Entities.
#include "entities/base/SVGBaseEntity.h"

//
//===============
// SVG_EntityKilled
// 
// Called when an entity is killed, or at least, about to be.
// Determine how to deal with it, usually resides in a callback to Die.
//===============
//
void SVG_EntityKilled(SVGBaseEntity *targ, SVGBaseEntity *inflictor, SVGBaseEntity *attacker, int damage, vec3_t point)
{
    // Ensure health isn't exceeding limits.
    if (targ->GetHealth() < -999)
        targ->SetHealth(-999);

    // Set the enemy pointer to the current attacker.
    targ->SetEnemy(attacker);

//    if ((targ->serverFlags & EntityServerFlags::Monster) && (targ->deadFlag != DEAD_DEAD)) {
////      targ->serverFlags |= EntityServerFlags::DeadMonster;   // now treat as a different content type
//        if (!(targ->monsterInfo.aiflags & AI_GOOD_GUY)) {
//            level.killedMonsters++;
//            if (coop->value && attacker->client)
//                attacker->client->respawn.score++;
//            // medics won't heal monsters that they kill themselves
//            if (strcmp(attacker->className, "monster_medic") == 0)
//                targ->owner = attacker;
//        }
//    }

    if (targ->GetMoveType() == MoveType::Push || targ->GetMoveType() == MoveType::Stop || targ->GetMoveType() == MoveType::None) {
        // Doors, triggers, etc
        if (targ) {
            targ->Die(inflictor, attacker, damage, point);
        }

        return;
    }

    //if ((targ->serverFlags & EntityServerFlags::Monster) && (targ->deadFlag != DEAD_DEAD)) {
    //    targ->Touch = NULL;
    //    monster_death_use(targ);
    //}
    if (targ) {
        targ->Die(inflictor, attacker, damage, point);
    }
    //targ->Die(targ, inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
void SpawnDamage(int type, const vec3_t &origin, const vec3_t &normal, int damage)
{
    if (damage > 255)
        damage = 255;
    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(type);
//  gi.WriteByte (damage);
    gi.WriteVector3(origin);
    gi.WriteVector3(normal);
    gi.Multicast(&origin, MultiCast::PVS);
}

//
//===============
// SVG_InflictDamage
//
// Inflicts actual damage on the targeted entity, the rest speaks for itself.
// Calls into the GameMode of course, to ensure whether things are solid to do at all.
// 
// If you'd like the old info...
// targ        entity that is being damaged
// inflictor   entity that is causing the damage
// attacker    entity that caused the inflictor to damage targ
// example : targ = monster, inflictor = rocket, attacker = player
//
// dir         direction of the attack
// point       point at which the damage is being inflicted
// normal      normal vector from that point
// damage      amount of damage being inflicted
// knockBack   force to be applied against targ as a result of the damage
//
// dflags      these flags are used to control how SVG_InflictDamage works
// DamageFlags::IndirectFromRadius           damage was indirect(from a nearby explosion)
// DamageFlags::NoArmorProtection         armor does not protect from this damage
// DamageFlags::EnergyBasedWeapon           damage is from an energy based weapon
// DamageFlags::NoKnockBack     do not affect velocity, just view angles
// DamageFlags::Bullet           damage is from a bullet(used for ricochets)
// DamageFlags::IgnoreProtection    kills godmode, armor, everything
//===============
//
void SVG_InflictDamage(SVGBaseEntity *targ, SVGBaseEntity *inflictor, SVGBaseEntity *attacker, const vec3_t &dmgDir, const vec3_t &point, const vec3_t &normal, int damage, int knockBack, int dflags, int mod)
{
    int damageTaken = 0;   // Damage taken.
    int damageSaved = 0;   // Damaged saved, from being taken.

    // Best be save than sorry.
    if (!targ || !inflictor || !attacker)
    {
        return;
    }

    if (!targ->GetTakeDamage())
        return;

    // WID: This sticks around, cuz of reference, but truly will be all but this itself was.
    // friendly fire avoidance
    // if enabled you can't hurt teammates (but you can hurt yourself)
    // knockBack still occurs
    //if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (GameModeFlags::ModelTeams | GameModeFlags::SkinTeams))) || coop->value)) {
    //    if (game.gameMode->OnSameTeam(targ, attacker)) {
    //        if ((int)(dmflags->value) & GameModeFlags::NoFriendlyFire)
    //            damage = 0;
    //        else
    //            mod |= MeansOfDeath::FriendlyFire;
    //    }
    //}
    meansOfDeath = mod;

    // Fetch client.
    GameClient *client = targ->GetClient();

    // Lame thing, regarding the sparks to use. Ancient code, keeping it for now.
    int te_sparks = TempEntityEvent::Sparks;
    if (dflags & DamageFlags::Bullet)
        te_sparks = TempEntityEvent::BulletSparks;
    
    // Retrieve normalized direction.
    vec3_t dir = vec3_normalize(dmgDir);
    //VectorNormalize2(dmgDir, dir);

    if (targ->GetFlags() & EntityFlags::NoKnockBack)
        knockBack = 0;

    // Figure momentum add
    if (!(dflags & DamageFlags::NoKnockBack)) {
        if ((knockBack) && (targ->GetMoveType() != MoveType::None) && (targ->GetMoveType() != MoveType::Bounce) && (targ->GetMoveType() != MoveType::Push) && (targ->GetMoveType() != MoveType::Stop)) {
            vec3_t  kvel;
            float   mass;

            if (targ->GetMass() < 50)
                mass = 50;
            else
                mass = targ->GetMass();

            if (targ->GetClient() && attacker == targ)
                VectorScale(dir, 1600.0 * (float)knockBack / mass, kvel);   // the rocket jump hack...
            else
                VectorScale(dir, 500.0 * (float)knockBack / mass, kvel);

            targ->SetVelocity(targ->GetVelocity() + kvel);
        }
    }

    // Setup damages, so we can maths with them, yay. Misses code cuz we got no armors no more :P
    damageTaken = damage;       // Damage taken.
    damageSaved = 0;            // Damaged saved, from being taken.

    // check for godmode
    if ((targ->GetFlags() & EntityFlags::GodMode) && !(dflags & DamageFlags::IgnoreProtection)) {
        damageTaken = 0;
        damageSaved = damage;
        SpawnDamage(te_sparks, point, normal, damageSaved);
    }

    // Team damage avoidance
    if (!(dflags & DamageFlags::IgnoreProtection) && game.gameMode->OnSameTeam(targ, attacker))
        return;

    // Inflict the actual damage, in case we got to deciding to do so based on the above.
    if (damageTaken) {
        if ((targ->GetServerFlags() & EntityServerFlags::Monster) || (client))
        {
            // SpawnDamage(TempEntityEvent::Blood, point, normal, take);
            SpawnDamage(TempEntityEvent::Blood, point, dir, damageTaken);
        }
        else
            SpawnDamage(te_sparks, point, normal, damageTaken);


        targ->SetHealth(targ->GetHealth() - damageTaken);

        if (targ->GetHealth() <= 0) {
            if ((targ->GetServerFlags() & EntityServerFlags::Monster) || (client))
                targ->SetFlags(targ->GetFlags() | EntityFlags::NoKnockBack);
            SVG_EntityKilled(targ, inflictor, attacker, damageTaken, point);
            return;
        }
    }

    // Special damage handling for monsters.
    if (targ->GetServerFlags() &EntityServerFlags::Monster) {
        // WID: Maybe do some check for monster entities here sooner or later? Who knows...
        // Gotta have them cunts react to it. But we'll see, might as well be on TakeDamage :)
        //M_ReactToDamage(targ, attacker);

        //if (!(targ->monsterInfo.aiflags & AI_DUCKED) && (take)) {
            targ->TakeDamage(attacker, knockBack, damageTaken);
            //// nightmare mode monsters don't go into pain frames often
            //if (skill->value == 3)
            //    targ->debouncePainTime = level.time + 5;
        //}
    } else {
        if (client) {
            //if (!(targ->flags & EntityFlags::GodMode) && (take))
            //    targ->Pain(targ, attacker, knockBack, take);
            if (!(targ->GetFlags() & EntityFlags::GodMode) && (damageTaken)) {
                targ->TakeDamage(attacker, knockBack, damageTaken);
            }
        } else if (damageTaken) {
            targ->TakeDamage(attacker, knockBack, damageTaken);
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


//
//===============
// SVG_InflictRadiusDamage
//
// Similar to SVG_InflictDamage, however, we scan for entities within a radious to then
// go haywire on because we appreciate to put them into a lot of misery. Just sayin' ;-) :P
//===============
//
void SVG_InflictRadiusDamage(SVGBaseEntity *inflictor, SVGBaseEntity *attacker, float damage, SVGBaseEntity *ignore, float radius, int mod)
{
    float   points = 0.f;
    SVGBaseEntity *ent = NULL;
    vec3_t  v = { 0.f, 0.f, 0.f };
    vec3_t  dir = { 0.f, 0.f, 0.f };

    // N&C: From Yamagi Q2, to prevent issues.
    if (!inflictor || !attacker) {
        return;
    }

    // Find entities within radius.
    while ((ent = SVG_FindEntitiesWithinRadius(ent, inflictor->GetOrigin(), radius)) != NULL) {
        // Continue in case this entity has to be ignored from applying damage.
        if (ent == ignore)
            continue;
        // Continue in case this entity CAN'T take any damage.
        if (!ent->GetTakeDamage())
            continue;

        // Calculate damage points.
        v = ent->GetMins() + ent->GetMaxs();
        v = vec3_fmaf(ent->GetOrigin(), 0.5, v);
        v -= inflictor->GetOrigin();
        points = damage - 0.5 * vec3_length(v);

        // In case the attacker is the own entity, half damage.
        if (ent == attacker)
            points = points * 0.5;

        // Apply damage points.
        if (points > 0) {
            // Ensure whether we CAN actually apply damage.
            if (game.gameMode->CanDamage(ent, inflictor)) {
                // Calculate direcion.
                dir = ent->GetOrigin() - inflictor->GetOrigin();

                // Apply damages.
                SVG_InflictDamage(ent, inflictor, attacker, dir, inflictor->GetOrigin(), vec3_zero(), (int)points, (int)points, DamageFlags::IndirectFromRadius, mod);
            }
        }
    }
}
