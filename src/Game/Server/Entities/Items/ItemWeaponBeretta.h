/***
*
*	License here.
*
*	@file
*
*   Beretta Weapon.
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseItem;
class SVGBaseItemWeapon;

class ItemWeaponBeretta : public SVGBaseItemWeapon {
public:
    // Constructor/Deconstructor.
    ItemWeaponBeretta(Entity* svEntity, const std::string& displayString, uint32_t identifier);
    virtual ~ItemWeaponBeretta();

    DefineItemMapClass("Beretta", "beretta", ItemID::Beretta, "item_weapon_beretta", ItemWeaponBeretta, SVGBaseItemWeapon);

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
        return "models/weapons/v_beretta/tris.iqm"; 
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
        return "models/weapons/w_beretta/tris.iqm"; 
    };
    /**
    *   @return Returns the model index of the world model.
    **/
    inline virtual const uint32_t GetWorldModelIndex() override {
        return SVG_PrecacheModel(GetWorldModel()); 
    }


    /**
    *
    *   Weapon Instance functionality.
    *
    **/
    /**
    *   @brief  The mother of all instance weapon callbacks. Calls upon the others depending on state.
    **/
    void InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) override;
    /**
    * @brief   A callback which can be implemented by weapons in order to fire code one time
    *          when the weapon has switched to a new state. 
    * 
    *          (Mainly used for setting animations, but can be used for anything really.)
    * 
    * @param newState The current new state that the weapon resides in.
    * @param oldState Old previous state the weapon was residing in.
    **/
    void InstanceWeaponOnSwitchState(SVGBasePlayer *player, SVGBaseItemWeapon* weapon, ServerClient *client,int32_t newState, int32_t oldState) override;
    /**
    *   @brief Called when an animation has finished. Usually used to then switch states.
    **/
    void InstanceWeaponOnAnimationFinished(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) override;

    /**
    *   @brief  Called each frame the weapon is in Draw state.
    **/
    virtual void InstanceWeaponProcessDrawState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called each frame the weapon is in Holster state.
    **/
    virtual void InstanceWeaponProcessHolsterState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);
    /**
    *   @brief  Called each frame the weapon is in Holster state.
    **/
    virtual void InstanceWeaponProcessIdleState(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);


    /**
    *
    *   Callback Functions.
    *
    **/
    /**
    *   @brief  Checks whether to add to inventory or not. In case of adding it 
    *           to the inventory it also checks whether to change weapon or not.
    **/
    qboolean WeaponBerettaPickup(SVGBaseEntity* other);

    /**
    *   @brief Changes the player's weapon to the Beretta if it has one that is.
    **/
    void InstanceWeaponBerettaUse(SVGBaseEntity* user, SVGBaseItem* item);

private:

};
