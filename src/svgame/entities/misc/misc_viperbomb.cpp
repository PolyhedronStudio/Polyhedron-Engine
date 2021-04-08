// LICENSE HERE.

//
// svgame/entities/misc_viperbomb.c
//
//
// misc_viperbomb entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../effects.h"
#include "../../utils.h"

//=====================================================
/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"   how much boom should the bomb make?
*/
void misc_viper_bomb_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf)
{
    UTIL_UseTargets(self, self->activator);

    self->s.origin[2] = self->absMin[2] + 1;
    T_RadiusDamage(self, self, self->dmg, NULL, self->dmg + 40, MOD_BOMB);
    BecomeExplosion2(self);
}

void misc_viper_bomb_prethink(entity_t* self)
{
    vec3_t  v;
    float   diff;

    self->groundEntityPtr = NULL;

    diff = self->timestamp - level.time;
    if (diff < -1.0)
        diff = -1.0;

    VectorScale(self->moveInfo.dir, 1.0 + diff, v);
    v[2] = diff;

    diff = self->s.angles[2];
    vectoangles(v, self->s.angles);
    self->s.angles[2] = diff + 10;
}

void misc_viper_bomb_use(entity_t* self, entity_t* other, entity_t* activator)
{
    entity_t* viper;

    self->solid = SOLID_BBOX;
    self->svFlags &= ~SVF_NOCLIENT;
    self->s.effects |= EF_ROCKET;
    self->Use = NULL;
    self->moveType = MOVETYPE_TOSS;
    self->PreThink = misc_viper_bomb_prethink;
    self->Touch = misc_viper_bomb_touch;
    self->activator = activator;

    viper = G_Find(NULL, FOFS(classname), "misc_viper");
    VectorScale(viper->moveInfo.dir, viper->moveInfo.speed, self->velocity);

    self->timestamp = level.time;
    VectorCopy(viper->moveInfo.dir, self->moveInfo.dir);
}

void SP_misc_viper_bomb(entity_t* self)
{
    self->moveType = MOVETYPE_NONE;
    self->solid = SOLID_NOT;
    VectorSet(self->mins, -8, -8, -8);
    VectorSet(self->maxs, 8, 8, 8);

    self->s.modelindex = gi.ModelIndex("models/objects/bomb/tris.md2");

    if (!self->dmg)
        self->dmg = 1000;

    self->Use = misc_viper_bomb_use;
    self->svFlags |= SVF_NOCLIENT;

    gi.LinkEntity(self);
}