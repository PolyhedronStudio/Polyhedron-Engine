// LICENSE HERE.

//
// svgame/entities/target_spawner.c
//
//
// target_spawner entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.


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
void ED_CallSpawn(entity_t* ent);

void use_target_spawner(entity_t* self, entity_t* other, entity_t* activator)
{
    entity_t* ent;

    ent = G_Spawn();
    ent->classname = self->target;
    VectorCopy(self->state.origin, ent->state.origin);
    VectorCopy(self->state.angles, ent->state.angles);
    ED_CallSpawn(ent);
    gi.UnlinkEntity(ent);
    KillBox(ent);
    gi.LinkEntity(ent);
    if (self->speed)
        VectorCopy(self->moveDirection, ent->velocity);
}

void SP_target_spawner(entity_t* self)
{
    self->Use = use_target_spawner;
    self->serverFlags = EntityServerFlags::NoClient;
    if (self->speed) {
        UTIL_SetMoveDir(self->state.angles, self->moveDirection);
        VectorScale(self->moveDirection, self->speed, self->moveDirection);
    }
}