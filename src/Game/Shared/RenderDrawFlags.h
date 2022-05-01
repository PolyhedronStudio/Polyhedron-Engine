/***
*
*	License here.
*
*	@file
*
*   @brief  Render Draw Flags for telling the client how to "render the screen".
*			This can be an overlay, or actually tell it to render a 3D model on
*			the screen instead of inside the actual world.
*		
*			These can be set by the ServerGame and if set will be transfered to
*			the clients 'on the wire' along with the rest of an entity state update.
*
***/
#pragma once


/**
*   @brief  Render Draw Flags for telling the client how to "render the screen".
*			This can be an overlay, or actually tell it to render a 3D model on
*			the screen instead of inside the actual world.
**/
struct RenderDrawFlags {
    static constexpr int32_t Underwater = 1;    // warp the screen as apropriate
    static constexpr int32_t NoWorldModel = 2;  // used for player configuration screen
    static constexpr int32_t InfraRedGoggles = 4;
    static constexpr int32_t UVGoggles = 8;
};