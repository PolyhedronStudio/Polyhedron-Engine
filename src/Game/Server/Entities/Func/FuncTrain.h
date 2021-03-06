#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;
class SVGBaseMover;
class PathCorner;

class FuncTrain : public SVGBaseMover {
public:
	FuncTrain( Entity* entity );
	virtual ~FuncTrain() = default;

	DefineMapClass( "func_train", FuncTrain, SVGBaseMover );

	// Spawnflags
	static constexpr int32_t SF_StartOn = 1 << 0;
	static constexpr int32_t SF_Toggled = 1 << 1;
	static constexpr int32_t SF_StopWhenBlocked = 1 << 2;

	void			Precache() override;
	void			Spawn() override;
	// Find the initial path_corner and teleport to it
	void			PostSpawn() override;
	void			SpawnKey( const std::string& key, const std::string& value ) override;

	void			TrainUse( IServerGameEntity* other, IServerGameEntity* activator );

	// Finds the next path corner target, if any.
	void			FindNextTarget();
	// Travels to the next path_corner
	void			NextCornerThink();
	// Resumes the path when the train is used but was stopped before
	void			ResumePath();
	// Waits at the arrived path_corner
	void			WaitAtCorner();
	// Callback for the brush movement code
	static void		OnWaitAtCorner( IServerGameEntity* ent );
	// The train has been blocked by an obstacle, damage it or stop?
	void			TrainBlocked( IServerGameEntity* other );

private:
	PathCorner*		currentPathEntity = nullptr;
	GameTime		damageDebounceTime = GameTime::zero();
};
