#pragma once

class SVGBaseEntity;
class SVGBaseMover;

class FuncRotating : public SVGBaseEntity {
public:
	FuncRotating( Entity* entity );
	virtual ~FuncRotating() = default;

	DefineMapClass( "func_rotating", FuncRotating, SVGBaseEntity );

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

	void RotatorBlocked( IServerGameEntity* other );
	void RotatorHurtTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	void RotatorUse( IServerGameEntity* other, IServerGameEntity* activator );
	void RotatorThink();

private:
	vec3_t moveDirection = vec3_zero();
	float speed = 0.f;
};
