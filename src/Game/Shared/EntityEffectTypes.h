/***
*
*	License here.
*
*	@file
*
*	SharedGame Entity Effect Types which determine how an entity is to be rendered.
*	Whether it should cycle through sprite frames, have a color shell applied to it
*	or rotate etc.
*
*	These can be set by the client entities and server entities. In the case of the
*	server entities the information is transfered 'over the wire' along with the
*	entity state delta updates.
*
***/
#pragma once


/**
*   @details    Effects are things handled on the client side (lights, particles,
*               frame animations) that happen constantly on a given entity.
*
*               NOTE: An entity that has effects will be sent to the client even if it has a zero
*               index model.
**/
struct EntityEffectType {
    // Animation Effects.
    //! Auto cycle between the frames 0, and 1, at 2 hz.
    static constexpr uint32_t AnimCycleFrames01hz2  = (1 << 0);
    //! Auto cycle between the frames 2, and 3, at 2 hz.
    static constexpr uint32_t AnimCycleFrames23hz2  = (1 << 1);
    //! Auto cycle through all frames at 2 hz.
    static constexpr uint32_t AnimCycleAll2hz       = (1 << 2);
    //! Auto cycle through all frames at 30 hz.
    static constexpr uint32_t AnimCycleAll30hz      = (1 << 3);
    
    //! Color Shell around model.
    static constexpr uint32_t ColorShell    = (1 << 6);
    //! Rotate (Items.)
    static constexpr uint32_t Rotate        = (1 << 8);

    // Entity 'type' Effects that dictate special entity 'type' treatment.
    //! Entity is of type 'gib', and needs special treatment.
    static constexpr uint32_t Gib           = (1 << 10);
    //! Entity is of type 'corpse', and needs special treatment.
    static constexpr uint32_t Corpse        = (1 << 11);

    // 'Other' Effects. (Mostly null model entity stuff, weapon particles.)
    static constexpr uint32_t Blaster       = (1 << 16);
    static constexpr uint32_t Torch         = (1 << 17);
    static constexpr uint32_t Teleporter    = (1 << 24);

    // Maximum last effect slot, feel free to rename it and use it.
    static constexpr uint32_t Max = (1 << 31);
};