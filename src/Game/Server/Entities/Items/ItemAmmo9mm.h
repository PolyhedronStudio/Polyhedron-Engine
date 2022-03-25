/***
*
*	License here.
*
*	@file
*
*	Basic 9mm ammo clip item. Used for most short range weapons.
*   
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseItem;

class ItemAmmo9mm : public SVGBaseItemAmmo {
public:

    //! Constructor/Deconstructor.
    ItemAmmo9mm(Entity* svEntity, const std::string& displayString, uint32_t identifier);
    virtual ~ItemAmmo9mm();

    //! Abstract Class TypeInfo registry.
    DefineItemMapClass("9 Millimeter Ammo", "ammo_9mm", ItemID::Ammo9mm, "item_ammo_9mm", ItemAmmo9mm, SVGBaseItemAmmo);


    /*** 
    *   Item flags
    ***/
    //static constexpr int32_t IF_IgnoreMaxHealth     = 1 << 0;
    //static constexpr int32_t IF_TimedHealth         = 1 << 1;


    /***
    * 
    *   Interface implementation functions.
    *
    ***/
    virtual void Precache() override;    // Precaches data.
    virtual void Spawn() override;       // Spawns the entity.


    /***
    * 
    *   Entity functions.
    * 
    ***/
    /**
    *   @return The maximum ammo cap limit to carry around for this ammo type.
    **/
    virtual uint32_t GetCapLimit() { return 144; }


    /**
    * 
    *   Callback functions.
    *
    ***/
    qboolean Ammo9mmPickup(IServerGameEntity *other);

private:

};
