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
    SVGBaseEntity::Precache();
}
void PlayerClient::Spawn() {
    // Spawn.
    SVGBaseEntity::Spawn();

    SetMoveType(MoveType::Walk);
    SetGroundEntity(nullptr);

    // Set the die function.
    SetDieCallback(&PlayerClient::PlayerClientDie);
    gi.DPrintf("PlayerClient::Spawn();");
}
void PlayerClient::PostSpawn() {
    SVGBaseEntity::PostSpawn();
}
void PlayerClient::Think() {
    SVGBaseEntity::Think();
}

void PlayerClient::PlayerClientDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {

    // Fetch server entity.
    Entity* serverEntity = GetServerEntity();

    // Fetch client.
    gclient_s* client = GetClient();

    // Clear out angular velocity.
    SetAngularVelocity(vec3_zero());

    // Can still take damage.
    SetTakeDamage(TakeDamage::Yes);

    // Let our dead body toss itself.
    SetMoveType(MoveType::Toss);

    // Remove the linked weapon model (third person thingy.)
    SetModelIndex2(0);

    // Our effect type is now: CORPSE. Beautiful dead corpse :P
    SetEffects(EntityEffectType::Corpse);

    // Fetch angles, only maintain the yaw, reset the others.
    SetAngles(vec3_t{ 0.f, GetAngles()[vec3_t::PYR::Yaw], 0.f });

    // Ensure our client entity is playing no sounds anymore.
    SetSound(0);
    client->weaponSound = 0;

    // Retreive maxes, adjust height (z)
    SetMaxs(GetMaxs() - vec3_t{ 0.f, 0.f, -8.f });

    // Change server flags, we're a dead monster now.
    SetServerFlags(GetServerFlags() | EntityServerFlags::DeadMonster);

    // If we're not dead yet, we got some death initializing to do.
    if (!GetDeadFlag()) {
        // Set respawn time.
        client->respawnTime = level.time + 1.0;
        SVG_LookAtKiller(serverEntity, inflictor->GetServerEntity(), attacker->GetServerEntity());
        client->playerState.pmove.type = EnginePlayerMoveType::Dead;
        SVG_ClientUpdateObituary(this, inflictor, attacker);
        SVG_TossClientWeapon(serverEntity);
        if (deathmatch->value)
            SVG_Command_Score_f(serverEntity);       // show scores

        // Clear inventory this is kind of ugly, but it's how we want to handle keys in coop
        for (int32_t i = 0; i < game.numberOfItems; i++) {
            if (coop->value && itemlist[i].flags & ItemFlags::IsKey)
                client->respawn.persistentCoopRespawn.inventory[i] = client->persistent.inventory[i];
            client->persistent.inventory[i] = 0;
        }
    }

    // Remove powerups.
    SetFlags(GetFlags() & ~EntityFlags::PowerArmor);

    // In case our health went under -40, shred this body to gibs!
    if (GetHealth() < -40) {
        // Play a nasty gib sound, yughh :)
        SVG_Sound(this, CHAN_BODY, gi.SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);

        // Throw some gibs around, true horror oh boy.
        ThrowGib(serverEntity, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowGib(serverEntity, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowGib(serverEntity, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowGib(serverEntity, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowClientHead(serverEntity, damage);

        // Can't take damage if we're already busted.
        SetTakeDamage(TakeDamage::No);

    // Normal death.
    } else {
        // Ensure we aren't dead flagged already.
        if (!GetDeadFlag()) {
            static int i;

            i = (i + 1) % 3;
            // start a death animation
            client->animation.priorityAnimation = PlayerAnimation::Death;
            if (client->playerState.pmove.flags & PMF_DUCKED) {
                SetFrame(FRAME_crdeath1 - 1);
                client->animation.endFrame = FRAME_crdeath5;
            }
            else switch (i) {
            case 0:
                SetFrame(FRAME_death101 - 1);
                client->animation.endFrame = FRAME_death106;
                break;
            case 1:
                SetFrame(FRAME_death201 - 1);
                client->animation.endFrame = FRAME_death206;
                break;
            case 2:
                SetFrame(FRAME_death301 - 1);
                client->animation.endFrame = FRAME_death308;
                break;
            }
            SVG_Sound(this, CHAN_VOICE, gi.SoundIndex(va("*death%i.wav", (rand() % 4) + 1)), 1, ATTN_NORM, 0);
        }
    }

    // Set the dead flag to: DEAD, duhh.
    SetDeadFlag(DEAD_DEAD);

    // Link our entity back in for collision purposes.
    LinkEntity();
}