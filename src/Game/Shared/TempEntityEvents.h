/***
*
*	License here.
*
*	@file
*
*	SharedGame Temp entity events are for things that happen at a location seperate from 
*	any existing entity. Temporary entity messages are explicitly constructed and broadcast 
*	'on the wire' to a specific or multiple specific clients.*
*
*	Examples of this could be bullet hit effects, such as the current sad but true smoke puff
*	effect.
*
***/
#pragma once


/**
*	@details	Temp entity events are for things that happen at a location seperate from 
*				any existing entity. Temporary entity messages are explicitly constructed and broadcast 
*				'on the wire' to a specific or multiple specific clients.*
*
*				Examples of this could be bullet hit effects, such as the current sad but true smoke puff
*				effect.
**/
struct TempEntityEvent {
    //! General gunshot particle effect.
    static constexpr uint8_t Gunshot = 0;
    //! Shotgun particle effect.
    static constexpr uint8_t Shotgun = 1;
    //! Blaster particle effect.
    static constexpr uint8_t Blaster = 2;
    //! Flare particle effect.
    static constexpr uint8_t Flare = 3;
    //! Bl00d particle effect.
    static constexpr uint8_t Blood = 10;
    //! M0r3 bl00d particle effect.
    static constexpr uint8_t MoreBlood = 11;

    //! Explosion 1 sprite and particle effect.
    static constexpr uint8_t Explosion1 = 20;
    //! Explosion 2 sprite and particle effect.
    static constexpr uint8_t Explosion2 = 21;
    //! Plain Explosion sprite and particle effect.
    static constexpr uint8_t PlainExplosion = 22;
    //! Big Explosion sprite and particle effect.
    static constexpr uint8_t BigExplosion1 = 23;
    //! Same as Explosion1, but without particles.
    static constexpr uint8_t NoParticleExplosion1 = 24;

	//! Spawns body gibs of said count.
	static constexpr uint8_t BodyGib = 30;
	//! Spawns debris gibs of said count and said debris type.
	static constexpr uint8_t DebrisGib = 31;

    //! General sparks particle effect.
    static constexpr uint8_t Sparks = 50;
    //! Bullet sparks particle effect.
    static constexpr uint8_t BulletSparks = 51;
    //! Electrical sparks particle effect.
    static constexpr uint8_t ElectricSparks = 52;
    //! Splash particle effect.
    static constexpr uint8_t Splash = 60;

    //! Bubble Trail 1 particle effect.
    static constexpr uint8_t BubbleTrailA = 70;
    //! Bubble Trail 2 particle effect.
    static constexpr uint8_t BubbleTrailB = 71;

    //! Flame sprite particle effect.
    static constexpr uint8_t Flame = 80;
    //! Steam sprite particle effect.
    static constexpr uint8_t Steam = 90;

    static constexpr uint8_t ForceWall = 100;
    static constexpr uint8_t TeleportEffect = 101;

    static constexpr uint8_t DebugTrail = 254;

    static constexpr uint8_t Max = 255;
};


/**
*   @brief  Splash Type determining the effect to be displayed for the Flash TE.
**/
struct SplashType {
    static constexpr uint8_t Unknown = 0;
    static constexpr uint8_t Sparks = 1;
    static constexpr uint8_t BlueWater = 2;
    static constexpr uint8_t BrownWater = 3;
    static constexpr uint8_t Slime = 4;
    static constexpr uint8_t Lava = 5;
    static constexpr uint8_t Blood = 6;
};