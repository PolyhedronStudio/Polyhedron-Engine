// LICENSE HERE.

//
// svgame/player/player.cpp
//
// N&C SVGame: Player pain, and die, callback code.
// 
#include "../g_local.h" // Include SVGame header.
#include "../effects.h" // Include Effects header.
#include "client.h"     // Include Player Client header.

#include "sharedgame/pmove.h"   // Include SG PMove.
#include "animations.h"         // Include Player Client Animations.

void Player_Pain(entity_t* self, entity_t* other, float kick, int damage)
{
    // player pain is handled at the end of the frame in P_ApplyDamageFeedback
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller(entity_t* self, entity_t* inflictor, entity_t* attacker)
{
    vec3_t      dir;

    if (attacker && attacker != world && attacker != self) {
        VectorSubtract(attacker->state.origin, self->state.origin, dir);
    }
    else if (inflictor && inflictor != world && inflictor != self) {
        VectorSubtract(inflictor->state.origin, self->state.origin, dir);
    }
    else {
        self->client->killerYaw = self->state.angles[vec3_t::Yaw];
        return;
    }

    if (dir[0])
        self->client->killerYaw = 180 / M_PI * atan2(dir[1], dir[0]);
    else {
        self->client->killerYaw = 0;
        if (dir[1] > 0)
            self->client->killerYaw = 90;
        else if (dir[1] < 0)
            self->client->killerYaw = -90;
    }
    if (self->client->killerYaw < 0)
        self->client->killerYaw += 360;


}

/*
==================
Player_Die
==================
*/
void Player_Die(entity_t* self, entity_t* inflictor, entity_t* attacker, int damage, const vec3_t& point)
{
    int     n;

    // Clear out angular velocity.
    self->avelocity = vec3_zero();

    self->takeDamage = TakeDamage::Yes;
    self->moveType = MoveType::Toss;

    self->state.modelIndex2 = 0;    // remove linked weapon model
    
    self->state.effects = EntityEffectType::Corpse;

    self->state.angles[0] = 0;
    self->state.angles[2] = 0;

    self->state.sound = 0;
    self->client->weaponSound = 0;

    self->maxs[2] = -8;

    //  self->solid = Solid::Not;
    self->serverFlags |= EntityServerFlags::DeadMonster;

    if (!self->deadFlag) {
        self->client->respawnTime = level.time + 1.0;
        LookAtKiller(self, inflictor, attacker);
        self->client->playerState.pmove.type = EnginePlayerMoveType::Dead;
        ClientUpdateObituary(self, inflictor, attacker);
        TossClientWeapon(self);
        if (deathmatch->value)
            Cmd_Score_f(self);       // show scores

        // clear inventory
        // this is kind of ugly, but it's how we want to handle keys in coop
        for (n = 0; n < game.num_items; n++) {
            if (coop->value && itemlist[n].flags & IT_KEY)
                self->client->respawn.coop_respawn.inventory[n] = self->client->persistent.inventory[n];
            self->client->persistent.inventory[n] = 0;
        }
    }

    // remove powerups
    self->flags &= ~EntityFlags::PowerArmor;

    if (self->health < -40) {
        // gib
        gi.Sound(self, CHAN_BODY, gi.SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n = 0; n < 4; n++)
            ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowClientHead(self, damage);

        self->takeDamage = TakeDamage::No;
    }
    else {
        // normal death
        if (!self->deadFlag) {
            static int i;

            i = (i + 1) % 3;
            // start a death animation
            self->client->animation.priorityAnimation = ANIM_DEATH;
            if (self->client->playerState.pmove.flags & PMF_DUCKED) {
                self->state.frame = FRAME_crdeath1 - 1;
                self->client->animation.endFrame = FRAME_crdeath5;
            }
            else switch (i) {
            case 0:
                self->state.frame = FRAME_death101 - 1;
                self->client->animation.endFrame = FRAME_death106;
                break;
            case 1:
                self->state.frame = FRAME_death201 - 1;
                self->client->animation.endFrame = FRAME_death206;
                break;
            case 2:
                self->state.frame = FRAME_death301 - 1;
                self->client->animation.endFrame = FRAME_death308;
                break;
            }
            gi.Sound(self, CHAN_VOICE, gi.SoundIndex(va("*death%i.wav", (rand() % 4) + 1)), 1, ATTN_NORM, 0);
        }
    }

    self->deadFlag = DEAD_DEAD;

    gi.LinkEntity(self);
}