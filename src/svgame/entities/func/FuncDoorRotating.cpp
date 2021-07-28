/*
// LICENSE HERE.

// FuncDoorRotating.cpp
*/

#include "../../g_local.h"
#include "../../effects.h"
#include "../../entities.h"
#include "../../utils.h"
#include "../../physics/stepmove.h"
#include "../../brushfuncs.h"

#include "../base/SVGBaseEntity.h"
#include "../base/SVGBaseTrigger.h"
#include "../base/SVGBaseMover.h"

#include "../trigger/TriggerAutoDoor.h"

#include "FuncDoor.h"
#include "FuncDoorRotating.h"

//===============
// FuncDoorRotating::ctor
//===============
FuncDoorRotating::FuncDoorRotating( Entity* entity )
	: Base( entity ) {

}

//===============
// FuncDoorRotating::Spawn
//===============
void FuncDoorRotating::Spawn() {
	Base::Spawn();

	SetAngles( vec3_zero() );

	// Set the axis of rotation
	moveDirection = vec3_zero();
	if ( GetSpawnFlags() & SF_XAxis ) {
		moveDirection.z = 1.0f;
	} else if ( GetSpawnFlags() & SF_YAxis ) {
		moveDirection.x = 1.0f;
	} else { // ZAxis
		moveDirection.y = 1.0f;
	}

	// Check for reverse rotation
	if ( GetSpawnFlags() & SF_Reverse ) {
		moveDirection = vec3_negate( moveDirection );
	}

	if ( !distance ) {
		gi.DPrintf( "%s with no distance set\n", GetClassName() );
		distance = 90.0f;
	}

	SetStartPosition( GetAngles() );
	SetEndPosition( vec3_fmaf( GetAngles(), distance, moveDirection ) );
	moveInfo.distance = distance;

	// If it starts open, switch the positions
	if ( GetSpawnFlags() & SF_StartOpen ) {
		moveDirection = vec3_negate( moveDirection );
	}

	moveInfo.startOrigin = GetOrigin();
	moveInfo.endOrigin = GetOrigin();
	moveInfo.startAngles = GetStartPosition();
	moveInfo.endAngles = GetEndPosition();

	LinkEntity();
}


/*  VectorClear(ent->s.angles);

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

*/

//===============
// FuncDoorRotating::SpawnKey
//===============
void FuncDoorRotating::SpawnKey( const std::string& key, const std::string& value ) {
	if ( key == "distance" ) {
		ParseFloatKeyValue( key, value, distance );
	} else {
		Base::SpawnKey( key, value );
	}
}
