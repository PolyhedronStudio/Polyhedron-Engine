/***
*
*	License here.
*
*	@file
*
*	Client-Side FuncPlat Entity Support.
*
***/
#pragma once

class CLGBasePacketEntity;
class CLGBaseMover;

class FuncRotating : public CLGBaseMover {
public:
	FuncRotating( Entity* entity );
	virtual ~FuncRotating() = default;

	DefineMapClass( "xfunc_rotating", FuncRotating, CLGBaseMover );

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

	//// TEMPORARILY.
    // Return the 'acceleration' float value.
    virtual inline float GetAcceleration() override {
        return 15.f;
    }
    // Return the 'deceleration' float value.
    virtual inline float GetDeceleration() override {
        return 15.f;
    }
    // Return the 'speed' float value.
    virtual inline float GetSpeed() override {
        return 75;
    }
	/// EOF TEMPORARILY
	/**
	*	@brief
	**/
	void Spawn() override;
	/**
	*	@brief
	**/
	void PostSpawn() override;
	/**
	*	@brief	Implements triggering door state, effectively allowing a slight client-side prediction.
	**/
	virtual void OnEventID(uint32_t eventID) override;

	/**
	*	@brief
	**/
	void SpawnKey(const std::string& key, const std::string& value) override;


	/***
	*
	*
	*	Callbacks/
	*
	*
	***/
	/**
	*	@brief
	**/
	void RotatorBlocked( IClientGameEntity* other );
	/**
	*	@brief
	**/
	void RotatorHurtTouch( IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	/**
	*	@brief
	**/
	void RotatorUse( IClientGameEntity* other, IClientGameEntity* activator );
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
	vec3_t moveDirection = vec3_zero();
	PushMoveInfo moveInfo;
	float speed = 0.f;
};
