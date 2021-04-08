// LICENSE HERE.

//
// svgame/entities/misc_viper.c
//
//
// misc_viper entity implementation.
//

// Include local game header.
#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.
//=====================================================
/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32)
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"     How fast the Viper should fly
*/
extern void train_use(entity_t* self, entity_t* other, entity_t* activator);
extern void func_train_find(entity_t* self);

void misc_viper_use(entity_t* self, entity_t* other, entity_t* activator)
{
    self->svFlags &= ~SVF_NOCLIENT;
    self->Use = train_use;
    train_use(self, other, activator);
}

void SP_misc_viper(entity_t* ent)
{
    if (!ent->target) {
        gi.DPrintf("misc_viper without a target at %s\n", Vec3ToString(ent->absMin));
        G_FreeEntity(ent);
        return;
    }

    if (!ent->speed)
        ent->speed = 300;

    ent->moveType = MOVETYPE_PUSH;
    ent->solid = SOLID_NOT;
    ent->s.modelindex = gi.ModelIndex("models/ships/viper/tris.md2");
    VectorSet(ent->mins, -16, -16, 0);
    VectorSet(ent->maxs, 16, 16, 32);

    ent->Think = func_train_find;
    ent->nextThink = level.time + FRAMETIME;
    ent->Use = misc_viper_use;
    ent->svFlags |= SVF_NOCLIENT;
    ent->moveInfo.accel = ent->moveInfo.decel = ent->moveInfo.speed = ent->speed;

    gi.LinkEntity(ent);
}