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

class SVGBaseItemAmmo : public SVGBaseItem {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    using WeaponThinkCallbackPointer = void (SVGBaseItem::*)(SVGBaseEntity* user);


    //! Constructor/Deconstructor.
    SVGBaseItemAmmo(PODEntity *svEntity, const std::string& displayString, uint32_t identifier);
    virtual ~SVGBaseItemAmmo();

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseItemAmmo, SVGBaseItem);


    /***
    * 
    *   Interface functions.
    *
    ***/
    virtual void Precache() override;
    virtual void Spawn() override;



    /***
    * 
    *   Entity functions.
    * 
    ***/
    /**
    *   @return The maximum ammo cap limit to carry around for this ammo type.
    **/
    virtual uint32_t GetCapLimit() { return 0; }


    /**
    *   @return Pointer to an ammo item instance. Does a typeinfo check to make sure. 
    *           Returns nullptr if not found or a type mismatch occures.
    **/
    static SVGBaseItemAmmo* GetAmmoInstanceByID(uint32_t identifier) {
	    // Get the regular instance.
        SVGBaseItem* itemInstance = GetItemInstanceByID(identifier);

        // Do a type check and return it casted to SVGBaseItemAmmo
	    if (itemInstance->IsSubclassOf<SVGBaseItemAmmo>()) {
		    return dynamic_cast<SVGBaseItemAmmo*>(itemInstance);
	    } else {
		    return nullptr;
	    }
    }
    ///**
    //*   @return Returns the path to this weapon's vorld model.
    //**/
    //inline virtual const std::string    GetWorldModel() { return ""; };
    ///**
    //*   @return Returns the model index of the world model.
    //**/
    //inline virtual const uint32_t       GetWorldModelIndex() { return 0; };


public:
    // Sets the 'WeaponThink' callback function.
    //template<typename function>
    //inline void SetWeaponThinkCallback(function f)
    //{
    //    weaponThinkFunction = static_cast<WeaponThinkCallbackPointer>(f);
    //}
    //inline qboolean HasWeaponThinkCallback() {
    //    return (weaponThinkFunction != nullptr ? true : false);
    //}

protected:
    /***
    * 
    *   Weapon Item function pointers.
    *
    ***/
    //WeaponThinkCallbackPointer  weaponThinkFunction = nullptr;
};