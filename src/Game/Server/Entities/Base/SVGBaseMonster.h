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
class SVGBaseSkeletalAnimator;

class SVGBaseMonster : public SVGBaseSkeletalAnimator {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    //! Constructor/Deconstructor.
    SVGBaseMonster(PODEntity *svEntity);
    virtual ~SVGBaseMonster();

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseMonster, SVGBaseSkeletalAnimator);


    /***
    * 
    *   Interface functions.
    *
    ***/
    void Precache() override;
    void Spawn() override;
    void PostSpawn() override;
	void Respawn() override;
    void Think() override;

    void SpawnKey(const std::string& key, const std::string& value) override;


    /***
    * 
    *   Entity functions.
    * 
    ***/

    

public:


protected:
    /***
    * 
    *   Monster Logic function pointers.
    *
    ***/
};