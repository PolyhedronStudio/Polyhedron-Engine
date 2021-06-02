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
#include "entities/base/SVGBaseEntity.h"

/*
============
SVG_CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean SVG_CanDamage(SVGBaseEntity *targ, SVGBaseEntity *inflictor)
{
    vec3_t  dest;
    SVGTrace trace;

// bmodels need special checking because their origin is 0,0,0
    if (targ->GetMoveType() == MoveType::Push) {
        // Calculate destination.
        dest = vec3_scale(targ->GetAbsoluteMin() + targ->GetAbsoluteMax(), 0.5f);
        trace = SVG_Trace(inflictor->GetOrigin(), vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
        if (trace.fraction == 1.0)
            return true;
        if (trace.ent == targ)
            return true;
        return false;
    }

    trace = SVG_Trace(inflictor->GetOrigin(), vec3_origin, vec3_origin, targ->GetOrigin(), inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->GetOrigin(), dest);
    dest[0] += 15.0;
    dest[1] += 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->GetOrigin(), dest);
    dest[0] += 15.0;
    dest[1] -= 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->GetOrigin(), dest);
    dest[0] -= 15.0;
    dest[1] += 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->GetOrigin(), dest);
    dest[0] -= 15.0;
    dest[1] -= 15.0;
    trace = SVG_Trace(inflictor->GetOrigin(), vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;


    return false;
}


/*
============
Killed
============
*/
void Killed(SVGBaseEntity *targ, SVGBaseEntity *inflictor, SVGBaseEntity *attacker, int damage, vec3_t point)
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
        // doors, triggers, etc
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


/*
============
SVG_Damage

targ        entity that is being damaged
inflictor   entity that is causing the damage
attacker    entity that caused the inflictor to damage targ
    example: targ=monster, inflictor=rocket, attacker=player

dir         direction of the attack
point       point at which the damage is being inflicted
normal      normal vector from that point
damage      amount of damage being inflicted
knockback   force to be applied against targ as a result of the damage

dflags      these flags are used to control how SVG_Damage works
    DamageFlags::IndirectFromRadius           damage was indirect (from a nearby explosion)
    DamageFlags::NoArmorProtection         armor does not protect from this damage
    DamageFlags::EnergyBasedWeapon           damage is from an energy based weapon
    DamageFlags::NoKnockBack     do not affect velocity, just view angles
    DamageFlags::Bullet           damage is from a bullet (used for ricochets)
    DamageFlags::IgnoreProtection    kills godmode, armor, everything
============
*/
static int CheckPowerArmor(SVGBaseEntity *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
    GameClient   *client;
    int         save;
    int         power_armor_type;
    int         index;
    int         damagePerCell = 0;
    int         pa_te_type = 0;
    int         power;
    int         power_used;

    if (!damage)
        return 0;

    client = ent->GetClient();

    if (dflags & DamageFlags::NoArmorProtection)
        return 0;

    index = 0;  // shut up gcc

    if (ent->GetServerFlags() & EntityServerFlags::Monster) {
        //power_armor_type = ent->monsterInfo.power_armor_type;
        //power = ent->monsterInfo.power_armor_power;
    } else
        return 0;

    if (power_armor_type == POWER_ARMOR_NONE)
        return 0;
    if (!power)
        return 0;

    save = power * damagePerCell;
    if (!save)
        return 0;
    if (save > damage)
        save = damage;

    SpawnDamage(pa_te_type, point, normal, save);
    //ent->powerArmorTime = level.time + 0.2;

    power_used = save / damagePerCell;

    if (client)
        client->persistent.inventory[index] -= power_used;
    //else
    //    ent->monsterInfo.power_armor_power -= power_used;
    return save;
}

static int CheckArmor(SVGBaseEntity *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
    GameClient   *client;
    int         save;
    int         index;
    gitem_t     *armor;

    if (!damage)
        return 0;

    client = ent->GetClient();

    if (!client)
        return 0;

    if (dflags & DamageFlags::NoArmorProtection)
        return 0;

    return 0;
    index = SVG_ArmorIndex(ent);
    if (!index)
        return 0;

    armor = SVG_GetItemByIndex(index);

    if (dflags & DamageFlags::EnergyBasedWeapon)
        save = ceil(((gitem_armor_t *)armor->info)->energyProtection * damage);
    else
        save = ceil(((gitem_armor_t *)armor->info)->normalProtection * damage);
    if (save >= client->persistent.inventory[index])
        save = client->persistent.inventory[index];

    if (!save)
        return 0;

    client->persistent.inventory[index] -= save;
    SpawnDamage(te_sparks, point, normal, save);

    return save;
}

qboolean CheckTeamDamage(SVGBaseEntity *targ, SVGBaseEntity *attacker)
{
    //FIXME make the next line real and uncomment this block
    // if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
    return false;
}

