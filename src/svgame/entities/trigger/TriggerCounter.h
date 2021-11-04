
#pragma once

class TriggerCounter : public SVGBaseTrigger {
public:
	TriggerCounter( Entity* entity );
	virtual ~TriggerCounter() = default;

	DefineMapClass( "trigger_counter", TriggerCounter, SVGBaseTrigger );

	constexpr static int32_t SF_NoMessage = 1 << 0;

	void Spawn() override;
	void SpawnKey( const std::string& key, const std::string& value );

	void CounterUse( SVGBaseEntity* other, SVGBaseEntity* activator );

private:
	uint32_t count{ 2U };
};
