// LICENSE HERE.

//
// svgame/entities/func_plat.c
//
//
// func_plat entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../brushfuncs.h"
#include "../../effects.h"

//=====================================================
void plat_go_down(Entity* ent);

void plat_hit_top(Entity* ent)
{
    if (!(ent->flags & EntityFlags::TeamSlave)) {
        if (ent->moveInfo.endSoundIndex)
            gi.Sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveInfo.endSoundIndex, 1, ATTN_STATIC, 0);
        ent->state.sound = 0;
    }
    ent->moveInfo.state = STATE_TOP;

    ent->Think = plat_go_down;
    ent->nextThinkTime = level.time + 3;
}

void plat_hit_bottom(Entity* ent)
{
    if (!(ent->flags & EntityFlags::TeamSlave)) {
        if (ent->moveInfo.endSoundIndex)
            gi.Sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveInfo.endSoundIndex, 1, ATTN_STATIC, 0);
        ent->state.sound = 0;
    }
    ent->moveInfo.state = STATE_BOTTOM;
}

void plat_go_down(Entity* ent)
{
    if (!(ent->flags & EntityFlags::TeamSlave)) {
        if (ent->moveInfo.startSoundIndex)
            gi.Sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveInfo.startSoundIndex, 1, ATTN_STATIC, 0);
        ent->state.sound = ent->moveInfo.middleSoundIndex;
    }
    ent->moveInfo.state = STATE_DOWN;
    Brush_Move_Calc(ent, ent->moveInfo.endOrigin, plat_hit_bottom);
}

void plat_go_up(Entity* ent)
{
    if (!(ent->flags & EntityFlags::TeamSlave)) {
        if (ent->moveInfo.startSoundIndex)
            gi.Sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveInfo.startSoundIndex, 1, ATTN_STATIC, 0);
        ent->state.sound = ent->moveInfo.middleSoundIndex;
    }
    ent->moveInfo.state = STATE_UP;
    Brush_Move_Calc(ent, ent->moveInfo.startOrigin, plat_hit_top);
}

void plat_blocked(Entity* self, Entity* other)
{
    if (!(other->serverFlags & EntityServerFlags::Monster) && (!other->client)) {
        // give it a chance to go away on it's own terms (like gibs)
        SVG_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, 100000, 1, 0, MeansOfDeath::Crush);
        // if it's still there, nuke it
        if (other)
            BecomeExplosion1(other);
        return;
    }

    SVG_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, self->damage, 1, 0, MeansOfDeath::Crush);

    if (self->moveInfo.state == STATE_UP)
        plat_go_down(self);
    else if (self->moveInfo.state == STATE_DOWN)
        plat_go_up(self);
}


void Use_Plat(Entity* ent, Entity* other, Entity* activator)
{
    if (ent->Think)
        return;     // already down
    plat_go_down(ent);
}


void Touch_Plat_Center(Entity* ent, Entity* other, cplane_t* plane, csurface_t* surf)
{
    if (!other->client)
        return;

    if (other->health <= 0)
        return;

    ent = ent->enemy;   // now point at the plat, not the trigger
    if (ent->moveInfo.state == STATE_BOTTOM)
        plat_go_up(ent);
    else if (ent->moveInfo.state == STATE_TOP)
        ent->nextThinkTime = level.time + 1;    // the player is still on the plat, so delay going down
}

void plat_spawn_inside_trigger(Entity* ent)
{
    Entity* trigger;
    vec3_t  tmin, tmax;

    //
    // middle trigger
    //
    trigger = SVG_Spawn();
    trigger->Touch = Touch_Plat_Center;
    trigger->moveType = MoveType::None;
    trigger->solid = Solid::Trigger;
    trigger->enemy = ent;

    tmin[0] = ent->mins[0] + 25;
    tmin[1] = ent->mins[1] + 25;
    tmin[2] = ent->mins[2];

    tmax[0] = ent->maxs[0] - 25;
    tmax[1] = ent->maxs[1] - 25;
    tmax[2] = ent->maxs[2] + 8;

    tmin[2] = tmax[2] - (ent->position1[2] - ent->position2[2] + st.lip);

    if (ent->spawnFlags & PLAT_LOW_TRIGGER)
        tmax[2] = tmin[2] + 8;

    if (tmax[0] - tmin[0] <= 0) {
        tmin[0] = (ent->mins[0] + ent->maxs[0]) * 0.5;
        tmax[0] = tmin[0] + 1;
    }
    if (tmax[1] - tmin[1] <= 0) {
        tmin[1] = (ent->mins[1] + ent->maxs[1]) * 0.5;
        tmax[1] = tmin[1] + 1;
    }

    VectorCopy(tmin, trigger->mins);
    VectorCopy(tmax, trigger->maxs);

    gi.LinkEntity(trigger);
}


/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed   default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in the extended position until it is trigger, when it will lower and become a normal plat.

"speed" overrides default 200.
"acceleration" overrides default 500
"lip"   overrides default 8 pixel lip

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/
void SP_func_plat(Entity* ent)
{
    VectorClear(ent->state.angles);
    ent->solid = Solid::BSP;
    ent->moveType = MoveType::Push;

    gi.SetModel(ent, ent->model);

    ent->Blocked = plat_blocked;

    if (!ent->speed)
        ent->speed = 20;
    else
        ent->speed *= 0.1;

    if (!ent->acceleration)
        ent->acceleration = 5;
    else
        ent->acceleration *= 0.1;

    if (!ent->deceleration)
        ent->deceleration = 5;
    else
        ent->deceleration *= 0.1;

    if (!ent->damage)
        ent->damage = 2;

    if (!st.lip)
        st.lip = 8;

    // position1 is the top position, position2 is the bottom
    VectorCopy(ent->state.origin, ent->position1);
    VectorCopy(ent->state.origin, ent->position2);
    if (st.height)
        ent->position2[2] -= st.height;
    else
        ent->position2[2] -= (ent->maxs[2] - ent->mins[2]) - st.lip;

    ent->Use = Use_Plat;

    plat_spawn_inside_trigger(ent);     // the "start moving" trigger

    if (ent->targetName) {
        ent->moveInfo.state = STATE_UP;
    }
    else {
        VectorCopy(ent->position2, ent->state.origin);
        gi.LinkEntity(ent);
        ent->moveInfo.state = STATE_BOTTOM;
    }

    ent->moveInfo.speed = ent->speed;
    ent->moveInfo.acceleration = ent->acceleration;
    ent->moveInfo.deceleration = ent->deceleration;
    ent->moveInfo.wait = ent->wait;
    VectorCopy(ent->position1, ent->moveInfo.startOrigin);
    VectorCopy(ent->state.angles, ent->moveInfo.startAngles);
    VectorCopy(ent->position2, ent->moveInfo.endOrigin);
    VectorCopy(ent->state.angles, ent->moveInfo.endAngles);

    ent->moveInfo.startSoundIndex = gi.SoundIndex("plats/pt1_strt.wav");
    ent->moveInfo.middleSoundIndex = gi.SoundIndex("plats/pt1_mid.wav");
    ent->moveInfo.endSoundIndex = gi.SoundIndex("plats/pt1_end.wav");
}