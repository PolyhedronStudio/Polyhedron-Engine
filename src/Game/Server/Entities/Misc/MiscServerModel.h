/***
*
*	License here.
*
*	@file
*
*	Server Model Entity: Takes up an entity slot on the wire and should be used only if
*	a Client Model is insuficient. (Example case: You really badly need lightstyles on a
*	light model.)
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;

class MiscServerModel : public SVGBaseTrigger {
public:
    // Constructor/Deconstructor.
    MiscServerModel(PODEntity *svEntity);
    virtual ~MiscServerModel() = default;

    DefineMapClass("misc_servermodel", MiscServerModel, SVGBaseTrigger);



    /**
	*
	*
	*	Interface functions. 
    *
	*
	**/
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void Respawn() override;     // Respawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value) override;



    /**
	*
	*
	*	Entity functions. 
    *
	*
	**/
	/**
	*	@brief	Set the 'endFrame' value.
	**/
    inline void SetEndFrame(const uint32_t endFrame) {
        this->endFrame = endFrame;
    }
	/**
	*	@brief	Set the 'startFrame' value.
	**/
    inline void SetStartFrame(const uint32_t startFrame) {
        this->startFrame = startFrame;
    }
	/**
	*	@brief	Get the 'endFrame' value.
	**/
    inline const uint32_t GetEndFrame() {
        return this->endFrame;
    }
	/**
	*	@brief	Get the 'noisePath' value.
	**/
    inline const std::string& GetNoisePath() {
        return this->noisePath;
    }
	/**
	*	@brief	Get the 'startFrame' value.
	**/
    inline const uint32_t GetStartFrame() {
        return this->startFrame;
    }
	/**
	*	@brief	Get the 'boundingboxMaxs' value.
	**/
    inline const vec3_t& GetBoundingBoxMaxs() {
        return this->boundingBoxMaxs;
    }
	/**
	*	@brief	Get the 'boundingboxMins' value.
	**/
    inline const vec3_t& GetBoundingBoxMins() {
        return this->boundingBoxMins;
    }
	/**
	*	@brief	Get the 'customLightStyle' value.
	**/
    inline const char *GetCustomLightStyle() {
        return customLightStyle.c_str();
    }
	/**
	*	@brief	Set the 'customLightStyle' value.
	**/
    void SetCustomLightStyle(const std::string& lightStyle) {
        this->customLightStyle = lightStyle;
    }

    /**
	*
	*
	*	Callback Functions.
    *
	*
	**/
    void MiscServerModelThink(void);
	void MiscServerModelUse( GameEntity* other, GameEntity* activator );
    void MiscServerModelDie( GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point );

private:
	//! Actual Light States.
	struct LightState {
		//! Whenever this is its state, it means we're in "a" style. (Light is off.)
		static constexpr int32_t Off	= 0;
		//! In any other case, it is On. (It has a lightstyle higher than 'a').
		static constexpr int32_t On		= 1;
	};
    //! Model Light State flags. (Is it currently off, or triggered?)
    uint32_t lightState = LightState::On;
	//! Stores this light's custom light style string. (If set.)
	std::string customLightStyle = "";

    // The noise path that got parsed and is in use.
    std::string noisePath = "";
    uint32_t precachedNoiseIndex = 0;

    // The actual frame that this model its animation should start off with.
    float startFrame = 0;

    // The actual frame that this model its animation should end at.
    float endFrame = 0;

    // The bounding box its bottom left, this can be custom set in map editors.
    vec3_t boundingBoxMins = { -16, -16, 0 };
    vec3_t boundingBoxMaxs = { 16, 16, 40 };
};
