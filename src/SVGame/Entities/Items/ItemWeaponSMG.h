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
    virtual void		     InstanceSpawn();


public:
    inline virtual const std::string GetViewModel() override {
        return "models/weapons/v_smg/tris.iqm"; 
    };
    inline virtual const uint32_t GetViewModelIndex() { 
        return SVG_PrecacheModel( GetViewModel() ); 
    }
    inline virtual const std::string GetWorldModel() override { 
        return "models/weapons/w_smg/tris.iqm"; 
    };
    inline virtual const uint32_t GetWorldModelIndex() {
        return SVG_PrecacheModel(GetWorldModel()); 
    }

    //
    //  
    //
    void WeaponSMGThink(void);

    //
    // Callback Functions.
    //
    /**
    *   @brief  Checks whether to add to inventory or not. In case of adding it 
    *           to the inventory it also checks whether to change weapon or not.
    **/
    qboolean WeaponSMGPickup(SVGBaseEntity* other);

    /**
    *   @brief Checks if it should switch weapon, and does so if required.
    **/
    void WeaponSMGUseInstance(SVGBaseEntity* user, SVGBaseItem* item);

private:

};
