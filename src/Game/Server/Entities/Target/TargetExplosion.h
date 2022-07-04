
#pragma once

class TargetExplosion : public SVGBaseEntity {
public:
	TargetExplosion( Entity* entity );
	virtual ~TargetExplosion() = default;
	
	DefineMapClass( "target_explosion", TargetExplosion, SVGBaseEntity );

	void Spawn() override;

	void ExplosionUse( IServerGameEntity* other, IServerGameEntity* activator );
	void ExplosionThink();
};
