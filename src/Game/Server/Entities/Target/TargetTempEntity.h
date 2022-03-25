
#pragma once

class TargetTempEntity : public SVGBaseEntity {
public:
	TargetTempEntity( Entity* entity );
	virtual ~TargetTempEntity() = default;

	DefineMapClass( "target_temp_entity", TargetTempEntity, SVGBaseEntity );

	void Spawn() override;

	void TempEntityUse( IServerGameEntity* other, IServerGameEntity* activator );
};
