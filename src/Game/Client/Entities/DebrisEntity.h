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
    static DebrisEntity* Create(GameEntity* debrisser, const std::string& debrisModel, const vec3_t &origin, float speed);

private:
    DebrisEntity(PODEntity *svEntity);
    virtual ~DebrisEntity();

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
	*	@brief	Die callback.
	**/
	void DebrisEntityDie(GameEntity* inflictor, GameEntity* attacker, int32_t damage, const vec3_t& point);
};