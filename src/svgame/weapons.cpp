/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "g_local.h"
#include "utils.h"

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
static void check_dodge(entity_t *self, const vec3_t &start, const vec3_t &dir, int speed)
{
    vec3_t  end;
    vec3_t  v;
    trace_t tr;
    float   eta;

    // easy mode only ducks one quarter the time
    if (skill->value == 0) {
        if (random() > 0.25)
            return;
    }
    VectorMA(start, 8192, dir, end);
    tr = gi.Trace(start, vec3_origin, vec3_origin, end, self, CONTENTS_MASK_SHOT);
    if ((tr.ent) && (tr.ent->svFlags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterInfo.dodge) && infront(tr.ent, self)) {
        VectorSubtract(tr.endpos, start, v);
        eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
        tr.ent->monsterInfo.dodge(tr.ent, self, eta);
    }
}


/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit(entity_t *self, vec3_t &aim, int damage, int kick)
{
    trace_t     tr;
    vec3_t      forward, right, up;
    vec3_t      v;
    vec3_t      point;
    float       range;
    vec3_t      dir;

    // Make sure we have an enemy..
    if (!self->enemy)
        return false;

    //see if enemy is in range
    VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
    range = VectorLength(dir);
    if (range > aim[0])
        return false;

    if (aim[1] > self->mins[0] && aim[1] < self->maxs[0]) {
        // the hit is straight on so back the range up to the edge of their bbox
        range -= self->enemy->maxs[0];
    } else {
        // this is a side hit so adjust the "right" value out to the edge of their bbox
        if (aim[1] < 0)
            aim[1] = self->enemy->mins[0];
        else
            aim[1] = self->enemy->maxs[0];
    }

    VectorMA(self->s.origin, range, dir, point);

    tr = gi.Trace(self->s.origin, vec3_origin, vec3_origin, point, self, CONTENTS_MASK_SHOT);
    if (tr.fraction < 1) {
        if (!tr.ent->takedamage)
            return false;
        // if it will hit any client/monster then hit the one we wanted to hit
        if ((tr.ent->svFlags & SVF_MONSTER) || (tr.ent->client))
            tr.ent = self->enemy;
    }

    AngleVectors(self->s.angles, &forward, &right, &up);
    VectorMA(self->s.origin, range, forward, point);
    VectorMA(point, aim[1], right, point);
    VectorMA(point, aim[2], up, point);
    VectorSubtract(point, self->enemy->s.origin, dir);

    // do the damage
    T_Damage(tr.ent, self, self, dir, point, vec3_origin, damage, kick / 2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

    if (!(tr.ent->svFlags & SVF_MONSTER) && (!tr.ent->client))
        return false;

    // do our special form of knockback here
    VectorMA(self->enemy->absMin, 0.5, self->enemy->size, v);
    VectorSubtract(v, point, v);
    VectorNormalize(v);
    VectorMA(self->enemy->velocity, kick, v, self->enemy->velocity);
    if (self->enemy->velocity[2] > 0)
        self->enemy->groundEntityPtr = NULL;
    return true;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
    trace_t     tr;
    vec3_t      dir;
    vec3_t      forward, right, up;
    vec3_t      end;
    float       r;
    float       u;
    vec3_t      water_start;
    qboolean    water = false;
    int         content_mask = CONTENTS_MASK_SHOT | CONTENTS_MASK_LIQUID;

    tr = gi.Trace(self->s.origin, vec3_origin, vec3_origin, start, self, CONTENTS_MASK_SHOT);
    if (!(tr.fraction < 1.0)) {
        vectoangles(aimdir, dir);
        AngleVectors(dir, &forward, &right, &up);

        r = crandom() * hspread;
        u = crandom() * vspread;
        VectorMA(start, 8192, forward, end);
        VectorMA(end, r, right, end);
        VectorMA(end, u, up, end);

        if (gi.PointContents(start) & CONTENTS_MASK_LIQUID) {
            water = true;
            VectorCopy(start, water_start);
            content_mask &= ~CONTENTS_MASK_LIQUID;
        }

        tr = gi.Trace(start, vec3_origin, vec3_origin, end, self, content_mask);

        // see if we hit water
        if (tr.contents & CONTENTS_MASK_LIQUID) {
            int     color;

            water = true;
            VectorCopy(tr.endpos, water_start);

            if (!VectorCompare(start, tr.endpos)) {
                if (tr.contents & CONTENTS_WATER) {
                    if (strcmp(tr.surface->name, "*brwater") == 0)
                        color = SPLASH_BROWN_WATER;
                    else
                        color = SPLASH_BLUE_WATER;
                } else if (tr.contents & CONTENTS_SLIME)
                    color = SPLASH_SLIME;
                else if (tr.contents & CONTENTS_LAVA)
                    color = SPLASH_LAVA;
                else
                    color = SPLASH_UNKNOWN;

                if (color != SPLASH_UNKNOWN) {
                    gi.WriteByte(svg_temp_entity);
                    gi.WriteByte(TE_SPLASH);
                    gi.WriteByte(8);
                    gi.WritePosition(tr.endpos);
                    gi.WriteDirection(tr.plane.normal);
                    gi.WriteByte(color);
                    gi.Multicast(&tr.endpos, MULTICAST_PVS);
                }

                // change bullet's course when it enters water
                VectorSubtract(end, start, dir);
                vectoangles(dir, dir);
                AngleVectors(dir, &forward, &right, &up);
                r = crandom() * hspread * 2;
                u = crandom() * vspread * 2;
                VectorMA(water_start, 8192, forward, end);
                VectorMA(end, r, right, end);
                VectorMA(end, u, up, end);
            }

            // re-trace ignoring water this time
            tr = gi.Trace(water_start, vec3_origin, vec3_origin, end, self, CONTENTS_MASK_SHOT);
        }
    }

    // send gun puff / flash
    if (!((tr.surface) && (tr.surface->flags & SURF_SKY))) {
        if (tr.fraction < 1.0) {
            if (tr.ent->takedamage) {
                T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
            } else {
                if (strncmp(tr.surface->name, "sky", 3) != 0) {
                    gi.WriteByte(svg_temp_entity);
                    gi.WriteByte(te_impact);
                    gi.WritePosition(tr.endpos);
                    gi.WriteDirection(tr.plane.normal);
                    gi.Multicast(&tr.endpos, MULTICAST_PVS);

                    if (self->client)
                        PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
                }
            }
        }
    }

    // if went through water, determine where the end and make a bubble trail
    if (water) {
        vec3_t  pos;

        VectorSubtract(tr.endpos, water_start, dir);
        VectorNormalize(dir);
        VectorMA(tr.endpos, -2, dir, pos);
        if (gi.PointContents(pos) & CONTENTS_MASK_LIQUID)
            VectorCopy(pos, tr.endpos);
        else
            tr = gi.Trace(pos, vec3_origin, vec3_origin, water_start, tr.ent, CONTENTS_MASK_LIQUID);

        VectorAdd(water_start, tr.endpos, pos);
        VectorScale(pos, 0.5, pos);

        gi.WriteByte(svg_temp_entity);
        gi.WriteByte(TE_BUBBLETRAIL);
        gi.WritePosition(water_start);
        gi.WritePosition(tr.endpos);
        gi.Multicast(&pos, MULTICAST_PVS);
    }
}


/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
    fire_lead(self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
    int     i;

    for (i = 0; i < count; i++)
        fire_lead(self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch(entity_t *self, entity_t *other, cplane_t *plane, csurface_t *surf)
{
    int mod;

    // N&C: From Yamagi Q2, this seems to resolve our random crashes at times.
    if (!self || !other) // Plane and Surf can be NULL
    {
        G_FreeEntity(self);
        return;
    }

    if (other == self->owner)
        return;

    if (surf && (surf->flags & SURF_SKY)) {
        G_FreeEntity(self);
        return;
    }

    if (self->owner->client)
        PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

    if (other->takedamage) {
        if (self->spawnFlags & 1)
            mod = MOD_HYPERBLASTER;
        else
            mod = MOD_BLASTER;

        // N&C: Fix for when there is no plane to base a normal of. (Taken from Yamagi Q2)
        if (plane)
        {
            T_Damage(other, self, self->owner, self->velocity, self->s.origin,
                plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
        }
        else
        {
            T_Damage(other, self, self->owner, self->velocity, self->s.origin,
                vec3_zero(), self->dmg, 1, DAMAGE_ENERGY, mod);
        }

    } else {
        gi.WriteByte(svg_temp_entity);
        gi.WriteByte(TE_BLASTER);
        gi.WritePosition(self->s.origin);
        if (!plane)
            gi.WriteDirection(vec3_zero());
        else
            gi.WriteDirection(plane->normal);
        gi.Multicast(&self->s.origin, MULTICAST_PVS);
    }

    G_FreeEntity(self);
}

//
//===============
// fire_blaster
//
// Fire projectiles as deadmonsters, so that if prediction is used against 
// the objects the player isn't solid clipped against it. 
// Otherwise, running into a firing hyperblaster is very jerky since you'd be
// predicted against the shots.
//===============
//
void fire_blaster(entity_t *self, const vec3_t& start, const vec3_t &aimdir, int damage, int speed, int effect, qboolean hyper)
{
    entity_t *bolt;
    trace_t tr;
    
    // Calculate direction vector.
    vec3_t dir = vec3_normalize(aimdir);

    // Fetch first free entity slot to use, by calling G_Spawn.
    bolt = G_Spawn();

    // Setup basic entity attributes.
    bolt->classname = "bolt";   // Classname.
    bolt->owner = self;         // Setup owner.
    bolt->dmg = damage;         // Setup damage.
    if (hyper)                  // Hyperblaster?
        bolt->spawnFlags = 1;
    bolt->svFlags = SVF_DEADMONSTER;    // Set Dead Monster flag so the projectiles 
                                        // won't clip against players.
    bolt->moveType = MOVETYPE_FLYMISSILE;   // Movetype FLYMISSILE
    bolt->clipMask = CONTENTS_MASK_SHOT;    // CONTENTS_MASK_SHOT
    bolt->solid = SOLID_BBOX;               // SOLID_BBOX
    bolt->s.effects |= effect;              // Apply effect argument to entity.
    
    // Setup entity physics attribute values.
    bolt->s.origin = start;     // Initial origin.
    bolt->s.old_origin = start; // Initial origin, same to origin, since this entity had no frame life yet.
    bolt->s.angles = vec3_euler(dir);       // Calculate euler radian entity satate angles.
    bolt->velocity = vec3_scale(dir, speed);// Calculate entity state velocity.
    bolt->mins = vec3_zero();   // Clear bounding mins.
    bolt->maxs = vec3_zero();   // Clear bounding maxs.

    // Setup (and precache) sound, and model.
    bolt->s.modelindex = gi.ModelIndex("models/objects/laser/tris.md2");
    bolt->s.sound = gi.SoundIndex("misc/lasfly.wav");
       
    // Setup touch and Think function pointers.
    bolt->Touch = blaster_touch;
    bolt->nextThink = level.time + 2;
    bolt->Think = G_FreeEntity;
    
    // Link entity in for collision.
    gi.LinkEntity(bolt);

    // If a client is firing this bolt, let AI check for dodges.
    if (self->client)
        check_dodge(self, bolt->s.origin, dir, speed);

    // Trace bolt.
    tr = gi.Trace(self->s.origin, vec3_zero(), vec3_zero(), bolt->s.origin, bolt, CONTENTS_MASK_SHOT);

    // Did we hit anything?
    if (tr.fraction < 1.0) {
        bolt->s.origin = vec3_fmaf(bolt->s.origin, -10, dir);
        bolt->Touch(bolt, tr.ent, NULL, NULL);
    }
}


/*
=================
fire_grenade
=================
*/
void Grenade_Explode(entity_t *ent)
{
    vec3_t      origin;
    int         mod;

    if (ent->owner->client)
        PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    //FIXME: if we are onground then raise our Z just a bit since we are a point?
    if (ent->enemy) {
        float   points;
        vec3_t  v;
        vec3_t  dir;

        VectorAdd(ent->enemy->mins, ent->enemy->maxs, v);
        VectorMA(ent->enemy->s.origin, 0.5, v, v);
        VectorSubtract(ent->s.origin, v, v);
        points = ent->dmg - 0.5 * VectorLength(v);
        VectorSubtract(ent->enemy->s.origin, ent->s.origin, dir);
        if (ent->spawnFlags & 1)
            mod = MOD_HANDGRENADE;
        else
            mod = MOD_GRENADE;
        T_Damage(ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
    }

    if (ent->spawnFlags & 2)
        mod = MOD_HELD_GRENADE;
    else if (ent->spawnFlags & 1)
        mod = MOD_HG_SPLASH;
    else
        mod = MOD_G_SPLASH;
    T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

    VectorMA(ent->s.origin, -0.02, ent->velocity, origin);
    gi.WriteByte(svg_temp_entity);
    if (ent->waterLevel) {
        if (ent->groundEntityPtr)
            gi.WriteByte(TE_GRENADE_EXPLOSION_WATER);
        else
            gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
    } else {
        if (ent->groundEntityPtr)
            gi.WriteByte(TE_GRENADE_EXPLOSION);
        else
            gi.WriteByte(TE_ROCKET_EXPLOSION);
    }
    gi.WritePosition(origin);
    gi.Multicast(&ent->s.origin, MULTICAST_PHS);

    G_FreeEntity(ent);
}

void Grenade_Touch(entity_t *ent, entity_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other == ent->owner)
        return;

    if (surf && (surf->flags & SURF_SKY)) {
        G_FreeEntity(ent);
        return;
    }

    if (!other->takedamage) {
        if (ent->spawnFlags & 1) {
            if (random() > 0.5)
                gi.Sound(ent, CHAN_VOICE, gi.SoundIndex("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
            else
                gi.Sound(ent, CHAN_VOICE, gi.SoundIndex("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
        } else {
            gi.Sound(ent, CHAN_VOICE, gi.SoundIndex("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
        }
        return;
    }

    ent->enemy = other;
    Grenade_Explode(ent);
}

void fire_grenade(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int speed, float timer, float damage_radius)
{
    entity_t *grenade;
    vec3_t  dir;
    vec3_t  forward, right, up;

    vectoangles(aimdir, dir);
    AngleVectors(dir, &forward, &right, &up);

    grenade = G_Spawn();
    VectorCopy(start, grenade->s.origin);
    VectorScale(aimdir, speed, grenade->velocity);
    VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
    VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);
    VectorSet(grenade->avelocity, 300, 300, 300);
    grenade->moveType = MOVETYPE_BOUNCE;
    grenade->clipMask = CONTENTS_MASK_SHOT;
    grenade->solid = SOLID_BBOX;
    grenade->s.effects |= EF_GRENADE;
    VectorClear(grenade->mins);
    VectorClear(grenade->maxs);
    grenade->s.modelindex = gi.ModelIndex("models/objects/grenade/tris.md2");
    grenade->owner = self;
    grenade->Touch = Grenade_Touch;
    grenade->nextThink = level.time + timer;
    grenade->Think = Grenade_Explode;
    grenade->dmg = damage;
    grenade->dmg_radius = damage_radius;
    grenade->classname = "grenade";

    gi.LinkEntity(grenade);
}

void fire_grenade2(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
    entity_t *grenade;
    vec3_t  dir;
    vec3_t  forward, right, up;

    vectoangles(aimdir, dir);
    AngleVectors(dir, &forward, &right, &up);

    grenade = G_Spawn();
    VectorCopy(start, grenade->s.origin);
    VectorScale(aimdir, speed, grenade->velocity);
    VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
    VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);
    VectorSet(grenade->avelocity, 300, 300, 300);
    grenade->moveType = MOVETYPE_BOUNCE;
    grenade->clipMask = CONTENTS_MASK_SHOT;
    grenade->solid = SOLID_BBOX;
    grenade->s.effects |= EF_GRENADE;
    VectorClear(grenade->mins);
    VectorClear(grenade->maxs);
    grenade->s.modelindex = gi.ModelIndex("models/objects/grenade2/tris.md2");
    grenade->owner = self;
    grenade->Touch = Grenade_Touch;
    grenade->nextThink = level.time + timer;
    grenade->Think = Grenade_Explode;
    grenade->dmg = damage;
    grenade->dmg_radius = damage_radius;
    grenade->classname = "hgrenade";
    if (held)
        grenade->spawnFlags = 3;
    else
        grenade->spawnFlags = 1;
    grenade->s.sound = gi.SoundIndex("weapons/hgrenc1b.wav");

    if (timer <= 0.0)
        Grenade_Explode(grenade);
    else {
        gi.Sound(self, CHAN_WEAPON, gi.SoundIndex("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
        gi.LinkEntity(grenade);
    }
}


/*
=================
fire_rocket
=================
*/
void rocket_touch(entity_t *ent, entity_t *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t      origin;
    int         n;

    if (other == ent->owner)
        return;

    if (surf && (surf->flags & SURF_SKY)) {
        G_FreeEntity(ent);
        return;
    }

    if (ent->owner->client)
        PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    // calculate position for the explosion entity
    VectorMA(ent->s.origin, -0.02, ent->velocity, origin);

    if (other->takedamage) {
        T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
    } else {
        // don't throw any debris in net games
        if (!deathmatch->value && !coop->value) {
            if ((surf) && !(surf->flags & (SURF_WARP | SURF_TRANS33 | SURF_TRANS66 | SURF_FLOWING))) {
                n = rand() % 5;
                while (n--)
                    ThrowDebris(ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
            }
        }
    }

    T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

    gi.WriteByte(svg_temp_entity);
    if (ent->waterLevel)
        gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
    else
        gi.WriteByte(TE_ROCKET_EXPLOSION);
    gi.WritePosition(origin);
    gi.Multicast(&ent->s.origin, MULTICAST_PHS);

    G_FreeEntity(ent);
}

void fire_rocket(entity_t *self, const vec3_t& start, const vec3_t& dir, int damage, int speed, float damage_radius, int radius_damage)
{
    entity_t *rocket;

    rocket = G_Spawn();
    VectorCopy(start, rocket->s.origin);
    VectorCopy(dir, rocket->moveDirection);
    vectoangles(dir, rocket->s.angles);
    VectorScale(dir, speed, rocket->velocity);
    rocket->moveType = MOVETYPE_FLYMISSILE;
    rocket->clipMask = CONTENTS_MASK_SHOT;
    rocket->solid = SOLID_BBOX;
    rocket->s.effects |= EF_ROCKET;
    VectorClear(rocket->mins);
    VectorClear(rocket->maxs);
    rocket->s.modelindex = gi.ModelIndex("models/objects/rocket/tris.md2");
    rocket->owner = self;
    rocket->Touch = rocket_touch;
    rocket->nextThink = level.time + 8000 / speed;
    rocket->Think = G_FreeEntity;
    rocket->dmg = damage;
    rocket->radius_dmg = radius_damage;
    rocket->dmg_radius = damage_radius;
    rocket->s.sound = gi.SoundIndex("weapons/rockfly.wav");
    rocket->classname = "rocket";

    if (self->client)
        check_dodge(self, rocket->s.origin, dir, speed);

    gi.LinkEntity(rocket);
}


/*
=================
fire_rail
=================
*/
void fire_rail(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick)
{
    vec3_t      from;
    vec3_t      end;
    trace_t     tr;
    entity_t     *ignore;
    int         mask;
    qboolean    water;

    VectorMA(start, 8192, aimdir, end);
    VectorCopy(start, from);
    ignore = self;
    water = false;
    mask = CONTENTS_MASK_SHOT | CONTENTS_SLIME | CONTENTS_LAVA;
    while (ignore) {
        tr = gi.Trace(from, vec3_origin, vec3_origin, end, ignore, mask);

        if (tr.contents & (CONTENTS_SLIME | CONTENTS_LAVA)) {
            mask &= ~(CONTENTS_SLIME | CONTENTS_LAVA);
            water = true;
        } else {
            //ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
            if ((tr.ent->svFlags & SVF_MONSTER) || (tr.ent->client) ||
                (tr.ent->solid == SOLID_BBOX))
                ignore = tr.ent;
            else
                ignore = NULL;

            if ((tr.ent != self) && (tr.ent->takedamage))
                T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
        }

        VectorCopy(tr.endpos, from);
    }

    // send gun puff / flash
    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TE_RAILTRAIL);
    gi.WritePosition(start);
    gi.WritePosition(tr.endpos);
    gi.Multicast(&self->s.origin, MULTICAST_PHS);
//  gi.multicast (start, MULTICAST_PHS);
    if (water) {
        gi.WriteByte(svg_temp_entity);
        gi.WriteByte(TE_RAILTRAIL);
        gi.WritePosition(start);
        gi.WritePosition(tr.endpos);
        gi.Multicast(&tr.endpos, MULTICAST_PHS);
    }

    if (self->client)
        PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}


/*
=================
fire_bfg
=================
*/
void bfg_explode(entity_t *self)
{
    entity_t *ent;
    float   points;
    vec3_t  v;
    float   dist;

    if (self->s.frame == 0) {
        // the BFG effect
        ent = NULL;
        while ((ent = G_FindEntitiesWithinRadius(ent, self->s.origin, self->dmg_radius)) != NULL) {
            if (!ent->takedamage)
                continue;
            if (ent == self->owner)
                continue;
            if (!CanDamage(ent, self))
                continue;
            if (!CanDamage(ent, self->owner))
                continue;

            VectorAdd(ent->mins, ent->maxs, v);
            VectorMA(ent->s.origin, 0.5, v, v);
            VectorSubtract(self->s.origin, v, v);
            dist = VectorLength(v);
            points = self->radius_dmg * (1.0 - std::sqrtf(dist / self->dmg_radius));
            if (ent == self->owner)
                points = points * 0.5;

            gi.WriteByte(svg_temp_entity);
            gi.WriteByte(TE_BFG_EXPLOSION);
            gi.WritePosition(ent->s.origin);
            gi.Multicast(&ent->s.origin, MULTICAST_PHS);
            T_Damage(ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
        }
    }

    self->nextThink = level.time + FRAMETIME;
    self->s.frame++;
    if (self->s.frame == 5)
        self->Think = G_FreeEntity;
}

void bfg_touch(entity_t *self, entity_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other == self->owner)
        return;

    if (surf && (surf->flags & SURF_SKY)) {
        G_FreeEntity(self);
        return;
    }

    if (self->owner->client)
        PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

    // core explosion - prevents firing it into the wall/floor
    if (other->takedamage)
        T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_BFG_BLAST);
    T_RadiusDamage(self, self->owner, 200, other, 100, MOD_BFG_BLAST);

    gi.Sound(self, CHAN_VOICE, gi.SoundIndex("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
    self->solid = SOLID_NOT;
    self->Touch = NULL;
    VectorMA(self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
    VectorClear(self->velocity);
    self->s.modelindex = gi.ModelIndex("sprites/s_bfg3.sp2");
    self->s.frame = 0;
    self->s.sound = 0;
    self->s.effects &= ~EF_ANIM_ALLFAST;
    self->Think = bfg_explode;
    self->nextThink = level.time + FRAMETIME;
    self->enemy = other;

    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TE_BFG_BIGEXPLOSION);
    gi.WritePosition(self->s.origin);
    gi.Multicast(&self->s.origin, MULTICAST_PVS);
}


void bfg_think(entity_t *self)
{
    entity_t *ent;
    entity_t *ignore;
    vec3_t  point;
    vec3_t  dir;
    vec3_t  start;
    vec3_t  end;
    int     dmg;
    trace_t tr;

    if (deathmatch->value)
        dmg = 5;
    else
        dmg = 10;

    ent = NULL;
    while ((ent = G_FindEntitiesWithinRadius(ent, self->s.origin, 256)) != NULL) {
        if (ent == self)
            continue;

        if (ent == self->owner)
            continue;

        if (!ent->takedamage)
            continue;

        if (!(ent->svFlags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
            continue;

        VectorMA(ent->absMin, 0.5, ent->size, point);

        VectorSubtract(point, self->s.origin, dir);
        VectorNormalize(dir);

        ignore = self;
        VectorCopy(self->s.origin, start);
        VectorMA(start, 2048, dir, end);
        while (1) {
            tr = gi.Trace(start, vec3_origin, vec3_origin, end, ignore, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

            if (!tr.ent)
                break;

            // hurt it if we can
            if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
                T_Damage(tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

            // if we hit something that's not a monster or player we're done
            if (!(tr.ent->svFlags & SVF_MONSTER) && (!tr.ent->client)) {
                gi.WriteByte(svg_temp_entity);
                gi.WriteByte(TE_LASER_SPARKS);
                gi.WriteByte(4);
                gi.WritePosition(tr.endpos);
                gi.WriteDirection(tr.plane.normal);
                gi.WriteByte(self->s.skinnum);
                gi.Multicast(&tr.endpos, MULTICAST_PVS);
                break;
            }

            ignore = tr.ent;
            VectorCopy(tr.endpos, start);
        }

        gi.WriteByte(svg_temp_entity);
        gi.WriteByte(TE_BFG_LASER);
        gi.WritePosition(self->s.origin);
        gi.WritePosition(tr.endpos);
        gi.Multicast(&self->s.origin, MULTICAST_PHS);
    }

    self->nextThink = level.time + FRAMETIME;
}


void fire_bfg(entity_t *self, const vec3_t& start, const vec3_t& dir, int damage, int speed, float damage_radius)
{
    entity_t *bfg;

    bfg = G_Spawn();
    VectorCopy(start, bfg->s.origin);
    VectorCopy(dir, bfg->moveDirection);
    vectoangles(dir, bfg->s.angles);
    VectorScale(dir, speed, bfg->velocity);
    bfg->moveType = MOVETYPE_FLYMISSILE;
    bfg->clipMask = CONTENTS_MASK_SHOT;
    bfg->solid = SOLID_BBOX;
    bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
    VectorClear(bfg->mins);
    VectorClear(bfg->maxs);
    bfg->s.modelindex = gi.ModelIndex("sprites/s_bfg1.sp2");
    bfg->owner = self;
    bfg->Touch = bfg_touch;
    bfg->nextThink = level.time + 8000 / speed;
    bfg->Think = G_FreeEntity;
    bfg->radius_dmg = damage;
    bfg->dmg_radius = damage_radius;
    bfg->classname = "bfg blast";
    bfg->s.sound = gi.SoundIndex("weapons/bfg__l1a.wav");

    bfg->Think = bfg_think;
    bfg->nextThink = level.time + FRAMETIME;
    bfg->teamMasterPtr = bfg;
    bfg->teamChainPtr = NULL;

    if (self->client)
        check_dodge(self, bfg->s.origin, dir, speed);

    gi.LinkEntity(bfg);
}

/*
 * Drops a spark from the flare flying thru the air.  Checks to make
 * sure we aren't in the water.
 */
void flare_sparks(entity_t *self)
{
	vec3_t dir;
	vec3_t forward, right, up;
	// Spawn some sparks.  This isn't net-friendly at all, but will 
	// be fine for single player. 
	// 
	gi.WriteByte(svg_temp_entity);
	gi.WriteByte(TE_FLARE);

    gi.WriteShort(self - g_edicts);
    // if this is the first tick of flare, set count to 1 to start the sound
    gi.WriteByte( self->timestamp - level.time < 14.75 ? 0 : 1);

    gi.WritePosition(self->s.origin);

	// If we are still moving, calculate the normal to the direction 
	 // we are travelling. 
	 // 
	if (VectorLength(self->velocity) > 0.0)
	{
		vectoangles(self->velocity, dir);
		AngleVectors(dir, &forward, &right, &up);

		gi.WriteDirection(up);
	}
	// If we're stopped, just write out the origin as our normal 
	// 
	else
	{
		gi.WriteDirection(vec3_origin);
	}
	gi.Multicast(&self->s.origin, MULTICAST_PVS);
}

/*
   void flare_think( entity_t *self )

   Purpose: The Think function of a flare round.  It generates sparks
			on the flare using a temp entity, and kills itself after
			self->timestamp runs out.
   Parameters:
	 self: A pointer to the entity_t structure representing the
		   flare round.  self->timestamp is the value used to
		   measure the lifespan of the round, and is set in
		   fire_flaregun blow.

   Notes:
	 - I'm not sure how much bandwidth is eaten by spawning a temp
	   entity every FRAMETIME seconds.  It might very well turn out
	   that the sparks need to go bye-bye in favor of less bandwidth
	   usage.  Then again, why the hell would you use this gun on
	   a DM server????

	 - I haven't seen self->timestamp used anywhere else in the code,
	   but I never really looked that hard.  It doesn't seem to cause
	   any problems, and is aptly named, so I used it.
 */
void flare_think(entity_t *self)
{
	// self->timestamp is 15 seconds after the flare was spawned. 
	// 
	if (level.time > self->timestamp)
	{
		G_FreeEntity(self);
		return;
	}

	// We're still active, so lets shoot some sparks. 
	// 
	flare_sparks(self);
	
	// We'll Think again in .2 seconds 
	// 
	self->nextThink = level.time + 0.2;
}

void flare_touch(entity_t *ent, entity_t *other,
	cplane_t *plane, csurface_t *surf)
{
	// Flares don't weigh that much, so let's have them stop 
	// the instant they whack into anything. 
	// 
	VectorClear(ent->velocity);
}

void fire_flaregun(entity_t *self, const vec3_t& start, const vec3_t& aimdir,
	int damage, int speed, float timer,
	float damage_radius)
{
	entity_t *flare;
	vec3_t dir;
	vec3_t forward, right, up;

	vectoangles(aimdir, dir);
	AngleVectors(dir, &forward, &right, &up);

	flare = G_Spawn();
	VectorCopy(start, flare->s.origin);
	VectorScale(aimdir, speed, flare->velocity);
	VectorSet(flare->avelocity, 300, 300, 300);
	flare->moveType = MOVETYPE_BOUNCE;
	flare->clipMask = CONTENTS_MASK_SHOT;
	flare->solid = SOLID_BBOX;

	const float size = 4;
	VectorSet(flare->mins, -size, -size, -size);
	VectorSet(flare->maxs, size, size, size);

	flare->s.modelindex = gi.ModelIndex("models/objects/flare/tris.md2");
	flare->owner = self;
	flare->Touch = flare_touch;
	flare->nextThink = FRAMETIME;
	flare->Think = flare_think;
	flare->radius_dmg = damage;
	flare->dmg_radius = damage_radius;
	flare->classname = "flare";
	flare->timestamp = level.time + 15.0; //live for 15 seconds 
	gi.LinkEntity(flare);
}