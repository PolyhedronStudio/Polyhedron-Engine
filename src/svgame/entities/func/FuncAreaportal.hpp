#pragma once

class SVGBaseEntity;

class FuncAreaportal : public SVGBaseEntity {
public:
	FuncAreaportal( Entity* entity );
	virtual ~FuncAreaportal() = default;

	DefineMapClass( "func_areaportal", FuncAreaportal, SVGBaseEntity );

	void Spawn() override;
	void SpawnKey( const std::string& key, const std::string& value ) override;

	void PortalUse( SVGBaseEntity* other, SVGBaseEntity* activator );

	void ActivatePortal( bool open );

protected:
	bool turnedOn{ false };
};
