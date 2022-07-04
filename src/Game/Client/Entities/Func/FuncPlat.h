/***
*
*	License here.
*
*	@file
*
*	Client-Side FuncPlat Entity Support.
*
***/
#pragma once

class CLGBaseMover;
class TriggerAutoPlatform;

//===============
// A standard platform.
//===============
class FuncPlat : public CLGBaseMover {
public:
    //
    // Friend classes.
    //
    friend class TriggerAutoPlatform;

    //
    // CTor/DTor.
    //
    FuncPlat( Entity* entity );
    virtual ~FuncPlat() = default;

    //
    // Class Definition.
    //
    DefineMapClass( "func_platzz", FuncPlat, CLGBaseMover );

    //
    // Spawn flags
    //
    static constexpr int32_t SF_PlatLowTriggered    = 1 << 0;

    //
    // Core Functionality.
    //
    void            Precache() override;
    virtual void    Spawn() override;
    void            PostSpawn() override;
    void            SpawnKey( const std::string& key, const std::string& value ) override;

    //
    // Get/Set.
    //
    // Height.
    inline const float& GetHeight() {
        return this->height;
    }
    inline void SetHeight(const float& height) {
        this->height = height;
    }

protected:
    //
    // Callbacks for FuncPlat.
    //
    void        PlatformUse( GameEntity* other, GameEntity* activator );
    void        PlatformBlocked( GameEntity* other );
    
    void        PlatformGoUp();
    void        PlatformGoDown();

    //
    // Inner workings.
    //
    virtual void DoGoUp();
    virtual void DoGoDown();

    void        HitTop();
    void        HitBottom();

    //
    // Callbacks for moveinfo.
    // 
    // These are leftovers from the legacy brush movement functions
    // Soon, we'll have a... better way... of doing this
    static void OnPlatformHitTop( GameEntity* self );
    static void OnPlatformHitBottom( GameEntity* self );

    //
    // Private Utilities.
    //
    void CalculateMoveSpeed();
    void SpawnPlatformTrigger();

    //
    // Member Variables.
    //
    // Waiting time until it can be triggered again.
    float debounceTouchTime = 0.0f;
    // Height distance for travelling.
    float height = 0.f;

    // Sound file to use for when showing a message.
    static constexpr const char* MessageSoundPath = "misc/talk.wav";
};
