#pragma once

class SVGBaseTrigger;

//===============
// Automatic trigger for func_door
//===============
class TriggerAutoDoor : public SVGBaseTrigger {
public:
	TriggerAutoDoor( Entity* entity );
	virtual ~TriggerAutoDoor() = default;

	void					Spawn() override;
	// Responds to players touching this trigger
	void					AutoDoorTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf );
	// Creates an automatic door trigger and sets everything up for it
	static TriggerAutoDoor* Create( SVGBaseEntity* ownerEntity, vec3_t ownerMaxs, vec3_t ownerMins );

protected:
	float					debounceTouchTime;
};
