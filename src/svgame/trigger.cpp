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

void InitTrigger(Entity *self)
{
    //if (!VectorCompare(self->state.angles, vec3_zero()))
    //    UTIL_SetMoveDir(self->state.angles, self->moveDirection);

    //self->solid = Solid::Trigger;
    ////self->moveType = MoveType::None;
    //gi.SetModel(self, self->model);
    //self->serverFlags = EntityServerFlags::NoClient;
}


// the wait time has passed, so set back up for another activation
void multi_wait(Entity *ent)
{
//    ent->nextThinkTime = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger(Entity *ent)
{
//    if (ent->nextThinkTime)
//        return;     // already been triggered

    //UTIL_UseTargets(ent, ent->activator);

    //if (ent->wait > 0) {
    //    //ent->Think = multi_wait;
    //    ent->nextThinkTime = level.time + ent->wait;
    //} else {
    //    // we can't just remove (self) here, because this is a touch function
    //    // called while looping through area links...
    //    //ent->Touch = NULL;
    //    ent->nextThinkTime = level.time + FRAMETIME;
    //    //ent->Think = SVG_FreeEntity;
    //}
}

void Use_Multi(Entity *ent, Entity *other, Entity *activator)
{
    //ent->activator = activator;
    multi_trigger(ent);
}

void Touch_Multi(Entity *self, Entity *other, cplane_t *plane, csurface_t *surf)
{
    //if (other->client) {
    //    if (self->spawnFlags & 2)
    //        return;
    //} else if (other->serverFlags & EntityServerFlags::Monster) {
    //    if (!(self->spawnFlags & 1))
    //        return;
    //} else
    //    return;

    if (!VectorCompare(self->moveDirection, vec3_zero())) {
        vec3_t  forward;

        AngleVectors(other->state.angles, &forward, NULL, NULL);
        if (DotProduct(forward, self->moveDirection) < 0)
            return;
    }

    //self->activator = other;
    multi_trigger(self);
}