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

    // Determine whether it is a monster, and if it IS set to being dead....
    if ((targ->GetServerFlags() & EntityServerFlags::Monster) && (targ->GetDeadFlag() != DEAD_DEAD)) {
        targ->SetServerFlags(targ->GetServerFlags() | EntityServerFlags::DeadMonster);   // Now treat as a different content type

//        if (!(targ->monsterInfo.aiflags & AI_GOOD_GUY)) {
//            level.killedMonsters++;
//            if (coop->value && attacker->client)
//                attacker->client->respawn.score++;
//            // medics won't heal monsters that they kill themselves
//            if (strcmp(attacker->className, "monster_medic") == 0)
//                targ->owner = attacker;
//        }
    }

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
    game.gameMode->InflictDamage(targ, inflictor, attacker, dmgDir, point, normal, damage, knockBack, dflags, mod);
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
    game.gameMode->InflictRadiusDamage(inflictor, attacker, damage, ignore, radius, mod);
}
