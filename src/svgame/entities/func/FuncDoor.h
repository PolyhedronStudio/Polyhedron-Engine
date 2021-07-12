#pragma once

class SVGBaseMover;

//===============
// A standard, sliding door
//===============
class FuncDoor : public SVGBaseMover {
public:
    FuncDoor( Entity* entity );
    virtual ~FuncDoor() = default;

    DefineMapClass( "func_door", FuncDoor, SVGBaseMover );

    // Spawn flags
    static constexpr int32_t SF_StartOpen   = 1 << 0;
    static constexpr int32_t SF_Reverse     = 1 << 1;
    static constexpr int32_t SF_Crusher     = 1 << 2;
    static constexpr int32_t SF_NoMonsters  = 1 << 3;
    static constexpr int32_t SF_Toggle      = 1 << 4;
    static constexpr int32_t SF_XAxis       = 1 << 5;
    static constexpr int32_t SF_YAxis       = 1 << 6;

    // Functions.
    void		Precache() override;
	void		Spawn() override;
    void        PostSpawn() override;
	//void		SpawnKey( const std::string& key, const std::string& value ) override;

    // Callbacks.
    void DoorUse(SVGBaseEntity* caller, SVGBaseEntity* activator);
    void DoorThink(void);
    void DoorDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void DoorTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);
    void DoorBlocked(SVGBaseEntity* other);

protected:
    // Moves the door "down", aka shuts it.
    void GoDown(FuncDoor *ent);
    void GoUp(FuncDoor* ent, SVGBaseEntity *activator);

    void        CalculateMoveSpeed();
    void        SpawnDoorTrigger();
    void        UseAreaportals( bool open ) const;
};
