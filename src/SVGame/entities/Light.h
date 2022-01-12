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
    inline const char *GetCustomLightStyle() {
        return customLightStyle.c_str();
    }
    void SetCustomLightStyle(const std::string& lightStyle) {
        this->customLightStyle = lightStyle;
    }

    //
    // Callback functions.
    //
    void LightUse(SVGBaseEntity* other, SVGBaseEntity* activator);
    void LightThink(void);

private:
    // Custom lightstyle string.
    std::string customLightStyle;

    // Light State flags. (Is it currently off, or triggered?)
    uint32_t lightState;
};


#pragma once

class SVGBaseEntity;

class PathCorner : public SVGBaseEntity {
public:
    PathCorner(Entity* entity);
    virtual ~PathCorner() = default;

    DefineMapClass("path_corner", PathCorner, SVGBaseEntity);

    const vec3_t	BboxSize = vec3_t(8.0f, 8.0f, 8.0f);

    // Spawnflags
    static constexpr int32_t SF_Teleport = 1 << 0;

    void			Spawn() override;
    void			SpawnKey(const std::string& key, const std::string& value) override;

    // For AI
    virtual void	OnReachedCorner(SVGBaseEntity* traveler);

    inline const char* GetPathTarget() override {
        return pathTarget.c_str();
    }

private:
    std::string		pathTarget;
};

#endif // __SVGAME_ENTITIES_LIGHT_H__