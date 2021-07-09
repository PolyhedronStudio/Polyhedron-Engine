/*
// LICENSE HERE.

//
// PlayerClient.cpp
//
//
*/
#include "../../g_local.h"              // SVGame.
#include "../../effects.h"              // Effects.
#include "../../entities.h"             // Entities.
#include "../../player/client.h"        // Player Client functions.
#include "../../player/animations.h"    // Include Player Client Animations.
#include "../../player/view.h"          // Include Player View functions..
#include "../../utils.h"                // Util funcs.

// Game Mode interface.
#include "../../gamemodes/IGameMode.h"

// Class Entities.
#include "../base/SVGBaseEntity.h"
#include "PlayerClient.h"

// Constructor/Deconstructor.
PlayerClient::PlayerClient(Entity* svEntity) : SVGBaseEntity(svEntity) {

}
PlayerClient::~PlayerClient() {

}

//
//===============
// PlayerClient::Precache
//
//===============
//
void PlayerClient::Precache() {
    Base::Precache();
}

//
//===============
// PlayerClient::Spawn
//
//===============
//
void PlayerClient::Spawn() {
    // Spawn.
    Base::Spawn();

    // When spawned, we aren't on any ground, make sure of that.
    SetGroundEntity(nullptr);

//    ent->client = &game.clients[index];
    SetTakeDamage(TakeDamage::Aim);
    SetMoveType(MoveType::Walk);
    SetViewHeight(22);
    SetInUse(true);
    SetMass(200);
    SetSolid(Solid::BoundingBox);
    SetDeadFlag(DEAD_NO);
    SetAirFinishedTime(level.time + 12);
    SetClipMask(CONTENTS_MASK_PLAYERSOLID);
    SetModel("players/male/tris.md2");

    // Setup WaterLevel and Type.
    SetWaterLevel(WaterLevel::None);
    SetWaterType(0);

    // Setup Flags.
    SetFlags(GetFlags() & ~EntityFlags::NoKnockBack);
    SetServerFlags(GetServerFlags() & ~EntityServerFlags::DeadMonster);

    // Default Player Move bounding box.
    SetMins(vec3_scale(PM_MINS, PM_SCALE));
    SetMaxs(vec3_scale(PM_MAXS, PM_SCALE));
    
    // Zero velocity.
    SetVelocity(vec3_zero());

    // Set the die function.
    SetDieCallback(&PlayerClient::PlayerClientDie);

    // Debug.
    gi.DPrintf("PlayerClient::Spawn();");
}

//
//===============
// PlayerClient::Respawn
//
//===============
//
void PlayerClient::Respawn() {
    gi.DPrintf("PlayerClient::Respawn();");
}

//
//===============
// PlayerClient::PostSpawn
//
//===============
//
void PlayerClient::PostSpawn() {
    Base::PostSpawn();
}

//
//===============
// PlayerClient::Think
//
//===============
//
void PlayerClient::Think() {
    // Parent class Think.
    Base::Think();
}

//
//===============
// PlayerClient::SpawnKey
//
// PlayerClient spawn key handling.
//===============
//
void PlayerClient::SpawnKey(const std::string& key, const std::string& value) {
    // Parent class spawnkey.
    Base::SpawnKey(key, value);
}

//
//===============
// PlayerClient::PlayerClientDie
//
// Callback that is fired any time the player dies. As such, it kindly takes care of doing this.
//===============
//
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
        SetRespawnTime(level.time + 1.0);

        // Ensure we are looking at our killer.
        LookAtKiller(inflictor, attacker);

        // Dead players don't move ;-)
        SetPlayerMoveType(EnginePlayerMoveType::Dead);

        // Update the obituary.
        game.gameMode->ClientUpdateObituary(this, inflictor, attacker);
        //SVG_ClientUpdateObituary(this, inflictor, attacker);

        // Toss our weapon, assuming we had any.
        SVG_TossClientWeapon(this);

        // Show the scoreboard in case of a deathmatch mode.
        if (deathmatch->value)
            SVG_Command_Score_f(this);

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
            SetPriorityAnimation(PlayerAnimation::Death);

            if (client->playerState.pmove.flags & PMF_DUCKED) {
                SetFrame(FRAME_crdeath1 - 1);
                SetAnimationEndFrame(FRAME_crdeath5);
            }
            else switch (i) {
            case 0:
                SetFrame(FRAME_death101 - 1);
                SetAnimationEndFrame(FRAME_death106);
                break;
            case 1:
                SetFrame(FRAME_death201 - 1);
                SetAnimationEndFrame(FRAME_death206);
                break;
            case 2:
                SetFrame(FRAME_death301 - 1);
                SetAnimationEndFrame(FRAME_death308);
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

//
//===============
// PlayerClient::LookAtKiller
//
// Sets the clients view to look at the killer.
//===============
//
void PlayerClient::LookAtKiller(SVGBaseEntity* inflictor, SVGBaseEntity* attacker)
{
    // Fetch client.
    gclient_s* client = GetClient();

    // Is the attack, not us, or the world?
    if (attacker && attacker != SVG_GetWorldClassEntity() && attacker != this) {
        float yaw = vec3_to_yaw(attacker->GetOrigin() - GetOrigin());
        SetKillerYaw(yaw);
    // Is the inflictor, and not an attack, NOT us or the WORLD?
    } else if (inflictor && inflictor != SVG_GetWorldClassEntity() && inflictor != this) {
        float yaw = vec3_to_yaw(inflictor->GetOrigin() - GetOrigin());
        SetKillerYaw(yaw);
    // If none of the above, set the yaw as is.
    } else {
        SetKillerYaw(GetAngles()[vec3_t::Yaw]);
        return;
    }
}