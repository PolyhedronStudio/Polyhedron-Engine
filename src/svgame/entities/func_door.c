// LICENSE HERE.

//
// svgame/entities/func_door.c
//
//
// func_door entity implementation.
//

// Include local game header.
#include "../g_local.h"

// Include Brush funcs header.
#include "../brushfuncs.h"

// Include func_door header.
#include "func_door.h"

//=====================================================
/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER NOMONSTER ANIMATED TOGGLE ANIMATED_FAST
TOGGLE      wait in both the start and end states for a trigger event.
START_OPEN  the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER   monsters will not trigger this door

"message"   is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"     determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"    if set, door must be shot open
"speed"     movement speed (100 default)
"wait"      wait before returning (3 default, -1 = never return)
"lip"       lip remaining at end of move (8 default)
"dmg"       damage to inflict when blocked (2 default)
"sounds"
1)  silent
2)  light
3)  medium
4)  heavy
*/

void door_use_areaportals(edict_t* self, qboolean open)
{
    edict_t* t = NULL;

    if (!self->target)
        return;

    while ((t = G_Find(t, FOFS(targetname), self->target))) {
        if (Q_stricmp(t->classname, "func_areaportal") == 0) {
            gi.SetAreaPortalState(t->style, open);
        }
    }
}

void door_go_down(edict_t* self);

void door_hit_top(edict_t* self)
{
    if (!(self->flags & FL_TEAMSLAVE)) {
        if (self->moveinfo.sound_end)
            gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
        self->s.sound = 0;
    }
    self->moveinfo.state = STATE_TOP;
    if (self->spawnflags & DOOR_TOGGLE)
        return;
    if (self->moveinfo.wait >= 0) {
        self->think = door_go_down;
        self->nextthink = level.time + self->moveinfo.wait;
    }
}

void door_hit_bottom(edict_t* self)
{
    if (!(self->flags & FL_TEAMSLAVE)) {
        if (self->moveinfo.sound_end)
            gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_end, 1, ATTN_STATIC, 0);
        self->s.sound = 0;
    }
    self->moveinfo.state = STATE_BOTTOM;
    door_use_areaportals(self, qfalse);
}

void door_go_down(edict_t* self)
{
    if (!(self->flags & FL_TEAMSLAVE)) {
        if (self->moveinfo.sound_start)
            gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
        self->s.sound = self->moveinfo.sound_middle;
    }
    if (self->max_health) {
        self->takedamage = DAMAGE_YES;
        self->health = self->max_health;
    }

    self->moveinfo.state = STATE_DOWN;
    if (strcmp(self->classname, "func_door") == 0)
        Brush_Move_Calc(self, self->moveinfo.start_origin, door_hit_bottom);
    else if (strcmp(self->classname, "func_door_rotating") == 0)
        Brush_AngleMove_Calc(self, door_hit_bottom);
}

void door_go_up(edict_t* self, edict_t* activator)
{
    if (self->moveinfo.state == STATE_UP)
        return;     // already going up

    if (self->moveinfo.state == STATE_TOP) {
        // reset top wait time
        if (self->moveinfo.wait >= 0)
            self->nextthink = level.time + self->moveinfo.wait;
        return;
    }

    if (!(self->flags & FL_TEAMSLAVE)) {
        if (self->moveinfo.sound_start)
            gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
        self->s.sound = self->moveinfo.sound_middle;
    }
    self->moveinfo.state = STATE_UP;
    if (strcmp(self->classname, "func_door") == 0)
        Brush_Move_Calc(self, self->moveinfo.end_origin, door_hit_top);
    else if (strcmp(self->classname, "func_door_rotating") == 0)
        Brush_AngleMove_Calc(self, door_hit_top);

    G_UseTargets(self, activator);
    door_use_areaportals(self, qtrue);
}

