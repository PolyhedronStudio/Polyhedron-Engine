/***
*
*	License here.
*
*	@file
*
*	Debris entity, spawned only by using its static create method.
* 
***/
#pragma once

class CLGBaseLocalEntity;

class DebrisEntity : public CLGBaseLocalEntity {
public:
    /**
    *   @brief  Used by game modes to spawn server side gibs.
    *   @param  debrisser The entity that is about to spawn debris.
    **/
    static DebrisEntity* Create(GameEntity* debrisser, const std::string& debrisModel, const vec3_t &origin, const float speed, const int32_t damage );

private:
    DebrisEntity(PODEntity *svEntity);
    virtual ~DebrisEntity() = default;

public:
    DefineClass(DebrisEntity, CLGBaseLocalEntity);

	/**
	*
	*
	*	Callback functions.
	*
	*
	**/
	/**
	*	@brief	Think callback, checks for ground, applies gravity, and sets a free callback after deathTime passed.
	**/
	void DebrisEntityThink();

	/**
	*	@brief	Die callback.
	**/
	void DebrisEntityDie( GameEntity* inflictor, GameEntity* attacker, int32_t damage, const vec3_t& point );
    
	/**
	*	@brief	Touch callback.
	**/
	void DebrisEntityTouch( GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf );

	//void GibEntityThink();
	//void GibEntityStopBleeding();
    //void GibEntityDie(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point);
    //void GibEntityTouch(GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf);


private:
	//! Time of spawning, used for checking when to 'die', since our think method is occupied
	//! handling the debris itself.
	GameTime deathTime = GameTime::zero();
};