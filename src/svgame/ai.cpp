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
// g_ai.c

#include "g_local.h"         // Include SVGame funcs.
#include "utils.h"           // Include Utilities funcs.

qboolean FindTarget(entity_t *self);
extern cvar_t   *maxClients;

qboolean ai_checkattack(entity_t *self, float dist);

qboolean    enemy_vis;
int         enemy_range;
float       enemy_yaw;

//============================================================================


/*
=================
AI_SetSightClient

Called once each frame to set level.sight_client to the
player to be checked for in findtarget.

If all clients are either dead or in notarget, sight_client
will be null.

In coop games, sight_client will cycle between the clients.
=================
*/
void AI_SetSightClient(void)
{
    entity_t *ent;
    int     start, check;

    if (level.sight_client == NULL)
        start = 1;
    else
        start = level.sight_client - g_edicts;

    check = start;
    while (1) {
        check++;
        if (check > game.maxClients)
            check = 1;
        ent = &g_edicts[check];
        if (ent->inUse
            && ent->health > 0
            && !(ent->flags & EntityFlags::NoTarget)) {
            level.sight_client = ent;
            return;     // got one
        }
        if (check == start) {
            level.sight_client = NULL;
            return;     // nobody to see
        }
    }
}

//============================================================================

/*
=============
ai_move

Move the specified distance at current facing.
This replaces the QC functions: ai_forward, ai_back, ai_pain, and ai_painforward
==============
*/
void ai_move(entity_t *self, float dist)
{
    M_walkmove(self, self->state.angles[vec3_t::Yaw], dist);
}


/*
=============
ai_stand

Used for standing around and looking for players
Distance is for slight position adjustments needed by the animations
==============
*/
void ai_stand(entity_t *self, float dist)
{
    vec3_t  v;

    if (dist)
        M_walkmove(self, self->state.angles[vec3_t::Yaw], dist);

    if (self->monsterInfo.aiflags & AI_STAND_GROUND) {
        if (self->enemy) {
            VectorSubtract(self->enemy->state.origin, self->state.origin, v);
            self->idealYaw = vectoyaw(v);
            if (self->state.angles[vec3_t::Yaw] != self->idealYaw && self->monsterInfo.aiflags & AI_TEMP_STAND_GROUND) {
                self->monsterInfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
                self->monsterInfo.run(self);
            }
            M_ChangeYaw(self);
            ai_checkattack(self, 0);
        } else
            FindTarget(self);
        return;
    }

    if (FindTarget(self))
        return;

    if (level.time > self->monsterInfo.pausetime) {
        self->monsterInfo.walk(self);
        return;
    }

    if (!(self->spawnFlags & 1) && (self->monsterInfo.idle) && (level.time > self->monsterInfo.idle_time)) {
        if (self->monsterInfo.idle_time) {
            self->monsterInfo.idle(self);
            self->monsterInfo.idle_time = level.time + 15 + random() * 15;
        } else {
            self->monsterInfo.idle_time = level.time + random() * 15;
        }
    }
}


/*
=============
ai_walk

The monster is walking it's beat
=============
*/
void ai_walk(entity_t *self, float dist)
{
    M_MoveToGoal(self, dist);

    // check for noticing a player
    if (FindTarget(self))
        return;

    if ((self->monsterInfo.search) && (level.time > self->monsterInfo.idle_time)) {
        if (self->monsterInfo.idle_time) {
            self->monsterInfo.search(self);
            self->monsterInfo.idle_time = level.time + 15 + random() * 15;
        } else {
            self->monsterInfo.idle_time = level.time + random() * 15;
        }
    }
}


/*
=============
ai_charge

Turns towards target and advances
Use this call with a distnace of 0 to replace ai_face
==============
*/
void ai_charge(entity_t *self, float dist)
{
    vec3_t  v;

    VectorSubtract(self->enemy->state.origin, self->state.origin, v);
    self->idealYaw = vectoyaw(v);
    M_ChangeYaw(self);

    if (dist)
        M_walkmove(self, self->state.angles[vec3_t::Yaw], dist);
}


