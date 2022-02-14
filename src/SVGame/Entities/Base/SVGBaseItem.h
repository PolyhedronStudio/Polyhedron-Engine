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
    SVGBaseItem(Entity* svEntity, const std::string& displayString, uint32_t identifier);
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
    // Getters.
    //
    inline uint32_t GetIdentifier() {
        return itemIdentifier;
    }
    inline const std::string GetDisplayString() {
        return displayString;
    }

    //
    // Entity functions.
    //
    virtual void SetRespawn(const float delay);  // Sets the item in respawn mode.

    //
    // Callback Functions.
    //
    void BaseItemUse(SVGBaseEntity* caller, SVGBaseEntity* activator);
    void BaseItemTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    void BaseItemDropToFloor(void);
    void BaseItemDoRespawn(void);

private:
    // Item identifier.
    uint32_t itemIdentifier;

    // Item textual display string.
    std::string displayString;

    // Respawn wait time.
    float respawnWaitTime = 0.f;

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