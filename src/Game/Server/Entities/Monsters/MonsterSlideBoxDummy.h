/***
*
*	License here.
*
*	@file
*
*	"Test Dummy" slidebox monster. 
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;
//class SVGBaseAnimated;
class SVGBaseSkeletalAnimator;
class SVGBaseRootMotionAnimator;


class MonsterSlideBoxDummy : public SVGBaseRootMotionAnimator {
public:
    //! Constructor/Deconstructor.
    MonsterSlideBoxDummy( PODEntity *podEntity );
    virtual ~MonsterSlideBoxDummy() = default;

    DefineMapClass( "monster_slideboxdummy", MonsterSlideBoxDummy, SVGBaseRootMotionAnimator );



    /**
    *
    *   Interface functions.
    *
    **/
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
    *
    *   TestDummy Temporary WIP Area.
    *
    **/




    /**
    *
    *   Callback functions.
    *
    **/  
	/**
	*	@brief	'Use' callback: Engages the test dummy to follow its 'User'.
	**/
    void UseCallback_EngageGoal( IServerGameEntity* caller, IServerGameEntity* activator );
	/**
	*	@brief	'Think' callback: Check for animation updates, navigate to movegoal and process animations.
	**/
    void ThinkCallback_General(void);
	/**
	*	@brief	'Die' callback: Switch animation to 'WalkingToDying' and leave the body until damaged enough to gib.
	**/
    void DieCallback_FallDead(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);
	/**
	*	@brief	Temporary for determining and kicking off the animation to spawn with.
	**/
	void Callback_DetermineSpawnAnimation(void);
	/**
	*	@brief	Prepares the entity for removement after spawning various client gib events.
	**/
	void Callback_MorphToClientGibs(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);




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
	inline void SetMoveSpeed( const double &newMoveSpeed ) { moveSpeed = newMoveSpeed; }
	inline double GetMoveSpeed() { return moveSpeed; }

	/**
	*	@brief	Prepare, Perform and process a root bone motion move frame for this monster.
	*	@return	A mask containing the final Root Motion Move results. (Blocked, stepped, etc.)
	**/
	const int32_t PerformRootMotionMove();
	/**
	*	@brief	Useful for debugging slidemoves, enable on an entity by setting spawnflag 128
	**/
	void DebugPrint( const int32_t entityNumber, const int32_t resultMask, const int32_t previousResultMask, const int32_t moveFlags, const int32_t moveFlagTime );


	/***
	*
	*
	*	Temporary Monster Logic for testing SlideBox.
	*
	*
	***/
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

	//! Movespeed.
	double moveSpeed = 0.f;

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
};