/*
=============
ai_turn

don't move, but turn towards ideal_yaw
Distance is for slight position adjustments needed by the animations
=============
*/
void ai_turn(entity_t *self, float dist)
{
    if (dist)
        M_walkmove(self, self->state.angles[vec3_t::Yaw], dist);

    if (FindTarget(self))
        return;

    M_ChangeYaw(self);
}


/*

.enemy
Will be world if not currently angry at anyone.

.movetarget
The next path spot to walk toward.  If .enemy, ignore .movetarget.
When an enemy is killed, the monster will try to return to it's path.

.hunt_time
Set to time + something when the player is in sight, but movement straight for
him is Blocked.  This causes the monster to use wall following code for
movement direction instead of sighting on the player.

.ideal_yaw
A yaw angle of the intended direction, which will be turned towards at up
to 45 deg / state.  If the enemy is in view and hunt_time is not active,
this will be the exact line towards the enemy.

.pausetime
A monster will leave it's stand state and head towards it's .movetarget when
time > .pausetime.

walkmove(angle, speed) primitive is all or nothing
*/

/*
=============
range

returns the range catagorization of an entity reletive to self
0   melee range, will become hostile even if back is turned
1   visibility and infront, or visibility and show hostile
2   infront and show hostile
3   only triggered by damage
=============
*/
int range(entity_t *self, entity_t *other)
{
    vec3_t  v;
    float   len;

    VectorSubtract(self->state.origin, other->state.origin, v);
    len = VectorLength(v);
    if (len < MELEE_DISTANCE)
        return RANGE_MELEE;
    if (len < 500)
        return RANGE_NEAR;
    if (len < 1000)
        return RANGE_MID;
    return RANGE_FAR;
}

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean visible(entity_t *self, entity_t *other)
{
    vec3_t  spot1;
    vec3_t  spot2;
    trace_t trace;

    VectorCopy(self->state.origin, spot1);
    spot1[2] += self->viewHeight;
    VectorCopy(other->state.origin, spot2);
    spot2[2] += other->viewHeight;
    trace = gi.Trace(spot1, vec3_origin, vec3_origin, spot2, self, CONTENTS_MASK_OPAQUE);

    if (trace.fraction == 1.0)
        return true;
    return false;
}


/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean infront(entity_t *self, entity_t *other)
{
    vec3_t  vec;
    float   dot;
    vec3_t  forward;

    AngleVectors(self->state.angles, &forward, NULL, NULL);
    VectorSubtract(other->state.origin, self->state.origin, vec);
    VectorNormalize(vec);
    dot = DotProduct(vec, forward);

    if (dot > 0.3)
        return true;
    return false;
}


//============================================================================

void HuntTarget(entity_t *self)
{
    vec3_t  vec;

    self->goalEntityPtr = self->enemy;
    if (self->monsterInfo.aiflags & AI_STAND_GROUND)
        self->monsterInfo.stand(self);
    else
        self->monsterInfo.run(self);
    VectorSubtract(self->enemy->state.origin, self->state.origin, vec);
    self->idealYaw = vectoyaw(vec);
    // wait a while before first attack
    if (!(self->monsterInfo.aiflags & AI_STAND_GROUND))
        AttackFinished(self, 1);
}

void FoundTarget(entity_t *self)
{
    // let other monsters see this monster for a while
    if (self->enemy->client) {
        level.sight_entity = self;
        level.sight_entity_framenum = level.frameNumber;
        level.sight_entity->lightLevel = 128;
    }

    self->showHostile = level.time + 1;        // wake up other monsters

    VectorCopy(self->enemy->state.origin, self->monsterInfo.last_sighting);
    self->monsterInfo.trail_time = level.time;

    if (!self->combatTarget) {
        HuntTarget(self);
        return;
    }

    self->goalEntityPtr = self->moveTargetPtr = G_PickTarget(self->combatTarget);
    if (!self->moveTargetPtr) {
        self->goalEntityPtr = self->moveTargetPtr = self->enemy;
        HuntTarget(self);
        gi.DPrintf("%s at %s, combatTarget %s not found\n", self->classname, vec3_to_str(self->state.origin), self->combatTarget);
        return;
    }

    // clear out our combatTarget, these are a one shot deal
    self->combatTarget = NULL;
    self->monsterInfo.aiflags |= AI_COMBAT_POINT;

    // clear the targetName, that point is ours!
    self->moveTargetPtr->targetName = NULL;
    self->monsterInfo.pausetime = 0;

    // run for it
    self->monsterInfo.run(self);
}