void SVG_Damage(SVGBaseEntity *targ, SVGBaseEntity *inflictor, SVGBaseEntity *attacker, const vec3_t &dmgDir, const vec3_t &point, const vec3_t &normal, int damage, int knockback, int dflags, int mod)
{
    GameClient   *client;
    int         take;
    int         save;
    int         asave;
    int         psave;
    int         te_sparks;

    if (!targ || !inflictor || !attacker)
    {
        return;
    }

    if (!targ->GetTakeDamage())
        return;

    // friendly fire avoidance
    // if enabled you can't hurt teammates (but you can hurt yourself)
    // knockback still occurs
    if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DeathMatchFlags::ModelTeams | DeathMatchFlags::SkinTeams))) || coop->value)) {
        if (SVG_OnSameTeam(targ, attacker)) {
            if ((int)(dmflags->value) & DeathMatchFlags::NoFriendlyFire)
                damage = 0;
            else
                mod |= MeansOfDeath::FriendlyFire;
        }
    }
    meansOfDeath = mod;

    // easy mode takes half damage
    if (skill->value == 0 && deathmatch->value == 0 && targ->GetClient()) {
        damage *= 0.5;
        if (!damage)
            damage = 1;
    }

    client = targ->GetClient();

    if (dflags & DamageFlags::Bullet)
        te_sparks = TempEntityEvent::BulletSparks;
    else
        te_sparks = TempEntityEvent::Sparks;

    // Retrieve normalized direction.
    vec3_t dir = vec3_normalize(dmgDir);
    //VectorNormalize2(dmgDir, dir);


// bonus damage for suprising a monster
    if (!(dflags & DamageFlags::IndirectFromRadius) && (targ->GetServerFlags() & EntityServerFlags::Monster) && (attacker->GetClient()) && (!targ->GetEnemy()) && (targ->GetHealth() > 0))
        damage *= 2;

    if (targ->GetFlags() & EntityFlags::NoKnockBack)
        knockback = 0;

// figure momentum add
    if (!(dflags & DamageFlags::NoKnockBack)) {
        if ((knockback) && (targ->GetMoveType() != MoveType::None) && (targ->GetMoveType() != MoveType::Bounce) && (targ->GetMoveType() != MoveType::Push) && (targ->GetMoveType() != MoveType::Stop)) {
            vec3_t  kvel;
            float   mass;

            if (targ->GetMass() < 50)
                mass = 50;
            else
                mass = targ->GetMass();

            if (targ->GetClient() && attacker == targ)
                VectorScale(dir, 1600.0 * (float)knockback / mass, kvel);   // the rocket jump hack...
            else
                VectorScale(dir, 500.0 * (float)knockback / mass, kvel);

            targ->SetVelocity(targ->GetVelocity() + kvel);
        }
    }

    take = damage;
    save = 0;

    // check for godmode
    if ((targ->GetFlags() & EntityFlags::GodMode) && !(dflags & DamageFlags::IgnoreProtection)) {
        take = 0;
        save = damage;
        SpawnDamage(te_sparks, point, normal, save);
    }

    psave = CheckPowerArmor(targ, point, normal, take, dflags);
    take -= psave;

    asave = CheckArmor(targ, point, normal, take, te_sparks, dflags);
    take -= asave;

    //treat cheat/powerup savings the same as armor
    asave += save;

    // team damage avoidance
    if (!(dflags & DamageFlags::IgnoreProtection) && CheckTeamDamage(targ, attacker))
        return;

// do the damage
    if (take) {
        if ((targ->GetServerFlags() & EntityServerFlags::Monster) || (client))
        {
            // SpawnDamage(TempEntityEvent::Blood, point, normal, take);
            SpawnDamage(TempEntityEvent::Blood, point, dir, take);
        }
        else
            SpawnDamage(te_sparks, point, normal, take);


        targ->SetHealth(targ->GetHealth() - take);

        if (targ->GetHealth() <= 0) {
            if ((targ->GetServerFlags() & EntityServerFlags::Monster) || (client))
                targ->SetFlags(targ->GetFlags() | EntityFlags::NoKnockBack);
            Killed(targ, inflictor, attacker, take, point);
            return;
        }
    }

    //if (targ->serverFlags & EntityServerFlags::Monster) {
    //    M_ReactToDamage(targ, attacker);
    //    if (!(targ->monsterInfo.aiflags & AI_DUCKED) && (take)) {
    //        targ->Pain(targ, attacker, knockback, take);
    //        // nightmare mode monsters don't go into pain frames often
    //        if (skill->value == 3)
    //            targ->debouncePainTime = level.time + 5;
    //    }
    //} else 
    if (client) {
        //if (!(targ->flags & EntityFlags::GodMode) && (take))
        //    targ->Pain(targ, attacker, knockback, take);
        if (!(targ->GetFlags() & EntityFlags::GodMode) && (take)) {
            targ->TakeDamage(attacker, knockback, take);
        }
    } else if (take) {
        targ->TakeDamage(attacker, knockback, take);
    }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    if (client) {
        client->damages.powerArmor += psave;
        client->damages.armor += asave;
        client->damages.blood += take;
        client->damages.knockBack += knockback;
        VectorCopy(point, client->damages.from);
    }
}


/*
============
SVG_RadiusDamage
============
*/
void SVG_RadiusDamage(SVGBaseEntity *inflictor, SVGBaseEntity *attacker, float damage, SVGBaseEntity *ignore, float radius, int mod)
{
    float   points;
    SVGBaseEntity *ent = NULL;
    vec3_t  v;
    vec3_t  dir;

    // N&C: From Yamagi Q2, to prevent issues.
    if (!inflictor || !attacker)
    {
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
            if (SVG_CanDamage(ent, inflictor)) {
                // Calculate direcion.
                dir = ent->GetOrigin() - inflictor->GetOrigin();

                // Apply damages.
                SVG_Damage(ent, inflictor, attacker, dir, inflictor->GetOrigin(), vec3_zero(), (int)points, (int)points, DamageFlags::IndirectFromRadius, mod);
            }
        }
    }
}
