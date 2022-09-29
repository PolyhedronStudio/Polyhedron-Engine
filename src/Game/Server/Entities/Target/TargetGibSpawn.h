/***
*
*	License here.
*
*	@file
*
*	Target Gib Spawn Entity.
* 
***/
#pragma once

// Predeclarations.
class IServerGameEntity;
class SVGBaseTrigger;


/**
*	@brief	When targetted its Use is thus dispatched, resulting in spawning Gibs at the
*			targetted entity's position.
**/
class TargetGibSpawn : public SVGBaseTrigger {
public:
    // Constructor/Deconstructor.
    TargetGibSpawn(PODEntity *svEntity);
    virtual ~TargetGibSpawn() = default;

	// Inherit from SVGBaseTrigger.
    DefineMapClass( "target_gib_spawn", TargetGibSpawn, SVGBaseTrigger);

	/**
	*	@brief
	**/
	virtual void Spawn() override;
	virtual void PostSpawn() override;

	/**
	*	@brief	Additional spawnkeys.
	**/
    virtual void SpawnKey(const std::string& key, const std::string& value)  override;

private:
	/**
	*	Callbacks.
	**/
	void Callback_Use( IServerGameEntity* other, IServerGameEntity* activator );

private:
	//! Targetted entity.
};