// LICENSE HERE.

//
// svgame/player/player.cpp
//
// N&C SVGame: Player pain, and die, callback code.
// 
#include "../g_local.h" // Include SVGame header.
#include "client.h"     // Include Player Client header.

#include "sharedgame/pmove.h"   // Include SG PMove.
#include "animations.h"         // Include Player Client Animations.

void player_pain(edict_t* self, edict_t* other, float kick, int damage)
{
    // player pain is handled at the end of the frame in P_DamageFeedback
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller(edict_t* self, edict_t* inflictor, edict_t* attacker)
{
    vec3_t      dir;

    if (attacker && attacker != world && attacker != self) {
        VectorSubtract(attacker->s.origin, self->s.origin, dir);
    }
    else if (inflictor && inflictor != world && inflictor != self) {
        VectorSubtract(inflictor->s.origin, self->s.origin, dir);
    }
    else {
        self->client->killer_yaw = self->s.angles[YAW];
        return;
    }

    if (dir[0])
        self->client->killer_yaw = 180 / M_PI * atan2(dir[1], dir[0]);
    else {
        self->client->killer_yaw = 0;
        if (dir[1] > 0)
            self->client->killer_yaw = 90;
        else if (dir[1] < 0)
            self->client->killer_yaw = -90;
    }
    if (self->client->killer_yaw < 0)
        self->client->killer_yaw += 360;


}

/*
==================
player_die
==================
*/
void player_die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t& point)
{
    int     n;

    VectorClear(self->avelocity);

    self->takedamage = DAMAGE_YES;
    self->movetype = MOVETYPE_TOSS;

    self->s.modelindex2 = 0;    // remove linked weapon model

    self->s.angles[0] = 0;
    self->s.angles[2] = 0;

    self->s.sound = 0;
    self->client->weapon_sound = 0;

    self->maxs[2] = -8;

    //  self->solid = SOLID_NOT;
    self->svflags |= SVF_DEADMONSTER;

    if (!self->deadflag) {
        self->client->respawn_time = level.time + 1.0;
        LookAtKiller(self, inflictor, attacker);
        self->client->ps.pmove.type = PM_DEAD;
        ClientUpdateObituary(self, inflictor, attacker);
        TossClientWeapon(self);
        if (deathmatch->value)
            Cmd_Help_f(self);       // show scores

        // clear inventory
        // this is kind of ugly, but it's how we want to handle keys in coop
        for (n = 0; n < game.num_items; n++) {
            if (coop->value && itemlist[n].flags & IT_KEY)
                self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
            self->client->pers.inventory[n] = 0;
        }
    }

    // remove powerups
    self->client->quad_framenum = 0;
    self->client->invincible_framenum = 0;
    self->client->breather_framenum = 0;
    self->client->enviro_framenum = 0;
    self->flags &= ~FL_POWER_ARMOR;

    if (self->health < -40) {
        // gib
        gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n = 0; n < 4; n++)
            ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowClientHead(self, damage);

        self->takedamage = DAMAGE_NO;
    }
    else {
        // normal death
        if (!self->deadflag) {
            static int i;

            i = (i + 1) % 3;
            // start a death animation
            self->client->anim_priority = ANIM_DEATH;
            if (self->client->ps.pmove.flags & PMF_DUCKED) {
                self->s.frame = FRAME_crdeath1 - 1;
                self->client->anim_end = FRAME_crdeath5;
            }
            else switch (i) {
            case 0:
                self->s.frame = FRAME_death101 - 1;
                self->client->anim_end = FRAME_death106;
                break;
            case 1:
                self->s.frame = FRAME_death201 - 1;
                self->client->anim_end = FRAME_death206;
                break;
            case 2:
                self->s.frame = FRAME_death301 - 1;
                self->client->anim_end = FRAME_death308;
                break;
            }
            gi.sound(self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand() % 4) + 1)), 1, ATTN_NORM, 0);
        }
    }

    self->deadflag = DEAD_DEAD;

    gi.linkentity(self);
}