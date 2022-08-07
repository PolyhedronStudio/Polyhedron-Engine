/***
*
*	License here.
*
*	@file
*
*	Root Motion MOvement Monster class. Utilize with a skeletal IQM model that has a bone named "root"
*	which determines how far to step in the current animation frame. 
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
#include "Game/Shared/Physics/RootMotionMove.h"

class SVGBaseRootMotionMonster : public SVGBaseSkeletalAnimator {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    //! Constructor/Deconstructor.
    SVGBaseRootMotionMonster(PODEntity *svEntity);
    virtual ~SVGBaseRootMotionMonster() = default;

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseRootMotionMonster, SVGBaseSkeletalAnimator);


    /***
    * 
    *   Interface functions.
    *
    ***/
    virtual void Precache() override;
    virtual void Spawn() override;
    virtual void PostSpawn() override;
	virtual void Respawn() override;
    virtual void Think() override;

    virtual void SpawnKey(const std::string& key, const std::string& value) override;


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
	const int32_t GetAnimationStateFrame( const EntityAnimationState *animationState );

	/**
	*	@brief	Sets the 'translate' vector to the value of the 'root bone's' requested frame number 
	*			translation
	*	@return	True if the 'translate' frame data exists. False otherwise.
	**/
	const bool GetAnimationFrameTranslate( const int32_t actionIndex, const int32_t actionFrame, vec3_t& rootBoneTranslation );
	const bool GetAnimationFrameTranslate( const std::string &actionName, const int32_t actionFrame, vec3_t& rootBoneTranslation );
	/**
	*	@brief	Sets the 'distance' double to the value of the 'root bones' requested frame number 
	*			translation distance. (vec3_dlength)
	*	@return	True if the 'distance' frame data exists. False otherwise.
	**/
	const bool GetAnimationFrameDistance( const int32_t actionIndex, const int32_t actionFrame, double &rootBoneDistance );
	const bool GetAnimationFrameDistance( const std::string &actionName, const int32_t actionFrame, double &rootBoneDistance );
	/**
	*	@brief	Calculated the move speed of the root bone for the given 'moveDistance' and moveTranslate.
	*	@return	Value of the calculated move speed.
	**/
	const double GetMoveSpeedForTraversedFrameDistance(const double &totalMoveDistance, const float &frameMoveDistance, const double &unitScale);


	/**
	*	@brief	Updates, and(if needed) switches to a new animation.
	**/
	void RefreshAnimationState();
	
	/**
	*	@brief
	**/
	const bool AnimationFinished( const EntityAnimationState *animationState );
	/**
	*	@brief
	**/
	const bool CanSwitchAnimation( const EntityAnimationState *animationState, const int32_t wishedAnimationIndex );
	/**
	*	@brief
	**/
	const bool HasExoticMoveResults( const int32_t resultsMask );


private:


protected:



	/***
	*
	*
	*	Move Functionality:
	*
	*
	***/
public:
	/**
	*	@brief	Sets the 'speed' multiplier value for the small velocities which
	*			are generated based on the 'root bone' translational distances.
	**/
	inline void SetMoveSpeed(const double newMoveSpeed) { moveSpeed = newMoveSpeed; }
	inline double GetMoveSpeed() { return moveSpeed; }

	/**
	*	@brief	Prepare, Perform and process a root bone motion move frame for this monster.
	*	@return	A mask containing the final Root Motion Move results. (Blocked, stepped, etc.)
	**/
	const int32_t PerformRootMotionMove();
	/**
	*	@brief	Useful for debugging slidemoves, enable on an entity by setting spawnflag 128
	**/
	void DebugPrint(const int32_t entityNumber, const int32_t resultMask, const int32_t previousResultMask, const int32_t moveFlags, const int32_t moveFlagTime);


private:
	//! Current Move Result Mask.
	int32_t currentMoveResultMask	= 0;
	//! Previous Move Result Mask.
	int32_t previousMoveResultMask	= 0;

	//! Move Speed.
	double moveSpeed = 0.;
	//! Only set for debug printing purposes. Can be ignored.
	//int32_t previousMoveResultMask = 0;
	//! Actual Root Motion Move state we're operating with.
	RootMotionMoveState currentMoveState;
	//! Actual Root Motion Move state we're operating with.
	RootMotionMoveState previousMoveState;

	/**
	*	@brief	Applies the state data (origin, velocity, etc) to this entity's state.
	**/
	void ApplyMoveState( RootMotionMoveState *moveState );

protected:
	/**
	*	@brief 
	**/
	const bool RootMotionMove_CheckBottom( );
	/**
	*	@brief 
	**/
	void RootMotionMove_FixCheckBottom( );



	/***
	*
	*
	*	Monster Logic function pointers.
	*
	*
	***/
public:
	//
	//! Goal Entity.
	//
	/**
	*	@brief	Sets the goal entity to reach out for. Note that, a goal
	*			is NOT an enemy, however it can be. The enemy pointer is used
	*			for which enemy this monster currently is targetting.
	*
	*			A goal is an entity he is trying to head out for.
	*			Even though they can be the same, the enemy pointer is for combat,
	*			and goal pointer is for navigation.
	*			
	*			TODO: 
	**/
	virtual void SetGoalEntity(GameEntity *geGoal) { this->geGoalEntity = geGoal; }

	/**
	*	@return	A pointer to what is our Goal entity to try and reach out for.
	**/
	virtual GameEntity *GetGoalEntity() { return this->geGoalEntity; }

	/**
	*	@brief	Rotates/Turns the monster a frame into the Ideal Yaw angle direction.
	*	@return	The delta yaw angles of this Turn.
	**/
	virtual float TurnToIdealYawAngle();


private:
	//! Goal entity to try and reach out to.
	GameEntity* geGoalEntity = nullptr;

protected:
	//! Stores parsed monster goal targetname.
	std::string strMonsterGoalTarget = "";


	/***
	*
	*
	*
	*	Navigation Support Routines:
	*
	*
	*
	***/
public:
	/**
	*
	**/
	virtual const int32_t NavigateToOrigin( const vec3_t &navigationOrigin );

private:


protected:


public:


};