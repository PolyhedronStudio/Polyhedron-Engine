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
