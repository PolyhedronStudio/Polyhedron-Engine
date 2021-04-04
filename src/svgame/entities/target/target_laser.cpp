// LICENSE HERE.

//
// svgame/entities/target_laser.c
//
//
// target_laser entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

//=====================================================
/*QUAKED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT
When triggered, fires a laser.  You can either set a target
or a direction.
*/

void target_laser_think(entity_t* self)
{
    entity_t* ignore;
    vec3_t  start;
    vec3_t  end;
    trace_t tr;
    vec3_t  point;
    vec3_t  last_movedir;
    int     count;

    if (self->spawnFlags & 0x80000000)
        count = 8;
    else
        count = 4;

    if (self->enemy) {
        VectorCopy(self->moveDirection, last_movedir);
        VectorMA(self->enemy->absMin, 0.5, self->enemy->size, point);
        VectorSubtract(point, self->s.origin, self->moveDirection);
        self->moveDirection = vec3_normalize(self->moveDirection);
        if (!VectorCompare(self->moveDirection, last_movedir))
            self->spawnFlags |= 0x80000000;
    }

    ignore = self;
    VectorCopy(self->s.origin, start);
    VectorMA(start, 2048, self->moveDirection, end);
    while (1) {
        tr = gi.Trace(start, vec3_zero(), vec3_zero(), end, ignore, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

        if (!tr.ent)
            break;

        // hurt it if we can
        if ((tr.ent->takeDamage) && !(tr.ent->flags & FL_IMMUNE_LASER))
            T_Damage(tr.ent, self, self->activator, self->moveDirection, tr.endPosition, vec3_zero(), self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER);

        // if we hit something that's not a monster or player or is immune to lasers, we're done
        if (!(tr.ent->svFlags & SVF_MONSTER) && (!tr.ent->client)) {
            if (self->spawnFlags & 0x80000000) {
                self->spawnFlags &= ~0x80000000;
                gi.WriteByte(svg_temp_entity);
                gi.WriteByte(TE_LASER_SPARKS);
                gi.WriteByte(count);
                gi.WritePosition(tr.endPosition);
                gi.WriteDirection(tr.plane.normal);
                gi.WriteByte(self->s.skinnum);
                gi.Multicast(&tr.endPosition, MULTICAST_PVS);
            }
            break;
        }

        ignore = tr.ent;
        VectorCopy(tr.endPosition, start);
    }

    VectorCopy(tr.endPosition, self->s.old_origin);

    self->nextThink = level.time + FRAMETIME;
}

void target_laser_on(entity_t* self)
{
    if (!self->activator)
        self->activator = self;
    self->spawnFlags |= 0x80000001;
    self->svFlags &= ~SVF_NOCLIENT;
    target_laser_think(self);
}

void target_laser_off(entity_t* self)
{
    self->spawnFlags &= ~1;
    self->svFlags |= SVF_NOCLIENT;
    self->nextThink = 0;
}

void target_laser_use(entity_t* self, entity_t* other, entity_t* activator)
{
    self->activator = activator;
    if (self->spawnFlags & 1)
        target_laser_off(self);
    else
        target_laser_on(self);
}

void target_laser_start(entity_t* self)
{
    entity_t* ent;

    self->moveType = MOVETYPE_NONE;
    self->solid = SOLID_NOT;
    self->s.renderfx |= RF_BEAM | RF_TRANSLUCENT;
    self->s.modelindex = 1;         // must be non-zero

    // set the beam diameter
    if (self->spawnFlags & 64)
        self->s.frame = 16;
    else
        self->s.frame = 4;

    // set the color
    if (self->spawnFlags & 2)
        self->s.skinnum = 0xf2f2f0f0;
    else if (self->spawnFlags & 4)
        self->s.skinnum = 0xd0d1d2d3;
    else if (self->spawnFlags & 8)
        self->s.skinnum = 0xf3f3f1f1;
    else if (self->spawnFlags & 16)
        self->s.skinnum = 0xdcdddedf;
    else if (self->spawnFlags & 32)
        self->s.skinnum = 0xe0e1e2e3;

    if (!self->enemy) {
        if (self->target) {
            ent = G_Find(NULL, FOFS(targetName), self->target);
            if (!ent)
                gi.DPrintf("%s at %s: %s is a bad target\n", self->classname, Vec3ToString(self->s.origin), self->target);
            self->enemy = ent;
        }
        else {
            UTIL_SetMoveDir(self->s.angles, self->moveDirection);
        }
    }
    self->Use = target_laser_use;
    self->Think = target_laser_think;

    if (!self->dmg)
        self->dmg = 1;

    VectorSet(self->mins, -8, -8, -8);
    VectorSet(self->maxs, 8, 8, 8);
    gi.LinkEntity(self);

    if (self->spawnFlags & 1)
        target_laser_on(self);
    else
        target_laser_off(self);
}

void SP_target_laser(entity_t* self)
{
    // let everything else get spawned before we start firing
    self->Think = target_laser_start;
    self->nextThink = level.time + 1;
}