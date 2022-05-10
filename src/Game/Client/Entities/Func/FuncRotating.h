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

class CLGBaseEntity;
class CLGBaseMover;

class FuncRotating : public CLGBaseMover {
public:
	FuncRotating( Entity* entity );
	virtual ~FuncRotating() = default;

	DefineMapClass( "func_rotatingzz", FuncRotating, CLGBaseMover );

	// Spawn flags
	static constexpr int32_t SF_StartOn = 1 << 0;
	static constexpr int32_t SF_Reverse = 1 << 1;
	static constexpr int32_t SF_YAxis = 1 << 2;
	static constexpr int32_t SF_XAxis = 1 << 3;
	static constexpr int32_t SF_HurtTouch = 1 << 4;
	static constexpr int32_t SF_StopOnBlock = 1 << 5;
	static constexpr int32_t SF_Animated = 1 << 6;
	static constexpr int32_t SF_AnimatedFast = 1 << 7;

	void Spawn() override;

	void SpawnKey(const std::string& key, const std::string& value) override;

	void RotatorBlocked( GameEntity* other );
	void RotatorHurtTouch( GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	void RotatorUse( GameEntity* other, GameEntity* activator );
};
