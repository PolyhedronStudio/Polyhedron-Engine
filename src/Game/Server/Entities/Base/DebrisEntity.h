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

class SVGBaseEntity;

class DebrisEntity : public SVGBaseEntity {
public:
    /**
    *   @brief  Used by game modes to spawn server side gibs.
    *   @param  debrisser The entity that is about to spawn debris.
    **/
    static DebrisEntity* Create(GameEntity* debrisser, const std::string& debrisModel, const vec3_t &origin, float speed);

private:
    DebrisEntity(Entity* svEntity);
    virtual ~DebrisEntity();

public:
    DefineClass(DebrisEntity, SVGBaseEntity);

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
	void DebrisEntityDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);
};