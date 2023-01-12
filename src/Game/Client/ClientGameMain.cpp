/***
*
*	License here.
*
*	@file
* 
*   General functionality things of the ClientGame module goes here.
*
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"



/**
*   Contains the function pointers being passed in from the engine.
**/
ClientGameImport clgi;

/**
*   Pointer to the client frame state, and the client shared data.
**/
// Pointer to the actual client frame state.
ClientState* cl     = nullptr;
// Pointer to the actual client shared data.
ClientShared* cs    = nullptr;



/***
*
*	CVars.
*
***/
/**
*	General/View/Gameplay.
**/
cvar_t *cl_disable_particles    = nullptr;
cvar_t *cl_disable_explosions   = nullptr;
cvar_t *cl_explosion_sprites    = nullptr;
cvar_t *cl_explosion_frametime  = nullptr;
cvar_t *cl_footsteps            = nullptr;
cvar_t *cl_kickangles           = nullptr;
cvar_t *cl_monsterfootsteps     = nullptr;
cvar_t *cl_noglow               = nullptr;
cvar_t *cl_noskins              = nullptr;
cvar_t *cl_player_model         = nullptr;
cvar_t *cl_predict              = nullptr;
cvar_t *cl_rollhack             = nullptr;
cvar_t *cl_thirdperson_angle    = nullptr;
cvar_t *cl_thirdperson_range    = nullptr;
cvar_t *cl_thirdperson_traceshape    = nullptr;
cvar_t *cl_vwep                 = nullptr;

/**
*	User Info.
**/
cvar_t *info_fov        = nullptr;
cvar_t *info_hand       = nullptr;
cvar_t *info_msg        = nullptr;
cvar_t *info_name       = nullptr;
cvar_t *info_password   = nullptr;
cvar_t *info_skin       = nullptr;
cvar_t *info_spectator  = nullptr;
cvar_t *info_uf         = nullptr;
//! Is set to 1  at boot time when loading mainmenu.bsp, and is set 
//! to 1 when disconnecting from a server hence, once again, loading mainmenu.bsp
cvar_t *info_in_bspmenu = nullptr;

/**
*	Chat.
**/
cvar_t *cl_chat_notify          = nullptr;
cvar_t *cl_chat_sound           = nullptr;
cvar_t *cl_chat_filter          = nullptr;

/**
*	Refresh. TODO: Move.
**/
cvar_t* cvar_pt_beam_lights     = nullptr;

/**
*	Server.
**/
cvar_t *sv_paused		= nullptr;
cvar_t *sv_maxvelocity	= nullptr;
cvar_t *sv_gravity		= nullptr;

/**
*	Video.
**/
cvar_t* vid_rtx = nullptr;

/**
*	Developer cvars.
**/
//! Developer view weapon offset cvars.
cvar_t *cl_vwep_x				= nullptr;
cvar_t *cl_vwep_y				= nullptr;
cvar_t *cl_vwep_z				= nullptr;




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
*	@brief	Wraps around Com_Error.
*	@param	errorType		One of the error types in ErrorType::
*	@param	errorMessage	A string, possibly formatted using fmt::format.
**/
void CLG_Error( int32_t errorType, const std::string &errorMessage ) {
	// Call and pass on the arguments to our imported Com_Error.
	clgi.Com_Error( errorType, "%s", errorMessage.c_str() );
}

/**
*	@brief	Wraps around Com_LPrintf
*	@param	printType		One of the print types in PrintType::
*	@param	printMessage	A string, possibly formatted using fmt::format.
**/
void CLG_Print( int32_t printType, const std::string &printMessage ) {
	// Call and pass on the arguments to our imported Com_LPrintf.
	clgi.Com_LPrintf( printType, "%s", printMessage.c_str() );
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




// this is only here so the functions in q_shared.c can link
void Com_LPrintf( int32_t printType , const char *fmt, ...) {
    va_list     argptr;
    char        text[MAX_STRING_CHARS];

    if (printType == PrintType::Developer) {
        return;
    }

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    clgi.Com_LPrintf( PrintType::Developer, "%s", text );
}

void Com_Error( int32_t errorType, const char *fmt, ...) {
    va_list     argptr;
    char        text[MAX_STRING_CHARS];

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    clgi.Com_Error( ErrorType::Drop, "%s", text );
}