/*
===========
FindTarget

Self is currently not attacking anything, so try to find a target

Returns TRUE if an enemy was sighted

When a player fires a missile, the point of impact becomes a fakeplayer so
that monsters that see the impact will respond as if they had seen the
player.

To avoid spending too much time, only a single client (or fakeclient) is
checked each frame.  This means multi player games will have slightly
slower noticing monsters.
============
*/
qboolean FindTarget(entity_t *self)
{
    entity_t     *client;
    qboolean    heardit;
    int         r;

    if (self->monsterInfo.aiflags & AI_GOOD_GUY) {
        if (self->goalEntityPtr && self->goalEntityPtr->inUse && self->goalEntityPtr->classname) {
            if (strcmp(self->goalEntityPtr->classname, "target_actor") == 0)
                return false;
        }

        //FIXME look for monsters?
        return false;
    }

    // if we're going to a combat point, just proceed
    if (self->monsterInfo.aiflags & AI_COMBAT_POINT)
        return false;

// if the first spawnflag bit is set, the monster will only wake up on
// really seeing the player, not another monster getting angry or hearing
// something

// revised behavior so they will wake up if they "see" a player make a noise
// but not weapon impact/explosion noises

    heardit = false;
    if ((level.sight_entity_framenum >= (level.frameNumber - 1)) && !(self->spawnFlags & 1)) {
        client = level.sight_entity;
        if (client->enemy == self->enemy) {
            return false;
        }
    } else if (level.sound_entity_framenum >= (level.frameNumber - 1)) {
        client = level.sound_entity;
        heardit = true;
    } else if (!(self->enemy) && (level.sound2_entity_framenum >= (level.frameNumber - 1)) && !(self->spawnFlags & 1)) {
        client = level.sound2_entity;
        heardit = true;
    } else {
        client = level.sight_client;
        if (!client)
            return false;   // no clients to get mad at
    }

    // if the entity went away, forget it
    if (!client->inUse)
        return false;

    if (client == self->enemy)
        return true;    // JDC false;

    if (client->client) {
        if (client->flags & EntityFlags::NoTarget)
            return false;
    } else if (client->serverFlags & EntityServerFlags::Monster) {
        if (!client->enemy)
            return false;
        if (client->enemy->flags & EntityFlags::NoTarget)
            return false;
    } else if (heardit) {
        if (client->owner->flags & EntityFlags::NoTarget)
            return false;
    } else
        return false;

    if (!heardit) {
        r = range(self, client);

        if (r == RANGE_FAR)
            return false;

// this is where we would check invisibility

        // is client in an spot too dark to be seen?
        if (client->lightLevel <= 5)
            return false;

        if (!visible(self, client)) {
            return false;
        }

        if (r == RANGE_NEAR) {
            if (client->showHostile < level.time && !infront(self, client)) {
                return false;
            }
        } else if (r == RANGE_MID) {
            if (!infront(self, client)) {
                return false;
            }
        }

        self->enemy = client;

        if (strcmp(self->enemy->classname, "player_noise") != 0) {
            self->monsterInfo.aiflags &= ~AI_SOUND_TARGET;

            if (!self->enemy->client) {
                self->enemy = self->enemy->enemy;
                if (!self->enemy->client) {
                    self->enemy = NULL;
                    return false;
                }
            }
        }
    } else { // heardit
        vec3_t  temp;

        if (self->spawnFlags & 1) {
            if (!visible(self, client))
                return false;
        } else {
            if (!gi.InPHS(self->state.origin, client->state.origin))
                return false;
        }

        VectorSubtract(client->state.origin, self->state.origin, temp);

        if (VectorLength(temp) > 1000) { // too far to hear
            return false;
        }

        // check area portals - if they are different and not connected then we can't hear it
        if (client->areaNumber != self->areaNumber)
            if (!gi.AreasConnected(self->areaNumber, client->areaNumber))
                return false;

        self->idealYaw = vectoyaw(temp);
        M_ChangeYaw(self);

        // hunt the sound for a bit; hopefully find the real player
        self->monsterInfo.aiflags |= AI_SOUND_TARGET;
        self->enemy = client;
    }

//
// got one
//
    FoundTarget(self);

    if (!(self->monsterInfo.aiflags & AI_SOUND_TARGET) && (self->monsterInfo.sight))
        self->monsterInfo.sight(self, self->enemy);

    return true;
}


