/***
*
*	License here.
*
*	@file
*
*	Slide Movement based Monster Class. Inherit from this to utilize Step Move functionality for your
*	non controllable (-character) entities.
*
*	Comes with all basic kung-fu.
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;
class SVGBaseSkeletalAnimator;

class SVGBaseSlideMonster : public SVGBaseSkeletalAnimator {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    //! Constructor/Deconstructor.
    SVGBaseSlideMonster(PODEntity *svEntity);
    virtual ~SVGBaseSlideMonster();

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseSlideMonster, SVGBaseSkeletalAnimator);


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
	*	@brief	Prepare, Perform and process a slidemove frame for this monster.
	*	@return	A mask containing the final slide move results. (Blocked, stepped, etc.)
	**/
	const int32_t SlideMove();

	/**
	*	@brief	Useful for debugging slidemoves, enable on an entity by setting spawnflag 128
	**/
	void DebugPrint(const int32_t entityNumber, const int32_t blockedMask, const int32_t previousBlockedMask, const int32_t moveFlags, const int32_t moveFlagTime);

private:
	//! Only set for debug printing purposes. Can be ignored.
	int32_t previousMoveBlockedMask = 0;

	//! Actual slide move state we're operating with.
	SlideMoveState slideMoveState;


protected:
	const bool SlideMove_CheckBottom( );
	void SlideMove_FixCheckBottom( );



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