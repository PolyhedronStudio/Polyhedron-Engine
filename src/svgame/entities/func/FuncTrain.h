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

	void			Spawn() override;
	// Find the initial path_corner and teleport to it
	void			PostSpawn() override;

	// Travels to the next path_corner
	void			NextCornerThink();
	// Waits at the arrived path_corner
	void			WaitAtCorner();
	static void		OnWaitAtCorner( Entity* ent );
	// The train has been blocked by an obstacle, damage it or stop?
	void			TrainBlocked( SVGBaseEntity* other );

private:
	PathCorner*		currentPathEntity{ nullptr };
	float			damageDebounceTime{ 0.0f };
};
