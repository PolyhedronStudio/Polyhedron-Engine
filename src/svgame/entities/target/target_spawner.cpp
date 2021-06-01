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
void ED_CallSpawn(Entity* ent);

void use_target_spawner(Entity* self, Entity* other, Entity* activator)
{
    Entity* ent;

    ent = SVG_Spawn();
    ent->className = self->target;
    VectorCopy(self->state.origin, ent->state.origin);
    VectorCopy(self->state.angles, ent->state.angles);
    ED_CallSpawn(ent);
    gi.UnlinkEntity(ent);
    SVG_KillBox(ent);
    gi.LinkEntity(ent);
    if (self->speed)
        VectorCopy(self->moveDirection, ent->velocity);
}

void SP_target_spawner(Entity* self)
{
    self->Use = use_target_spawner;
    self->serverFlags = EntityServerFlags::NoClient;
    if (self->speed) {
        UTIL_SetMoveDir(self->state.angles, self->moveDirection);
        VectorScale(self->moveDirection, self->speed, self->moveDirection);
    }
}