
#pragma once

class TriggerRelay : public SVGBaseTrigger {
public:
	TriggerRelay( Entity* entity );
	virtual ~TriggerRelay() = default;

	DefineMapClass( "trigger_relay", TriggerRelay, SVGBaseTrigger );

	void Spawn() override;
	void RelayUse( IServerGameEntity* other, IServerGameEntity* activator );
};