//=============================================================================

/*
============
FacingIdeal

============
*/
qboolean FacingIdeal(entity_t *self)
{
    float   delta;

    delta = anglemod(self->state.angles[vec3_t::Yaw] - self->idealYaw);
    if (delta > 45 && delta < 315)
        return false;
    return true;
}


//=============================================================================

qboolean M_CheckAttack(entity_t *self)
{
    vec3_t  spot1, spot2;
    float   chance;
    trace_t tr;

    if (self->enemy->health > 0) {
        // see if any entities are in the way of the shot
        VectorCopy(self->state.origin, spot1);
        spot1[2] += self->viewHeight;
        VectorCopy(self->enemy->state.origin, spot2);
        spot2[2] += self->enemy->viewHeight;

        tr = gi.Trace(spot1, vec3_origin, vec3_origin, spot2, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_WINDOW);

        // do we have a clear shot?
        if (tr.ent != self->enemy)
            return false;
    }

    // melee attack
    if (enemy_range == RANGE_MELEE) {
        // don't always melee in easy mode
        if (skill->value == 0 && (rand() & 3))
            return false;
        if (self->monsterInfo.melee)
            self->monsterInfo.attack_state = AS_MELEE;
        else
            self->monsterInfo.attack_state = AS_MISSILE;
        return true;
    }

// missile attack
    if (!self->monsterInfo.attack)
        return false;

    if (level.time < self->monsterInfo.attack_finished)
        return false;

    if (enemy_range == RANGE_FAR)
        return false;

    if (self->monsterInfo.aiflags & AI_STAND_GROUND) {
        chance = 0.4;
    } else if (enemy_range == RANGE_MELEE) {
        chance = 0.2;
    } else if (enemy_range == RANGE_NEAR) {
        chance = 0.1;
    } else if (enemy_range == RANGE_MID) {
        chance = 0.02;
    } else {
        return false;
    }

    if (skill->value == 0)
        chance *= 0.5;
    else if (skill->value >= 2)
        chance *= 2;

    if (random() < chance) {
        self->monsterInfo.attack_state = AS_MISSILE;
        self->monsterInfo.attack_finished = level.time + 2 * random();
        return true;
    }

    if (self->flags & EntityFlags::Fly) {
        if (random() < 0.3)
            self->monsterInfo.attack_state = AS_SLIDING;
        else
            self->monsterInfo.attack_state = AS_STRAIGHT;
    }

    return false;
}


/*
=============
ai_run_melee

Turn and close until within an angle to launch a melee attack
=============
*/
void ai_run_melee(entity_t *self)
{
    self->idealYaw = enemy_yaw;
    M_ChangeYaw(self);

    if (FacingIdeal(self)) {
        self->monsterInfo.melee(self);
        self->monsterInfo.attack_state = AS_STRAIGHT;
    }
}


/*
=============
ai_run_missile

Turn in place until within an angle to launch a missile attack
=============
*/
void ai_run_missile(entity_t *self)
{
    self->idealYaw = enemy_yaw;
    M_ChangeYaw(self);

    if (FacingIdeal(self)) {
        self->monsterInfo.attack(self);
        self->monsterInfo.attack_state = AS_STRAIGHT;
    }
}


