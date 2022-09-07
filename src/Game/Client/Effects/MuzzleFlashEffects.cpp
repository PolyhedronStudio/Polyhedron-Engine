/***
*
*	License here.
*
*	@file
*
*	Client Game MuzzleFlash Effects.
* 
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! ClientGame Local headers.
#include "Game/Client/ClientGameLocals.h"
//! ClientGame World
#include "Game/Client/World/ClientGameWorld.h"

#include "Game/Client/TemporaryEntities.h"

#include "Game/Client/Effects/MuzzleFlashEffects.h"
#include "Game/Client/Effects/DynamicLights.h"
#include "Game/Client/Effects/Particles.h"
#include "Game/Client/Effects/ParticleEffects.h"



/**
*   @brief  Client Entity Muzzleflash Effects.
**/
void MuzzleFlashEffects::ClientMuzzleFlash() {
    float       volume;
    char        soundname[MAX_QPATH];
    
#ifdef _DEBUG
    if (clgi.GetDeveloperLevel() > 0) {
        clge->CheckEntityPresent(mzParameters.entity, "muzzleflash");
    }
#endif

	// Determine whether to flash, we don't wanna flash each shot.
	const bool addDynamicLight = (RandomRangeui(0, 2048) < 128 ? true : false);

    PODEntity *pl = &cs->entities[mzParameters.entity];
    cdlight_t* dl = DynamicLights::GetDynamicLight( mzParameters.entity );
    dl->origin = pl->currentState.origin;//, dl->origin;

	// Get forward and right vectors to calculate origin with.
    vec3_t vForward = vec3_zero();
	vec3_t vRight = vec3_zero();
	AngleVectors(pl->currentState.angles, &vForward, &vRight, NULL);
	// Offset from said origin a tiny whee bit.
    dl->origin = vec3_fmaf(dl->origin, RandomRangef(18, 32), vForward);
    dl->origin = vec3_fmaf(dl->origin, 16, vRight);
	
    if (mzParameters.silenced)
        dl->radius = 100 + (rand() & 31);
    else
        dl->radius = 20 + (rand() & 31);
    //dl->minlight = 32;
    dl->die = cl->time + CLG_1_FRAMETIME * 4; // + CLG_FRAMETIME;

    if (mzParameters.silenced)
        volume = 0.2;
    else
        volume = 1;

    switch (mzParameters.weapon) {
    case MuzzleFlashType::Smg45: {
//0,886274509
//0,756862745
//0,607843137
        dl->color = vec3_t{
			0.886274509,
			0.756862745,
			0.607843137
		};
		if (addDynamicLight) {
	        dl->die = cl->time + FRAMERATE_MS.count(); //CLG_1_FRAMETIME * 8;
		} else {
			dl->die = 0;
		}

        //dl->radius = 350 + (rand() & 31);
        dl->decay = CLG_1_FRAMETIME;
        //dl->velocity = vec3_fmaf(pl->currentState.origin, 5, pl->currentState.angles);//, vec3_t { 0.f, 0.f, 1.f };
        dl->radius = RandomRangeui(48, 64);

        // Toss a dice, 50/50, which fire effect to play.
        uint32_t fireSound = RandomRangeui(0, 2);

        // Let the player entity play the 'draw SMG' sound.
        if (fireSound == 0) {
            clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/smg45/fire1.wav"), volume, Attenuation::Normal, 0);
            //client->weaponSound = SVG_PrecacheSound("weapons/smg45/fire1.wav");
            //SVG_Sound(player, SoundChannel::Weapon, SVG_PrecacheSound("weapons/smg45/fire1.wav"), 1.f, Attenuation::Normal, 0.f);
        } else {
            clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/smg45/fire2.wav"), volume, Attenuation::Normal, 0);
            //client->weaponSound = SVG_PrecacheSound("weapons/smg45/fire2.wav");
            //SVG_Sound(player, SoundChannel::Weapon, SVG_PrecacheSound("weapons/smg45/fire2.wav"), 1.f, Attenuation::Normal, 0.f);
        }
    break;
    }
    case MuzzleFlashType::MachineGun:
        //dl->color = vec3_t{1, 1, 0};
        //Q_snprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
        //clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound(soundname), volume, Attenuation::Normal, 0);
        break;
    case MuzzleFlashType::Shotgun:
        //dl->color = vec3_t{1, 1, 0};
        //clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/shotgf1b.wav"), volume, Attenuation::Normal, 0);
        ////  S_StartSound(NULL, mzParameters.entity, SoundChannel::Auto,   S_RegisterSound("weapons/shotgr1b.wav"), volume, Attenuation::Normal, 0.1);
        break;
    case MuzzleFlashType::SuperShotgun:
        //dl->color = vec3_t{1, 1, 0};
        //clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/sshotf1b.wav"), volume, Attenuation::Normal, 0);
        break;
    case MuzzleFlashType::Login:
        dl->color = vec3_t{0, 1, 0};
        dl->die = cl->time + 1.0f;
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("fx/playerspawn.wav"), 1, Attenuation::Normal, 0);
        ParticleEffects::Logout(pl->currentState.origin, mzParameters.weapon);
        break;
    case MuzzleFlashType::Logout:
        dl->color = vec3_t{1, 0, 0};
        dl->die = cl->time + 1.0f;
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("fx/playerspawn.wav"), 1, Attenuation::Normal, 0);
        ParticleEffects::Logout(pl->currentState.origin, mzParameters.weapon);
        break;
    case MuzzleFlashType::Respawn:
        dl->color = vec3_t{1, 1, 0};
        dl->die = cl->time + 1.0f;
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("fx/playerspawn.wav"), 1, Attenuation::Normal, 0);
        ParticleEffects::Logout(pl->currentState.origin, mzParameters.weapon);
        break;
    }

    if (vid_rtx->integer)
    {
        // don't add muzzle flashes in RTX mode
        //dl->radius = 0.f;
    }
}

