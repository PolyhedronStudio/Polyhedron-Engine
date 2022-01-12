
#pragma once

class TriggerPush : public SVGBaseTrigger {
public:
	TriggerPush( Entity* entity );
	virtual ~TriggerPush() = default;

	DefineMapClass( "trigger_push", TriggerPush, SVGBaseTrigger );

	static constexpr int32_t SF_PushOnce = 1 << 0;

	void Spawn() override;
	void SpawnKey( const std::string& key, const std::string& value );

	void PushTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf );

private:
	float pushForce{ 1000.0f };
	vec3_t pushDirection{ vec3_zero() };
	inline static int32_t WindSound = 0;
};
