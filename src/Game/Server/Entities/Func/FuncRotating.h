#pragma once

class SVGBaseEntity;
class SVGBaseMover;

class FuncRotating : public SVGBaseMover {
public:
	FuncRotating( Entity* entity );
	virtual ~FuncRotating() = default;

	DefineMapClass( "func_rotating", FuncRotating, SVGBaseMover );

	// Spawn flags
	static constexpr int32_t SF_StartOn = 1;//1 << 0;
	static constexpr int32_t SF_Reverse = 2;//1 << 1;
	static constexpr int32_t SF_XAxis = 4;//1 << 3;
	static constexpr int32_t SF_YAxis = 8;//1 << 2;
	static constexpr int32_t SF_HurtTouch = 16;//1 << 4;
	static constexpr int32_t SF_StopOnBlock = 32;//1 << 5;
	static constexpr int32_t SF_Animated = 64;//1 << 6;
	static constexpr int32_t SF_AnimatedFast = 128;//1 << 7;

	/**
	*	State we're working at.
	**/
	struct FuncRotateState {
		static constexpr uint32_t Stopped = 0;
		static constexpr uint32_t Accelerating = 1;
		static constexpr uint32_t FullSpeed = 2;
		static constexpr uint32_t Decelarting = 3;
	};

	/**
	*	@brief
	**/
	void Spawn() override;
	/**
	*	@brief
	**/
	void PostSpawn() override;

	/**
	*	@brief
	**/
	void SpawnKey(const std::string& key, const std::string& value) override;

	/**
	*	@brief
	**/
	void RotatorBlocked( IServerGameEntity* other );
	/**
	*	@brief
	**/
	void RotatorHurtTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	/**
	*	@brief
	**/
	void RotatorUse( IServerGameEntity* other, IServerGameEntity* activator );
	/**
	*	@brief
	**/
	void RotatorThink();

	/**
	*	@brief
	**/
	void Callback_AccelerateThink();
	/**
	*	@brief
	**/
	void Callback_DecelerateThink();

private:
	uint32_t moveState = 0;
};
