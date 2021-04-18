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
#include "g_local.h"         // Include SVGame funcs.
#include "utils.h"           // Include Utilities funcs.


//
// monster weapons
//

//FIXME mosnters should call these with a totally accurate direction
// and we can mess it up based on skill.  Spread should be for normal
// and we can tighten or loosen based on skill.  We could muck with
// the damages too, but I'm not sure that's such a good idea.
void monster_fire_bullet(entity_t *self, const vec3_t &start, const vec3_t &dir, int damage, int kick, int hspread, int vspread, int flashtype)
{
    fire_bullet(self, start, dir, damage, kick, hspread, vspread, MOD_UNKNOWN);

    gi.WriteByte(svg_muzzleflash2);
    gi.WriteShort(self - g_edicts);
    gi.WriteByte(flashtype);
    gi.Multicast(&start, MULTICAST_PVS);
}


//
// Monster utility functions
//


void M_FlyCheck(entity_t *self)
{

}

void AttackFinished(entity_t *self, float time)
{
    self->monsterInfo.attack_finished = level.time + time;
}


void M_CheckGround(entity_t *ent)
{
    vec3_t      point;
    trace_t     trace;

    if (ent->flags & (FL_SWIM | FL_FLY))
        return;

    if (ent->velocity[2] > 100) {
        ent->groundEntityPtr = NULL;
        return;
    }

// if the hull point one-quarter unit down is solid the entity is on ground
    point[0] = ent->s.origin[0];
    point[1] = ent->s.origin[1];
    point[2] = ent->s.origin[2] - 0.25;

    trace = gi.Trace(ent->s.origin, ent->mins, ent->maxs, point, ent, CONTENTS_MASK_MONSTERSOLID);

    // check steepness
    if (trace.plane.normal[2] < 0.7 && !trace.startSolid) {
        ent->groundEntityPtr = NULL;
        return;
    }

//  ent->groundEntityPtr = trace.ent;
//  ent->groundEntityLinkCount = trace.ent->linkCount;
//  if (!trace.startSolid && !trace.allSolid)
//      VectorCopy (trace.endPosition, ent->s.origin);
    if (!trace.startSolid && !trace.allSolid) {
        VectorCopy(trace.endPosition, ent->s.origin);
        ent->groundEntityPtr = trace.ent;
        ent->groundEntityLinkCount = trace.ent->linkCount;
        ent->velocity[2] = 0;
    }
}


void M_CatagorizePosition(entity_t *ent)
{
    vec3_t      point;
    int         cont;

//
// get waterlevel
//
    point[0] = ent->s.origin[0];
    point[1] = ent->s.origin[1];
    point[2] = ent->s.origin[2] + ent->mins[2] + 1;
    cont = gi.PointContents(point);

    if (!(cont & CONTENTS_MASK_LIQUID)) {
        ent->waterLevel = 0;
        ent->waterType = 0;
        return;
    }

    ent->waterType = cont;
    ent->waterLevel = 1;
    point[2] += 26;
    cont = gi.PointContents(point);
    if (!(cont & CONTENTS_MASK_LIQUID))
        return;

    ent->waterLevel = 2;
    point[2] += 22;
    cont = gi.PointContents(point);
    if (cont & CONTENTS_MASK_LIQUID)
        ent->waterLevel = 3;
}


