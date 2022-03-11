/***
*
*	License here.
*
*	@file
*
*	'None' weapon, currently functions as a stub for ID = 0.
* 
*   Can of course be modified to function as literal arms.
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseItem;
class SVGBaseItemWeapon;

class ItemWeaponNone: public SVGBaseItemWeapon {
public:
    // Constructor/Deconstructor.
    ItemWeaponNone(Entity* svEntity, const std::string& displayString, uint32_t identifier);
    virtual ~ItemWeaponNone();

    DefineItemMapClass("None", "none", ItemID::Barehands, "item_weapon_none", ItemWeaponNone, SVGBaseItemWeapon);

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
    inline virtual uint32_t GetPrimaryAmmoIdentifier() override { return 0; }
    /**
    *   @return The item index of the secondary ammo for this weapon.
    **/
    inline virtual uint32_t GetSecondaryAmmoIdentifier() override { return 0; }

    /**
    *   @return Returns the path to this weapon's view model.
    **/
    inline virtual const std::string GetViewModel() override {
        return ""; 
    };
    /**
    *   @return Returns the model index of the view model.
    **/
    inline virtual const uint32_t GetViewModelIndex() override { 
        return 0; 
    }

    /**
    *   @return Returns the path to this weapon's vorld model.
    **/
    inline virtual const std::string GetWorldModel() override { 
        return ""; 
    };
    /**
    *   @return Returns the model index of the world model.
    **/
    inline virtual const uint32_t GetWorldModelIndex() override {
        return 0; 
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
    *
    *   Callback Functions.
    *
    **/
    /**
    *   @brief Changes the player's weapon to the SMG if it has one that is.
    **/
    void InstanceWeaponNoneUse(SVGBaseEntity* user, SVGBaseItem* item);

private:

};
