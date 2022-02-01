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
	void					AutoPlatformTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf );
	// Creates an automatic platform trigger and sets everything up for it
	static TriggerAutoPlatform* Create( SVGBaseEntity* ownerEntity, vec3_t ownerMins, vec3_t ownerMaxs );

protected:
	float					debounceTouchTime;
};
