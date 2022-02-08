// Core.
#include "../ServerGameLocal.h"          // Include SVGame header.

// Entities.
#include "../Entities.h"
//#include "../Entities/Base/SVGEntityHandle.h"
#include "../Entities/Base/PlayerClient.h"

// Gamemodes.
#include "../Gamemodes/IGamemode.h"
#include "../Gamemodes/DefaultGamemode.h"
#include "../Gamemodes/CoopGamemode.h"
#include "../Gamemodes/DeathmatchGamemode.h"

// Gameworld.
#include "../World/GameWorld.h"

// Cvars.
extern cvar_t *gamemode;

/**
*	@brief Initializes the gameworld and its member objects.
***/
void Gameworld::Initialize() { 
	SetupGamemode();
}

/**
*	@brief Shutsdown the gameworld and its member objects.
**/
void Gameworld::Shutdown() {
	DestroyGamemode();
}



/**
*	@brief	Creates the correct gamemode object instance based on the gamemode cvar.
**/
void Gameworld::SetupGamemode() {
	// Fetch gamemode as std::string
	std::string gamemodeStr = gamemode->string;

	// Detect which game ode to allocate for this game round.
	if (gamemodeStr == "deathmatch") {
		// Deathmatch gameplay mode.
	    currentGamemode = new DeathmatchGamemode();
	} else if (gamemodeStr == "coop") {
		// Cooperative gameplay mode.
	    currentGamemode = new CoopGamemode();
	} else {
		// Acts as a singleplayer game mode.
	    currentGamemode = new DefaultGamemode();
	}
}

/**
*	@brief	Destroys the current gamemode object.
**/
void Gameworld::DestroyGamemode() {
	// Always make sure it is valid to begin with.
    if (currentGamemode != nullptr) {
		delete currentGamemode;
		currentGamemode = nullptr;
	}
}