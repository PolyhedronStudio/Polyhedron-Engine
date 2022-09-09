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

class FuncRotating : public CLGBasePacketEntity {
public:
	FuncRotating( Entity* entity );
	virtual ~FuncRotating() = default;

	DefineMapClass( "xfunc_rotating", FuncRotating, CLGBasePacketEntity );

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
	void PostSpawn() override;

	void SpawnKey(const std::string& key, const std::string& value) override;

	void RotatorBlocked( IClientGameEntity* other );
	void RotatorHurtTouch( IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	void RotatorUse( IClientGameEntity* other, IClientGameEntity* activator );
	void RotatorThink();

private:
	vec3_t moveDirection = vec3_zero();
	float speed = 0.f;
};
