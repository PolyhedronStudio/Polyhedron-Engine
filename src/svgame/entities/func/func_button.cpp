// LICENSE HERE.

//
// svgame/entities/func_button.c
//
//
// func_button entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../brushfuncs.h"

//=====================================================
/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"angle"     determines the opening direction
"target"    all entities with a matching targetname will be used
"speed"     override the default 40 speed
"wait"      override the default 1 second wait (-1 = never return)
"lip"       override the default 4 pixel lip remaining at end of move
"health"    if set, the button must be killed instead of touched
"sounds"
1) silent
2) steam metal
3) wooden clunk
4) metallic click
5) in-out
*/

void button_done(edict_t* self)
{
    self->moveinfo.state = STATE_BOTTOM;
    self->s.effects &= ~EF_ANIM23;
    self->s.effects |= EF_ANIM01;
}

void button_return(edict_t* self)
{
    self->moveinfo.state = STATE_DOWN;

    Brush_Move_Calc(self, self->moveinfo.start_origin, button_done);

    self->s.frame = 0;

    if (self->health)
        self->takedamage = DAMAGE_YES;
}

void button_wait(edict_t* self)
{
    self->moveinfo.state = STATE_TOP;
    self->s.effects &= ~EF_ANIM01;
    self->s.effects |= EF_ANIM23;

    G_UseTargets(self, self->activator);
    self->s.frame = 1;
    if (self->moveinfo.wait >= 0) {
        self->nextthink = level.time + self->moveinfo.wait;
        self->think = button_return;
    }
}

void button_fire(edict_t* self)
{
    if (self->moveinfo.state == STATE_UP || self->moveinfo.state == STATE_TOP)
        return;

    self->moveinfo.state = STATE_UP;
    if (self->moveinfo.sound_start && !(self->flags & FL_TEAMSLAVE))
        gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveinfo.sound_start, 1, ATTN_STATIC, 0);
    Brush_Move_Calc(self, self->moveinfo.end_origin, button_wait);
}

void button_use(edict_t* self, edict_t* other, edict_t* activator)
{
    self->activator = activator;
    button_fire(self);
}

void button_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    if (!other->client)
        return;

    if (other->health <= 0)
        return;

    self->activator = other;
    button_fire(self);
}

void button_killed(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point)
{
    self->activator = attacker;
    self->health = self->max_health;
    self->takedamage = DAMAGE_NO;
    button_fire(self);
}

void SP_func_button(edict_t* ent)
{
    vec3_t  abs_movedir;
    float   dist;

    G_SetMovedir(ent->s.angles, ent->movedir);
    ent->movetype = MOVETYPE_STOP;
    ent->solid = SOLID_BSP;
    gi.setmodel(ent, ent->model);

    if (ent->sounds != 1)
        ent->moveinfo.sound_start = gi.soundindex("switches/butn2.wav");

    if (!ent->speed)
        ent->speed = 40;
    if (!ent->accel)
        ent->accel = ent->speed;
    if (!ent->decel)
        ent->decel = ent->speed;

    if (!ent->wait)
        ent->wait = 3;
    if (!st.lip)
        st.lip = 4;

    Vec3_Copy(ent->s.origin, ent->pos1);
    abs_movedir[0] = fabs(ent->movedir[0]);
    abs_movedir[1] = fabs(ent->movedir[1]);
    abs_movedir[2] = fabs(ent->movedir[2]);
    dist = abs_movedir[0] * ent->size[0] + abs_movedir[1] * ent->size[1] + abs_movedir[2] * ent->size[2] - st.lip;
    Vec3_MA(ent->pos1, dist, ent->movedir, ent->pos2);

    ent->use = button_use;
    ent->s.effects |= EF_ANIM01;

    if (ent->health) {
        ent->max_health = ent->health;
        ent->die = button_killed;
        ent->takedamage = DAMAGE_YES;
    }
    else if (!ent->targetname)
        ent->touch = button_touch;

    ent->moveinfo.state = STATE_BOTTOM;

    ent->moveinfo.speed = ent->speed;
    ent->moveinfo.accel = ent->accel;
    ent->moveinfo.decel = ent->decel;
    ent->moveinfo.wait = ent->wait;
    Vec3_Copy(ent->pos1, ent->moveinfo.start_origin);
    Vec3_Copy(ent->s.angles, ent->moveinfo.start_angles);
    Vec3_Copy(ent->pos2, ent->moveinfo.end_origin);
    Vec3_Copy(ent->s.angles, ent->moveinfo.end_angles);

    gi.linkentity(ent);
}