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
// g_turret.c

#include "g_local.h"         // Include SVGame funcs.
#include "utils.h"           // Include Utilities funcs.

static void AnglesWithinZeroAnd360(vec3_t &vec)
{
    while (vec[0] > 360)
        vec[0] -= 360;
    while (vec[0] < 0)
        vec[0] += 360;
    while (vec[1] > 360)
        vec[1] -= 360;
    while (vec[1] < 0)
        vec[1] += 360;
}

float SnapToEights(float x)
{
    //x *= 8.0;
    //if (x > 0.0)
    //    x += 0.5;
    //else
    //    x -= 0.5;
    //return 0.125 * (int)x;
    return x;
}


void turret_blocked(entity_t *self, entity_t *other)
{
    entity_t *attacker;

    if (other->takeDamage) {
        if (self->teamMasterPtr->owner)
            attacker = self->teamMasterPtr->owner;
        else
            attacker = self->teamMasterPtr;
        T_Damage(other, self, attacker, vec3_zero(), other->s.origin, vec3_zero(), self->teamMasterPtr->dmg, 10, 0, MOD_CRUSH);
    }
}

/*QUAKED turret_breach (0 0 0) ?
This portion of the turret can change both pitch and yaw.
The model  should be made with a flat pitch.
It (and the associated base) need to be oriented towards 0.
Use "angle" to set the starting angle.

"speed"     default 50
"dmg"       default 10
"angle"     point this forward
"target"    point this at an info_notnull at the muzzle tip
"minpitch"  min acceptable pitch angle : default -30
"maxpitch"  max acceptable pitch angle : default 30
"minyaw"    min acceptable yaw angle   : default 0
"maxyaw"    max acceptable yaw angle   : default 360
*/

