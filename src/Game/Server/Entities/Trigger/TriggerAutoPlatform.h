#pragma once

class SVGBaseTrigger;

//===============
// Automatic trigger for func_plat
//===============
class TriggerAutoPlatform : public SVGBaseTrigger {
public:
	TriggerAutoPlatform( Entity* entity );
	virtual ~TriggerAutoPlatform() = default;

	DefineClass( TriggerAutoPlatform, SVGBaseTrigger );

	void					Spawn() override;

	// Responds to players touching this trigger
	void					AutoPlatformTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	// Creates an automatic platform trigger and sets everything up for it
	static TriggerAutoPlatform* Create( SVGBaseEntity *ownerEntity, const vec3_t &triggerOrigin, const vec3_t &triggerMins, const vec3_t &triggerMaxs );

protected:
	GameTime					debounceTouchTime = GameTime::zero();
};
