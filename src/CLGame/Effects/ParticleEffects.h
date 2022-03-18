/***
*
*	License here.
*
*	@file
*
*	Client Game Particle Effects.
* 
***/
#pragma once

//---------------------------------------------------------------------
// Client Game View IMPLEMENTATION.
//---------------------------------------------------------------------
class ParticleEffects {
public:
    /**
    *   Tweak Settings.
    **/
    static constexpr int32_t ParticleGravity    = 120;
    static constexpr float InstantParticle      = -10000.f;

    /**
    *
    *    Particle Effects.
    * 
    **/
    /**
    *   @brief  'Blood Splatters' like particle effect.
    **/
    static void BloodSplatters(const vec3_t &origin, const vec3_t &direction, int32_t color, int32_t count);

    /**
    *   @brief  'Bubble Trail A' like particle effect.
    **/
    static void BubbleTrailA(const vec3_t &start, const vec3_t &end);
    /**
    *   @brief  'Bubble Trail B' like particle effect.
    **/
    static void BubbleTrailB(const vec3_t &start, const vec3_t &end, int32_t distance);

    /**
    *   @brief  'Dirt' and 'Sparks' like particle effect.
    **/
    static void DirtAndSparks(const vec3_t &origin, const vec3_t &direction, int32_t color, int32_t count);

    /**
    *   @brief  'Diminishing Trail' like particle effect.
    **/
    static void DiminishingTrail(const vec3_t &start, const vec3_t &end, ClientEntity *oldTrailEntity, int32_t flags = 0);

    /**
    *   @brief  'Explosion Sparks' like particle effect.
    **/
    static void ExplosionSparks(const vec3_t &origin);

    /**
    *   @brief  'Force Wall' particle effect.
    **/
    static void ForceWall(const vec3_t &start, const vec3_t &end, int32_t color);

    /**
    *   @brief  'Heat Beam' particle effect.
    **/
    static void HeatBeam(const vec3_t &start, const vec3_t &forward);

    /**
    *   @brief  'ItemRespawn' like particle effect.
    **/
    static void ItemRespawn(const vec3_t& origin);

    /**
    *   @brief  'Logout' like particle effect.
    **/
    static void Logout(const vec3_t& origin, int32_t type);

    /**
    *   @brief  'Monster Plasma Shell' like particle effect.
    **/
    static void MonsterPlasmaShell(const vec3_t &origin);

    /**
    *   @brief  'Steam Puffs' like particle effect.
    **/
    static void SteamPuffs(const vec3_t &origin, const vec3_t &direction, int32_t color, int32_t count, int32_t magnitude);

    /**
    *   @brief  'Teleporter' like particle effect.
    **/
    static void Teleporter(const vec3_t& origin);

    /**
    *   @brief  'Water Splash' like particle effect.
    **/
    static void WaterSplash(const vec3_t &origin, const vec3_t &direction, int32_t color, int32_t count);
};
