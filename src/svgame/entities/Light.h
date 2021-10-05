/*
// LICENSE HERE.

//
// Light.h
//
// Light entity definition.
//
*/
#ifndef __SVGAME_ENTITIES_LIGHT_H__
#define __SVGAME_ENTITIES_LIGHT_H__

class SVGBaseTrigger;

enum LightState : uint32_t {
    Off = 1,
    On = 2,
};

class Light : public SVGBaseTrigger {
public:
    // Constructor/Deconstructor.
    Light(Entity* svEntity);
    virtual ~Light();

    DefineMapClass( "light", Light, SVGBaseTrigger );

    //
    // Interface functions. 
    //
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value) override;

    //
    // Get/Set
    // 
    // 'customLightStyle'.
    const std::string& GetCustomLightStyle() {
        return this->customLightStyle;
    }
    void SetCustomLightStyle(const std::string& lightStyle) {
        this->customLightStyle = lightStyle;
    }

    //
    // Callback functions.
    //
    void LightUse(SVGBaseEntity* other, SVGBaseEntity* activator);

private:
    // Custom lightstyle string.
    std::string customLightStyle;

    // Light State flags. (Is it currently off, or triggered?)
    uint32_t lightState;
};

#endif // __SVGAME_ENTITIES_LIGHT_H__