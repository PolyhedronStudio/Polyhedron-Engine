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
    virtual void Think() override;
    virtual void Precache() override;
    virtual void Spawn() override;

    // Overrided to automatically fetch model handle.
    inline void SetModelIndex(const int32_t& index) { 
        // Base set model index.
        Base::SetModelIndex(index);

        // Acquire handle.
	    //modelHandle = gi.GetModelByHandle(index);
    }

    // Animation functions.
    virtual void IsProcessingState();

    /***
    * 
    *   Entity functions.
    * 
    ***/

    /**
    *   @return The maximum ammo cap limit to carry around for this ammo type.
    **/
    // Set animation.
    virtual inline void SetAnimation(uint32_t index) {
	    

        animationIndex = index;
        animationStartTime = level.time;
    }

    // Set framerate. (1.0f = fps of iqm.)
    virtual inline void SetAnimationFrametime(Frametime frametime) {
        animationFrametime = frametime;
    }
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
    *   Animation State Related.
    *
    ***/

    //! The index of the animation we're playing. (0 = static or none)
    short animationIndex = 0;

    //! The time the current(thus also last) animation started.
    Frametime animationStartTime = Frametime::zero();

    //! The framerate the current animation is playing at.
    Frametime animationFrametime = 1s;

    //! Are we animating?
    qboolean isAnimating = false;


    /***
    * 
    *   Monster Logic function pointers.
    *
    ***/
    //WeaponThinkCallbackPointer  weaponThinkFunction = nullptr;
};