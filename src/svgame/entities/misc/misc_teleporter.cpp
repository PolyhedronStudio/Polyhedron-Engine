// LICENSE HERE.

//
// svgame/entities/misc_teleporter.c
//
//
// misc_teleporter entity implementation.
//

// Include local game header.
#include "../../g_local.h"

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

    Vec3_Copy(dest->s.origin, other->s.origin);
    Vec3_Copy(dest->s.origin, other->s.old_origin);
    other->s.origin[2] += 10;

    // clear the velocity and hold them in place briefly
    Vec3_Clear(other->velocity);
    other->client->ps.pmove.time = 160 >> 3;     // hold time
    other->client->ps.pmove.flags |= PMF_TIME_TELEPORT;

    // draw the teleport splash at source and on the player
    self->owner->s.event = EV_PLAYER_TELEPORT;
    other->s.event = EV_PLAYER_TELEPORT;

    // set angles
    for (i = 0; i < 3; i++) {
        other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);
    }

    Vec3_Clear(other->s.angles);
    Vec3_Clear(other->client->ps.viewangles);
    Vec3_Clear(other->client->v_angle);

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

    Vec3_Set(ent->mins, -32, -32, -24);
    Vec3_Set(ent->maxs, 32, 32, -16);
    gi.linkentity(ent);

    trig = G_Spawn();
    trig->touch = teleporter_touch;
    trig->solid = SOLID_TRIGGER;
    trig->target = ent->target;
    trig->owner = ent;
    Vec3_Copy(ent->s.origin, trig->s.origin);
    Vec3_Set(trig->mins, -8, -8, 8);
    Vec3_Set(trig->maxs, 8, 8, 24);
    gi.linkentity(trig);

}