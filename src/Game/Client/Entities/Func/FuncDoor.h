#pragma once

#include "Game/Client/Entities/Base/CLGBaseMover.h"

//===============
// A standard, sliding door
//===============
class FuncDoor : public CLGBaseMover {
public:
    FuncDoor( Entity* entity );
    virtual ~FuncDoor() = default;

    DefineMapClass( "_func_door", FuncDoor, CLGBaseMover );

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
	void		SpawnKey( const std::string& key, const std::string& value ) override;

protected:
    void        DoorUse( GameEntity* other, GameEntity* activator );
    void        DoorShotOpen( GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point );
    void        DoorBlocked( GameEntity* other );
    void        DoorTouch( GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf );

    void        DoorGoUp( GameEntity* activator );
    void        DoorGoDown();
   
    // These two are overridden by FuncDoorRotating
    virtual void DoGoUp();
    virtual void DoGoDown();

    void        HitTop();
    void        HitBottom();

    // These are leftovers from the legacy brush movement functions
    // Soon, we'll have a... better way... of doing this
    static void OnDoorHitTop( GameEntity* self );
    static void OnDoorHitBottom( GameEntity* self );

    // Admer: some of these could be moved to SVGBaseMover
    void        CalculateMoveSpeed();
    void        SpawnDoorTrigger();
    void        UseAreaportals( bool open ) const;

    GameTime		debounceTouchTime = GameTime::zero();
};
