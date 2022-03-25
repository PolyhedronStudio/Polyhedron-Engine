#pragma once

class SVGBaseMover;

//===============
// A standard button, able to trigger entities once pressed,
// and changes its texture depending on its state
//===============
class FuncButton : public SVGBaseMover {
public:
	FuncButton( Entity* svEntity );
	virtual ~FuncButton() = default;

	DefineMapClass( "func_button", FuncButton, SVGBaseMover );

	void		Precache() override;
	void		Spawn() override;
	void		SpawnKey( const std::string& key, const std::string& value ) override;

	// These static methods here are required for mover logic, since the legacy code
	// we ported simply operates on global functions & function pointers
	static void OnButtonDone( IServerGameEntity* self );
	void		ButtonDone(); // The button is done moving, it is fully pressed
	void		ButtonReturn(); // The button is returning from "pressed" to "sticking out"
	static void	OnButtonWait( IServerGameEntity* self );
	void		ButtonWait(); // The button is waiting for further interactions
	void		ButtonFire(); // The button has just been pressed, do something
	
	void		ButtonUse( IServerGameEntity* other, IServerGameEntity* activator );
	void		ButtonTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
	void		ButtonDie( IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point );
};


