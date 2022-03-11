/***
*
*	License here.
*
*	@file
*
*	Sub Machine Gun weapon.
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseItem;
class SVGBaseItemWeapon;

class ItemWeaponSMG : public SVGBaseItemWeapon {
public:
    // Constructor/Deconstructor.
    ItemWeaponSMG(Entity* svEntity, const std::string& displayString, uint32_t identifier);
    virtual ~ItemWeaponSMG();

    DefineItemMapClass("Sub Machine Gun", "smg", ItemID::SMG, "item_weapon_smg", ItemWeaponSMG, SVGBaseItemWeapon);

    // Item flags
    //static constexpr int32_t IF_xxx = 1 << 0;
    //static constexpr int32_t IF_xxx = 1 << 1;

    //
    // Interface functions.
    //
    virtual void Precache() override;   // Precaches data.
    virtual void Spawn() override;      // Spawns the entity.
    virtual void Respawn() override;	// Respawns the entity.
    virtual void PostSpawn() override;	// PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think() override;      // General entity thinking routine.

    /**
    *
    *   Instance Interface implementation functions.
    *
    **/
protected:
    /**
    *   @brief  Only for use by CreateIns
    **/
    virtual void InstanceSpawn() override;


public:
    /**
    *   @return The maximum amount one is allowed to carry of this weapon.
    **/
    inline virtual uint32_t GetCarryLimit() override { return 1; }

    /**
    *   @return The item index of the primary ammo for this weapon.
    **/
    inline virtual uint32_t GetPrimaryAmmoIdentifier() override { return ItemID::Ammo9mm; }
    /**
    *   @return The item index of the secondary ammo for this weapon.
    **/
    inline virtual uint32_t GetSecondaryAmmoIdentifier() override { return 0; }

    /**
    *   @return Returns the path to this weapon's view model.
    **/
    inline virtual const std::string GetViewModel() override {
        return "models/weapons/smg45/v_smg45.iqm"; 
    };
    /**
    *   @return Returns the model index of the view model.
    **/
    inline virtual const uint32_t GetViewModelIndex() override { 
        return SVG_PrecacheModel( GetViewModel() ); 
    }

    /**
    *   @return Returns the path to this weapon's vorld model.
    **/
    inline virtual const std::string GetWorldModel() override { 
        return "models/weapons/smg45/w_smg45.iqm"; 
    };
    /**
    *   @return Returns the model index of the world model.
    **/
    inline virtual const uint32_t GetWorldModelIndex() override {
        return SVG_PrecacheModel(GetWorldModel()); 
    }


public:
    /**
    *
    *   Weapon Instance functionality.
    *
    **/
    /**
    *   @brief  The mother of all instance weapon callbacks. Calls upon the others depending on state.
    **/
    void InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) final;


    /**
    * @brief   A callback which can be implemented by weapons in order to fire code one time
    *          when the weapon has switched to a new state. 
    * 
    *          (Mainly used for setting animations, but can be used for anything really.)
    * 
    * @param newState The current new state that the weapon resides in.
    * @param oldState Old previous state the weapon was residing in.
    **/
    void InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient *client,int32_t newState, int32_t oldState) final;
    /**
    *   @brief Called when an animation has finished. Usually used to then switch states.
    **/
    void InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) final;


    /**
    *   @brief  Called each frame the weapon is in Holster state.
    **/
    virtual void InstanceWeaponProcessHolsterState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) final;
    /**
    *   @brief  Called each frame the weapon is in Draw state.
    **/
    virtual void InstanceWeaponProcessDrawState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) final;
    /**
    *   @brief  Called each frame the weapon is in Idle state.
    **/
    virtual void InstanceWeaponProcessIdleState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) final;
    /**
    *   @brief  Called each frame the weapon is in Reload state.
    **/
    virtual void InstanceWeaponProcessReloadState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called each frame the weapon is in 'Primary Fire' state.
    **/
    virtual void InstanceWeaponProcessPrimaryFireState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) final;
    /**
    *   @brief  Called each frame the weapon is in 'Secondary Fire' state.
    **/
    virtual void InstanceWeaponProcessSecondaryFireState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) final;

public:
    /**
    *
    *   Callback Functions.
    *
    **/
    /**
    *   @brief  Checks whether to add to inventory or not. If added to a client's
    *           inventory it'll also engage a weapon switch.
    **/
    qboolean WeaponSMGPickup(SVGBaseEntity* other);

    /**
    *   @brief Changes the player's weapon to the SMG if it has one that is.
    **/
    void InstanceWeaponSMGUse(SVGBaseEntity* user, SVGBaseItem* item);

private:

};
