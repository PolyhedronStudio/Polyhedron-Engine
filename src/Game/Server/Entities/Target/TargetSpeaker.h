
#pragma once

class TargetSpeaker : public SVGBaseEntity {
public:
	TargetSpeaker( Entity* entity );
	virtual ~TargetSpeaker() = default;

	DefineMapClass( "target_speaker", TargetSpeaker, SVGBaseEntity );
	
	static constexpr int32_t SF_LoopedOn = 1 << 0;
	static constexpr int32_t SF_LoopedOff = 1 << 1;
	static constexpr int32_t SF_Reliable = 1 << 2;

	void Spawn() override;
	void SpawnKey( const std::string& key, const std::string& value ) override;

	void SpeakerUse( IServerGameEntity* other, IServerGameEntity* activator );

private:
	std::string soundFile;
	float volume{ 1.0f };
	float attenuation{ 1.0f };
};
