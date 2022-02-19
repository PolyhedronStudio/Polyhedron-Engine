/*
// LICENSE HERE.

//
// SVGBasePlayer.h
//
//
*/
#ifndef __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__
#define __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__

class SVGBaseEntity;

class SVGBasePlayer : public SVGBaseEntity {
public:
    /**
    *   @brief  Used by game modes to (re-)create a fresh player entity for the client.
    **/
    static SVGBasePlayer* Create(Entity* serverEntity);

private:
    //! Private constructor. Players are created using the Create function.
    SVGBasePlayer(Entity* svEntity);
    virtual ~SVGBasePlayer();

public:
    DefineClass(SVGBasePlayer, SVGBaseEntity);

    /***
    * 
    *   Interface implementation functions.
    *
    ***/
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void Respawn() override;     // Respawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value)  override;

    /***
    * 
    *   Callback functions.
    *
    ***/
    void SVGBasePlayerDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);


    /***
    * 
    *   Weapon functions.
    *
    ***/
    /**
    *   @brief  Adds ammo to the player's inventory.
    *   @return True on success, false on failure. (Meaning the player has too much of that ammo type.)
    **/
    qboolean AddAmmo(uint32_t ammoIdentifier, uint32_t amount);
    /**
    *   @brief  Takes ammo from the player's inventory.
    *   @return True on success, false on failure. (Meaning the player has no more ammo left of the specific type.)
    **/
    qboolean TakeAmmo(uint32_t ammoIdentifier, uint32_t amount);

    /**
    *   @brief  Adds ammo to the player's inventory.
    *   @return True on success, false on failure. (Meaning the player has too much of that ammo type.)
    **/
    // ...

    /***
    * 
    *   Get/Set
    *
    ***/
    // Active Weapon.
    /**
    *   @return A pointer to the active weapon instance item. nullptr otherwise.
    **/
    inline SVGBaseItemWeapon* GetActiveWeapon() { return GetClient()->persistent.activeWeapon; }
    /**
    *   @brief  Set the active weapon.
    *   @param  weapon  Expected to be an instance item.
    **/
    inline void	SetActiveWeapon(SVGBaseItemWeapon* weapon) { GetClient()->persistent.activeWeapon = weapon; }
    
    /**
    *   @brief  Sets the server entity's client pointer.
    **/
    void SetClient(gclient_s* client) { serverEntity->client = client; }

    /**
    *   @return The killer yaw.
    **/
    inline const float GetKillerYaw() { return GetClient()->killerYaw; }
    /**
    *   @brief  Sets the killer yaw.
    **/
    inline void SetKillerYaw(const float& killerYaw) { GetClient()->killerYaw = killerYaw; }

    /**
    *   @return Current movetype of the player.
    **/
    inline const int32_t GetPlayerMoveType() { return GetClient()->playerState.pmove.type; }
    /**
    *   @brief  Sets the movetype of the player.
    **/
    inline void SetPlayerMoveType(const int32_t& type) { GetClient()->playerState.pmove.type = type; }

    /**
    *   @return The frame of when a client's animation "ends".
    **/
    inline const float GetAnimationEndFrame() { return GetClient()->animation.endFrame; }
    /**
    *   @brief  Sets the frame of when a client's animation "ends".
    **/
    inline void	SetAnimationEndFrame(const float& endFrame) { GetClient()->animation.endFrame = endFrame; }
    /**
    *   @return Current animation that is prioritized for this player.
    **/
    inline const float GetPriorityAnimation() { return GetClient()->animation.priorityAnimation; }
    /**
    *   @brief  Sets the new prioritized animation.
    **/
    inline void SetPriorityAnimation(const float& priorityAnimation) { GetClient()->animation.priorityAnimation = priorityAnimation; }

    /**
    *   @return Time at which air has been finished.
    **/
    inline const float GetAirFinishedTime() { return airFinishedTime; }
    /**
    *   @brief  Sets the air finished time.
    **/
    inline void SetAirFinishedTime(const float& airFinishedTime) { this->airFinishedTime = airFinishedTime; }
    /**
    *   @return Debounce touch time.
    **/
    inline const float GetDebounceTouchTime() { return debounceTouchTime; }
    /**
    *   @brief  Sets the debounce touch time.
    **/
    void SetDebounceTouchTime(const float& debounceTouchTime) { this->debounceTouchTime = debounceTouchTime; }
    /**
    *   @return Debounce pain time.
    **/
    inline const float GetDebouncePainTime() { return debouncePainTime; }
    /**
    *   @brief  Sets the debounce pain time.
    **/
    void SetDebouncePainTime(const float& debouncePainTime) { this->debouncePainTime = debouncePainTime; }
    /**
    *   @return Debounce damage time.
    **/
    inline const float GetDebounceDamageTime() { return debounceDamageTime; }
    /**
    *   @brief  Sets the debounce damage time.
    **/
    void SetDebounceDamageTime(const float& debounceDamageTime) { this->debounceDamageTime = debounceDamageTime; }
    /**
    *   @return Debounce sound time.
    **/
    inline const float GetDebounceSoundTime() { return debounceSoundTime; }
    /**
    *   @brief  Sets the debounce sound time.
    **/
    void SetDebounceSoundTime(const float& debounceSoundTime) { this->debounceSoundTime = debounceSoundTime; }
    /**
    *   @return The time till the next drown event.
    **/
    inline const float GetNextDrownTime() { return GetClient()->nextDrownTime; }
    /**
    *   @brief  Sets the time for the next drown event to occure.
    **/
    inline void SetNextDrownTime(const float& nextDrownTime) { GetClient()->nextDrownTime = nextDrownTime; }
    
    /**
    *   @return The time when this player is allowed to respawn again.
    **/
    inline float GetRespawnTime() { return GetClient()->respawnTime; }
    /**
    *   @brief  Sets the next respawn time for this player.
    **/
    inline void SetRespawnTime(float time) { GetClient()->respawnTime = time; }


