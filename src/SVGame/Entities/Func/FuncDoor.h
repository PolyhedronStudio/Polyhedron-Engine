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

    static constexpr const char* MessageSoundPath = "misc/talk.wav";

    void		Precache() override;
	virtual void Spawn() override;
    void        PostSpawn() override;
	//void		SpawnKey( const std::string& key, const std::string& value ) override;

protected:
    void        DoorUse( SVGBaseEntity* other, SVGBaseEntity* activator );
    void        DoorShotOpen( SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point );
    void        DoorBlocked( SVGBaseEntity* other );
    void        DoorTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf );

    void        DoorGoUp( SVGBaseEntity* activator );
    void        DoorGoDown();
   
    // These two are overridden by FuncDoorRotating
    virtual void DoGoUp();
    virtual void DoGoDown();

    void        HitTop();
    void        HitBottom();

    // These are leftovers from the legacy brush movement functions
    // Soon, we'll have a... better way... of doing this
    static void OnDoorHitTop( Entity* self );
    static void OnDoorHitBottom( Entity* self );

    // Admer: some of these could be moved to SVGBaseMover
    void        CalculateMoveSpeed();
    void        SpawnDoorTrigger();
    void        UseAreaportals( bool open ) const;

    float		debounceTouchTime{ 0.0f };
};
