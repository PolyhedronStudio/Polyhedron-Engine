/*
// LICENSE HERE.

//
// PlayerClient.cpp
//
//
*/
#include "../../g_local.h"              // SVGame.
#include "../../effects.h"              // Effects.
#include "../../player/client.h"        // Player Client functions.
#include "../../player/animations.h"    // Include Player Client Animations.
#include "../../player/view.h"          // Include Player View functions..
#include "../../utils.h"                // Util funcs.

#include "../base/SVGBaseEntity.h"

#include "PlayerClient.h"

// Constructor/Deconstructor.
PlayerClient::PlayerClient(Entity* svEntity) : SVGBaseEntity(svEntity) {

}
PlayerClient::~PlayerClient() {

}

// Interface functions. 
void PlayerClient::Precache() {

}
void PlayerClient::Spawn() {
    // Set the die function.
    SetDieCallback(&PlayerClient::PlayerClientDie);
    gi.DPrintf("PlayerClient::Spawn();");
}
void PlayerClient::PostSpawn() {

}
void PlayerClient::Think() {

}

void PlayerClient::PlayerClientDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    int     n;

    Entity* self = GetServerEntity();

    // Clear out angular velocity.
    self->angularVelocity = vec3_zero();

    SetTakeDamage(TakeDamage::Yes);
    SetMoveType(MoveType::Toss);

    SetModelIndex2(0);    // remove linked weapon model

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
        SVG_LookAtKiller(self, inflictor->GetServerEntity(), attacker->GetServerEntity());
        self->client->playerState.pmove.type = EnginePlayerMoveType::Dead;
        SVG_ClientUpdateObituary(self, inflictor->GetServerEntity(), attacker->GetServerEntity());
        SVG_TossClientWeapon(self);
        if (deathmatch->value)
            SVG_Command_Score_f(self);       // show scores

        // clear inventory
        // this is kind of ugly, but it's how we want to handle keys in coop
        for (n = 0; n < game.numberOfItems; n++) {
            if (coop->value && itemlist[n].flags & ItemFlags::IsKey)
                self->client->respawn.persistentCoopRespawn.inventory[n] = self->client->persistent.inventory[n];
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
            self->client->animation.priorityAnimation = PlayerAnimation::Death;
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

// Functions.