void M_WorldEffects(entity_t *ent)
{
    int     dmg;

    if (ent->health > 0) {
        if (!(ent->flags & FL_SWIM)) {
            if (ent->waterLevel < 3) {
                ent->air_finished = level.time + 12;
            } else if (ent->air_finished < level.time) {
                // drown!
                if (ent->debouncePainTime < level.time) {
                    dmg = 2 + 2 * floor(level.time - ent->air_finished);
                    if (dmg > 15)
                        dmg = 15;
                    T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
                    ent->debouncePainTime = level.time + 1;
                }
            }
        } else {
            if (ent->waterLevel > 0) {
                ent->air_finished = level.time + 9;
            } else if (ent->air_finished < level.time) {
                // suffocate!
                if (ent->debouncePainTime < level.time) {
                    dmg = 2 + 2 * floor(level.time - ent->air_finished);
                    if (dmg > 15)
                        dmg = 15;
                    T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
                    ent->debouncePainTime = level.time + 1;
                }
            }
        }
    }

    if (ent->waterLevel == 0) {
        if (ent->flags & FL_INWATER) {
            gi.Sound(ent, CHAN_BODY, gi.SoundIndex("player/watr_out.wav"), 1, ATTN_NORM, 0);
            ent->flags &= ~FL_INWATER;
        }
        return;
    }

    if ((ent->waterType & CONTENTS_LAVA) && !(ent->flags & FL_IMMUNE_LAVA)) {
        if (ent->debounceDamageTime < level.time) {
            ent->debounceDamageTime = level.time + 0.2;
            T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 10 * ent->waterLevel, 0, 0, MOD_LAVA);
        }
    }
    if ((ent->waterType & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME)) {
        if (ent->debounceDamageTime < level.time) {
            ent->debounceDamageTime = level.time + 1;
            T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 4 * ent->waterLevel, 0, 0, MOD_SLIME);
        }
    }

    if (!(ent->flags & FL_INWATER)) {
        if (!(ent->svFlags & SVF_DEADMONSTER)) {
            if (ent->waterType & CONTENTS_LAVA)
                if (random() <= 0.5)
                    gi.Sound(ent, CHAN_BODY, gi.SoundIndex("player/lava1.wav"), 1, ATTN_NORM, 0);
                else
                    gi.Sound(ent, CHAN_BODY, gi.SoundIndex("player/lava2.wav"), 1, ATTN_NORM, 0);
            else if (ent->waterType & CONTENTS_SLIME)
                gi.Sound(ent, CHAN_BODY, gi.SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
            else if (ent->waterType & CONTENTS_WATER)
                gi.Sound(ent, CHAN_BODY, gi.SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
        }

        ent->flags |= FL_INWATER;
        ent->debounceDamageTime = 0;
    }
}


void M_droptofloor(entity_t *ent)
{
    vec3_t      end;
    trace_t     trace;

    ent->s.origin[2] += 1;
    VectorCopy(ent->s.origin, end);
    end[2] -= 256;

    trace = gi.Trace(ent->s.origin, ent->mins, ent->maxs, end, ent, CONTENTS_MASK_MONSTERSOLID);

    if (trace.fraction == 1 || trace.allSolid)
        return;

    VectorCopy(trace.endPosition, ent->s.origin);

    gi.LinkEntity(ent);
    M_CheckGround(ent);
    M_CatagorizePosition(ent);
}


void M_SetEffects(entity_t *ent)
{
    ent->s.renderfx &= ~(RenderEffects::RedShell | RenderEffects::GreenShell | RenderEffects::BlueShell);
}


void M_MoveFrame(entity_t *self)
{
    mmove_t *move;
    int     index;

    move = self->monsterInfo.currentmove;
    self->nextThink = level.time + FRAMETIME;

    if ((self->monsterInfo.nextframe) && (self->monsterInfo.nextframe >= move->firstframe) && (self->monsterInfo.nextframe <= move->lastframe)) {
        self->s.frame = self->monsterInfo.nextframe;
        self->monsterInfo.nextframe = 0;
    } else {
        if (self->s.frame == move->lastframe) {
            if (move->endfunc) {
                move->endfunc(self);

                // regrab move, endfunc is very likely to change it
                move = self->monsterInfo.currentmove;

                // check for death
                if (self->svFlags & SVF_DEADMONSTER)
                    return;
            }
        }

        if (self->s.frame < move->firstframe || self->s.frame > move->lastframe) {
            self->monsterInfo.aiflags &= ~AI_HOLD_FRAME;
            self->s.frame = move->firstframe;
        } else {
            if (!(self->monsterInfo.aiflags & AI_HOLD_FRAME)) {
                self->s.frame++;
                if (self->s.frame > move->lastframe)
                    self->s.frame = move->firstframe;
            }
        }
    }

    index = self->s.frame - move->firstframe;
    if (move->frame[index].aifunc) {
        if (!(self->monsterInfo.aiflags & AI_HOLD_FRAME))
            move->frame[index].aifunc(self, move->frame[index].dist * self->monsterInfo.scale);
        else
            move->frame[index].aifunc(self, 0);
    }

    if (move->frame[index].thinkfunc)
        move->frame[index].thinkfunc(self);
}


void monster_think(entity_t *self)
{
    M_MoveFrame(self);
    if (self->linkCount != self->monsterInfo.linkCount) {
        self->monsterInfo.linkCount = self->linkCount;
        M_CheckGround(self);
    }
    M_CatagorizePosition(self);
    M_WorldEffects(self);
    M_SetEffects(self);
}


/*
================
monster_use

Using a monster makes it angry at the current activator
================
*/
void monster_use(entity_t *self, entity_t *other, entity_t *activator)
{
    if (self->enemy)
        return;
    if (self->health <= 0)
        return;
    if (activator->flags & FL_NOTARGET)
        return;
    if (!(activator->client) && !(activator->monsterInfo.aiflags & AI_GOOD_GUY))
        return;

// delay reaction so if the monster is teleported, its sound is still heard
    self->enemy = activator;
    FoundTarget(self);
}


void monster_start_go(entity_t *self);


void monster_triggered_spawn(entity_t *self)
{
    self->s.origin[2] += 1;
    KillBox(self);

    self->solid = SOLID_BBOX;
    self->moveType = MOVETYPE_STEP;
    self->svFlags &= ~SVF_NOCLIENT;
    self->air_finished = level.time + 12;
    gi.LinkEntity(self);

    monster_start_go(self);

    if (self->enemy && !(self->spawnFlags & 1) && !(self->enemy->flags & FL_NOTARGET)) {
        FoundTarget(self);
    } else {
        self->enemy = NULL;
    }
}

void monster_triggered_spawn_use(entity_t *self, entity_t *other, entity_t *activator)
{
    // we have a one frame delay here so we don't telefrag the guy who activated us
    self->Think = monster_triggered_spawn;
    self->nextThink = level.time + FRAMETIME;
    if (activator->client)
        self->enemy = activator;
    self->Use = monster_use;
}

void monster_triggered_start(entity_t *self)
{
    self->solid = SOLID_NOT;
    self->moveType = MOVETYPE_NONE;
    self->svFlags |= SVF_NOCLIENT;
    self->nextThink = 0;
    self->Use = monster_triggered_spawn_use;
}


/*
================
monster_death_use

When a monster dies, it fires all of its targets with the current
enemy as activator.
================
*/
void monster_death_use(entity_t *self)
{
    self->flags &= ~(FL_FLY | FL_SWIM);
    self->monsterInfo.aiflags &= AI_GOOD_GUY;

    if (self->item) {
        Drop_Item(self, self->item);
        self->item = NULL;
    }

    if (self->deathTarget)
        self->target = self->deathTarget;

    if (!self->target)
        return;

    UTIL_UseTargets(self, self->enemy);
}


//============================================================================

qboolean monster_start(entity_t *self)
{
    if (deathmatch->value) {
        G_FreeEntity(self);
        return false;
    }

    if ((self->spawnFlags & 4) && !(self->monsterInfo.aiflags & AI_GOOD_GUY)) {
        self->spawnFlags &= ~4;
        self->spawnFlags |= 1;
//      gi.DPrintf("fixed spawnFlags on %s at %s\n", self->classname, Vec3ToString(self->s.origin));
    }

    if (!(self->monsterInfo.aiflags & AI_GOOD_GUY))
        level.total_monsters++;

    self->nextThink = level.time + FRAMETIME;
    self->svFlags |= SVF_MONSTER;
    self->s.renderfx |= RenderEffects::FrameLerp;
    self->takedamage = DAMAGE_AIM;
    self->air_finished = level.time + 12;
    self->Use = monster_use;
    self->maxHealth = self->health;
    self->clipMask = CONTENTS_MASK_MONSTERSOLID;

    self->s.skinnum = 0;
    self->deadFlag = DEAD_NO;
    self->svFlags &= ~SVF_DEADMONSTER;

    if (!self->monsterInfo.checkattack)
        self->monsterInfo.checkattack = M_CheckAttack;
    VectorCopy(self->s.origin, self->s.old_origin);

    if (st.item) {
        self->item = FindItemByClassname(st.item);
        if (!self->item)
            gi.DPrintf("%s at %s has bad item: %s\n", self->classname, Vec3ToString(self->s.origin), st.item);
    }

    // randomize what frame they start on
    if (self->monsterInfo.currentmove)
        self->s.frame = self->monsterInfo.currentmove->firstframe + (rand() % (self->monsterInfo.currentmove->lastframe - self->monsterInfo.currentmove->firstframe + 1));

    return true;
}

void monster_start_go(entity_t *self)
{
    vec3_t  v;

    if (self->health <= 0)
        return;

    // check for target to combat_point and change to combatTarget
    if (self->target) {
        qboolean    notcombat;
        qboolean    fixup;
        entity_t     *target;

        target = NULL;
        notcombat = false;
        fixup = false;
        while ((target = G_Find(target, FOFS(targetName), self->target)) != NULL) {
            if (strcmp(target->classname, "point_combat") == 0) {
                self->combatTarget = self->target;
                fixup = true;
            } else {
                notcombat = true;
            }
        }
        if (notcombat && self->combatTarget)
            gi.DPrintf("%s at %s has target with mixed types\n", self->classname, Vec3ToString(self->s.origin));
        if (fixup)
            self->target = NULL;
    }

    // validate combatTarget
    if (self->combatTarget) {
        entity_t     *target;

        target = NULL;
        while ((target = G_Find(target, FOFS(targetName), self->combatTarget)) != NULL) {
            if (strcmp(target->classname, "point_combat") != 0) {
                gi.DPrintf("%s at (%i %i %i) has a bad combatTarget %s : %s at (%i %i %i)\n",
                           self->classname, (int)self->s.origin[0], (int)self->s.origin[1], (int)self->s.origin[2],
                           self->combatTarget, target->classname, (int)target->s.origin[0], (int)target->s.origin[1],
                           (int)target->s.origin[2]);
            }
        }
    }

    if (self->target) {
        self->goalEntityPtr = self->moveTargetPtr = G_PickTarget(self->target);
        if (!self->moveTargetPtr) {
            gi.DPrintf("%s can't find target %s at %s\n", self->classname, self->target, Vec3ToString(self->s.origin));
            self->target = NULL;
            self->monsterInfo.pausetime = 100000000;
            self->monsterInfo.stand(self);
        } else if (strcmp(self->moveTargetPtr->classname, "path_corner") == 0) {
            VectorSubtract(self->goalEntityPtr->s.origin, self->s.origin, v);
            self->idealYaw = self->s.angles[vec3_t::Yaw] = vectoyaw(v);
            self->monsterInfo.walk(self);
            self->target = NULL;
        } else {
            self->goalEntityPtr = self->moveTargetPtr = NULL;
            self->monsterInfo.pausetime = 100000000;
            self->monsterInfo.stand(self);
        }
    } else {
        self->monsterInfo.pausetime = 100000000;
        self->monsterInfo.stand(self);
    }

    self->Think = monster_think;
    self->nextThink = level.time + FRAMETIME;
}


void walkmonster_start_go(entity_t *self)
{
    if (!(self->spawnFlags & 2) && level.time < 1) {
        M_droptofloor(self);

        if (self->groundEntityPtr)
            if (!M_walkmove(self, 0, 0))
                gi.DPrintf("%s in solid at %s\n", self->classname, Vec3ToString(self->s.origin));
    }

    if (!self->yawSpeed)
        self->yawSpeed = 20;
    self->viewHeight = 25;

    monster_start_go(self);

    if (self->spawnFlags & 2)
        monster_triggered_start(self);
}

void walkmonster_start(entity_t *self)
{
    self->Think = walkmonster_start_go;
    monster_start(self);
}


void flymonster_start_go(entity_t *self)
{
    if (!M_walkmove(self, 0, 0))
        gi.DPrintf("%s in solid at %s\n", self->classname, Vec3ToString(self->s.origin));

    if (!self->yawSpeed)
        self->yawSpeed = 10;
    self->viewHeight = 25;

    monster_start_go(self);

    if (self->spawnFlags & 2)
        monster_triggered_start(self);
}


void flymonster_start(entity_t *self)
{
    self->flags |= FL_FLY;
    self->Think = flymonster_start_go;
    monster_start(self);
}


void swimmonster_start_go(entity_t *self)
{
    if (!self->yawSpeed)
        self->yawSpeed = 10;
    self->viewHeight = 10;

    monster_start_go(self);

    if (self->spawnFlags & 2)
        monster_triggered_start(self);
}

void swimmonster_start(entity_t *self)
{
    self->flags |= FL_SWIM;
    self->Think = swimmonster_start_go;
    monster_start(self);
}
