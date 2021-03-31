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
#include "g_local.h"
#include "utils.h"
#include "trigger.h"

void InitTrigger(edict_t *self)
{
    if (!VectorCompare(self->s.angles, vec3_origin))
        UTIL_SetMoveDir(self->s.angles, self->movedir);

    self->solid = SOLID_TRIGGER;
    self->movetype = MOVETYPE_NONE;
    gi.setmodel(self, self->model);
    self->svflags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void multi_wait(edict_t *ent)
{
    ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger(edict_t *ent)
{
    if (ent->nextthink)
        return;     // already been triggered

    UTIL_UseTargets(ent, ent->activator);

    if (ent->wait > 0) {
        ent->think = multi_wait;
        ent->nextthink = level.time + ent->wait;
    } else {
        // we can't just remove (self) here, because this is a touch function
        // called while looping through area links...
        ent->touch = NULL;
        ent->nextthink = level.time + FRAMETIME;
        ent->think = G_FreeEdict;
    }
}

void Use_Multi(edict_t *ent, edict_t *other, edict_t *activator)
{
    ent->activator = activator;
    multi_trigger(ent);
}

void Touch_Multi(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other->client) {
        if (self->spawnflags & 2)
            return;
    } else if (other->svflags & SVF_MONSTER) {
        if (!(self->spawnflags & 1))
            return;
    } else
        return;

    if (!VectorCompare(self->movedir, vec3_origin)) {
        vec3_t  forward;

        AngleVectors(other->s.angles, &forward, NULL, NULL);
        if (DotProduct(forward, self->movedir) < 0)
            return;
    }

    self->activator = other;
    multi_trigger(self);
}