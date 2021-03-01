// LICENSE HERE.

//
// svgame/entities/func_rotating.c
//
//
// func_rotating entity implementation.
//

// Include local game header.
#include "../g_local.h"
#include "../brushfuncs.h"

//=====================================================
/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"dmg"   damage to inflict when blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
STOP mean it will stop moving instead of pushing entities
*/

void rotating_blocked(edict_t* self, edict_t* other)
{
    T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void rotating_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    if (self->avelocity[0] || self->avelocity[1] || self->avelocity[2])
        T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void rotating_use(edict_t* self, edict_t* other, edict_t* activator)
{
    if (!VectorCompare(self->avelocity, vec3_origin)) {
        self->s.sound = 0;
        VectorClear(self->avelocity);
        self->touch = NULL;
    }
    else {
        self->s.sound = self->moveinfo.sound_middle;
        VectorScale(self->movedir, self->speed, self->avelocity);
        if (self->spawnflags & 16)
            self->touch = rotating_touch;
    }
}

void SP_func_rotating(edict_t* ent)
{
    ent->solid = SOLID_BSP;
    if (ent->spawnflags & 32)
        ent->movetype = MOVETYPE_STOP;
    else
        ent->movetype = MOVETYPE_PUSH;

    // set the axis of rotation
    VectorClear(ent->movedir);
    if (ent->spawnflags & 4)
        ent->movedir[2] = 1.0;
    else if (ent->spawnflags & 8)
        ent->movedir[0] = 1.0;
    else // Z_AXIS
        ent->movedir[1] = 1.0;

    // check for reverse rotation
    if (ent->spawnflags & 2)
        VectorNegate(ent->movedir, ent->movedir);

    if (!ent->speed)
        ent->speed = 100;
    if (!ent->dmg)
        ent->dmg = 2;

    //  ent->moveinfo.sound_middle = "doors/hydro1.wav";

    ent->use = rotating_use;
    if (ent->dmg)
        ent->blocked = rotating_blocked;

    if (ent->spawnflags & 1)
        ent->use(ent, NULL, NULL);

    if (ent->spawnflags & 64)
        ent->s.effects |= EF_ANIM_ALL;
    if (ent->spawnflags & 128)
        ent->s.effects |= EF_ANIM_ALLFAST;

    gi.setmodel(ent, ent->model);
    gi.linkentity(ent);
}