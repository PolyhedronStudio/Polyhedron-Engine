/***
*
*	License here.
*
*	@file
*
*	Base monster class. Use this for animating NPC's etc.
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;

class SVGBaseMonster : public SVGBaseTrigger {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    //using WeaponThinkCallbackPointer = void (SVGBaseMonster::*)(SVGBaseEntity* user);


    //! Constructor/Deconstructor.
    SVGBaseMonster(Entity* svEntity);
    virtual ~SVGBaseMonster();

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseMonster, SVGBaseTrigger);


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
    //virtual uint32_t GetCapLimit() { return 0; }
    

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
    *   Monster Logic function pointers.
    *
    ***/
    //WeaponThinkCallbackPointer  weaponThinkFunction = nullptr;
};