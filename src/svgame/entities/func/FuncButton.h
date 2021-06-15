#pragma once

class SVGBaseEntity;

class FuncButton : public SVGBaseEntity
{
public:
	FuncButton( Entity* svEntity );
	virtual ~FuncButton() = default;

	void Precache() override;
	void Spawn() override;
	
	void SpawnKey( const std::string& key, const std::string& value ) override;

	void ButtonDone();
	void ButtonReturn();
	void ButtonWait();
	void ButtonFire();

	void ButtonUse( SVGBaseEntity* other, SVGBaseEntity* activator );
	void ButtonTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf );
	void ButtonDie( SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point );
};
