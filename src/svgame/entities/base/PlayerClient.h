/*
// LICENSE HERE.

//
// PlayerClient.h
//
//
*/
#ifndef __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__
#define __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__

class SVGBaseEntity;

class PlayerClient : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    PlayerClient(Entity* svEntity);
    virtual ~PlayerClient();

    //
    // Interface functions. 
    //
    void Precache();    // Precaches data.
    void Spawn();       // Spawns the entity.
    void Respawn();     // Respawns the entity.
    void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think();       // General entity thinking routine.

    //
    // Callback functions.
    //
    void PlayerClientDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);

    //
    // Get/Set
    //
    // Active Weapon.
    inline gitem_t *GetActiveWeapon() {
        return GetClient()->persistent.activeWeapon;
    }
    inline void GetActiveWeapon(gitem_t *weapon) {
        GetClient()->persistent.activeWeapon = weapon;
    }

    // Animation EndFrame.
    inline const int32_t GetAnimationEndFrame() {
        return GetClient()->animation.endFrame;
    }
    inline void SetAnimationEndFrame(const int32_t& endFrame) {
        GetClient()->animation.endFrame = endFrame;
    }

    // Client.
    // Sets the 'client' pointer.
    void SetClient(gclient_s *client) {
        serverEntity->client = client;
    }

    // Killer Yaw.
    inline void SetKillerYaw(const float& killerYaw) {
        GetClient()->killerYaw = killerYaw;
    }
    inline const float GetKillerYaw() {
        return GetClient()->killerYaw;
    }

    // Player Move Type.
    inline const int32_t GetPlayerMoveType() {
        return GetClient()->playerState.pmove.type;
    }
    inline void SetPlayerMoveType(const int32_t& type) {
        GetClient()->playerState.pmove.type = type;
    }

    // Priority animation.
    inline const int32_t GetPriorityAnimation() {
        return GetClient()->animation.priorityAnimation;
    }
    inline void SetPriorityAnimation(const int32_t &priorityAnimation) {
        GetClient()->animation.priorityAnimation = priorityAnimation;
    }

    // RespawnTime.
    inline float GetRespawnTime() {
        return GetClient()->respawnTime;
    }
    inline void SetRespawnTime(float time) {
        GetClient()->respawnTime = time;
    }

private:
    //
    // Private utility functions.
    //
    void LookAtKiller(SVGBaseEntity* inflictor, SVGBaseEntity* attacker);
};

#endif // __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__