void turret_breach_fire(entity_t *self)
{
    vec3_t  f, r, u;
    vec3_t  start;
    int     damage;
    int     speed;

    vec3_vectors(self->s.angles, &f, &r, &u);
    VectorMA(self->s.origin, self->moveOrigin[0], f, start);
    VectorMA(start, self->moveOrigin[1], r, start);
    VectorMA(start, self->moveOrigin[2], u, start);

    damage = 100 + random() * 50;
    speed = 550 + 50 * skill->value;
    Fire_Rocket(self->teamMasterPtr->owner, start, f, damage, speed, 150, damage);
    gi.PositionedSound(start, self, CHAN_WEAPON, gi.SoundIndex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
}

void turret_breach_think(entity_t *self)
{
    entity_t *ent;
    vec3_t  current_angles;
    vec3_t  delta;

    VectorCopy(self->s.angles, current_angles);
    AnglesWithinZeroAnd360(current_angles);

    AnglesWithinZeroAnd360(self->moveAngles);
    if (self->moveAngles[vec3_t::Pitch] > 180)
        self->moveAngles[vec3_t::Pitch] -= 360;

    // clamp angles to mins & maxs
    if (self->moveAngles[vec3_t::Pitch] > self->pos1[vec3_t::Pitch])
        self->moveAngles[vec3_t::Pitch] = self->pos1[vec3_t::Pitch];
    else if (self->moveAngles[vec3_t::Pitch] < self->pos2[vec3_t::Pitch])
        self->moveAngles[vec3_t::Pitch] = self->pos2[vec3_t::Pitch];

    if ((self->moveAngles[vec3_t::Yaw] < self->pos1[vec3_t::Yaw]) || (self->moveAngles[vec3_t::Yaw] > self->pos2[vec3_t::Yaw])) {
        float   dmin, dmax;

        dmin = fabs(self->pos1[vec3_t::Yaw] - self->moveAngles[vec3_t::Yaw]);
        if (dmin < -180)
            dmin += 360;
        else if (dmin > 180)
            dmin -= 360;
        dmax = fabs(self->pos2[vec3_t::Yaw] - self->moveAngles[vec3_t::Yaw]);
        if (dmax < -180)
            dmax += 360;
        else if (dmax > 180)
            dmax -= 360;
        if (fabs(dmin) < fabs(dmax))
            self->moveAngles[vec3_t::Yaw] = self->pos1[vec3_t::Yaw];
        else
            self->moveAngles[vec3_t::Yaw] = self->pos2[vec3_t::Yaw];
    }

    VectorSubtract(self->moveAngles, current_angles, delta);
    if (delta[0] < -180)
        delta[0] += 360;
    else if (delta[0] > 180)
        delta[0] -= 360;
    if (delta[1] < -180)
        delta[1] += 360;
    else if (delta[1] > 180)
        delta[1] -= 360;
    delta[2] = 0;

    if (delta[0] > self->speed * FRAMETIME)
        delta[0] = self->speed * FRAMETIME;
    if (delta[0] < -1 * self->speed * FRAMETIME)
        delta[0] = -1 * self->speed * FRAMETIME;
    if (delta[1] > self->speed * FRAMETIME)
        delta[1] = self->speed * FRAMETIME;
    if (delta[1] < -1 * self->speed * FRAMETIME)
        delta[1] = -1 * self->speed * FRAMETIME;

    VectorScale(delta, 1.0 / FRAMETIME, self->avelocity);

    self->nextThink = level.time + FRAMETIME;

    for (ent = self->teamMasterPtr; ent; ent = ent->teamChainPtr)
        ent->avelocity[1] = self->avelocity[1];

    // if we have adriver, adjust his velocities
    if (self->owner) {
        float   angle;
        float   target_z;
        float   diff;
        vec3_t  target;
        vec3_t  dir;

        // angular is easy, just copy ours
        self->owner->avelocity[0] = self->avelocity[0];
        self->owner->avelocity[1] = self->avelocity[1];

        // x & y
        angle = self->s.angles[1] + self->owner->moveOrigin[1];
        angle *= (M_PI * 2 / 360);
        // N&C: FF Precision.
        target[0] = (self->s.origin[0] + std::cosf(angle) * self->owner->moveOrigin[0]); //SnapToEights(self->s.origin[0] + cos(angle) * self->owner->moveOrigin[0]);
        target[1] = (self->s.origin[1] + std::sinf(angle) * self->owner->moveOrigin[0]); // SnapToEights(self->s.origin[1] + sin(angle) * self->owner->moveOrigin[0]);
        target[2] = self->owner->s.origin[2];

        VectorSubtract(target, self->owner->s.origin, dir);
        self->owner->velocity[0] = dir[0] * 1.0 / FRAMETIME;
        self->owner->velocity[1] = dir[1] * 1.0 / FRAMETIME;

        // z
        angle = self->s.angles[vec3_t::Pitch] * (M_PI * 2 / 360);
        target_z = SnapToEights(self->s.origin[2] + self->owner->moveOrigin[0] * tan(angle) + self->owner->moveOrigin[2]);

        diff = target_z - self->owner->s.origin[2];
        self->owner->velocity[2] = diff * 1.0 / FRAMETIME;

        if (self->spawnFlags & 65536) {
            turret_breach_fire(self);
            self->spawnFlags &= ~65536;
        }
    }
}

void turret_breach_finish_init(entity_t *self)
{
    // get and save info for muzzle location
    if (!self->target) {
        gi.DPrintf("%s at %s needs a target\n", self->classname, Vec3ToString(self->s.origin));
    } else {
        self->targetEntityPtr = G_PickTarget(self->target);
        VectorSubtract(self->targetEntityPtr->s.origin, self->s.origin, self->moveOrigin);
        G_FreeEntity(self->targetEntityPtr);
    }

    self->teamMasterPtr->dmg = self->dmg;
    self->Think = turret_breach_think;
    self->Think(self);
}

void SP_turret_breach(entity_t *self)
{
    self->solid = SOLID_BSP;
    self->moveType = MOVETYPE_PUSH;
    gi.SetModel(self, self->model);

    if (!self->speed)
        self->speed = 50;
    if (!self->dmg)
        self->dmg = 10;

    if (!st.minpitch)
        st.minpitch = -30;
    if (!st.maxpitch)
        st.maxpitch = 30;
    if (!st.maxyaw)
        st.maxyaw = 360;

    self->pos1[vec3_t::Pitch] = -1 * st.minpitch;
    self->pos1[vec3_t::Yaw]   = st.minyaw;
    self->pos2[vec3_t::Pitch] = -1 * st.maxpitch;
    self->pos2[vec3_t::Yaw]   = st.maxyaw;

    self->idealYaw = self->s.angles[vec3_t::Yaw];
    self->moveAngles[vec3_t::Yaw] = self->idealYaw;

    self->Blocked = turret_blocked;

    self->Think = turret_breach_finish_init;
    self->nextThink = level.time + FRAMETIME;
    gi.LinkEntity(self);
}


/*QUAKED turret_base (0 0 0) ?
This portion of the turret changes yaw only.
MUST be teamed with a turret_breach.
*/

void SP_turret_base(entity_t *self)
{
    self->solid = SOLID_BSP;
    self->moveType = MOVETYPE_PUSH;
    gi.SetModel(self, self->model);
    self->Blocked = turret_blocked;
    gi.LinkEntity(self);
}


/*QUAKED turret_driver (1 .5 0) (-16 -16 -24) (16 16 32)
Must NOT be on the team with the rest of the turret parts.
Instead it must target the turret_breach.
*/

//void infantry_die(entity_t *self, entity_t *inflictor, entity_t *attacker, int damage, const vec3_t& point);
//void infantry_stand(entity_t *self);
void monster_use(entity_t *self, entity_t *other, entity_t *activator);

void turret_driver_die(entity_t *self, entity_t *inflictor, entity_t *attacker, int damage, const vec3_t& point)
{
    entity_t *ent;

    // level the gun
    self->targetEntityPtr->moveAngles[0] = 0;

    // remove the driver from the end of them team chain
    for (ent = self->targetEntityPtr->teamMasterPtr; ent->teamChainPtr != self; ent = ent->teamChainPtr)
        ;
    ent->teamChainPtr = NULL;
    self->teamMasterPtr = NULL;
    self->flags &= ~FL_TEAMSLAVE;

    self->targetEntityPtr->owner = NULL;
    self->targetEntityPtr->teamMasterPtr->owner = NULL;

    //infantry_die(self, inflictor, attacker, damage, point);
}

qboolean AI_FindTarget(entity_t *self);

void turret_driver_think(entity_t *self)
{
    vec3_t  target;
    vec3_t  dir;
    float   reaction_time;

    self->nextThink = level.time + FRAMETIME;

    if (self->enemy && (!self->enemy->inUse || self->enemy->health <= 0))
        self->enemy = NULL;

    if (!self->enemy) {
        if (!AI_FindTarget(self))
            return;
        self->monsterInfo.trail_time = level.time;
        self->monsterInfo.aiflags &= ~AI_LOST_SIGHT;
    } else {
        if (AI_IsEntityVisibleToSelf(self, self->enemy)) {
            if (self->monsterInfo.aiflags & AI_LOST_SIGHT) {
                self->monsterInfo.trail_time = level.time;
                self->monsterInfo.aiflags &= ~AI_LOST_SIGHT;
            }
        } else {
            self->monsterInfo.aiflags |= AI_LOST_SIGHT;
            return;
        }
    }

    // let the turret know where we want it to aim
    VectorCopy(self->enemy->s.origin, target);
    target[2] += self->enemy->viewHeight;
    VectorSubtract(target, self->targetEntityPtr->s.origin, dir);
    vectoangles(dir, self->targetEntityPtr->moveAngles);

    // decide if we should shoot
    if (level.time < self->monsterInfo.attack_finished)
        return;

    reaction_time = (3 - skill->value) * 1.0;
    if ((level.time - self->monsterInfo.trail_time) < reaction_time)
        return;

    self->monsterInfo.attack_finished = level.time + reaction_time + 1.0;
    //FIXME how do we really want to pass this along?
    self->targetEntityPtr->spawnFlags |= 65536;
}

void turret_driver_link(entity_t *self)
{
    vec3_t  vec;
    entity_t *ent;

    self->Think = turret_driver_think;
    self->nextThink = level.time + FRAMETIME;

    self->targetEntityPtr = G_PickTarget(self->target);
    self->targetEntityPtr->owner = self;
    self->targetEntityPtr->teamMasterPtr->owner = self;
    VectorCopy(self->targetEntityPtr->s.angles, self->s.angles);

    vec[0] = self->targetEntityPtr->s.origin[0] - self->s.origin[0];
    vec[1] = self->targetEntityPtr->s.origin[1] - self->s.origin[1];
    vec[2] = 0;
    self->moveOrigin[0] = VectorLength(vec);

    VectorSubtract(self->s.origin, self->targetEntityPtr->s.origin, vec);
    vectoangles(vec, vec);
    AnglesWithinZeroAnd360(vec);
    self->moveOrigin[1] = vec[1];

    self->moveOrigin[2] = self->s.origin[2] - self->targetEntityPtr->s.origin[2];

    // add the driver to the end of them team chain
    for (ent = self->targetEntityPtr->teamMasterPtr; ent->teamChainPtr; ent = ent->teamChainPtr)
        ;
    ent->teamChainPtr = self;
    self->teamMasterPtr = self->targetEntityPtr->teamMasterPtr;
    self->flags |= FL_TEAMSLAVE;
}

void SP_turret_driver(entity_t *self)
{
    if (deathmatch->value) {
        G_FreeEntity(self);
        return;
    }

    self->moveType = MOVETYPE_PUSH;
    self->solid = SOLID_BBOX;
    self->s.modelindex = gi.ModelIndex("models/monsters/infantry/tris.md2");
    VectorSet(self->mins, -16, -16, -24);
    VectorSet(self->maxs, 16, 16, 32);

    self->health = 100;
    self->gibHealth = 0;
    self->mass = 200;
    self->viewHeight = 24;

    self->Die = turret_driver_die;
    //self->monsterInfo.stand = infantry_stand;

    self->flags |= FL_NO_KNOCKBACK;

    level.total_monsters++;

    self->svFlags |= SVF_MONSTER;
    self->s.renderfx |= RF_FRAMELERP;
    self->takeDamage = DAMAGE_AIM;
    self->Use = monster_use;
    self->clipMask = CONTENTS_MASK_MONSTERSOLID;
    VectorCopy(self->s.origin, self->s.old_origin);
    self->monsterInfo.aiflags |= AI_Stand_GROUND | AI_DUCKED;

    if (st.item) {
        self->item = FindItemByClassname(st.item);
        if (!self->item)
            gi.DPrintf("%s at %s has bad item: %s\n", self->classname, Vec3ToString(self->s.origin), st.item);
    }

    self->Think = turret_driver_link;
    self->nextThink = level.time + FRAMETIME;

    gi.LinkEntity(self);
}
