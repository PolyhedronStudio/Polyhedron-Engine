/*
// LICENSE HERE.

//
// SVGBasePlayer.h
//
//
*/
#pragma once

class SVGBaseEntity;

class SVGBasePlayer : public SVGBaseEntity {
public:
    /**
    *   @brief  Used by game modes to (re-)create a fresh player entity for the client.
    **/
    static SVGBasePlayer* Create(Entity* serverEntity);

private:
    //! Private constructor. Players are created using the Create function.
    SVGBasePlayer( PODEntity *svEntity );
    virtual ~SVGBasePlayer() = default;

public:
    DefineClass( SVGBasePlayer, SVGBaseEntity );

    /***
    * 
    *   Interface implementation functions.
    *
    ***/
    void Precache() override;
    void Spawn() override;


    /***
    * 
    *   Callback functions.
    *
    ***/
    /**
    *   @brief  Callback that is fired any time the player dies. As such, it kindly takes care of doing this.
    **/
    void SVGBasePlayerDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);



    /***
    * 
    *   Player Functions.
    *
    ***/
    /**
    *   @brief  Each player can have two 'noise sources' associated to it. One slot for
    *           player personal noises such as jumpin, pain, firing a weapon. The other
    *           slot for target noise, such as bullets impacting a wall.
    * 
    *           Use your imagination to think of what this is useful for ;-P
    **/
    virtual void PlayerNoise(SVGBaseEntity *noiseEntity, const vec3_t &noiseOrigin, int32_t noiseType);

    /**
    *   @brief  Tosses the player's weapon away from himself.
    **/
    virtual void TossWeapon();


    /***
    * 
    *   Weapon functions.
    *
    ***/
    /**
    *   @brief Gives the player's weapon a chance to "think".
    **/
    virtual void WeaponThink();

    /**
    *   @brief  Adds ammo to the player's inventory.
    *   @return True on success, false on failure. (Meaning the player has too much of that ammo type.)
    **/
    virtual qboolean GiveAmmo(uint32_t ammoIdentifier, uint32_t amount);
    /**
    *   @brief  Takes ammo from the player's inventory.
    *   @return True on success, false on failure. (Meaning the player has no more ammo left of the specific type.)
    **/
    virtual int32_t TakeAmmo(uint32_t ammoIdentifier, uint32_t amount);
    
    /**
    *   @brief  Gives a specific amount of weapon type to the player.
    *   @return True on success. If false, the player is out of ammo( <= 0 ). Assuming the first few sanity checks pass.
    **/
    virtual qboolean GiveWeapon(uint32_t weaponIdentifier, uint32_t amount);
    /**
    *   @brief  Takes away a specific amount of weapon type to the player.
    *   @param  amount  Defaults to 1, set it to more in example: grenades.
    *   @return True on success, false on failure. (Meaning the player has too much of that ammo type.)
    **/
    virtual qboolean TakeWeapon(uint32_t weaponIdentifier, uint32_t amount);

    /**
    *   @return True if the player has any ammo left for this weapon to refill its clip.
    **/
    virtual qboolean CanReloadWeaponClip(uint32_t weaponID);
    /**
    *   @brief  Refills the weapon's ammo clip.
    *   @return True on success, false when the player ran out of ammo to refill with.
    **/
    virtual qboolean ReloadWeaponClip(uint32_t weaponID);
    /**
    *   @brief  Takes ammo from the weapon clip.
    *   @return False if the clip is empty. True otherwise.
    **/
    virtual uint32_t TakeWeaponClipAmmo(uint32_t weaponID, uint32_t amount);
	/**
	*	@return	The amount of ammo currently residing in the player's weapon clip.
	**/
	virtual uint32_t GetWeaponClipAmmoCount( uint32_t weaponID );
    /**
    *   @return The amount this player is holding of the itemIdentifier. (Can be used for ammo, and weapons too.)
    **/
    virtual int32_t HasItem(uint32_t itemIdentifier);

    /**
    *   @brief  Engages the player to change to the new weapon.
    *   @param  weaponIdentifier The identifier used for acquiring the weapon type its instance pointer.
    *   @param  storeLastWeapon If set to true it'll store the current active weapon as its last weapon pointer,
    *           if set to false it'll set the lastWeapon pointer to nullptr.
    *   @return A pointer to the newly activated weapon, nullptr if something went wrong.
    **/
    virtual SVGBaseItemWeapon *ChangeWeapon(int32_t weaponID, qboolean storePreviousActiveWeaponID = true);
    
    /**
    *   @brief  Looks into the player entity's client structure for the active instance item weapon.
    *   @return Pointer to the instance item weapon that is active for the client.
    **/
    virtual SVGBaseItemWeapon *GetActiveWeaponInstance();



    /***
    * 
    *   Get/Set
    *
    ***/
    /**
    *   @brief  Sets the server entity's client pointer.
    **/
    void SetClient(ServerClient* client) { podEntity->client = client; }

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
    inline const GameTime &GetAirFinishedTime() { return airFinishedTime; }
    /**
    *   @brief  Sets the air finished time.
    **/
    inline void SetAirFinishedTime(const GameTime& airFinishedTime) { this->airFinishedTime = airFinishedTime; }
    /**
    *   @return Debounce touch time.
    **/
    inline const GameTime &GetDebounceTouchTime() { return debounceTouchTime; }
    /**
    *   @brief  Sets the debounce touch time.
    **/
    void SetDebounceTouchTime(const GameTime& debounceTouchTime) { this->debounceTouchTime = debounceTouchTime; }
    /**
    *   @return Debounce pain time.
    **/
    inline const GameTime &GetDebouncePainTime() { return debouncePainTime; }
    /**
    *   @brief  Sets the debounce pain time.
    **/
    void SetDebouncePainTime(const GameTime& debouncePainTime) { this->debouncePainTime = debouncePainTime; }
    /**
    *   @return Debounce damage time.
    **/
    inline const GameTime &GetDebounceDamageTime() { return debounceDamageTime; }
    /**
    *   @brief  Sets the debounce damage time.
    **/
    void SetDebounceDamageTime(const GameTime& debounceDamageTime) { this->debounceDamageTime = debounceDamageTime; }
    /**
    *   @return Debounce sound time.
    **/
    inline const GameTime &GetDebounceSoundTime() { return debounceSoundTime; }
    /**
    *   @brief  Sets the debounce sound time.
    **/
    void SetDebounceSoundTime(const GameTime& debounceSoundTime) { this->debounceSoundTime = debounceSoundTime; }
    /**
    *   @return The time till the next drown event.
    **/
    inline const GameTime GetNextDrownTime() { return GetClient()->nextDrownTime; }
    /**
    *   @brief  Sets the time for the next drown event to occure.
    **/
    inline void SetNextDrownTime(const GameTime& nextDrownTime) { GetClient()->nextDrownTime = nextDrownTime; }
    
    /**
    *   @return The time when this player is allowed to respawn again.
    **/
    inline GameTime GetRespawnTime() { return GetClient()->respawnTime; }
    /**
    *   @brief  Sets the next respawn time for this player.
    **/
    inline void SetRespawnTime(const GameTime &time) { GetClient()->respawnTime = time; }


protected:
    // The level.time when the "air" state finished. 
    GameTime airFinishedTime = GameTime::zero();

    // Debounce level.time values.
    GameTime debounceTouchTime = GameTime::zero();
    GameTime debouncePainTime = GameTime::zero();
    GameTime debounceDamageTime = GameTime::zero();
    GameTime debounceSoundTime = GameTime::zero();


    // View and BobMove functionality.
public:
    // BobMoveCycle is used for view bobbing, where the player FPS view looks like he is
    // walking instead of floating around.
    struct BobMoveCycle {
        // Forward, right, and up vectors.
        vec3_t  forward = vec3_zero(), right = vec3_zero(), up = vec3_zero();
        // Speed squared over the X/Y axis.
        float XYSpeed = 0.f;
        // bobMove counter.
        float move = 0.f;
        // Cycles are caculated over bobMove, uneven cycles = right foot.
        int cycle = 0;
        // Calculated as: // sin(bobfrac*M_PI)
        float fracSin = 0.f;
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
    void LookAtKiller(IServerGameEntity* inflictor, IServerGameEntity* attacker);

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