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
void plat_go_down(entity_t* ent);

void plat_hit_top(entity_t* ent)
{
    if (!(ent->flags & EntityFlags::TeamSlave)) {
        if (ent->moveInfo.sound_end)
            gi.Sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveInfo.sound_end, 1, ATTN_STATIC, 0);
        ent->state.sound = 0;
    }
    ent->moveInfo.state = STATE_TOP;

    ent->Think = plat_go_down;
    ent->nextThink = level.time + 3;
}

void plat_hit_bottom(entity_t* ent)
{
    if (!(ent->flags & EntityFlags::TeamSlave)) {
        if (ent->moveInfo.sound_end)
            gi.Sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveInfo.sound_end, 1, ATTN_STATIC, 0);
        ent->state.sound = 0;
    }
    ent->moveInfo.state = STATE_BOTTOM;
}

void plat_go_down(entity_t* ent)
{
    if (!(ent->flags & EntityFlags::TeamSlave)) {
        if (ent->moveInfo.sound_start)
            gi.Sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveInfo.sound_start, 1, ATTN_STATIC, 0);
        ent->state.sound = ent->moveInfo.sound_middle;
    }
    ent->moveInfo.state = STATE_DOWN;
    Brush_Move_Calc(ent, ent->moveInfo.end_origin, plat_hit_bottom);
}

void plat_go_up(entity_t* ent)
{
    if (!(ent->flags & EntityFlags::TeamSlave)) {
        if (ent->moveInfo.sound_start)
            gi.Sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveInfo.sound_start, 1, ATTN_STATIC, 0);
        ent->state.sound = ent->moveInfo.sound_middle;
    }
    ent->moveInfo.state = STATE_UP;
    Brush_Move_Calc(ent, ent->moveInfo.start_origin, plat_hit_top);
}

void plat_blocked(entity_t* self, entity_t* other)
{
    if (!(other->serverFlags & EntityServerFlags::Monster) && (!other->client)) {
        // give it a chance to go away on it's own terms (like gibs)
        T_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
        // if it's still there, nuke it
        if (other)
            BecomeExplosion1(other);
        return;
    }

    T_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

    if (self->moveInfo.state == STATE_UP)
        plat_go_down(self);
    else if (self->moveInfo.state == STATE_DOWN)
        plat_go_up(self);
}


void Use_Plat(entity_t* ent, entity_t* other, entity_t* activator)
{
    if (ent->Think)
        return;     // already down
    plat_go_down(ent);
}


void Touch_Plat_Center(entity_t* ent, entity_t* other, cplane_t* plane, csurface_t* surf)
{
    if (!other->client)
        return;

    if (other->health <= 0)
        return;

    ent = ent->enemy;   // now point at the plat, not the trigger
    if (ent->moveInfo.state == STATE_BOTTOM)
        plat_go_up(ent);
    else if (ent->moveInfo.state == STATE_TOP)
        ent->nextThink = level.time + 1;    // the player is still on the plat, so delay going down
}

void plat_spawn_inside_trigger(entity_t* ent)
{
    entity_t* trigger;
    vec3_t  tmin, tmax;

    //
    // middle trigger
    //
    trigger = G_Spawn();
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

    tmin[2] = tmax[2] - (ent->pos1[2] - ent->pos2[2] + st.lip);

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
"accel" overrides default 500
"lip"   overrides default 8 pixel lip

If the "height" key is set, that will determine the amount the plat moves, instead of being implicitly determoveinfoned by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/
void SP_func_plat(entity_t* ent)
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

    if (!ent->accel)
        ent->accel = 5;
    else
        ent->accel *= 0.1;

    if (!ent->decel)
        ent->decel = 5;
    else
        ent->decel *= 0.1;

    if (!ent->dmg)
        ent->dmg = 2;

    if (!st.lip)
        st.lip = 8;

    // pos1 is the top position, pos2 is the bottom
    VectorCopy(ent->state.origin, ent->pos1);
    VectorCopy(ent->state.origin, ent->pos2);
    if (st.height)
        ent->pos2[2] -= st.height;
    else
        ent->pos2[2] -= (ent->maxs[2] - ent->mins[2]) - st.lip;

    ent->Use = Use_Plat;

    plat_spawn_inside_trigger(ent);     // the "start moving" trigger

    if (ent->targetName) {
        ent->moveInfo.state = STATE_UP;
    }
    else {
        VectorCopy(ent->pos2, ent->state.origin);
        gi.LinkEntity(ent);
        ent->moveInfo.state = STATE_BOTTOM;
    }

    ent->moveInfo.speed = ent->speed;
    ent->moveInfo.accel = ent->accel;
    ent->moveInfo.decel = ent->decel;
    ent->moveInfo.wait = ent->wait;
    VectorCopy(ent->pos1, ent->moveInfo.start_origin);
    VectorCopy(ent->state.angles, ent->moveInfo.start_angles);
    VectorCopy(ent->pos2, ent->moveInfo.end_origin);
    VectorCopy(ent->state.angles, ent->moveInfo.end_angles);

    ent->moveInfo.sound_start = gi.SoundIndex("plats/pt1_strt.wav");
    ent->moveInfo.sound_middle = gi.SoundIndex("plats/pt1_mid.wav");
    ent->moveInfo.sound_end = gi.SoundIndex("plats/pt1_end.wav");
}