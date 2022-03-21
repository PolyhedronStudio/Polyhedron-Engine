
#pragma once

class TriggerGravity : public SVGBaseTrigger {
public:
	TriggerGravity( Entity* entity );
	virtual ~TriggerGravity() = default;

	DefineMapClass( "trigger_gravity", TriggerGravity, SVGBaseTrigger );

	void Spawn() override;

	void SpawnKey( const std::string& key, const std::string& value ) override;

	void GravityTouch( SVGBaseEntity* self, SVGBaseEntity* other, CollisionPlane* plane, CollisionSurface* surf );
};
