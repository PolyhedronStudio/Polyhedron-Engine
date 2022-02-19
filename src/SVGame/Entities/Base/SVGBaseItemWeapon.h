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
    using WeaponThinkCallbackPointer    = void(SVGBaseItem::*)(SVGBaseEntity* user);


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
	    if (itemInstance->IsSubclassOf<SVGBaseItemWeapon>()) {
	        return dynamic_cast<SVGBaseItemWeapon*>(itemInstance);
	    } else {
	        return nullptr;
	    }
    }

    /**
    *   @return The maximum amount one is allowed to carry of this weapon.
    **/
    inline virtual uint32_t GetCarryLimit() {
        return 0;
    }

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
    // The following functions look up a callback and fire it accordingly.
    // This may seem a bit convoluted or unnecesarry however it allows
    // for exchanging callbacks. By doing so, weapons can have more variety
    // and we won't clutter it all with if statements.
    //
    void WeaponThink(SVGBasePlayer* player);

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