protected:
    // The level.time when the "air" state finished. 
    float airFinishedTime;

    // Debounce level.time values.
    float debounceTouchTime;
    float debouncePainTime;
    float debounceDamageTime;
    float debounceSoundTime;


    // View and BobMove functionality.
public:
    // BobMoveCycle is used for view bobbing,
    // where the player FPS view looks like he is
    // walking instead of floating around.
    struct BobMoveCycle {
        // Forward, right, and up vectors.
        vec3_t  forward, right, up;
        // Speed squared over the X/Y axis.
        float XYSpeed;
        // bobMove counter.
        float move;
        // Cycles are caculated over bobMove, uneven cycles = right foot.
        int cycle;
        // Calculated as: // sin(bobfrac*M_PI)
        float fracSin;
    } bobMove;

    // Calculates the roll value that can be used for the view, 
    virtual float CalculateRoll(const vec3_t& angles, const vec3_t& velocity);

    // Check for which waterlevel (drowning), lava(burning) etc needs to
    // happen, if any.
    virtual void CheckWorldEffects();

    // Detect hitting the floor, and apply damage appropriately.
    virtual void CheckFallingDamage();

    // Apply all other the damage taken this frame
    virtual void ApplyDamageFeedback();

    // Determine the new frame's view offsets
    virtual void CalculateViewOffset();

    // Determine the gun offsets
    virtual void CalculateGunOffset();

    // Determine the full screen color blend
    // must be after viewOffset, so eye contents can be
    // accurately determined
    // FIXME: with client prediction, the contents
    // should be determined by the client
    virtual void CalculateScreenBlend();

    virtual void UpdateEvent();
    virtual void UpdateEffects();
    virtual void UpdateSound();
    virtual void UpdateAnimationFrame();

    // Reference to BobMoveCycle.
    BobMoveCycle &GetBobMoveCycle() {
        return bobMove;
    }

    // Private utility functions.
private:
    /**
    *   @brief  Will ensure the player client sets it view looking at its killer.
    **/
    void LookAtKiller(SVGBaseEntity* inflictor, SVGBaseEntity* attacker);

    //Adds the specific blend of colors on top of each other.
    static void AddScreenBlend(float r, float g, float b, float a, float *v_blend)
    {
        float   a2, a3;

        if (a <= 0)
            return;
        a2 = v_blend[3] + (1 - v_blend[3]) * a; // new total alpha
        a3 = v_blend[3] / a2;   // fraction of color from old

        v_blend[0] = v_blend[0] * a3 + r * (1 - a3);
        v_blend[1] = v_blend[1] * a3 + g * (1 - a3);
        v_blend[2] = v_blend[2] * a3 + b * (1 - a3);
        v_blend[3] = a2;
    }
};

#endif // __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__