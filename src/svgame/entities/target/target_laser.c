// LICENSE HERE.

//
// svgame/entities/target_laser.c
//
//
// target_laser entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT
When triggered, fires a laser.  You can either set a target
or a direction.
*/

void target_laser_think(edict_t* self)
{
    edict_t* ignore;
    vec3_t  start;
    vec3_t  end;
    trace_t tr;
    vec3_t  point;
    vec3_t  last_movedir;
    int     count;

    if (self->spawnflags & 0x80000000)
        count = 8;
    else
        count = 4;

    if (self->enemy) {
        VectorCopy(self->movedir, last_movedir);
        VectorMA(self->enemy->absmin, 0.5, self->enemy->size, point);
        VectorSubtract(point, self->s.origin, self->movedir);
        VectorNormalize(self->movedir);
        if (!VectorCompare(self->movedir, last_movedir))
            self->spawnflags |= 0x80000000;
    }

    ignore = self;
    VectorCopy(self->s.origin, start);
    VectorMA(start, 2048, self->movedir, end);
    while (1) {
        tr = gi.trace(start, NULL, NULL, end, ignore, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

        if (!tr.ent)
            break;

        // hurt it if we can
        if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER))
            T_Damage(tr.ent, self, self->activator, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER);

        // if we hit something that's not a monster or player or is immune to lasers, we're done
        if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client)) {
            if (self->spawnflags & 0x80000000) {
                self->spawnflags &= ~0x80000000;
                gi.WriteByte(svg_temp_entity);
                gi.WriteByte(TE_LASER_SPARKS);
                gi.WriteByte(count);
                gi.WritePosition(tr.endpos);
                gi.WriteDir(tr.plane.normal);
                gi.WriteByte(self->s.skinnum);
                gi.multicast(tr.endpos, MULTICAST_PVS);
            }
            break;
        }

        ignore = tr.ent;
        VectorCopy(tr.endpos, start);
    }

    VectorCopy(tr.endpos, self->s.old_origin);

    self->nextthink = level.time + FRAMETIME;
}

void target_laser_on(edict_t* self)
{
    if (!self->activator)
        self->activator = self;
    self->spawnflags |= 0x80000001;
    self->svflags &= ~SVF_NOCLIENT;
    target_laser_think(self);
}

void target_laser_off(edict_t* self)
{
    self->spawnflags &= ~1;
    self->svflags |= SVF_NOCLIENT;
    self->nextthink = 0;
}

void target_laser_use(edict_t* self, edict_t* other, edict_t* activator)
{
    self->activator = activator;
    if (self->spawnflags & 1)
        target_laser_off(self);
    else
        target_laser_on(self);
}

void target_laser_start(edict_t* self)
{
    edict_t* ent;

    self->movetype = MOVETYPE_NONE;
    self->solid = SOLID_NOT;
    self->s.renderfx |= RF_BEAM | RF_TRANSLUCENT;
    self->s.modelindex = 1;         // must be non-zero

    // set the beam diameter
    if (self->spawnflags & 64)
        self->s.frame = 16;
    else
        self->s.frame = 4;

    // set the color
    if (self->spawnflags & 2)
        self->s.skinnum = 0xf2f2f0f0;
    else if (self->spawnflags & 4)
        self->s.skinnum = 0xd0d1d2d3;
    else if (self->spawnflags & 8)
        self->s.skinnum = 0xf3f3f1f1;
    else if (self->spawnflags & 16)
        self->s.skinnum = 0xdcdddedf;
    else if (self->spawnflags & 32)
        self->s.skinnum = 0xe0e1e2e3;

    if (!self->enemy) {
        if (self->target) {
            ent = G_Find(NULL, FOFS(targetname), self->target);
            if (!ent)
                gi.dprintf("%s at %s: %s is a bad target\n", self->classname, vtos(self->s.origin), self->target);
            self->enemy = ent;
        }
        else {
            G_SetMovedir(self->s.angles, self->movedir);
        }
    }
    self->use = target_laser_use;
    self->think = target_laser_think;

    if (!self->dmg)
        self->dmg = 1;

    VectorSet(self->mins, -8, -8, -8);
    VectorSet(self->maxs, 8, 8, 8);
    gi.linkentity(self);

    if (self->spawnflags & 1)
        target_laser_on(self);
    else
        target_laser_off(self);
}

void SP_target_laser(edict_t* self)
{
    // let everything else get spawned before we start firing
    self->think = target_laser_start;
    self->nextthink = level.time + 1;
}