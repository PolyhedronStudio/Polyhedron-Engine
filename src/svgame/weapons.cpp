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
#include "entities.h"
#include "utils.h"

#include "entities/base/SVGBaseEntity.h"
#include "entities/weaponry/BlasterBolt.h"

/*
=================
TODO: Maybe... replace, rename, etc.
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
static void check_dodge(Entity *self, const vec3_t &start, const vec3_t &dir, int speed)
{
    //vec3_t  end;
    //vec3_t  v;
    //trace_t tr;
    //float   eta;

    //// easy mode only ducks one quarter the time
    //if (skill->value == 0) {
    //    if (random() > 0.25)
    //        return;
    //}
    //VectorMA(start, WORLD_SIZE, dir, end);
    //tr = gi.Trace(start, vec3_zero(), vec3_zero(), end, self, CONTENTS_MASK_SHOT);
    //if ((tr.ent) && (tr.ent->serverFlags & EntityServerFlags::Monster) && (tr.ent->health > 0) && (tr.ent->monsterInfo.dodge) && infront(tr.ent, self)) {
    //    VectorSubtract(tr.endPosition, start, v);
    //    eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
    //    
    //    //tr.ent->monsterInfo.dodge(tr.ent, self, eta);
    //}
}


/*
=================
SVG_FireHit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean SVG_FireHit(SVGBaseEntity *self, vec3_t &aim, int32_t damage, int32_t kick)
{
    SVGTrace     tr;
    vec3_t      forward, right, up;
    vec3_t      v;
    vec3_t      point;
    float       range;

    // Make sure we have an enemy..
    if (!self->GetEnemy())
        return false;

    //see if enemy is in range
    vec3_t dir = self->GetEnemy()->GetOrigin() - self->GetOrigin();
    range = VectorLength(dir);
    if (range > aim[0])
        return false;

    if (aim[1] > self->GetMins().x && aim[1] < self->GetMaxs().x) {
        // the hit is straight on so back the range up to the edge of their bbox
        range -= self->GetEnemy()->GetMaxs().x;
    } else {
        // this is a side hit so adjust the "right" value out to the edge of their bbox
        if (aim[1] < 0)
            aim[1] = self->GetEnemy()->GetMaxs().x;
        else
            aim[1] = self->GetEnemy()->GetMaxs().x;
    }

    point = vec3_fmaf(self->GetOrigin(), range, dir);

    tr = SVG_Trace(self->GetOrigin(), vec3_zero(), vec3_zero(), point, self, CONTENTS_MASK_SHOT);
    if (tr.fraction < 1) {
        if (!tr.ent->GetTakeDamage())
            return false;
        // if it will hit any client/monster then hit the one we wanted to hit
        if ((tr.ent->GetServerFlags() & EntityServerFlags::Monster) || (tr.ent->GetClient()))
            tr.ent = self->GetEnemy();
    }

    AngleVectors(self->GetServerEntity()->state.angles, &forward, &right, &up);
    point = vec3_fmaf(self->GetOrigin(), range, forward);
    point = vec3_fmaf(point, aim[1], right);
    point = vec3_fmaf(point, aim[2], up);
    dir = point - self->GetEnemy()->GetOrigin();

    // do the damage
    SVG_InflictDamage(tr.ent, self, self, dir, point, vec3_zero(), damage, kick / 2, DamageFlags::NoKnockBack, MeansOfDeath::Hit);

    if (!(tr.ent->GetServerFlags() & EntityServerFlags::Monster) && (!tr.ent->GetClient()))
        return false;

    // do our special form of knockback here
    v = vec3_fmaf(self->GetEnemy()->GetAbsoluteMin(), 0.5, self->GetEnemy()->GetSize());
    v = v - point;
    v = vec3_normalize(v);
    self->GetEnemy()->SetVelocity(vec3_fmaf(self->GetEnemy()->GetVelocity(), kick, v));
    if (self->GetEnemy()->GetVelocity().z > 0)
        self->GetEnemy()->SetGroundEntity(nullptr);
    return true;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead(SVGBaseEntity *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
    SVGTrace     tr;
    vec3_t      dir;
    vec3_t      forward, right, up;
    vec3_t      end;
    float       r;
    float       u;
    vec3_t      water_start;
    qboolean    water = false;
    int         content_mask = CONTENTS_MASK_SHOT | CONTENTS_MASK_LIQUID;

    tr = SVG_Trace(self->GetOrigin(), vec3_zero(), vec3_zero(), start, self, CONTENTS_MASK_SHOT);
    if (!(tr.fraction < 1.0)) {
        dir = vec3_euler(aimdir);
        AngleVectors(dir, &forward, &right, &up);

        r = crandom() * hspread;
        u = crandom() * vspread;
        VectorMA(start, WORLD_SIZE, forward, end);
        VectorMA(end, r, right, end);
        VectorMA(end, u, up, end);

        if (gi.PointContents(start) & CONTENTS_MASK_LIQUID) {
            water = true;
            VectorCopy(start, water_start);
            content_mask &= ~CONTENTS_MASK_LIQUID;
        }

        tr = SVG_Trace(start, vec3_zero(), vec3_zero(), end, self, content_mask);

        // see if we hit water
        if (tr.contents & CONTENTS_MASK_LIQUID) {
            int     color;

            water = true;
            VectorCopy(tr.endPosition, water_start);

            if (!VectorCompare(start, tr.endPosition)) {
                if (tr.contents & CONTENTS_WATER) {
                    if (strcmp(tr.surface->name, "*brwater") == 0)
                        color = SplashType::BrownWater;
                    else
                        color = SplashType::BlueWater;
                } else if (tr.contents & CONTENTS_SLIME)
                    color = SplashType::Slime;
                else if (tr.contents & CONTENTS_LAVA)
                    color = SplashType::Lava;
                else
                    color = SplashType::Unknown;

                if (color != SplashType::Unknown) {
                    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
                    gi.WriteByte(TempEntityEvent::Splash);
                    gi.WriteByte(8);
                    gi.WriteVector3(tr.endPosition);
                    gi.WriteVector3(tr.plane.normal);
                    gi.WriteByte(color);
                    gi.Multicast(tr.endPosition, MultiCast::PVS);
                }

                // change bullet's course when it enters water
                vec3_t dir = vec3_euler(end - start);
                AngleVectors(dir, &forward, &right, &up);
                r = crandom() * hspread * 2;
                u = crandom() * vspread * 2;
                VectorMA(water_start, WORLD_SIZE, forward, end);
                VectorMA(end, r, right, end);
                VectorMA(end, u, up, end);
            }

            // re-trace ignoring water this time
            tr = SVG_Trace(water_start, vec3_zero(), vec3_zero(), end, self, CONTENTS_MASK_SHOT);
        }
    }

    // send gun puff / flash
    if (!((tr.surface) && (tr.surface->flags & SURF_SKY))) {
        if (tr.fraction < 1.0) {
            if (tr.ent->GetTakeDamage()) {
                SVG_InflictDamage(tr.ent, self, self, aimdir, tr.endPosition, tr.plane.normal, damage, kick, DamageFlags::Bullet, mod);
            } else {
                if (strncmp(tr.surface->name, "sky", 3) != 0) {
                    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
                    gi.WriteByte(te_impact);
                    gi.WriteVector3(tr.endPosition);
                    gi.WriteVector3(tr.plane.normal);
                    gi.Multicast(tr.endPosition, MultiCast::PVS);

                    if (self->GetClient())
                        SVG_PlayerNoise(self, tr.endPosition, PNOISE_IMPACT);
                }
            }
        }
    }

    // if went through water, determine where the end and make a bubble trail
    if (water) {
        vec3_t  pos;

        VectorSubtract(tr.endPosition, water_start, dir);
        VectorNormalize(dir);
        VectorMA(tr.endPosition, -2, dir, pos);
        if (gi.PointContents(pos) & CONTENTS_MASK_LIQUID)
            VectorCopy(pos, tr.endPosition);
        else
            tr = SVG_Trace(pos, vec3_zero(), vec3_zero(), water_start, tr.ent, CONTENTS_MASK_LIQUID);

        VectorAdd(water_start, tr.endPosition, pos);
        VectorScale(pos, 0.5, pos);

        gi.WriteByte(SVG_CMD_TEMP_ENTITY);
        gi.WriteByte(TempEntityEvent::BubbleTrail);
        gi.WriteVector3(water_start);
        gi.WriteVector3(tr.endPosition);
        gi.Multicast(pos, MultiCast::PVS);
    }
}


/*
=================
SVG_FireBullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void SVG_FireBullet(SVGBaseEntity *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
    fire_lead(self, start, aimdir, damage, kick, TempEntityEvent::Gunshot, hspread, vspread, mod);
}

//
//===============
// SVG_FireShotgun
// 
// Shoots shotgun pellets.  Used by shotgun and super shotgun.
//===============
//
void SVG_FireShotgun(SVGBaseEntity* self, const vec3_t &start, const vec3_t &aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
    int		i;

    for (i = 0; i < count; i++)
        fire_lead(self, start, aimdir, damage, kick, TempEntityEvent::Shotgun, hspread, vspread, mod);
}

//
//===============
// SVG_FireBlaster
//
// Fire projectiles as deadmonsters, so that if prediction is used against 
// the objects the player isn't solid clipped against it. 
// Otherwise, running into a firing hyperblaster is very jerky since you'd be
// predicted against the shots.
//===============
//
void SVG_FireBlaster(SVGBaseEntity *self, const vec3_t& start, const vec3_t &aimdir, int damage, int speed, int effect, qboolean hyper)
{
    // Calculate direction vector.
    vec3_t dir = vec3_normalize(aimdir);
 
    // Spawn the blaster bolt server entity.
    BlasterBolt* boltEntity = SVG_CreateClassEntity<BlasterBolt>();

    // Welp. It can happen sometimes
    if ( nullptr == boltEntity ) {
        return;
    }

    // Admer: TODO: perform this setup directly in BlasterBolt, e.g.
    // BlasterBolt::Create( start, aimdir, damage, speed, effect, hyper );
    // This stuff down there is... really rotten

    // Basic attributes.
    boltEntity->Precache();
    boltEntity->Spawn();

    boltEntity->SetOwner(self);
    boltEntity->SetDamage(damage);
    if (hyper)
        boltEntity->SetSpawnFlags(1);
    boltEntity->SetMoveType(MoveType::FlyMissile);
    boltEntity->SetSolid(Solid::BoundingBox);
    boltEntity->SetClipMask(CONTENTS_MASK_SHOT);
    boltEntity->SetEffects(boltEntity->GetEffects() | effect);

    // Physics attributes.
    boltEntity->SetOrigin(start);    // Initial origin.
    boltEntity->SetOldOrigin(start); // Initial origin, same to origin, since this entity had no frame life yet.
    boltEntity->SetAngles(vec3_euler(dir));
    boltEntity->SetVelocity(vec3_scale(dir, speed));
    boltEntity->SetMins(vec3_zero()); // Clear bounding mins.
    boltEntity->SetMaxs(vec3_zero()); // clear bounding maxs.

    // Model/Sound attributes.
    boltEntity->SetModel("models/objects/laser/tris.md2");
    boltEntity->SetSound(SVG_PrecacheSound("misc/lasfly.wav"));

    // Set Touch and Think callbacks.
    boltEntity->SetTouchCallback(&BlasterBolt::BlasterBoltTouch);

    // Set think.
    boltEntity->SetNextThinkTime(level.time + 2); // Admer: should this really be a thing?
    boltEntity->SetThinkCallback(&SVGBaseEntity::SVGBaseEntityThinkFree);

    // Link Bolt into world.
    boltEntity->LinkEntity();

    // If a client is firing this bolt, let AI check for dodges.
    //if (self->client)
    //    check_dodge(self, bolt->state.origin, dir, speed);

    // Trace bolt.
    SVGTrace trace = SVG_Trace(self->GetOrigin(), vec3_zero(), vec3_zero(), boltEntity->GetOrigin(), boltEntity, CONTENTS_MASK_SHOT);

    // Did we hit anything?
    if (trace.fraction < 1.0) {
        boltEntity->SetOrigin(vec3_fmaf(boltEntity->GetOrigin(), -10, dir));
        boltEntity->Touch(boltEntity, trace.ent, NULL, NULL);
    }
//}
// 
    ////bolt->Touch = blaster_touch;
////bolt->nextThinkTime = level.time + 2;
////bolt->Think = SVG_FreeEntity;

        //// If a client is firing this bolt, let AI check for dodges.
    //if (self->client)
    //    check_dodge(self, bolt->state.origin, dir, speed);

    //// Trace bolt.
    //tr = gi.Trace(self->state.origin, vec3_zero(), vec3_zero(), bolt->state.origin, bolt, CONTENTS_MASK_SHOT);

    //// Did we hit anything?
    //if (tr.fraction < 1.0) {
    //    bolt->state.origin = vec3_fmaf(bolt->state.origin, -10, dir);
    //    //bolt->Touch(bolt, tr.ent, NULL, NULL);
    //}
    // 
    //// Setup (and precache) sound, and model.
    //bolt->state.modelIndex = gi.ModelIndex("models/objects/laser/tris.md2");
    //bolt->state.sound = gi.SoundIndex("misc/lasfly.wav");
    //   
    //// Setup touch and Think function pointers.
    ////bolt->Touch = blaster_touch;
    ////bolt->nextThinkTime = level.time + 2;
    ////bolt->Think = SVG_FreeEntity;
    //
    //// Link entity in for collision.
    //gi.LinkEntity(bolt);

    //// If a client is firing this bolt, let AI check for dodges.
    //if (self->client)
    //    check_dodge(self, bolt->state.origin, dir, speed);

    //// Trace bolt.
    //tr = gi.Trace(self->state.origin, vec3_zero(), vec3_zero(), bolt->state.origin, bolt, CONTENTS_MASK_SHOT);

    //// Did we hit anything?
    //if (tr.fraction < 1.0) {
    //    bolt->state.origin = vec3_fmaf(bolt->state.origin, -10, dir);
    //    //bolt->Touch(bolt, tr.ent, NULL, NULL);
    //}

    //Entity *bolt;
    //trace_t tr;
    //
    //// Calculate direction vector.
    //vec3_t dir = vec3_normalize(aimdir);

    //// Fetch first free entity slot to use, by calling SVG_Spawn.
    //bolt = SVG_Spawn();

    //// Setup basic entity attributes.
    //bolt->className = "bolt";   // Classname.
    //bolt->owner = self;         // Setup owner.
    //bolt->damage = damage;         // Setup damage.
    //if (hyper)                  // Hyperblaster?
    //    bolt->spawnFlags = 1;
    //bolt->serverFlags = EntityServerFlags::DeadMonster;    // Set Dead Monster flag so the projectiles 
    //                                    // won't clip against players.
    //bolt->moveType = MoveType::FlyMissile;   // Movetype FLYMISSILE
    //bolt->clipMask = CONTENTS_MASK_SHOT;    // CONTENTS_MASK_SHOT
    //bolt->solid = Solid::BoundingBox;               // Solid::BoundingBox
    //bolt->state.effects |= effect;              // Apply effect argument to entity.
    //
    //// Setup entity physics attribute values.
    //bolt->state.origin = start;     // Initial origin.
    //bolt->state.oldOrigin = start; // Initial origin, same to origin, since this entity had no frame life yet.
    //bolt->state.angles = vec3_euler(dir);       // Calculate euler radian entity satate angles.
    //bolt->velocity = vec3_scale(dir, speed);// Calculate entity state velocity.
    //bolt->mins = vec3_zero();   // Clear bounding mins.
    //bolt->maxs = vec3_zero();   // Clear bounding maxs.

    //// Setup (and precache) sound, and model.
    //bolt->state.modelIndex = gi.ModelIndex("models/objects/laser/tris.md2");
    //bolt->state.sound = gi.SoundIndex("misc/lasfly.wav");
    //   
    //// Setup touch and Think function pointers.
    ////bolt->Touch = blaster_touch;
    ////bolt->nextThinkTime = level.time + 2;
    ////bolt->Think = SVG_FreeEntity;
    //
    //// Link entity in for collision.
    //gi.LinkEntity(bolt);

    //// If a client is firing this bolt, let AI check for dodges.
    //if (self->client)
    //    check_dodge(self, bolt->state.origin, dir, speed);

    //// Trace bolt.
    //tr = gi.Trace(self->state.origin, vec3_zero(), vec3_zero(), bolt->state.origin, bolt, CONTENTS_MASK_SHOT);

    //// Did we hit anything?
    //if (tr.fraction < 1.0) {
    //    bolt->state.origin = vec3_fmaf(bolt->state.origin, -10, dir);
    //    //bolt->Touch(bolt, tr.ent, NULL, NULL);
    //}
}
