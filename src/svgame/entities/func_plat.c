// LICENSE HERE.

//
// svgame/entities/func_plat.c
//
//
// func_plat entity implementation.
//

// Include local game header.
#include "../g_local.h"
#include "../brushfuncs.h"

//=====================================================
void plat_go_down(edict_t* ent);

void plat_hit_top(edict_t* ent)
{
    if (!(ent->flags & FL_TEAMSLAVE)) {
        if (ent->moveinfo.sound_end)
            gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_end, 1, ATTN_STATIC, 0);
        ent->s.sound = 0;
    }
    ent->moveinfo.state = STATE_TOP;

    ent->think = plat_go_down;
    ent->nextthink = level.time + 3;
}

void plat_hit_bottom(edict_t* ent)
{
    if (!(ent->flags & FL_TEAMSLAVE)) {
        if (ent->moveinfo.sound_end)
            gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_end, 1, ATTN_STATIC, 0);
        ent->s.sound = 0;
    }
    ent->moveinfo.state = STATE_BOTTOM;
}

void plat_go_down(edict_t* ent)
{
    if (!(ent->flags & FL_TEAMSLAVE)) {
        if (ent->moveinfo.sound_start)
            gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_start, 1, ATTN_STATIC, 0);
        ent->s.sound = ent->moveinfo.sound_middle;
    }
    ent->moveinfo.state = STATE_DOWN;
    Brush_Move_Calc(ent, ent->moveinfo.end_origin, plat_hit_bottom);
}

void plat_go_up(edict_t* ent)
{
    if (!(ent->flags & FL_TEAMSLAVE)) {
        if (ent->moveinfo.sound_start)
            gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, ent->moveinfo.sound_start, 1, ATTN_STATIC, 0);
        ent->s.sound = ent->moveinfo.sound_middle;
    }
    ent->moveinfo.state = STATE_UP;
    Brush_Move_Calc(ent, ent->moveinfo.start_origin, plat_hit_top);
}

void plat_blocked(edict_t* self, edict_t* other)
{
    if (!(other->svflags & SVF_MONSTER) && (!other->client)) {
        // give it a chance to go away on it's own terms (like gibs)
        T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
        // if it's still there, nuke it
        if (other)
            BecomeExplosion1(other);
        return;
    }

    T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);

    if (self->moveinfo.state == STATE_UP)
        plat_go_down(self);
    else if (self->moveinfo.state == STATE_DOWN)
        plat_go_up(self);
}


void Use_Plat(edict_t* ent, edict_t* other, edict_t* activator)
{
    if (ent->think)
        return;     // already down
    plat_go_down(ent);
}


void Touch_Plat_Center(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    if (!other->client)
        return;

    if (other->health <= 0)
        return;

    ent = ent->enemy;   // now point at the plat, not the trigger
    if (ent->moveinfo.state == STATE_BOTTOM)
        plat_go_up(ent);
    else if (ent->moveinfo.state == STATE_TOP)
        ent->nextthink = level.time + 1;    // the player is still on the plat, so delay going down
}

void plat_spawn_inside_trigger(edict_t* ent)
{
    edict_t* trigger;
    vec3_t  tmin, tmax;

    //
    // middle trigger
    //
    trigger = G_Spawn();
    trigger->touch = Touch_Plat_Center;
    trigger->movetype = MOVETYPE_NONE;
    trigger->solid = SOLID_TRIGGER;
    trigger->enemy = ent;

    tmin[0] = ent->mins[0] + 25;
    tmin[1] = ent->mins[1] + 25;
    tmin[2] = ent->mins[2];

    tmax[0] = ent->maxs[0] - 25;
    tmax[1] = ent->maxs[1] - 25;
    tmax[2] = ent->maxs[2] + 8;

    tmin[2] = tmax[2] - (ent->pos1[2] - ent->pos2[2] + st.lip);

    if (ent->spawnflags & PLAT_LOW_TRIGGER)
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

    gi.linkentity(trigger);
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
void SP_func_plat(edict_t* ent)
{
    VectorClear(ent->s.angles);
    ent->solid = SOLID_BSP;
    ent->movetype = MOVETYPE_PUSH;

    gi.setmodel(ent, ent->model);

    ent->blocked = plat_blocked;

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
    VectorCopy(ent->s.origin, ent->pos1);
    VectorCopy(ent->s.origin, ent->pos2);
    if (st.height)
        ent->pos2[2] -= st.height;
    else
        ent->pos2[2] -= (ent->maxs[2] - ent->mins[2]) - st.lip;

    ent->use = Use_Plat;

    plat_spawn_inside_trigger(ent);     // the "start moving" trigger

    if (ent->targetname) {
        ent->moveinfo.state = STATE_UP;
    }
    else {
        VectorCopy(ent->pos2, ent->s.origin);
        gi.linkentity(ent);
        ent->moveinfo.state = STATE_BOTTOM;
    }

    ent->moveinfo.speed = ent->speed;
    ent->moveinfo.accel = ent->accel;
    ent->moveinfo.decel = ent->decel;
    ent->moveinfo.wait = ent->wait;
    VectorCopy(ent->pos1, ent->moveinfo.start_origin);
    VectorCopy(ent->s.angles, ent->moveinfo.start_angles);
    VectorCopy(ent->pos2, ent->moveinfo.end_origin);
    VectorCopy(ent->s.angles, ent->moveinfo.end_angles);

    ent->moveinfo.sound_start = gi.soundindex("plats/pt1_strt.wav");
    ent->moveinfo.sound_middle = gi.soundindex("plats/pt1_mid.wav");
    ent->moveinfo.sound_end = gi.soundindex("plats/pt1_end.wav");
}