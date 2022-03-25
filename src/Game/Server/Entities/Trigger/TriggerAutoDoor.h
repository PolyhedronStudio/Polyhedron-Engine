#pragma once

class SVGBaseTrigger;

//===============
// Automatic trigger for func_door
//===============
class TriggerAutoDoor : public SVGBaseTrigger {
public:
	TriggerAutoDoor( Entity* entity );
	virtual ~TriggerAutoDoor() = default;

	DefineClass( TriggerAutoDoor, SVGBaseTrigger );

	void					Spawn() override;
	// Responds to players touching this trigger
	void					AutoDoorTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	// Creates an automatic door trigger and sets everything up for it
	static TriggerAutoDoor* Create( SVGBaseEntity* ownerEntity, vec3_t ownerMins, vec3_t ownerMaxs );

protected:
	float					debounceTouchTime;
};
