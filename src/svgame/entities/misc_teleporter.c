// LICENSE HERE.

//
// svgame/entities/misc_teleporter.c
//
//
// misc_teleporter entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================

void teleporter_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    edict_t* dest;
    int         i;

    if (!other->client)
        return;
    dest = G_Find(NULL, FOFS(targetname), self->target);
    if (!dest) {
        gi.dprintf("Couldn't find destination\n");
        return;
    }

    // unlink to make sure it can't possibly interfere with KillBox
    gi.unlinkentity(other);

    VectorCopy(dest->s.origin, other->s.origin);
    VectorCopy(dest->s.origin, other->s.old_origin);
    other->s.origin[2] += 10;

    // clear the velocity and hold them in place briefly
    VectorClear(other->velocity);
    other->client->ps.pmove.pm_time = 160 >> 3;     // hold time
    other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

    // draw the teleport splash at source and on the player
    self->owner->s.event = EV_PLAYER_TELEPORT;
    other->s.event = EV_PLAYER_TELEPORT;

    // set angles
    for (i = 0; i < 3; i++) {
        other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);
    }

    VectorClear(other->s.angles);
    VectorClear(other->client->ps.viewangles);
    VectorClear(other->client->v_angle);

    // kill anything at the destination
    KillBox(other);

    gi.linkentity(other);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter(edict_t* ent)
{
    edict_t* trig;

    if (!ent->target) {
        gi.dprintf("teleporter without a target.\n");
        G_FreeEdict(ent);
        return;
    }

    gi.setmodel(ent, "models/objects/dmspot/tris.md2");
    ent->s.skinnum = 1;
    ent->s.effects = EF_TELEPORTER;
    ent->s.sound = gi.soundindex("world/amb10.wav");
    ent->solid = SOLID_BBOX;

    VectorSet(ent->mins, -32, -32, -24);
    VectorSet(ent->maxs, 32, 32, -16);
    gi.linkentity(ent);

    trig = G_Spawn();
    trig->touch = teleporter_touch;
    trig->solid = SOLID_TRIGGER;
    trig->target = ent->target;
    trig->owner = ent;
    VectorCopy(ent->s.origin, trig->s.origin);
    VectorSet(trig->mins, -8, -8, 8);
    VectorSet(trig->maxs, 8, 8, 24);
    gi.linkentity(trig);

}