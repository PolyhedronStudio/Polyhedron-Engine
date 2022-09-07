#pragma once

class CLGBaseLocalEntity;
class IClientGameEntity;

//=============================================
// FuncAreaportal
// 
// This is a non-visible object that divides the world into
// areas that are seperated when this portal is not activated.
// Usually enclosed in the middle of a door.
//=============================================
class FuncAreaportal : public CLGBaseLocalEntity {
public:
	FuncAreaportal( Entity* entity );
	virtual ~FuncAreaportal() = default;

	DefineMapClass( "func_areaportal", FuncAreaportal, CLGBaseLocalEntity );

	void Spawn() override;
	void SpawnKey( const std::string& key, const std::string& value ) override;

	// For triggering by mappers
	void FuncAreaportalUse( IClientGameEntity* other, IClientGameEntity* activator );
	// For doors and other autonomous entities
	void ActivatePortal( const bool open );

protected:
	bool turnedOn{ false };
};