void door_use(edict_t* self, edict_t* other, edict_t* activator)
{
    edict_t* ent;

    if (self->flags & FL_TEAMSLAVE)
        return;

    if (self->spawnflags & DOOR_TOGGLE) {
        if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP) {
            // trigger all paired doors
            for (ent = self; ent; ent = ent->teamchain) {
                ent->message = NULL;
                ent->touch = NULL;
                door_go_down(ent);
            }
            return;
        }
    }

    // trigger all paired doors
    for (ent = self; ent; ent = ent->teamchain) {
        ent->message = NULL;
        ent->touch = NULL;
        door_go_up(ent, activator);
    }
}

void Touch_DoorTrigger(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    if (other->health <= 0)
        return;

    if (!(other->svflags & SVF_MONSTER) && (!other->client))
        return;

    if ((self->owner->spawnflags & DOOR_NOMONSTER) && (other->svflags & SVF_MONSTER))
        return;

    if (level.time < self->touch_debounce_time)
        return;
    self->touch_debounce_time = level.time + 1.0;

    door_use(self->owner, other, other);
}

void Think_CalcMoveSpeed(edict_t* self)
{
    edict_t* ent;
    float   min;
    float   time;
    float   newspeed;
    float   ratio;
    float   dist;

    if (self->flags & FL_TEAMSLAVE)
        return;     // only the team master does this

    // find the smallest distance any member of the team will be moving
    min = fabs(self->moveinfo.distance);
    for (ent = self->teamchain; ent; ent = ent->teamchain) {
        dist = fabs(ent->moveinfo.distance);
        if (dist < min)
            min = dist;
    }

    time = min / self->moveinfo.speed;

    // adjust speeds so they will all complete at the same time
    for (ent = self; ent; ent = ent->teamchain) {
        newspeed = fabs(ent->moveinfo.distance) / time;
        ratio = newspeed / ent->moveinfo.speed;
        if (ent->moveinfo.accel == ent->moveinfo.speed)
            ent->moveinfo.accel = newspeed;
        else
            ent->moveinfo.accel *= ratio;
        if (ent->moveinfo.decel == ent->moveinfo.speed)
            ent->moveinfo.decel = newspeed;
        else
            ent->moveinfo.decel *= ratio;
        ent->moveinfo.speed = newspeed;
    }
}

void Think_SpawnDoorTrigger(edict_t* ent)
{
    edict_t* other;
    vec3_t      mins, maxs;

    if (ent->flags & FL_TEAMSLAVE)
        return;     // only the team leader spawns a trigger

    VectorCopy(ent->absmin, mins);
    VectorCopy(ent->absmax, maxs);

    for (other = ent->teamchain; other; other = other->teamchain) {
        AddPointToBounds(other->absmin, mins, maxs);
        AddPointToBounds(other->absmax, mins, maxs);
    }

    // expand
    mins[0] -= 60;
    mins[1] -= 60;
    maxs[0] += 60;
    maxs[1] += 60;

    other = G_Spawn();
    VectorCopy(mins, other->mins);
    VectorCopy(maxs, other->maxs);
    other->owner = ent;
    other->solid = SOLID_TRIGGER;
    other->movetype = MOVETYPE_NONE;
    other->touch = Touch_DoorTrigger;
    gi.linkentity(other);

    if (ent->spawnflags & DOOR_START_OPEN)
        door_use_areaportals(ent, qtrue);

    Think_CalcMoveSpeed(ent);
}

void door_blocked(edict_t* self, edict_t* other)
{
    edict_t* ent;

    if (!(other->svflags & SVF_MONSTER) && (!other->client)) {
        // give it a chance to go away on it's own terms (like gibs)
        T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
        // if it's still there, nuke it
        if (other)
            BecomeExplosion1(other);
        return;
    }

    T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

    if (self->spawnflags & DOOR_CRUSHER)
        return;


    // if a door has a negative wait, it would never come back if blocked,
    // so let it just squash the object to death real fast
    if (self->moveinfo.wait >= 0) {
        if (self->moveinfo.state == STATE_DOWN) {
            for (ent = self->teammaster; ent; ent = ent->teamchain)
                door_go_up(ent, ent->activator);
        }
        else {
            for (ent = self->teammaster; ent; ent = ent->teamchain)
                door_go_down(ent);
        }
    }
}

