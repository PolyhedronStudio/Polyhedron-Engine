// LICENSE HERE.

//
// svgame/entities/func_door.c
//
//
// func_door entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.
#include "../../brushfuncs.h"   // Include Brush funcs.

#include "func_door.h"          // Include func_door entity header.

//=====================================================
/*QUAKED func_door_rotating (0 .5 .8) ? START_OPEN REVERSE CRUSHER NOMONSTER ANIMATED TOGGLE X_AXIS Y_AXIS
TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN  the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takeDamage doors).
NOMONSTER   monsters will not trigger this door

You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"distance" is how many degrees the door will be rotated.
"speed" determines how fast the door moves; default value is 100.

REVERSE will cause the door to rotate in the opposite direction.

"message"   is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"     determines the opening direction
"targetName" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"    if set, door must be shot open
"speed"     movement speed (100 default)
"wait"      wait before returning (3 default, -1 = never return)
"dmg"       damage to inflict when Blocked (2 default)
"sounds"
1)  silent
2)  light
3)  medium
4)  heavy
*/

void SP_func_door_rotating(entity_t* ent)
{
    VectorClear(ent->state.angles);

    // set the axis of rotation
    VectorClear(ent->moveDirection);
    if (ent->spawnFlags & DOOR_X_AXIS)
        ent->moveDirection[2] = 1.0;
    else if (ent->spawnFlags & DOOR_Y_AXIS)
        ent->moveDirection[0] = 1.0;
    else // Z_AXIS
        ent->moveDirection[1] = 1.0;

    // check for reverse rotation
    if (ent->spawnFlags & DOOR_REVERSE)
        VectorNegate(ent->moveDirection, ent->moveDirection);

    if (!st.distance) {
        gi.DPrintf("%s at %s with no distance set\n", ent->classname, Vec3ToString(ent->state.origin));
        st.distance = 90;
    }

    VectorCopy(ent->state.angles, ent->pos1);
    VectorMA(ent->state.angles, st.distance, ent->moveDirection, ent->pos2);
    ent->moveInfo.distance = st.distance;

    ent->moveType = MoveType::Push;
    ent->solid = Solid::BSP;
    gi.SetModel(ent, ent->model);

    ent->Blocked = door_blocked;
    ent->Use = door_use;

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
        ent->moveInfo.sound_start = gi.SoundIndex("doors/dr1_strt.wav");
        ent->moveInfo.sound_middle = gi.SoundIndex("doors/dr1_mid.wav");
        ent->moveInfo.sound_end = gi.SoundIndex("doors/dr1_end.wav");
    }

    // if it starts open, switch the positions
    if (ent->spawnFlags & DOOR_START_OPEN) {
        VectorCopy(ent->pos2, ent->state.angles);
        VectorCopy(ent->pos1, ent->pos2);
        VectorCopy(ent->state.angles, ent->pos1);
        VectorNegate(ent->moveDirection, ent->moveDirection);
    }

    if (ent->health) {
        ent->takeDamage = TakeDamage::Yes;
        ent->Die = door_killed;
        ent->maxHealth = ent->health;
    }

    if (ent->targetName && ent->message) {
        gi.SoundIndex("misc/talk.wav");
        ent->Touch = door_touch;
    }

    ent->moveInfo.state = STATE_BOTTOM;
    ent->moveInfo.speed = ent->speed;
    ent->moveInfo.accel = ent->accel;
    ent->moveInfo.decel = ent->decel;
    ent->moveInfo.wait = ent->wait;
    VectorCopy(ent->state.origin, ent->moveInfo.start_origin);
    VectorCopy(ent->pos1, ent->moveInfo.start_angles);
    VectorCopy(ent->state.origin, ent->moveInfo.end_origin);
    VectorCopy(ent->pos2, ent->moveInfo.end_angles);

    if (ent->spawnFlags & 16)
        ent->state.effects |= EntityEffectType::AnimCycleAll2hz;

    // to simplify logic elsewhere, make non-teamed doors into a team of one
    if (!ent->team)
        ent->teamMasterPtr = ent;

    gi.LinkEntity(ent);

    ent->nextThink = level.time + FRAMETIME;
    if (ent->health || ent->targetName)
        ent->Think = Think_CalcMoveSpeed;
    else
        ent->Think = Think_SpawnDoorTrigger;
}