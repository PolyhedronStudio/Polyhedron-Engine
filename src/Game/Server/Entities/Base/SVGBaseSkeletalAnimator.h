/***
*
*	License here.
*
*	@file
*
*	Used for any entity that makes use of a skeletal animated model. (IQM in our case.)
*
*	Brings basic animation utilities to the entity such as playing, pausing, and switching
*	to and from animations. 
*
*	On top of that, it also introduces model events. These can be customly defined by the
*	developer and hooked to by setting a callback function. Example uses of this can be:
*	 - Play a footstep audio exactly the frame where the foot hits the floor. (Perform a trace too.)
*	 - Spawn a muzzleflash and/or bullet shell at the exact frame where required.
*	 - ... Be creative and use it :-) ...
*
***/
#pragma once


//!
class SVGBaseEntity;
class SVGBaseTrigger;


/**
*
*	Brings Skeletal Animation utilities to entities derived from this class.
*
**/
class SVGBaseSkeletalAnimator : public SVGBaseTrigger {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    //! Constructor/Deconstructor.
    SVGBaseSkeletalAnimator(PODEntity *svEntity);
    virtual ~SVGBaseSkeletalAnimator();

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseSkeletalAnimator, SVGBaseTrigger);


    /***
    * 
    *   Interface functions.
    *
    ***/
    /**
    *   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
    **/
    virtual void Precache() override;    // Precaches data.
    /**
    *   @brief  Called when it is time to spawn this entity.
    **/
    virtual void Spawn() override;       // Spawns the entity.
    /**
    *   @brief  Called when it is time to respawn this entity.
    **/
    virtual void Respawn() override;     // Respawns the entity.
    /**
    *   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    **/
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    /**
    *   @brief  General entity thinking routine.
    **/
    virtual void Think() override;

    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) override; // Called for each key:value when parsing the entity dictionary.



    /**
    *	@brief	Processes the animation for the current skeletal animation state.
    **/
    virtual void ProcessSkeletalAnimationForTime(const GameTime &time);


    /***
    * 
    *   Entity functions.
    * 
    ***/



public:


protected:

};