void door_killed(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point)
{
    edict_t* ent;

    for (ent = self->teammaster; ent; ent = ent->teamchain) {
        ent->health = ent->max_health;
        ent->takedamage = DAMAGE_NO;
    }
    door_use(self->teammaster, attacker, attacker);
}

void door_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    if (!other->client)
        return;

    if (level.time < self->touch_debounce_time)
        return;
    self->touch_debounce_time = level.time + 5.0;

    gi.centerprintf(other, "%s", self->message);
    gi.sound(other, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_NORM, 0);
}

void SP_func_door(edict_t* ent)
{
    vec3_t  abs_movedir;

    if (ent->sounds != 1) {
        ent->moveinfo.sound_start = gi.soundindex("doors/dr1_strt.wav");
        ent->moveinfo.sound_middle = gi.soundindex("doors/dr1_mid.wav");
        ent->moveinfo.sound_end = gi.soundindex("doors/dr1_end.wav");
    }

    G_SetMovedir(ent->s.angles, ent->movedir);
    ent->movetype = MOVETYPE_PUSH;
    ent->solid = SOLID_BSP;
    gi.setmodel(ent, ent->model);

    ent->blocked = door_blocked;
    ent->use = door_use;

    if (!ent->speed)
        ent->speed = 100;
    if (deathmatch->value)
        ent->speed *= 2;

    if (!ent->accel)
        ent->accel = ent->speed;
    if (!ent->decel)
        ent->decel = ent->speed;

    if (!ent->wait)
        ent->wait = 3;
    if (!st.lip)
        st.lip = 8;
    if (!ent->dmg)
        ent->dmg = 2;

    // calculate second position
    VectorCopy(ent->s.origin, ent->pos1);
    abs_movedir[0] = fabs(ent->movedir[0]);
    abs_movedir[1] = fabs(ent->movedir[1]);
    abs_movedir[2] = fabs(ent->movedir[2]);
    ent->moveinfo.distance = abs_movedir[0] * ent->size[0] + abs_movedir[1] * ent->size[1] + abs_movedir[2] * ent->size[2] - st.lip;
    VectorMA(ent->pos1, ent->moveinfo.distance, ent->movedir, ent->pos2);

    // if it starts open, switch the positions
    if (ent->spawnflags & DOOR_START_OPEN) {
        VectorCopy(ent->pos2, ent->s.origin);
        VectorCopy(ent->pos1, ent->pos2);
        VectorCopy(ent->s.origin, ent->pos1);
    }

    ent->moveinfo.state = STATE_BOTTOM;

    if (ent->health) {
        ent->takedamage = DAMAGE_YES;
        ent->die = door_killed;
        ent->max_health = ent->health;
    }
    else if (ent->targetname && ent->message) {
        gi.soundindex("misc/talk.wav");
        ent->touch = door_touch;
    }

    ent->moveinfo.speed = ent->speed;
    ent->moveinfo.accel = ent->accel;
    ent->moveinfo.decel = ent->decel;
    ent->moveinfo.wait = ent->wait;
    VectorCopy(ent->pos1, ent->moveinfo.start_origin);
    VectorCopy(ent->s.angles, ent->moveinfo.start_angles);
    VectorCopy(ent->pos2, ent->moveinfo.end_origin);
    VectorCopy(ent->s.angles, ent->moveinfo.end_angles);

    if (ent->spawnflags & 16)
        ent->s.effects |= EF_ANIM_ALL;
    if (ent->spawnflags & 64)
        ent->s.effects |= EF_ANIM_ALLFAST;

    // to simplify logic elsewhere, make non-teamed doors into a team of one
    if (!ent->team)
        ent->teammaster = ent;

    gi.linkentity(ent);

    ent->nextthink = level.time + FRAMETIME;
    if (ent->health || ent->targetname)
        ent->think = Think_CalcMoveSpeed;
    else
        ent->think = Think_SpawnDoorTrigger;
}