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
	    //! Goal Entity.
    GameEntity* geGoalEntity = nullptr;
	virtual void SetGoalEntity(GameEntity *geGoalEntity) { this->geGoalEntity = geGoalEntity; }
	virtual GameEntity *GetGoalEntity() { return this->geGoalEntity; }



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
	/**
	*	@brief	Tries to correct the Yaw Angle to that which is desired (The 'idealYawAngle').
	*
	*	@todo	Should move to monster code.
	**/
	void StepMove_CorrectYawAngle( );


	const bool StepMove_CheckBottom( );
	void StepMove_FixCheckBottom( );
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