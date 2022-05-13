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

	void Spawn() override;

	void SpawnKey(const std::string& key, const std::string& value) override;

	void RotatorBlocked( GameEntity* other );
	void RotatorHurtTouch( GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	void RotatorUse( GameEntity* other, GameEntity* activator );
};
