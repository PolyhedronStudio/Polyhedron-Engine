/***
*
*	License here.
*
*	@file
*
*	SharedGame WaterLevels. These are set by both (Client-/Server-)Game Modules
*	in the Physics and PlayerMove code base on the results of PointContents testing.
* 
***/
#pragma once


/**
*   @brief  The water level that a certain entity is at.
**/
struct WaterLevel {
    static constexpr int32_t Unknown    = -1;
    static constexpr int32_t None       = 0;
    static constexpr int32_t Feet       = 1;
    static constexpr int32_t Waist      = 2;
    static constexpr int32_t Under      = 3;
};