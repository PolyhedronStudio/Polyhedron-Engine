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

class Light : public SVGBaseTrigger {
public:
    // Constructor/Deconstructor.
    Light(Entity* svEntity);
    virtual ~Light();

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
};

#endif // __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__