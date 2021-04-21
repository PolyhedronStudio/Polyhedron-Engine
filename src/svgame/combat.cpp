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
#include "utils.h"           // Include Utilities funcs.

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage(entity_t *targ, entity_t *inflictor)
{
    vec3_t  dest;
    trace_t trace;

// bmodels need special checking because their origin is 0,0,0
    if (targ->moveType == MOVETYPE_PUSH) {
        VectorAdd(targ->absMin, targ->absMax, dest);
        VectorScale(dest, 0.5, dest);
        trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
        if (trace.fraction == 1.0)
            return true;
        if (trace.ent == targ)
            return true;
        return false;
    }

    trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] += 15.0;
    dest[1] += 15.0;
    trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] += 15.0;
    dest[1] -= 15.0;
    trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] -= 15.0;
    dest[1] += 15.0;
    trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;

    VectorCopy(targ->s.origin, dest);
    dest[0] -= 15.0;
    dest[1] -= 15.0;
    trace = gi.Trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, CONTENTS_MASK_SOLID);
    if (trace.fraction == 1.0)
        return true;


    return false;
}


/*
============
Killed
============
*/
void Killed(entity_t *targ, entity_t *inflictor, entity_t *attacker, int damage, vec3_t point)
{
    if (targ->health < -999)
        targ->health = -999;

    targ->enemy = attacker;

    if ((targ->svFlags & SVF_MONSTER) && (targ->deadFlag != DEAD_DEAD)) {
//      targ->svFlags |= SVF_DEADMONSTER;   // now treat as a different content type
        if (!(targ->monsterInfo.aiflags & AI_GOOD_GUY)) {
            level.killed_monsters++;
            if (coop->value && attacker->client)
                attacker->client->resp.score++;
            // medics won't heal monsters that they kill themselves
            if (strcmp(attacker->classname, "monster_medic") == 0)
                targ->owner = attacker;
        }
    }

    if (targ->moveType == MOVETYPE_PUSH || targ->moveType == MOVETYPE_STOP || targ->moveType == MOVETYPE_NONE) {
        // doors, triggers, etc
        targ->Die(targ, inflictor, attacker, damage, point);
        return;
    }

    if ((targ->svFlags & SVF_MONSTER) && (targ->deadFlag != DEAD_DEAD)) {
        targ->Touch = NULL;
        monster_death_use(targ);
    }

    targ->Die(targ, inflictor, attacker, damage, point);
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
    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(type);
//  gi.WriteByte (damage);
    gi.WritePosition(origin);
    gi.WriteDirection(normal);
    gi.Multicast(&origin, MULTICAST_PVS);
}


