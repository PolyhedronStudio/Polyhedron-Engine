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
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void Respawn() override;     // Respawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value)  override;

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

    // airFinishedTime
    inline const float GetAirFinishedTime() {
        return airFinishedTime;
    }
    inline void SetAirFinishedTime(const float& airFinishedTime) {
        this->airFinishedTime = airFinishedTime;
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

    // Debounce Touch Time.
    inline const float GetDebounceTouchTime() {
        return debounceTouchTime;
    }
    void SetDebounceTouchTime(const float& debounceTouchTime) {
        this->debounceTouchTime = debounceTouchTime;
    }

    // Debounce Pain Time.
    inline const float GetDebouncePainTime() {
        return debouncePainTime;
    }
    void SetDebouncePainTime(const float& debouncePainTime) {
        this->debouncePainTime = debouncePainTime;
    }

    // Debounce Damage Time.
    inline const float GetDebounceDamageTime() {
        return debounceDamageTime;
    }
    void SetDebounceDamageTime(const float& debounceDamageTime) {
        this->debounceDamageTime = debounceDamageTime;
    }

    // Debounce Sound Time.
    inline const float GetDebounceSoundTime() {
        return debounceSoundTime;
    }
    void SetDebounceSoundTime(const float& debounceSoundTime) {
        this->debounceSoundTime = debounceSoundTime;
    }

    // Killer Yaw.
    inline void SetKillerYaw(const float& killerYaw) {
        GetClient()->killerYaw = killerYaw;
    }
    inline const float GetKillerYaw() {
        return GetClient()->killerYaw;
    }

    // NextDrawnTime.
    inline const float GetNextDrownTime() {
        return GetClient()->nextDrownTime;
    }
    inline void SetNextDrownTime(const float& nextDrownTime) {
        GetClient()->nextDrownTime = nextDrownTime;
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

protected:
    // The level.time when the "air" state finished. 
    float airFinishedTime;

    // Debounce level.time values.
    float debounceTouchTime;
    float debouncePainTime;
    float debounceDamageTime;
    float debounceSoundTime;

private:
    //
    // Private utility functions.
    //
    void LookAtKiller(SVGBaseEntity* inflictor, SVGBaseEntity* attacker);
};

#endif // __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__