/**
*   @brief  Regular Entity Muzzleflash Effects.
**/
void MuzzleFlashEffects::EntityMuzzleFlash() {
    //PODEntity* ent;
    //vec3_t      origin;
    //const vec_t* ofs;
    //cdlight_t* dl;
    //vec3_t      forward, right;

    //// locate the origin
    //ent = &cs->entities[mzParameters.entity];
    //AngleVectors(ent->currentState.angles, &forward, &right, NULL);
    //ofs = vec3_t { 10.6f * 1.2f, 7.7f * 1.2f, 7.8f * 1.2f };
    //origin[0] = ent->currentState.origin[0] + forward[0] * ofs[0] + right[0] * ofs[1];
    //origin[1] = ent->currentState.origin[1] + forward[1] * ofs[0] + right[1] * ofs[1];
    //origin[2] = ent->currentState.origin[2] + forward[2] * ofs[0] + right[2] * ofs[1] + ofs[2];

    //dl = CLG_AllocDLight(mzParameters.entity);
    //dl->origin = origin, dl->origin;
    //dl->radius = 200 + (rand() & 31);
    ////dl->minlight = 32;
    //dl->die = cl->time;  // + 0.1;

    //switch (mzParameters.weapon) {
    //case MZ2_SOLDIER_MACHINEGUN_1:
    //case MZ2_SOLDIER_MACHINEGUN_2:
    //case MZ2_SOLDIER_MACHINEGUN_3:
    //case MZ2_SOLDIER_MACHINEGUN_4:
    //case MZ2_SOLDIER_MACHINEGUN_5:
    //case MZ2_SOLDIER_MACHINEGUN_6:
    //case MZ2_SOLDIER_MACHINEGUN_7:
    //case MZ2_SOLDIER_MACHINEGUN_8:
    //    dl->color = vec3_t{1, 1, 0};
    //    CLG_ParticleEffect(origin, forward, 0, 40);
    //    CLG_SmokeAndFlash(origin);
    //    clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("soldier/solatck3.wav"), 1, Attenuation::Normal, 0);
    //    break;

    //case MZ2_SOLDIER_BLASTER_1:
    //case MZ2_SOLDIER_BLASTER_2:
    //case MZ2_SOLDIER_BLASTER_3:
    //case MZ2_SOLDIER_BLASTER_4:
    //case MZ2_SOLDIER_BLASTER_5:
    //case MZ2_SOLDIER_BLASTER_6:
    //case MZ2_SOLDIER_BLASTER_7:
    //case MZ2_SOLDIER_BLASTER_8:
    //    dl->color = vec3_t{1, 1, 0};
    //    clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("soldier/solatck2.wav"), 1, Attenuation::Normal, 0);
    //    break;

    //case MZ2_SOLDIER_SHOTGUN_1:
    //case MZ2_SOLDIER_SHOTGUN_2:
    //case MZ2_SOLDIER_SHOTGUN_3:
    //case MZ2_SOLDIER_SHOTGUN_4:
    //case MZ2_SOLDIER_SHOTGUN_5:
    //case MZ2_SOLDIER_SHOTGUN_6:
    //case MZ2_SOLDIER_SHOTGUN_7:
    //case MZ2_SOLDIER_SHOTGUN_8:
    //    dl->color = vec3_t{1, 1, 0};
    //    CLG_SmokeAndFlash(origin);
    //    clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("soldier/solatck1.wav"), 1, Attenuation::Normal, 0);
    //    break;
    //}
}