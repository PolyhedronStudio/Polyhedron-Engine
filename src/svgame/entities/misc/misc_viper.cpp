// LICENSE HERE.

//
// svgame/entities/misc_viper.c
//
//
// misc_viper entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32)
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"     How fast the Viper should fly
*/
extern void train_use(edict_t* self, edict_t* other, edict_t* activator);
extern void func_train_find(edict_t* self);

void misc_viper_use(edict_t* self, edict_t* other, edict_t* activator)
{
    self->svflags &= ~SVF_NOCLIENT;
    self->use = train_use;
    train_use(self, other, activator);
}

void SP_misc_viper(edict_t* ent)
{
    if (!ent->target) {
        gi.dprintf("misc_viper without a target at %s\n", vtos(ent->absmin));
        G_FreeEdict(ent);
        return;
    }

    if (!ent->speed)
        ent->speed = 300;

    ent->movetype = MOVETYPE_PUSH;
    ent->solid = SOLID_NOT;
    ent->s.modelindex = gi.modelindex("models/ships/viper/tris.md2");
    VectorSet(ent->mins, -16, -16, 0);
    VectorSet(ent->maxs, 16, 16, 32);

    ent->think = func_train_find;
    ent->nextthink = level.time + FRAMETIME;
    ent->use = misc_viper_use;
    ent->svflags |= SVF_NOCLIENT;
    ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

    gi.linkentity(ent);
}