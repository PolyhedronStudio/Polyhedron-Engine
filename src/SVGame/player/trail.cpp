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
#include "../g_local.h"      // SVGame funcs.
#include "../utils.h"        // Util funcs.


/*
==============================================================================

PLAYER TRAIL

==============================================================================

This is a circular list containing the a list of points of where
the player has been recently.  It is used by monsters for pursuit.

.origin     the spot
.owner      forward link
.aiment     backward link
*/


#define TRAIL_LENGTH    8

Entity     *trail[TRAIL_LENGTH];
int         trail_head;
qboolean    trail_active = false;

#define NEXT(n)     (((n) + 1) & (TRAIL_LENGTH - 1))
#define PREV(n)     (((n) - 1) & (TRAIL_LENGTH - 1))


void SVG_PlayerTrail_Init(void)
{
    //int     n;

    //if (deathmatch->value /* FIXME || coop */)
    //    return;

    //for (n = 0; n < TRAIL_LENGTH; n++) {
    //    trail[n] = SVG_Spawn();
    //    trail[n]->className = "player_trail";
    //}

    //trail_head = 0;
    //trail_active = true;
}


void SVG_PlayerTrail_Add(vec3_t spot)
{
    //vec3_t  temp;

    //if (!trail_active)
    //    return;

    //VectorCopy(spot, trail[trail_head]->state.origin);

    //trail[trail_head]->timeStamp = level.time;

    //VectorSubtract(spot, trail[PREV(trail_head)]->state.origin, temp);
    //trail[trail_head]->state.angles[1] = vec3_to_yaw(temp);

    //trail_head = NEXT(trail_head);
}


void SVG_PlayerTrail_New(vec3_t spot)
{
    //if (!trail_active)
    //    return;

    //SVG_PlayerTrail_Init();
    //SVG_PlayerTrail_Add(spot);
}


Entity *SVG_PlayerTrail_PickFirst(Entity *self)
{
//    int     marker;
//    int     n;
//
//    if (!trail_active)
//        return NULL;
//
//    for (marker = trail_head, n = TRAIL_LENGTH; n; n--) {
////        if (trail[marker]->timeStamp <= self->monsterInfo.trail_time)
//            marker = NEXT(marker);
//        //else
//        //    break;
//    }
//
//    //if (visible(self, trail[marker])) {
//    //    return trail[marker];
//    //}
//
//    //if (visible(self, trail[PREV(marker)])) {
//    //    return trail[PREV(marker)];
//    //}
//
//    return trail[marker];
    return nullptr;
}

Entity *SVG_PlayerTrail_PickNext(Entity *self)
{
//    int     marker;
//    int     n;
//
//    if (!trail_active)
//        return NULL;
//
//    for (marker = trail_head, n = TRAIL_LENGTH; n; n--) {
////        if (trail[marker]->timeStamp <= self->monsterInfo.trail_time)
//            marker = NEXT(marker);
//        //else
//        //    break;
//    }
//
//    return trail[marker];
    return nullptr;
}

Entity *SVG_PlayerTrail_LastSpot(void)
{
    return nullptr;
    //return trail[PREV(trail_head)];
}
