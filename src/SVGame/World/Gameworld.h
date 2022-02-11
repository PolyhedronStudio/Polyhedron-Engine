/***
*
*	License here.
*
*	@file
*
*	Gameworld class for managing entity lifetime. 
*	(Creation, Destruction, Spawning etc.)
* 
*	Aside from managing entity lifetimes, it is also the general keeper of the
*	current active gamemode. The gamemode and gameworld are close friends who
*	go along happily hand in hand.
*
***/
#pragma once

// Pre-define.
class SVGBaseEntity;
class SVGEntityHandle;
class IGamemode;

/**
*	@brief GameWorld regulates the lifetime management of all entities.
* 
*	@details 
**/
class Gameworld {
public:
	/**
	*	@brief Default constructor.
	**/
    Gameworld() = default;

    /**
	*	@brief Default destructor
	**/
    ~Gameworld() = default;

public:
    /**
	*	@brief Initializes the gameworld and its member objects.
	***/
    void Initialize();
	/**
	*	@brief Shutsdown the gameworld and its member objects.
	**/
    void Shutdown();


	/**
	*	@brief	Creates the correct gamemode object instance based on the gamemode cvar.
	**/
    void SetupGamemode();
    /**
	*	@brief	Destroys the current gamemode object.
	**/
    void DestroyGamemode();
	/**
	*	@return A pointer to the current active game mode.
	**/
    inline IGamemode* GetCurrentGamemode() { return currentGamemode; }

private:
	//! Pointer to the current active gamemode.
	IGamemode *currentGamemode = nullptr;
};