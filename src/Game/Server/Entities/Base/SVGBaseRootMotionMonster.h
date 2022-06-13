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

class SVGBaseEntity;
class SVGBaseTrigger;
class SVGBaseSkeletalAnimator;

class SVGBaseRootMotionMonster : public SVGBaseSkeletalAnimator {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    //! Constructor/Deconstructor.
    SVGBaseRootMotionMonster(PODEntity *svEntity);
    virtual ~SVGBaseRootMotionMonster();

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
    *   Monster Entity Functions.
    * 
    ***/
	/**
	*
	**/
	virtual void Move_NavigateToTarget();


	//
	//! Goal Entity.
	//
    GameEntity* geGoalEntity = nullptr;
	std::string strGoalEntity = "";
	virtual void SetGoalEntity(GameEntity *geGoal) { this->geGoalEntity = geGoal; }
	virtual GameEntity *GetGoalEntity() { return this->geGoalEntity; }

	/**
	*	@brief	Categorizes what other contents the entity resides in. (Water, Lava, or...)
	**/
	void CategorizePosition();

	/**
	*	@brief	Rotates/Turns the monster into the Ideal Yaw angle direction.
	*	@return	The delta yaw angles of this Turn.
	**/
	virtual float TurnToIdealYawAngle();


    /***
    * 
    *   Slide Move Functionality:
    * 
    ***/
public:
	/**
	*	@brief	Prepare, Perform and process a root bone motion move frame for this monster.
	*	@return	A mask containing the final Root Motion Move results. (Blocked, stepped, etc.)
	**/
	const int32_t PerformRootMotionMove();

	/**
	*	@brief	Useful for debugging slidemoves, enable on an entity by setting spawnflag 128
	**/
	void DebugPrint(const int32_t entityNumber, const int32_t blockedMask, const int32_t previousBlockedMask, const int32_t moveFlags, const int32_t moveFlagTime);

private:
	//! Only set for debug printing purposes. Can be ignored.
	int32_t previousMoveResultMask = 0;

	//! Actual Root Motion Move state we're operating with.
	RootMotionMoveState rootMotionMoveState;


protected:
	const bool RootMotionMove_CheckBottom( );
	void RootMotionMove_FixCheckBottom( );



	/***
	*
	*	SlideBox Entity Functions.
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