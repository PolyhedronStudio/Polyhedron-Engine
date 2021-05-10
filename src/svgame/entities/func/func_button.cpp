// LICENSE HERE.

//
// svgame/entities/func_button.c
//
//
// func_button entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "sharedgame/sharedgame.h"
#include "../../utils.h"        // Include Util funcs.
#include "../../brushfuncs.h"   // Include Brush funcs.

//=====================================================
/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"angle"     determines the opening direction
"target"    all entities with a matching targetName will be used
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

void button_done(entity_t* self)
{
    self->moveInfo.state = STATE_BOTTOM;
    self->state.effects &= ~EntityEffectType::AnimCycleFrames23hz2;
    self->state.effects |= EntityEffectType::AnimCycleFrames01hz2;
}

void button_return(entity_t* self)
{
    self->moveInfo.state = STATE_DOWN;

    Brush_Move_Calc(self, self->moveInfo.start_origin, button_done);

    self->state.frame = 0;

    if (self->health)
        self->takeDamage = TakeDamage::Yes;
}

void button_wait(entity_t* self)
{
    self->moveInfo.state = STATE_TOP;
    self->state.effects &= ~EntityEffectType::AnimCycleFrames01hz2;
    self->state.effects |= EntityEffectType::AnimCycleFrames23hz2;

    UTIL_UseTargets(self, self->activator);
    self->state.frame = 1;
    if (self->moveInfo.wait >= 0) {
        self->nextThink = level.time + self->moveInfo.wait;
        self->Think = button_return;
    }
}

void button_fire(entity_t* self)
{
    if (self->moveInfo.state == STATE_UP || self->moveInfo.state == STATE_TOP)
        return;

    self->moveInfo.state = STATE_UP;
    if (self->moveInfo.sound_start && !(self->flags & EntityFlags::TeamSlave))
        gi.Sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveInfo.sound_start, 1, ATTN_STATIC, 0);
    Brush_Move_Calc(self, self->moveInfo.end_origin, button_wait);
}

void button_use(entity_t* self, entity_t* other, entity_t* activator)
{
    self->activator = activator;
    button_fire(self);
}

void button_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf)
{
    if (!other->client)
        return;

    if (other->health <= 0)
        return;

    self->activator = other;
    button_fire(self);
}

void button_killed(entity_t* self, entity_t* inflictor, entity_t* attacker, int damage, const vec3_t &point)
{
    self->activator = attacker;
    self->health = self->maxHealth;
    self->takeDamage = TakeDamage::No;
    button_fire(self);
}

void SP_func_button(entity_t* ent)
{
    vec3_t  abs_movedir;
    float   dist;

    UTIL_SetMoveDir(ent->state.angles, ent->moveDirection);
    ent->moveType = MoveType::Stop;
    ent->solid = Solid::BSP;
    gi.SetModel(ent, ent->model);

    if (ent->sounds != 1)
        ent->moveInfo.sound_start = gi.SoundIndex("switches/butn2.wav");

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

    VectorCopy(ent->state.origin, ent->pos1);
    abs_movedir[0] = fabs(ent->moveDirection[0]);
    abs_movedir[1] = fabs(ent->moveDirection[1]);
    abs_movedir[2] = fabs(ent->moveDirection[2]);
    dist = abs_movedir[0] * ent->size[0] + abs_movedir[1] * ent->size[1] + abs_movedir[2] * ent->size[2] - st.lip;
    VectorMA(ent->pos1, dist, ent->moveDirection, ent->pos2);

    ent->Use = button_use;
    ent->state.effects |= EntityEffectType::AnimCycleFrames01hz2;

    if (ent->health) {
        ent->maxHealth = ent->health;
        ent->Die = button_killed;
        ent->takeDamage = TakeDamage::Yes;
    }
    else if (!ent->targetName)
        ent->Touch = button_touch;

    ent->moveInfo.state = STATE_BOTTOM;

    ent->moveInfo.speed = ent->speed;
    ent->moveInfo.accel = ent->accel;
    ent->moveInfo.decel = ent->decel;
    ent->moveInfo.wait = ent->wait;
    VectorCopy(ent->pos1, ent->moveInfo.start_origin);
    VectorCopy(ent->state.angles, ent->moveInfo.start_angles);
    VectorCopy(ent->pos2, ent->moveInfo.end_origin);
    VectorCopy(ent->state.angles, ent->moveInfo.end_angles);

    gi.LinkEntity(ent);
}