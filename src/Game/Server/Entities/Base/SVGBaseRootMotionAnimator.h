/***
*
*	License here.
*
*	@file
*
*	RootMotion based Animator class. 
*
*	By zero-ing out the root bone's translation ONLY when rendering, we create a more visually appealing
*	movement effect. Instead, we only take the translation into consideration for the physics.
*
*	The translation offsets between each frame, as well as the actual distance(vec3_length) are
*	calculated by the client and server during load time. When rendering the root bone's translation
*	is zero-ed out and only the actual offset between frames is applied.
*
*	The result is no more 'ice-skating' monsters, replaced by monsters that actually can stick their feet
*	to the ground.
*
*	Comes with all basic kung-fu.
*
***/
#pragma once

/**
*	Forward Declarations.
**/
class SVGBaseEntity;
class SVGBaseTrigger;
class SVGBaseSkeletalAnimator;



/**
*	RootMotion Move System.
**/
//#include "Game/Shared/Physics/RootMotionMove.h"

class SVGBaseRootMotionAnimator : public SVGBaseSkeletalAnimator {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    //! Constructor/Deconstructor.
    SVGBaseRootMotionAnimator( PODEntity *svEntity );
    virtual ~SVGBaseRootMotionAnimator() = default;

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass( SVGBaseRootMotionAnimator, SVGBaseSkeletalAnimator );


    /**
    *
    *   Interface functions.
    *
    **/
    /**
    *   @brief  Makes sure to set Solid type to OctagonBox and MoveType to RootMotionMove.
    **/
    virtual void Spawn() override;


	/***
	*
	*
	*	Animation Functionality:
	*
	*
	***/
public:
	/**
	*	@return	A pointer to the current animation state.
	**/
	const EntityAnimationState *GetCurrentAnimationState();

	/**
	*	@return	Calculates a 0 starting index based current frame for the given
	*			animation state.
	**/
	const int32_t GetAnimationStateRelativeFrame( const EntityAnimationState *animationState );

	/**
	*	@brief	Sets the 'translate' vector to the value of the 'root bone's' requested frame number 
	*			translation
	*	@return	True if the 'translate' frame data exists. False otherwise.
	**/
	const bool GetActionFrameTranslate( const int32_t actionIndex, const int32_t actionFrame, vec3_t& rootBoneTranslation );
	const bool GetActionFrameTranslate( const std::string &actionName, const int32_t actionFrame, vec3_t& rootBoneTranslation );

	/**
	*	@brief	Sets the 'distance' double to the value of the 'root bones' requested frame number 
	*			translation distance. (vec3_dlength)
	*	@return	True if the 'distance' frame data exists. False otherwise.
	**/
	const bool GetActionFrameDistance( const int32_t actionIndex, const int32_t actionFrame, double &rootBoneDistance );
	const bool GetActionFrameDistance( const std::string &actionName, const int32_t actionFrame, double &rootBoneDistance );

	/**
	*	@brief	Calculated the move speed of the root bone for the given 'moveDistance' and moveTranslate.
	*	@return	Value of the calculated move speed.
	**/
	const double GetMoveSpeedForTraversedFrameDistance(const double &totalMoveDistance, const float &frameMoveDistance, const double &unitScale);
		
	/**
	*	@brief
	**/
	virtual const bool AnimationFinished( const EntityAnimationState *animationState ) override;
	/**
	*	@brief
	**/
	virtual const bool CanSwitchAnimation( const EntityAnimationState *animationState, const int32_t wishedAnimationIndex ) override;
	/**
	*	@brief	Updates, and(if needed) switches to a new animation.
	**/
	void RefreshAnimationState();
};