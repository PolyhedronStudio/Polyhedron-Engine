// LICENSE HERE.

//
// svgame/entities/misc_teleporter.c
//
//
// misc_teleporter entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================

void teleporter_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf)
{
    entity_t* dest;
    int         i;

    if (!other->client)
        return;
    dest = G_Find(NULL, FOFS(targetName), self->target);
    if (!dest) {
        gi.DPrintf("Couldn't find destination\n");
        return;
    }

    // unlink to make sure it can't possibly interfere with KillBox
    gi.UnlinkEntity(other);

    VectorCopy(dest->state.origin, other->state.origin);
    VectorCopy(dest->state.origin, other->state.oldOrigin);
    other->state.origin[2] += 10;

    // clear the velocity and hold them in place briefly
    VectorClear(other->velocity);
    other->client->playerState.pmove.time = 160 >> 3;     // hold time
    other->client->playerState.pmove.flags |= PMF_TIME_TELEPORT;

    // draw the teleport splash at source and on the player
    self->owner->state.event = EntityEvent::PlayerTeleport;
    other->state.event = EntityEvent::PlayerTeleport;

    // set angles
    for (i = 0; i < 3; i++) {
        other->client->playerState.pmove.deltaAngles[i] = dest->state.angles[i] - other->client->respawn.commandViewAngles[i];
    }

    VectorClear(other->state.angles);
    VectorClear(other->client->playerState.pmove.viewAngles);
    VectorClear(other->client->aimAngles);

    // kill anything at the destination
    KillBox(other);

    gi.LinkEntity(other);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter(entity_t* ent)
{
    entity_t* trig;

    if (!ent->target) {
        gi.DPrintf("teleporter without a target.\n");
        G_FreeEntity(ent);
        return;
    }

    gi.SetModel(ent, "models/objects/dmspot/tris.md2");
    ent->state.skinNumber = 1;
    ent->state.effects = EntityEffectType::Teleporter;
    ent->state.sound = gi.SoundIndex("world/amb10.wav");
    ent->solid = Solid::BoundingBox;

    VectorSet(ent->mins, -32, -32, -24);
    VectorSet(ent->maxs, 32, 32, -16);
    gi.LinkEntity(ent);

    trig = G_Spawn();
    trig->Touch = teleporter_touch;
    trig->solid = Solid::Trigger;
    trig->target = ent->target;
    trig->owner = ent;
    VectorCopy(ent->state.origin, trig->state.origin);
    VectorSet(trig->mins, -8, -8, 8);
    VectorSet(trig->maxs, 8, 8, 24);
    gi.LinkEntity(trig);

}