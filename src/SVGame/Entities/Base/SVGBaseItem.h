/*
// LICENSE HERE.

//
// SVGBaseItem.h
//
//
*/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;

class SVGBaseItem : public SVGBaseTrigger {
public:
    //
    // Function pointers for item callbacks.
    // 
    using PickupCallbackPointer         = qboolean(SVGBaseItem::*)(SVGBaseEntity *other);
    using UseItemCallbackPointer        = void(SVGBaseItem::*)(SVGBaseEntity* other);
    using DropCallbackPointer           = void(SVGBaseItem::*)(SVGBaseEntity* other);
    using WeaponThinkCallbackPointer    = void(SVGBaseItem::*)(SVGBaseEntity* other);

    //
    // Constructor/Deconstructor.
    //
    SVGBaseItem(Entity* svEntity);
    virtual ~SVGBaseItem();

    DefineAbstractClass( SVGBaseItem, SVGBaseTrigger );

    //
    // Interface functions. 
    //
    virtual void Precache() override;    // Precaches data.
    virtual void Spawn() override;       // Spawns the entity.
    virtual void Respawn() override;     // Respawns the entity.
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think() override;       // General entity thinking routine.

    //
    // Callback Functions.
    //
    void BaseItemUse( SVGBaseEntity* caller, SVGBaseEntity* activator );
    void BaseItemThink(void);
    void BaseItemDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void BaseItemTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

public:
    //
    // Ugly, but effective callback SET methods.
    //
    // Sets the 'Pickup' callback function.
    template<typename function>
    inline void SetPickupCallback(function f)
    {
        pickupFunction = static_cast<PickupCallbackPointer>(f);
    }
    inline qboolean HasPickupCallback() {
        return (pickupFunction != nullptr ? true : false);
    }
    // Sets the 'Use Item' callback function.
    template<typename function>
    inline void SetUseItemCallback(function f)
    {
        useItemFunction = static_cast<UseItemCallbackPointer>(f);
    }
    inline qboolean HasUseItemCallback() {
        return (useItemFunction != nullptr ? true : false);
    }
    // Sets the 'Drop' callback function.
    template<typename function>
    inline void SetDropCallback(function f)
    {
        dropFunction = static_cast<DropCallbackPointer>(f);
    }
    inline qboolean HasDropCallback() {
        return (dropFunction != nullptr ? true : false);
    }
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
    PickupCallbackPointer       pickupFunction = nullptr;
    UseItemCallbackPointer      useItemFunction = nullptr;
    DropCallbackPointer         dropFunction = nullptr;
    WeaponThinkCallbackPointer  weaponThinkFunction = nullptr;
};