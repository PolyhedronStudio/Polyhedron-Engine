
#pragma once

class FuncExplosive : public SVGBaseEntity {
public:
	FuncExplosive( Entity* entity );
	virtual ~FuncExplosive() = default;

	DefineMapClass( "func_explosive", FuncExplosive, SVGBaseEntity );

	// Spawn flags
	static constexpr int32_t SF_StartDeactivated = 1 << 0;
	static constexpr int32_t SF_Animated = 1 << 1;
	static constexpr int32_t SF_AnimatedFast = 1 << 2;

	void Spawn() override;

	void ExplosiveDeath( IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point );
	void ExplosiveUse( IServerGameEntity* other, IServerGameEntity* activator );
	void ExplosiveAppearUse( IServerGameEntity* other, IServerGameEntity* activator );
};
