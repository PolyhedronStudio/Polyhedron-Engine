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
    //
    // Function pointers for item callbacks.
    // 
    //using PickupCallbackPointer         = qboolean(SVGBaseItem::*)(SVGBaseEntity *other);
    //using UseItemWeaponCallbackPointer  = void(SVGBaseItem::*)(SVGBaseEntity* other);
    //using DropCallbackPointer           = void(SVGBaseItem::*)(SVGBaseEntity* other);
    
    using ItemWeaponThinkCallbackPointer    = void(SVGBaseItem::*)(SVGBaseEntity* other);

    //
    // Constructor/Deconstructor.
    //
    SVGBaseItemWeapon(Entity* svEntity, const std::string& displayString, uint32_t identifier);
    virtual ~SVGBaseItemWeapon();

    DefineAbstractClass(SVGBaseItemWeapon, SVGBaseItem);

    //
    // Interface functions. 
    //
    virtual void Precache() override;    // Precaches data.
    virtual void Spawn() override;       // Spawns the entity.
    virtual void Respawn() override;     // Respawns the entity.
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think() override;       // General entity thinking routine.

    //
    // Entity functions.
    //
    inline virtual const std::string GetViewModel() { return ""; };
    inline virtual const uint32_t GetViewModelIndex() { return 0; };
    inline virtual const std::string GetWorldModel() { return ""; };
    inline virtual const uint32_t    GetWorldModelIndex() { return 0; };

    virtual void SetRespawn(const float delay) override;	 // Sets the item in respawn mode.

    //
    // The following functions look up a callback and fire it accordingly.
    // This may seem a bit convoluted or unnecesarry however it allows
    // for exchanging callbacks. By doing so, weapons can have more variety
    // and we won't clutter it all with if statements.
    //
    void WeaponThink(SVGBasePlayer* player);

public:


    //
    // Ugly, but effective callback SET methods.
    //
    // Sets the 'Pickup' callback function.
    //template<typename function>
    //inline void SetPickupCallback(function f)
    //{
    //    pickupFunction = static_cast<PickupCallbackPointer>(f);
    //}
    //inline qboolean HasPickupCallback() {
    //    return (pickupFunction != nullptr ? true : false);
    //}
    //// Sets the 'Use Item' callback function.
    //template<typename function>
    //inline void SetUseItemCallback(function f)
    //{
    //    useItemFunction = static_cast<UseItemCallbackPointer>(f);
    //}
    //inline qboolean HasUseItemCallback() {
    //    return (useItemFunction != nullptr ? true : false);
    //}
    //// Sets the 'Drop' callback function.
    //template<typename function>
    //inline void SetDropCallback(function f)
    //{
    //    dropFunction = static_cast<DropCallbackPointer>(f);
    //}
    //inline qboolean HasDropCallback() {
    //    return (dropFunction != nullptr ? true : false);
    //}
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
    //
    // Item function pointers.
    //
    //PickupCallbackPointer       pickupFunction = nullptr;
    //UseItemCallbackPointer      useItemFunction = nullptr;
    //DropCallbackPointer         dropFunction = nullptr;
    WeaponThinkCallbackPointer  weaponThinkFunction = nullptr;
};