
#pragma once

class TargetEarthquake : public SVGBaseEntity {
public:
	TargetEarthquake( Entity* entity );
	virtual ~TargetEarthquake() = default;

	DefineMapClass( "target_earthquake", TargetEarthquake, SVGBaseEntity );

	void Spawn() override;
	void SpawnKey( const std::string& key, const std::string& value ) override;

	void QuakeUse( IServerGameEntity* other, IServerGameEntity* activator );
	void QuakeThink();

private:
	float severity{ 200.0f };
	Frametime duration = 5s;
	GameTime timeStamp = GameTime::zero();
	GameTime lastQuakeTime = GameTime::zero();
};