/*
============
T_Damage

targ        entity that is being damaged
inflictor   entity that is causing the damage
attacker    entity that caused the inflictor to damage targ
    example: targ=monster, inflictor=rocket, attacker=player

dir         direction of the attack
point       point at which the damage is being inflicted
normal      normal vector from that point
damage      amount of damage being inflicted
knockback   force to be applied against targ as a result of the damage

dflags      these flags are used to control how T_Damage works
    DAMAGE_RADIUS           damage was indirect (from a nearby explosion)
    DAMAGE_NO_ARMOR         armor does not protect from this damage
    DAMAGE_ENERGY           damage is from an energy based weapon
    DAMAGE_NO_KNOCKBACK     do not affect velocity, just view angles
    DAMAGE_BULLET           damage is from a bullet (used for ricochets)
    DAMAGE_NO_PROTECTION    kills godmode, armor, everything
============
*/
static int CheckPowerArmor(entity_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
    gclient_t   *client;
    int         save;
    int         power_armor_type;
    int         index;
    int         damagePerCell;
    int         pa_te_type;
    int         power;
    int         power_used;

    if (!damage)
        return 0;

    client = ent->client;

    if (dflags & DAMAGE_NO_ARMOR)
        return 0;

    index = 0;  // shut up gcc

    if (ent->svFlags & SVF_MONSTER) {
        power_armor_type = ent->monsterInfo.power_armor_type;
        power = ent->monsterInfo.power_armor_power;
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
    ent->powerarmor_time = level.time + 0.2;

    power_used = save / damagePerCell;

    if (client)
        client->pers.inventory[index] -= power_used;
    else
        ent->monsterInfo.power_armor_power -= power_used;
    return save;
}

static int CheckArmor(entity_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
    gclient_t   *client;
    int         save;
    int         index;
    gitem_t     *armor;

    if (!damage)
        return 0;

    client = ent->client;

    if (!client)
        return 0;

    if (dflags & DAMAGE_NO_ARMOR)
        return 0;

    index = ArmorIndex(ent);
    if (!index)
        return 0;

    armor = GetItemByIndex(index);

    if (dflags & DAMAGE_ENERGY)
        save = ceil(((gitem_armor_t *)armor->info)->energy_protection * damage);
    else
        save = ceil(((gitem_armor_t *)armor->info)->normal_protection * damage);
    if (save >= client->pers.inventory[index])
        save = client->pers.inventory[index];

    if (!save)
        return 0;

    client->pers.inventory[index] -= save;
    SpawnDamage(te_sparks, point, normal, save);

    return save;
}

void M_ReactToDamage(entity_t *targ, entity_t *attacker)
{
    if (!(attacker->client) && !(attacker->svFlags & SVF_MONSTER))
        return;

    if (attacker == targ || attacker == targ->enemy)
        return;

    // dead monsters, like misc_deadsoldier, don't have AI functions, but 
    // M_ReactToDamage might still be called on them
    if (targ->svFlags & SVF_DEADMONSTER)
        return;

    // if we are a good guy monster and our attacker is a player
    // or another good guy, do not get mad at them
    if (targ->monsterInfo.aiflags & AI_GOOD_GUY) {
        if (attacker->client || (attacker->monsterInfo.aiflags & AI_GOOD_GUY))
            return;
    }

    // we now know that we are not both good guys

    // if attacker is a client, get mad at them because he's good and we're not
    if (attacker->client) {
        targ->monsterInfo.aiflags &= ~AI_SOUND_TARGET;

        // this can only happen in coop (both new and old enemies are clients)
        // only switch if can't see the current enemy
        if (targ->enemy && targ->enemy->client) {
            if (visible(targ, targ->enemy)) {
                targ->oldEnemyPtr = attacker;
                return;
            }
            targ->oldEnemyPtr = targ->enemy;
        }
        targ->enemy = attacker;
        if (!(targ->monsterInfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
        return;
    }

    // it's the same base (walk/swim/fly) type and a different classname and it's not a tank
    // (they spray too much), get mad at them
    if (((targ->flags & (FL_FLY | FL_SWIM)) == (attacker->flags & (FL_FLY | FL_SWIM))) &&
        (strcmp(targ->classname, attacker->classname) != 0) &&
        (strcmp(attacker->classname, "monster_tank") != 0) &&
        (strcmp(attacker->classname, "monster_supertank") != 0) &&
        (strcmp(attacker->classname, "monster_makron") != 0) &&
        (strcmp(attacker->classname, "monster_jorg") != 0)) {
        if (targ->enemy && targ->enemy->client)
            targ->oldEnemyPtr = targ->enemy;
        targ->enemy = attacker;
        if (!(targ->monsterInfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
    }
    // if they *meant* to shoot us, then shoot back
    else if (attacker->enemy == targ) {
        if (targ->enemy && targ->enemy->client)
            targ->oldEnemyPtr = targ->enemy;
        targ->enemy = attacker;
        if (!(targ->monsterInfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
    }
    // otherwise get mad at whoever they are mad at (help our buddy) unless it is us!
    else if (attacker->enemy && attacker->enemy != targ) {
        if (targ->enemy && targ->enemy->client)
            targ->oldEnemyPtr = targ->enemy;
        targ->enemy = attacker->enemy;
        if (!(targ->monsterInfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
    }
}

qboolean CheckTeamDamage(entity_t *targ, entity_t *attacker)
{
    //FIXME make the next line real and uncomment this block
    // if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
    return false;
}

void T_Damage(entity_t *targ, entity_t *inflictor, entity_t *attacker, const vec3_t &dmgDir, const vec3_t &point, const vec3_t &normal, int damage, int knockback, int dflags, int mod)
{
    gclient_t   *client;
    int         take;
    int         save;
    int         asave;
    int         psave;
    int         te_sparks;

    if (!targ || !inflictor || !attacker)
    {
        return;
    }

    if (!targ->takedamage)
        return;

    // friendly fire avoidance
    // if enabled you can't hurt teammates (but you can hurt yourself)
    // knockback still occurs
    if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DeathMatchFlags::ModelTeams | DeathMatchFlags::SkinTeams))) || coop->value)) {
        if (OnSameTeam(targ, attacker)) {
            if ((int)(dmflags->value) & DeathMatchFlags::NoFriendlyFire)
                damage = 0;
            else
                mod |= MOD_FRIENDLY_FIRE;
        }
    }
    meansOfDeath = mod;

    // easy mode takes half damage
    if (skill->value == 0 && deathmatch->value == 0 && targ->client) {
        damage *= 0.5;
        if (!damage)
            damage = 1;
    }

    client = targ->client;

    if (dflags & DAMAGE_BULLET)
        te_sparks = TempEntityEvent::BulletSparks;
    else
        te_sparks = TempEntityEvent::Sparks;

    // Retrieve normalized direction.
    vec3_t dir = vec3_normalize(dmgDir);
    //VectorNormalize2(dmgDir, dir);


// bonus damage for suprising a monster
    if (!(dflags & DAMAGE_RADIUS) && (targ->svFlags & SVF_MONSTER) && (attacker->client) && (!targ->enemy) && (targ->health > 0))
        damage *= 2;

    if (targ->flags & FL_NO_KNOCKBACK)
        knockback = 0;

// figure momentum add
    if (!(dflags & DAMAGE_NO_KNOCKBACK)) {
        if ((knockback) && (targ->moveType != MOVETYPE_NONE) && (targ->moveType != MOVETYPE_BOUNCE) && (targ->moveType != MOVETYPE_PUSH) && (targ->moveType != MOVETYPE_STOP)) {
            vec3_t  kvel;
            float   mass;

            if (targ->mass < 50)
                mass = 50;
            else
                mass = targ->mass;

            if (targ->client  && attacker == targ)
                VectorScale(dir, 1600.0 * (float)knockback / mass, kvel);   // the rocket jump hack...
            else
                VectorScale(dir, 500.0 * (float)knockback / mass, kvel);

            VectorAdd(targ->velocity, kvel, targ->velocity);
        }
    }

    take = damage;
    save = 0;

    // check for godmode
    if ((targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION)) {
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
    if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(targ, attacker))
        return;

// do the damage
    if (take) {
        if ((targ->svFlags & SVF_MONSTER) || (client))
        {
            // SpawnDamage(TempEntityEvent::Blood, point, normal, take);
            SpawnDamage(TempEntityEvent::Blood, point, dir, take);
        }
        else
            SpawnDamage(te_sparks, point, normal, take);


        targ->health = targ->health - take;

        if (targ->health <= 0) {
            if ((targ->svFlags & SVF_MONSTER) || (client))
                targ->flags |= FL_NO_KNOCKBACK;
            Killed(targ, inflictor, attacker, take, point);
            return;
        }
    }

    if (targ->svFlags & SVF_MONSTER) {
        M_ReactToDamage(targ, attacker);
        if (!(targ->monsterInfo.aiflags & AI_DUCKED) && (take)) {
            targ->Pain(targ, attacker, knockback, take);
            // nightmare mode monsters don't go into pain frames often
            if (skill->value == 3)
                targ->debouncePainTime = level.time + 5;
        }
    } else if (client) {
        if (!(targ->flags & FL_GODMODE) && (take))
            targ->Pain(targ, attacker, knockback, take);
    } else if (take) {
        if (targ->Pain)
            targ->Pain(targ, attacker, knockback, take);
    }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    if (client) {
        client->damage_parmor += psave;
        client->damage_armor += asave;
        client->damage_blood += take;
        client->damage_knockback += knockback;
        VectorCopy(point, client->damage_from);
    }
}


/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage(entity_t *inflictor, entity_t *attacker, float damage, entity_t *ignore, float radius, int mod)
{
    float   points;
    entity_t *ent = NULL;
    vec3_t  v;
    vec3_t  dir;

    // N&C: From Yamagi Q2, to prevent issues.
    if (!inflictor || !attacker)
    {
        return;
    }

    // Find entities within radius.
    while ((ent = G_FindEntitiesWithinRadius(ent, inflictor->s.origin, radius)) != NULL) {
        // Continue in case this entity has to be ignored from applying damage.
        if (ent == ignore)
            continue;
        // Continue in case this entity CAN'T take any damage.
        if (!ent->takedamage)
            continue;

        // Calculate damage points.
        v = ent->mins + ent->maxs, v;
        v = vec3_fmaf(ent->s.origin, 0.5, v);
        v -= inflictor->s.origin;
        points = damage - 0.5 * vec3_length(v);

        // In case the attacker is the own entity, half damage.
        if (ent == attacker)
            points = points * 0.5;

        // Apply damage points.
        if (points > 0) {
            // Ensure whether we CAN actually apply damage.
            if (CanDamage(ent, inflictor)) {
                // Calculate direcion.
                dir = ent->s.origin - inflictor->s.origin;

                // Apply damages.
                T_Damage(ent, inflictor, attacker, dir, inflictor->s.origin, vec3_zero(), (int)points, (int)points, DAMAGE_RADIUS, mod);
            }
        }
    }
}
