// LICENSE HERE.

//
// svgame/entities/target_spawner.c
//
//
// target_spawner entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED target_spawner (1 0 0) (-8 -8 -8) (8 8 8)
Set target to the type of entity you want spawned.
Useful for spawning monsters and gibs in the factory levels.

For monsters:
    Set direction to the facing you want it to have.

For gibs:
    Set direction if you want it moving and
    speed how fast it should be moving otherwise it
    will just be dropped
*/
void ED_CallSpawn(edict_t* ent);

void use_target_spawner(edict_t* self, edict_t* other, edict_t* activator)
{
    edict_t* ent;

    ent = G_Spawn();
    ent->classname = self->target;
    VectorCopy(self->s.origin, ent->s.origin);
    VectorCopy(self->s.angles, ent->s.angles);
    ED_CallSpawn(ent);
    gi.unlinkentity(ent);
    KillBox(ent);
    gi.linkentity(ent);
    if (self->speed)
        VectorCopy(self->movedir, ent->velocity);
}

void SP_target_spawner(edict_t* self)
{
    self->use = use_target_spawner;
    self->svflags = SVF_NOCLIENT;
    if (self->speed) {
        G_SetMovedir(self->s.angles, self->movedir);
        VectorScale(self->movedir, self->speed, self->movedir);
    }
}