#pragma once

class SVGBasePusher;

class FuncButton : public SVGBasePusher
{
public:
	FuncButton( Entity* svEntity );
	virtual ~FuncButton() = default;

	void Precache() override;
	void Spawn() override;
	
	void SpawnKey( const std::string& key, const std::string& value ) override;

	static void OnButtonDone( Entity* self );
	void ButtonDone();
	void ButtonReturn();
	static void OnButtonWait( Entity* self );
	void ButtonWait();
	void ButtonFire();

	void ButtonUse( SVGBaseEntity* other, SVGBaseEntity* activator );
	void ButtonTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf );
	void ButtonDie( SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point );

protected:
	void BrushMoveDone();
	void BrushMoveFinal();
	void BrushMoveBegin();
	void BrushMoveCalc( const vec3_t& destination, PushMoveEndFunction* function );

protected:
	float lip{ 0.0f };
};


