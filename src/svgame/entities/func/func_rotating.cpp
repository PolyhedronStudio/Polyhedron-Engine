// LICENSE HERE.

//
// svgame/entities/func_rotating.c
//
//
// func_rotating entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../brushfuncs.h"

//=====================================================
/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS TOUCH_PAIN STOP ANIMATED ANIMATED_FAST
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"damage"   damage to inflict when Blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
STOP mean it will stop moving instead of pushing entities
*/

void rotating_blocked(Entity* self, Entity* other)
{
    SVG_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, self->damage, 1, 0, MeansOfDeath::Crush);
}

void rotating_touch(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf)
{
    if (self->angularVelocity[0] || self->angularVelocity[1] || self->angularVelocity[2])
        SVG_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, self->damage, 1, 0, MeansOfDeath::Crush);
}

void rotating_use(Entity* self, Entity* other, Entity* activator)
{
    if (!VectorCompare(self->angularVelocity, vec3_origin)) {
        self->state.sound = 0;
        VectorClear(self->angularVelocity);
        self->Touch = NULL;
    }
    else {
        self->state.sound = self->moveInfo.middleSoundIndex;
        VectorScale(self->moveDirection, self->speed, self->angularVelocity);
        if (self->spawnFlags & 16)
            self->Touch = rotating_touch;
    }
}

void SP_func_rotating(Entity* ent)
{
    ent->solid = Solid::BSP;
    if (ent->spawnFlags & 32)
        ent->moveType = MoveType::Stop;
    else
        ent->moveType = MoveType::Push;

    // set the axis of rotation
    VectorClear(ent->moveDirection);
    if (ent->spawnFlags & 4)
        ent->moveDirection[2] = 1.0;
    else if (ent->spawnFlags & 8)
        ent->moveDirection[0] = 1.0;
    else // Z_AXIS
        ent->moveDirection[1] = 1.0;

    // check for reverse rotation
    if (ent->spawnFlags & 2)
        VectorNegate(ent->moveDirection, ent->moveDirection);

    if (!ent->speed)
        ent->speed = 100;
    if (!ent->damage)
        ent->damage = 2;

    //  ent->moveInfo.middleSoundIndex = "doors/hydro1.wav";

    ent->Use = rotating_use;
    if (ent->damage)
        ent->Blocked = rotating_blocked;

    if (ent->spawnFlags & 1)
        ent->Use(ent, NULL, NULL);

    if (ent->spawnFlags & 64)
        ent->state.effects |= EntityEffectType::AnimCycleAll2hz;
    if (ent->spawnFlags & 128)
        ent->state.effects |= EntityEffectType::AnimCycleAll30hz;

    gi.SetModel(ent, ent->model);
    gi.LinkEntity(ent);
}