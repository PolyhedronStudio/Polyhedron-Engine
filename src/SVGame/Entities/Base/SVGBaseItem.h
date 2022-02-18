/***
*
*	License here.
*
*	@file
*
*	Base item class. Provides picking up, and respawn item functionalities.
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;

class SVGBaseItem : public SVGBaseTrigger {
public:
    /**
    * 
    *   Item callback function pointers.
    *
    ***/
    using PickupCallbackPointer         = qboolean(SVGBaseItem::*)(SVGBaseEntity *picker);
    using UseInstanceCallbackPointer    = void(SVGBaseItem::*)(SVGBaseEntity *user, SVGBaseItem* item);
    using DropCallbackPointer           = void(SVGBaseItem::*)(SVGBaseEntity* other);
    using WeaponThinkCallbackPointer    = void(SVGBaseItem::*)(SVGBaseEntity* other);


    //! Constructor/Deconstructor.
    SVGBaseItem(Entity* svEntity, const std::string& displayString, uint32_t identifier);
    virtual ~SVGBaseItem();


    //! Abstract Class TypeInfo registry.
    DefineAbstractClass( SVGBaseItem, SVGBaseTrigger );


    /**
    * 
    *   Interface implementation functions.
    *
    ***/
    virtual void Precache() override;    // Precaches data.
    virtual void Spawn() override;       // Spawns the entity.
    virtual void Respawn() override;     // Respawns the entity.
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think() override;       // General entity thinking routine.

    /**
    *
    *   Instance Interface implementation functions.
    *
    **/
protected:
    /**
    *   @brief  Only for use by CreateIns
    **/
    virtual void InstanceSpawn();


public:

    /**
    * 
    *   Get/Set.
    *
    ***/
    /**
    *   @return The unique item type identifier. Used to lookup in inventory.
    **/
    inline uint32_t GetIdentifier() { return itemIdentifier; }
    /**
    *   @return The string to use for visually displaying an item's name.
    **/
    inline const std::string GetDisplayString() { return displayString; }
    

    /**
    * 
    *   Entity functions.
    *
    ***/
    /**
    *   @brief Engages this item in respawn mode waiting for the set delay to pass before respawning.
    **/
    virtual void SetRespawn(const float delay);

    /**
    *   @brief  Use for item instances, calls their "UseInstance" callback.
    * 
    *   @details    'UseInstance' is not to be confused with the general 'Use' dispatch 
    *               function for trigger callbacks.
    **/
    virtual void UseInstance(SVGBaseEntity* user, SVGBaseItem* item);



    /**
    * 
    *   Item Entity interface Callbacks.
    *
    ***/
    /**
    *   @brief Callback for when being triggered. Also known as "Use".
    **/
    void BaseItemUse(SVGBaseEntity* caller, SVGBaseEntity* activator);

    /**
    *   @brief Callback for item instance usage.
    **/
    void BaseItemUseInstance(SVGBaseEntity* user, SVGBaseItem* item);

    /**
    *   @brief Callback for when an entity touches this item.
    **/
    void BaseItemTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    /**
    *   @brief Callback for executing drop to floor behavior.
    **/
    void BaseItemDropToFloor(void);

    /**
    *   @brief Callback meant to be used by SetThink so one can delay a call to Respawn.
    **/
    void BaseItemDoRespawn(void);


protected:
    //! Item identifier.
    uint32_t itemIdentifier;

    //! Item textual display string.
    std::string displayString;

    //! Respawn wait time.
    float respawnWaitTime = 0.f;


protected:
    //! Static array holding space for each unique item ID. These instances are
    //! created at the start of the game and are used for callbacks. Each callback
    //! is served a pointer to the client's player entity when being fired.
    static SVGBaseItem* itemInstances[ItemIdentifier::MaxWeapons]; 

    //! A string index mapper to playerWeaponInstances.
    //!
    //! This could probably be done better, but this'll do for now.
    static std::map<std::string, uint32_t> lookupStrings;

    /**
    *   @brief  Creates an instance in the itemInstances array if none exists for this item yet.
    **/
    template<typename T> inline static T* CreateItemInstance(const std::string& displayString, const std::string &instanceString, uint32_t identifier) {
        if (itemInstances[identifier] == nullptr) {
	        // Allocate the item's player weapon instance class.
            itemInstances[identifier] = new T(nullptr, displayString, identifier);
       
            T *itemInstance = dynamic_cast<T*>(itemInstances[identifier]);

            // Add it into the string instance list.
	        lookupStrings[instanceString] = identifier;

            // Return the instance so we can call its InstanceSpawn in the DefineMapItemClass macro.
            return itemInstance;
        }

        return nullptr;
    }

public:
    /**
    *   @return Pointer to an item instance that is meant to be used for example, weapon logic.
    **/
    static SVGBaseItem* GetItemInstanceByID(uint32_t identifier) {
        if (identifier >= 0 && identifier <= ItemIdentifier::MaxWeapons) {
	        return itemInstances[identifier];
	    } else {
	        return nullptr;
        }
    }

    /**
    *   @return Pointer to an item instance that is meant to be used for example, weapon logic.
    **/
    static SVGBaseItem* GetItemInstanceByLookupString(const std::string& name) {
	    uint32_t identifier = 0;

        // First look it up in the instanceString map.
        if (lookupStrings.contains(name)) {
		    identifier = lookupStrings[name];
	    } else {
	        return nullptr;
        }

        // If we found one, get to action.
	    if (identifier >= 0 && identifier <= ItemIdentifier::Maximum) {
	        return itemInstances[identifier];
	    } else {
	        return nullptr;
	    }
    }


public:
    // Sets the 'Pickup' callback function.
    template<typename function>
    inline void SetPickupCallback(function f) {
        pickupFunction = static_cast<PickupCallbackPointer>(f);
    }
    inline qboolean HasPickupCallback() {
        return (pickupFunction != nullptr ? true : false);
    }
    //! Sets the 'Use Item Instance' callback function.
    template<typename function>
    inline void SetUseInstanceCallback(function f) {
        useInstanceFunction = static_cast<UseInstanceCallbackPointer>(f);
        int x = 10;
    }
    inline qboolean HasUseInstanceCallback() {
        return (useInstanceFunction != nullptr ? true : false);
    }
    //! Sets the 'Drop' callback function.
    template<typename function>
    inline void SetDropCallback(function f) {
        dropFunction = static_cast<DropCallbackPointer>(f);
    }
    inline qboolean HasDropCallback() {
        return (dropFunction != nullptr ? true : false);
    }
    //! Sets the 'WeaponThink' callback function.
    template<typename function>
    inline void SetWeaponThinkCallback(function f) {
        weaponThinkFunction = static_cast<WeaponThinkCallbackPointer>(f);
    }
    inline qboolean HasWeaponThinkCallback() {
        return (weaponThinkFunction != nullptr ? true : false);
    }


protected:
    // Callback function pointers.
    PickupCallbackPointer       pickupFunction = nullptr;
    UseInstanceCallbackPointer  useInstanceFunction = nullptr;
    DropCallbackPointer         dropFunction = nullptr;
    WeaponThinkCallbackPointer  weaponThinkFunction = nullptr;
};