/*
=============
ai_run_slide

Strafe sideways, but stay at aproximately the same range
=============
*/
void ai_run_slide(entity_t *self, float distance)
{
    float   ofs;

    self->idealYaw = enemy_yaw;
    M_ChangeYaw(self);

    if (self->monsterInfo.lefty)
        ofs = 90;
    else
        ofs = -90;

    if (M_walkmove(self, self->idealYaw + ofs, distance))
        return;

    self->monsterInfo.lefty = 1 - self->monsterInfo.lefty;
    M_walkmove(self, self->idealYaw - ofs, distance);
}


/*
=============
ai_checkattack

Decides if we're going to attack or do something else
used by ai_run and ai_stand
=============
*/
qboolean ai_checkattack(entity_t *self, float dist)
{
    vec3_t      temp;
    qboolean    hesDeadJim;

// this causes monsters to run blindly to the combat point w/o firing
    if (self->goalEntityPtr) {
        if (self->monsterInfo.aiflags & AI_COMBAT_POINT)
            return false;

        if (self->monsterInfo.aiflags & AI_SOUND_TARGET) {
            if ((level.time - self->enemy->teleportTime) > 5.0) {
                if (self->goalEntityPtr == self->enemy) {
                    if (self->moveTargetPtr)
                        self->goalEntityPtr = self->moveTargetPtr;
                    else
                        self->goalEntityPtr = NULL;
                }
                self->monsterInfo.aiflags &= ~AI_SOUND_TARGET;
                if (self->monsterInfo.aiflags & AI_TEMP_STAND_GROUND)
                    self->monsterInfo.aiflags &= ~(AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
            } else {
                self->showHostile = level.time + 1;
                return false;
            }
        }
    }

    enemy_vis = false;

// see if the enemy is dead
    hesDeadJim = false;
    if ((!self->enemy) || (!self->enemy->inUse)) {
        hesDeadJim = true;
    } else if (self->monsterInfo.aiflags & AI_MEDIC) {
        if (self->enemy->health > 0) {
            hesDeadJim = true;
            self->monsterInfo.aiflags &= ~AI_MEDIC;
        }
    } else {
        if (self->monsterInfo.aiflags & AI_BRUTAL) {
            if (self->enemy->health <= -80)
                hesDeadJim = true;
        } else {
            if (self->enemy->health <= 0)
                hesDeadJim = true;
        }
    }

    if (hesDeadJim) {
        self->enemy = NULL;
        // FIXME: look all around for other targets
        if (self->oldEnemyPtr && self->oldEnemyPtr->health > 0) {
            self->enemy = self->oldEnemyPtr;
            self->oldEnemyPtr = NULL;
            HuntTarget(self);
        } else {
            if (self->moveTargetPtr) {
                self->goalEntityPtr = self->moveTargetPtr;
                self->monsterInfo.walk(self);
            } else {
                // we need the pausetime otherwise the stand code
                // will just revert to walking with no target and
                // the monsters will wonder around aimlessly trying
                // to hunt the world entity
                self->monsterInfo.pausetime = level.time + 100000000;
                self->monsterInfo.stand(self);
            }
            return true;
        }
    }

    self->showHostile = level.time + 1;        // wake up other monsters

// check knowledge of enemy
    enemy_vis = visible(self, self->enemy);
    if (enemy_vis) {
        self->monsterInfo.search_time = level.time + 5;
        VectorCopy(self->enemy->state.origin, self->monsterInfo.last_sighting);
    }

// look for other coop players here
//  if (coop && self->monsterInfo.search_time < level.time)
//  {
//      if (FindTarget (self))
//          return true;
//  }

    enemy_range = range(self, self->enemy);
    VectorSubtract(self->enemy->state.origin, self->state.origin, temp);
    enemy_yaw = vectoyaw(temp);


    // JDC self->idealYaw = enemy_yaw;

    if (self->monsterInfo.attack_state == AS_MISSILE) {
        ai_run_missile(self);
        return true;
    }
    if (self->monsterInfo.attack_state == AS_MELEE) {
        ai_run_melee(self);
        return true;
    }

    // if enemy is not currently visible, we will never attack
    if (!enemy_vis)
        return false;

    return self->monsterInfo.checkattack(self);
}


/*
=============
ai_run

The monster has an enemy it is trying to kill
=============
*/
void ai_run(entity_t *self, float dist)
{
    vec3_t      v;
    entity_t     *tempgoal;
    entity_t     *save;
    qboolean    isNew; // CPP: Renamed, because new is a keyword in C++
    entity_t     *marker;
    float       d1, d2;
    trace_t     tr;
    vec3_t      v_forward, v_right;
    float       left, center, right;
    vec3_t      left_target, right_target;

    // if we're going to a combat point, just proceed
    if (self->monsterInfo.aiflags & AI_COMBAT_POINT) {
        M_MoveToGoal(self, dist);
        return;
    }

    if (self->monsterInfo.aiflags & AI_SOUND_TARGET) {
        VectorSubtract(self->state.origin, self->enemy->state.origin, v);
        if (VectorLength(v) < 64) {
            self->monsterInfo.aiflags |= (AI_STAND_GROUND | AI_TEMP_STAND_GROUND);
            self->monsterInfo.stand(self);
            return;
        }

        M_MoveToGoal(self, dist);

        if (!FindTarget(self))
            return;
    }

    if (ai_checkattack(self, dist))
        return;

    if (self->monsterInfo.attack_state == AS_SLIDING) {
        ai_run_slide(self, dist);
        return;
    }

    if (enemy_vis) {
//      if (self.aiflags & AI_LOST_SIGHT)
//          dprint("regained sight\n");
        M_MoveToGoal(self, dist);
        self->monsterInfo.aiflags &= ~AI_LOST_SIGHT;
        VectorCopy(self->enemy->state.origin, self->monsterInfo.last_sighting);
        self->monsterInfo.trail_time = level.time;
        return;
    }

    // coop will change to another enemy if visible
    if (coop->value) {
        // FIXME: insane guys get mad with this, which causes crashes!
        if (FindTarget(self))
            return;
    }

    if ((self->monsterInfo.search_time) && (level.time > (self->monsterInfo.search_time + 20))) {
        M_MoveToGoal(self, dist);
        self->monsterInfo.search_time = 0;
//      dprint("search timeout\n");
        return;
    }

    save = self->goalEntityPtr;
    tempgoal = G_Spawn();
    self->goalEntityPtr = tempgoal;

    isNew = false;

    if (!(self->monsterInfo.aiflags & AI_LOST_SIGHT)) {
        // just lost sight of the player, decide where to go first
//      dprint("lost sight of player, last seen at "); dprint(Vec3ToString(self.last_sighting)); dprint("\n");
        self->monsterInfo.aiflags |= (AI_LOST_SIGHT | AI_PURSUIT_LAST_SEEN);
        self->monsterInfo.aiflags &= ~(AI_PURSUE_NEXT | AI_PURSUE_TEMP);
        isNew = true;
    }

    if (self->monsterInfo.aiflags & AI_PURSUE_NEXT) {
        self->monsterInfo.aiflags &= ~AI_PURSUE_NEXT;
//      dprint("reached current goal: "); dprint(Vec3ToString(self.origin)); dprint(" "); dprint(Vec3ToString(self.last_sighting)); dprint(" "); dprint(ftos(vlen(self.origin - self.last_sighting))); dprint("\n");

        // give ourself more time since we got this far
        self->monsterInfo.search_time = level.time + 5;

        if (self->monsterInfo.aiflags & AI_PURSUE_TEMP) {
//          dprint("was temp goal; retrying original\n");
            self->monsterInfo.aiflags &= ~AI_PURSUE_TEMP;
            marker = NULL;
            VectorCopy(self->monsterInfo.saved_goal, self->monsterInfo.last_sighting);
            isNew = true;
        } else if (self->monsterInfo.aiflags & AI_PURSUIT_LAST_SEEN) {
            self->monsterInfo.aiflags &= ~AI_PURSUIT_LAST_SEEN;
            marker = PlayerTrail_PickFirst(self);
        } else {
            marker = PlayerTrail_PickNext(self);
        }

        if (marker) {
            VectorCopy(marker->state.origin, self->monsterInfo.last_sighting);
            self->monsterInfo.trail_time = marker->timestamp;
            self->state.angles[vec3_t::Yaw] = self->idealYaw = marker->state.angles[vec3_t::Yaw];
//          dprint("heading is "); dprint(ftos(self.ideal_yaw)); dprint("\n");

//          debug_drawline(self.origin, self.last_sighting, 52);
            isNew = true;
        }
    }

    VectorSubtract(self->state.origin, self->monsterInfo.last_sighting, v);
    d1 = VectorLength(v);
    if (d1 <= dist) {
        self->monsterInfo.aiflags |= AI_PURSUE_NEXT;
        dist = d1;
    }

    VectorCopy(self->monsterInfo.last_sighting, self->goalEntityPtr->state.origin);

    if (isNew) {
//      gi.DPrintf("checking for course correction\n");

        tr = gi.Trace(self->state.origin, self->mins, self->maxs, self->monsterInfo.last_sighting, self, CONTENTS_MASK_PLAYERSOLID);
        if (tr.fraction < 1) {
            VectorSubtract(self->goalEntityPtr->state.origin, self->state.origin, v);
            d1 = VectorLength(v);
            center = tr.fraction;
            d2 = d1 * ((center + 1) / 2);
            self->state.angles[vec3_t::Yaw] = self->idealYaw = vectoyaw(v);
            AngleVectors(self->state.angles, &v_forward, &v_right, NULL);

            VectorSet(v, d2, -16, 0);
            left_target = G_ProjectSource(self->state.origin, v, v_forward, v_right);
            tr = gi.Trace(self->state.origin, self->mins, self->maxs, left_target, self, CONTENTS_MASK_PLAYERSOLID);
            left = tr.fraction;

            VectorSet(v, d2, 16, 0);
            right_target = G_ProjectSource(self->state.origin, v, v_forward, v_right);
            tr = gi.Trace(self->state.origin, self->mins, self->maxs, right_target, self, CONTENTS_MASK_PLAYERSOLID);
            right = tr.fraction;

            center = (d1 * center) / d2;
            if (left >= center && left > right) {
                if (left < 1) {
                    VectorSet(v, d2 * left * 0.5, -16, 0);
                    left_target = G_ProjectSource(self->state.origin, v, v_forward, v_right);
//                  gi.DPrintf("incomplete path, go part way and adjust again\n");
                }
                VectorCopy(self->monsterInfo.last_sighting, self->monsterInfo.saved_goal);
                self->monsterInfo.aiflags |= AI_PURSUE_TEMP;
                VectorCopy(left_target, self->goalEntityPtr->state.origin);
                VectorCopy(left_target, self->monsterInfo.last_sighting);
                VectorSubtract(self->goalEntityPtr->state.origin, self->state.origin, v);
                self->state.angles[vec3_t::Yaw] = self->idealYaw = vectoyaw(v);
//              gi.DPrintf("adjusted left\n");
//              debug_drawline(self.origin, self.last_sighting, 152);
            } else if (right >= center && right > left) {
                if (right < 1) {
                    VectorSet(v, d2 * right * 0.5, 16, 0);
                    right_target = G_ProjectSource(self->state.origin, v, v_forward, v_right);
//                  gi.DPrintf("incomplete path, go part way and adjust again\n");
                }
                VectorCopy(self->monsterInfo.last_sighting, self->monsterInfo.saved_goal);
                self->monsterInfo.aiflags |= AI_PURSUE_TEMP;
                VectorCopy(right_target, self->goalEntityPtr->state.origin);
                VectorCopy(right_target, self->monsterInfo.last_sighting);
                VectorSubtract(self->goalEntityPtr->state.origin, self->state.origin, v);
                self->state.angles[vec3_t::Yaw] = self->idealYaw = vectoyaw(v);
//              gi.DPrintf("adjusted right\n");
//              debug_drawline(self.origin, self.last_sighting, 152);
            }
        }
//      else gi.DPrintf("course was fine\n");
    }

    M_MoveToGoal(self, dist);

    G_FreeEntity(tempgoal);

    if (self)
        self->goalEntityPtr = save;
}
