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

    DefineItemMapClass("Sub Machine Gun", "smg", ItemIdentifier::SMG, "item_weapon_smg", ItemWeaponSMG, SVGBaseItemWeapon);

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
    inline virtual uint32_t GetPrimaryAmmoIdentifier() override { return ItemIdentifier::Ammo9mm; }
    /**
    *   @return The item index of the secondary ammo for this weapon.
    **/
    inline virtual uint32_t GetSecondaryAmmoIdentifier() override { return 0; }

    /**
    *   @return Returns the path to this weapon's view model.
    **/
    inline virtual const std::string GetViewModel() override {
        return "models/weapons/v_smg/tris.iqm"; 
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
        return "models/weapons/w_smg/tris.iqm"; 
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
    virtual void InstanceWeaponThink(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client) override;

    /**
    *   @brief  Callback used for idling a weapon. (Show idle animation, what have ya..)
    **/
    virtual void InstanceWeaponIdle(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);

    /**
    *   @brief  Draw weapon callback.
    **/
    virtual void InstanceWeaponDraw(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);

    /**
    *   @brief  Holster weapon callback.
    **/
    virtual void InstanceWeaponHolster(SVGBasePlayer* player, SVGBaseItemWeapon* weapon, ServerClient* client);


    /**
    *
    *   Callback Functions.
    *
    **/
    /**
    *   @brief  Checks whether to add to inventory or not. In case of adding it 
    *           to the inventory it also checks whether to change weapon or not.
    **/
    qboolean WeaponSMGPickup(SVGBaseEntity* other);

    /**
    *   @brief Changes the player's weapon to the SMG if it has one that is.
    **/
    void InstanceWeaponSMGUse(SVGBaseEntity* user, SVGBaseItem* item);

private:

};
