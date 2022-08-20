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
    //! Constructor/Deconstructor.
    SVGBaseSkeletalAnimator(PODEntity *svEntity);
    virtual ~SVGBaseSkeletalAnimator() = default;

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseSkeletalAnimator, SVGBaseTrigger);


    /***
    * 
    *   Interface functions.
    *
    ***/
    /**
    *   @brief  General entity thinking routine.
    **/
    virtual void Think() override;

    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) override; // Called for each key:value when parsing the entity dictionary.



	/**
	*
	*
	*	Animation System:
	*
	*
	**/
	/**
	*	@brief	Updates, and(if needed) switches to a new animation.
	**/
	//void RefreshAnimationState();
    /**
    *	@brief	Processes the animation for the current skeletal animation state.
    **/
    virtual void ProcessSkeletalAnimationForTime(const GameTime &time);
	
	/**
	*	@brief	Switches the animation by blending from the current animation into the next.
	*	@return	The animation index on success, -1 on failure.
	**/
	int32_t SwitchAnimation(const std::string &name);
	/**
	*	@brief	Prepares an animation to switch to after the current active animation has
	*			finished its current cycle from start to end -frame.
	**/
	int32_t PrepareAnimation(const std::string &name, const bool force = false);

	/**
	*	@brief
	**/
	virtual const bool AnimationFinished( const EntityAnimationState *animationState );
	/**
	*	@brief
	**/
	virtual const bool CanSwitchAnimation( const EntityAnimationState *animationState, const int32_t wishedAnimationIndex );








	/***
	*
	*
	*	Utility Functions, for easy bounds checking and sorts of tasks alike.
	*	(Wraps around clgi).
	*
	*
	***/
	/**
	*	@brief	Utility function to test whether an animation is existent and within range.
	*	@return	(nullptr) on failure. Otherwise a pointer to the specified action.
	**/
	SkeletalAnimation *GetAnimation( const std::string &name );
	SkeletalAnimation *GetAnimation( const int32_t index );
	/**
	*	@brief	Utility function to easily get a pointer to an Action by name or index.
	*	@return	(nullptr) on failure. Otherwise a pointer to the specified Action.
	**/
	SkeletalAnimationAction *GetAction( const std::string &name );
	SkeletalAnimationAction *GetAction( const int32_t index );
	/**
	*	@brief	Utility function to test whether a BlendAction is existent and within range.
	*	@return	(nullptr) on failure. Otherwise a pointer to the specified BlendAction action.
	**/
	SkeletalAnimationBlendAction *GetBlendAction( SkeletalAnimation *animation, const int32_t index );
	/**
	*	@brief	Utility function to test whether a BlendActionState is existent and within range for the specified Animation.
	*	@return	(nullptr) on failure. Otherwise a pointer to the specified BlendActionState action.
	**/
	EntitySkeletonBlendActionState *GetBlendActionState( const int32_t animationIndex, const int32_t blendActionIndex );



	/**
	*
	*
	*	This stores the model data for now.
	*
	*
	**/
	qhandle_t modelHandle = 0;
	SkeletalModelData *skm = nullptr;
	EntitySkeleton entitySkeleton;

public:
	//! Animation to switch to.
	int32_t animationToSwitchTo = -1;
	//! ...
	bool forcedAnimationSwitch = false;


protected:

};