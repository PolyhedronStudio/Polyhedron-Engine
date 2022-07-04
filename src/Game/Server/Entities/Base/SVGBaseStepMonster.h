/***
*
*	License here.
*
*	@file
*
*	Step Movement based Monster Class. Inherit from this to utilize Step Move functionality for your
*	non controllable (-character) entities.
*
*	Comes with all basic kung-fu.
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;
class SVGBaseSkeletalAnimator;

class SVGBaseStepMonster : public SVGBaseSkeletalAnimator {
public:
    /***
    * 
    *   Weapon Item callback function pointers.
    *
    ***/
    //! Constructor/Deconstructor.
    SVGBaseStepMonster(PODEntity *svEntity);
    virtual ~SVGBaseStepMonster();

    //! Abstract Class TypeInfo registry.
    DefineAbstractClass(SVGBaseStepMonster, SVGBaseSkeletalAnimator);


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
	//
	//! Goal Entity.
	//
    GameEntity* geGoalEntity = nullptr;
	std::string strGoalEntity = "";
	virtual void SetGoalEntity(GameEntity *geGoalEntity) { this->geGoalEntity = geGoalEntity; }
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
    *   Step Entity functions.
    * 
    ***/
public:
	/**
	*	@brief	Walks(By 'stepping') the entity 'dist' distance into the given yaw angle direction.
	*	@param	yawTurnAngle	The angle to turn and step into.
	*	@param	stepDistance	The distance to step towards the yawTurnAngle with.
	*	@return	True if successful, false otherwise.
	**/
	const bool StepMove_WalkDirection( const float yawDirectionAngle, const float stepDistance );

	/**
	*	@brief	This is the actual SG_Stepmove implementation. Tries to move
	*			the entity a given distance over a second while trying to 
	*			step over and off obstacles if needed. It is influenced by
	*			and dependent on the game's tick rate.
	*
	*	@return	False if the move has failed and the entity remains at its position.
	*			True otherwise.
	**/
	const bool StepMove_Step( const vec3_t &stepOffset, bool relink = false );

protected:
	const bool StepMove_CheckBottom( );
	void StepMove_FixCheckBottom( );
};