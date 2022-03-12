/***
*
*	License here.
*
*	@file
*
*	Base weapon item class. Provides instance functionalities for weapon items.
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;
class SVGBaseItem;

class SVGBaseItemWeapon : public SVGBaseItem {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    using WeaponThinkCallbackPointer    = void(SVGBaseItemWeapon::*)(SVGBaseEntity* user, SVGBaseItemWeapon *weapon, ServerClient *client);


    //! Constructor/Deconstructor.
    SVGBaseItemWeapon(Entity* svEntity, const std::string& displayString, uint32_t identifier);
    virtual ~SVGBaseItemWeapon();

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseItemWeapon, SVGBaseItem);


    /***
    * 
    *   Interface functions.
    *
    ***/
    virtual void Precache() override;
    virtual void Spawn() override;
    virtual void Respawn() override;
    virtual void PostSpawn() override;
    virtual void Think() override;


    /***
    * 
    *   Instance Weapon functions.
    * 
    *   Keep in mind that these operate purely on data stored in the player entity, 
    *   its client, and depending on the case, also 
    *   
    *
    ***/
    /**
    *   @brief  Only called when allowed to think.
    **/
    virtual void InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  When the weapon isn't occupied processing other states, will switch it to reload state.
    **/
    virtual void InstanceWeaponReload(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
 
    /**
    *   @brief  Updates the view model(client gun index) and ammo display (client ammo index)
    *           to that of the weapon pointer passed to it. In case of an invalid weapon item
    *           (nullptr), it'll set the gun and ammo indices to 0.
    **/
    virtual void InstanceWeaponUpdateViewModel(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    
    
    /**
    * @brief    Sets the weapon's animation properties.
    * @param    frameTime Determines the time taken for each frame, this can be used to either speed up or slow down an animation.
    **/
    virtual void InstanceWeaponSetAnimation(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient *client, 
        int64_t startTime, int32_t startFrame, int32_t endFrame, int32_t loopCount = 0, qboolean forceLoop = false, float frameTime = ANIMATION_FRAMETIME);
    /**
    *   @brief  Call whenever an animation needs to be processed for another game frame.
    **/
    virtual void InstanceWeaponProcessAnimation(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called only once during the same frame when the weapon has finished playing an animation.
    **/
    virtual void InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);


    /**
    *   @brief  Instantly sets the current state.
    **/
    virtual void InstanceWeaponSetCurrentState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient* client, int32_t state);
    /** 
    *   @brief  Queues a state and sets it as the next current state when the state currently processing has finished.
    **/
    virtual void InstanceWeaponQueueNextState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient* client, int32_t state);
    /**
    * @brief   A callback which can be implemented by weapons in order to fire code one time
    *          when the weapon has switched to a new state. 
    * 
    *          (Mainly used for setting animations, but can be used for anything really.)
    * 
    * @param newState The current new state that the weapon resides in.
    * @param oldState Old previous state the weapon was residing in.
    **/
    virtual void InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient *client,int32_t newState, int32_t oldState);

    /**
    *   @brief  Called each frame the weapon is in Holster state.
    **/
    virtual void InstanceWeaponProcessHolsterState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called each frame the weapon is in Draw state.
    **/
    virtual void InstanceWeaponProcessDrawState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called each frame the weapon is in Idle state.
    **/
    virtual void InstanceWeaponProcessIdleState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called each frame the weapon is in Reload state.
    **/
    virtual void InstanceWeaponProcessReloadState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called each frame the weapon is in 'Primary Fire' state.
    **/
    virtual void InstanceWeaponProcessPrimaryFireState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called each frame the weapon is in 'Secondary Fire' state.
    **/
    virtual void InstanceWeaponProcessSecondaryFireState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);

    /**
    *   @brief  Called to execute the animation of the current weaponstate.
    **/
    //virtual void InstanceWeaponAnimate(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    
    /***
    * 
    *   Entity functions.
    * 
    *   NOTE:   For primary and secondary ammo, returning 0 means that the
    *           primary/secondary ammo isn't in use for this weapon.
    *
    ***/
    /**
    *   @return Pointer to a weapon item instance. Does a typeinfo check to make sure. 
    *           Returns nullptr if not found or a type mismatch occures.
    **/
    static SVGBaseItemWeapon* GetWeaponInstanceByID(uint32_t identifier) {
	    // Get the regular instance.
	    SVGBaseItem* itemInstance = GetItemInstanceByID(identifier);

	    // Do a type check and return it casted to SVGBaseItemAmmo
        return dynamic_cast<SVGBaseItemWeapon*>(itemInstance);
    }

    /**
    *   @return The maximum amount one is allowed to carry of this weapon.
    **/
    inline virtual uint32_t GetCarryLimit() { return 0; }
    /**
    *   @return The maximum amount of ammo a clip can contain.
    **/
    inline virtual uint32_t GetClipAmmoLimit() { return 0; }

    /**
    *   @return The item index of the primary ammo for this weapon.
    **/
    inline virtual uint32_t GetPrimaryAmmoIdentifier() { return 0; }
    /**
    *   @return The item index of the secondary ammo for this weapon.
    **/
    inline virtual uint32_t GetSecondaryAmmoIdentifier() { return 0; }

    /**
    *   @return Returns the path to this weapon's view model.
    **/
    inline virtual const std::string    GetViewModel() { return ""; };
    /**
    *   @return Returns the model index of the view model.
    **/
    inline virtual const uint32_t       GetViewModelIndex() { return 0; };
    /**
    *   @return Returns the path to this weapon's vorld model.
    **/
    inline virtual const std::string    GetWorldModel() { return ""; };
    /**
    *   @return Returns the model index of the world model.
    **/
    inline virtual const uint32_t       GetWorldModelIndex() { return 0; };

    virtual void SetRespawn(const float delay) override;	 // Sets the item in respawn mode.

    //
    // The following function looks up a callback and will fire it accordingly.
    // This may seem a bit convoluted or unnecesarry however it allows
    // for exchanging callbacks. By doing so, weapons can have more variety
    // and we won't clutter it all with if statements.
    //
    //virtual void WeaponSMGIdle(SVGBasePlayer* player, ServerClient* client);

public:

    // Sets the 'WeaponThink' callback function.
    template<typename function>
    inline void SetWeaponThinkCallback(function f)
    {
        weaponThinkFunction = static_cast<WeaponThinkCallbackPointer>(f);
    }
    inline qboolean HasWeaponThinkCallback() {
        return (weaponThinkFunction != nullptr ? true : false);
    }

protected:
    /***
    * 
    *   Weapon Item function pointers.
    *
    ***/
    WeaponThinkCallbackPointer  weaponThinkFunction = nullptr;
};