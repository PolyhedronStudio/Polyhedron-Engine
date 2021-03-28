// LICENSE HERE.

//
// svgame/entities/func_door.c
//
//
// func_door entity implementation.
//

// Include local game header.
#include "../../g_local.h"

// Include Brush funcs header.
#include "../../brushfuncs.h"

// Include func_door header.
#include "func_door.h"

//=====================================================
/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS
TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN  the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER   monsters will not trigger this door

You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"distance" is how many degrees the door will be rotated.
"speed" determines how fast the door moves; default value is 100.

REVERSE will cause the door to rotate in the opposite direction.

"message"   is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"     determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"    if set, door must be shot open
"speed"     movement speed (100 default)
"wait"      wait before returning (3 default, -1 = never return)
"dmg"       damage to inflict when blocked (2 default)
"sounds"
1)  silent
2)  light
3)  medium
4)  heavy
*/

void SP_func_door_rotating(edict_t* ent)
{
    VectorClear(ent->s.angles);

    // set the axis of rotation
    VectorClear(ent->movedir);
    if (ent->spawnflags & DOOR_X_AXIS)
        ent->movedir[2] = 1.0;
    else if (ent->spawnflags & DOOR_Y_AXIS)
        ent->movedir[0] = 1.0;
    else // Z_AXIS
        ent->movedir[1] = 1.0;

    // check for reverse rotation
    if (ent->spawnflags & DOOR_REVERSE)
        VectorNegate(ent->movedir, ent->movedir);

    if (!st.distance) {
        gi.dprintf("%s at %s with no distance set\n", ent->classname, vtos(ent->s.origin));
        st.distance = 90;
    }

    VectorCopy(ent->s.angles, ent->pos1);
    VectorMA(ent->s.angles, st.distance, ent->movedir, ent->pos2);
    ent->moveinfo.distance = st.distance;

    ent->movetype = MOVETYPE_PUSH;
    ent->solid = SOLID_BSP;
    gi.setmodel(ent, ent->model);

    ent->blocked = door_blocked;
    ent->use = door_use;

    if (!ent->speed)
        ent->speed = 100;
    if (!ent->accel)
        ent->accel = ent->speed;
    if (!ent->decel)
        ent->decel = ent->speed;

    if (!ent->wait)
        ent->wait = 3;
    if (!ent->dmg)
        ent->dmg = 2;

    if (ent->sounds != 1) {
        ent->moveinfo.sound_start = gi.soundindex("doors/dr1_strt.wav");
        ent->moveinfo.sound_middle = gi.soundindex("doors/dr1_mid.wav");
        ent->moveinfo.sound_end = gi.soundindex("doors/dr1_end.wav");
    }

    // if it starts open, switch the positions
    if (ent->spawnflags & DOOR_START_OPEN) {
        VectorCopy(ent->pos2, ent->s.angles);
        VectorCopy(ent->pos1, ent->pos2);
        VectorCopy(ent->s.angles, ent->pos1);
        VectorNegate(ent->movedir, ent->movedir);
    }

    if (ent->health) {
        ent->takedamage = DAMAGE_YES;
        ent->die = door_killed;
        ent->max_health = ent->health;
    }

    if (ent->targetname && ent->message) {
        gi.soundindex("misc/talk.wav");
        ent->touch = door_touch;
    }

    ent->moveinfo.state = STATE_BOTTOM;
    ent->moveinfo.speed = ent->speed;
    ent->moveinfo.accel = ent->accel;
    ent->moveinfo.decel = ent->decel;
    ent->moveinfo.wait = ent->wait;
    VectorCopy(ent->s.origin, ent->moveinfo.start_origin);
    VectorCopy(ent->pos1, ent->moveinfo.start_angles);
    VectorCopy(ent->s.origin, ent->moveinfo.end_origin);
    VectorCopy(ent->pos2, ent->moveinfo.end_angles);

    if (ent->spawnflags & 16)
        ent->s.effects |= EF_ANIM_ALL;

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