/***
*
*	License here.
*
*	@file
* 
*   General functionality things of the ClientGame module goes here.
*
***/
// Core.
#include "ClientGameLocals.h"          // Include SVGame header.

// Entities.
//#include "Entities/Worldspawn.h"

// GameModes.
//#include "Gamemodes/IGamemode.h"
//#include "Gamemodes/DefaultGamemode.h"
//#include "Gamemodes/CoopGamemode.h"
//#include "Gamemodes/DeathMatchGamemode.h"

// GameWorld.
#include "World/ClientGameWorld.h"

// Player related.
//#include "Player/Client.h"      // Include Player Client header.

// Physics related.
//#include "Physics/StepMove.h"



/**
*
*
*   ClientGame Module Entry Point.
*
*
**/
// CPP: These might need to be re-enabled on Linux.
extern "C" {
q_exported IClientGameExports* GetClientGameAPI(ClientGameImport* clgimp) {
    // Store a copy of the engine imported function pointer struct.
    clgi = *clgimp;

    // Store pointers to client  data.
    cl = clgimp->cl;
    cs = clgimp->cs;

    // Test if it is compatible, if not, return clge with only the apiversion set.
    // The client will handle the issue from there on.
    if (clgimp->apiversion.major != CGAME_API_VERSION_MAJOR ||
        clgimp->apiversion.minor != CGAME_API_VERSION_MINOR) {
        //clge.apiversion = { CGAME_API_VERSION_MAJOR, CGAME_API_VERSION_MINOR, CGAME_API_VERSION_POINT };
        
        // WID: TODO: Gotta do something alternative here.
        return nullptr;
        //return &clge;
    }

    // Initialize Game Entity TypeInfo system.
    TypeInfo::SetupSuperClasses();

    // Allocate the client game exports interface and its member implementations.
    clge = new ClientGameExports();

    // Return cgame function pointer struct.
    return clge;
}

}; // Extern "C"



/**
*
*
*   Common Wrappers. (Debug-) Printing, etc.
*
*
**/
// Prints a message of type PrintType::All. Using variable arg formatting.
void Com_Print(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PrintType::All, "%s", buffer);
    va_end (args);
}
// Prints a message of type PrintType::Developer. Using variable arg formatting.
// Only prints when developer cvar is set to 1.
void Com_DPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PrintType::Developer, "%s", buffer);
    va_end (args);
}
// Prints a message of type PrintType::Warning. Using variable arg formatting.
void Com_WPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PrintType::Warning, "%s", buffer);
    va_end (args);
}
// Prints a message of type PrintType::Error. Using variable arg formatting.
void Com_EPrint(const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PrintType::Error, "%s", buffer);
    va_end (args);
}

// Triggers an error code of type. Using variable arg formatting.
void Com_Error (int32_t errorType, const char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_Error(errorType, "%s", buffer);
    va_end (args);
}

// Prints a message of a type of your own liking. Using variable arg formatting
void Com_LPrintf(int32_t printType, const char* fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(printType, "%s", buffer);
    va_end(args);
}


/**
*
*
*   Client Game Specific Functions.
*
*
**/
/**
*	@brief	... Sound.
**/
void CLG_Sound(GameEntity* ent, int32_t channel, int32_t soundIndex, float volume, float attenuation, float timeOffset) {
    if (!ent)
        return;

	// Todo: Fix...

	//clgi.S_StartSound(origin, entityNumber, channel, soundIndex, volume, attenuation, timeOffset);
}

/**
*	@brief	Precaches the model and returns the model index qhandle_t.
**/
qhandle_t CLG_PrecacheModel(const std::string& filename) {
    return 0; //clgi.R_RegisterModel(filename.c_str());
}

/**
*	@brief	Precaches the image and returns the image index qhandle_t.
**/
qhandle_t CLG_PrecacheImage(const std::string& filename) {
    //return clgi.R_RegisterPic2(filename.c_str());
	return 0;
}

/**
*	@brief	Precaches the sound and returns the sound index qhandle_t.
**/
qhandle_t CLG_PrecacheSound(const std::string& filename) {
    return 0;//return clgi.S_RegisterSound(filename.c_str());
}