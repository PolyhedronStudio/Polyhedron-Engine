// License here.
// 
//
// Interface that a client game dll his exports have to implement in order to
// be fully coherent with the actual client loading it in.
// 
// WID: Time to re-adjust here with new files. I agree, at last.
#pragma once

//---------------------------------------------------------------------
// CORE Export Interface.
//---------------------------------------------------------------------
class IClientGameExportCore {
	//---------------------------------------------------------------------
	// API Version.
	// 
	// The version numbers will always be equal to those that were set in 
	// CMake at the time of building the engine/game(dll/so) binaries.
	// 
	// In an ideal world, we comply to proper version releasing rules.
	// For Nail & Crescent, the general following rules apply:
	// --------------------------------------------------------------------
	// MAJOR: Ground breaking new features, you can expect anything to be 
	// incompatible at that.
	// 
	// MINOR : Everytime we have added a new feature, or if the API between
	// the Client / Server and belonging game counter-parts has actually 
	// changed.
	// 
	// POINT : Whenever changes have been made, and the above condition 
	// is not met.
	//---------------------------------------------------------------------
	struct APIVersion {
		int32_t major{ VERSION_MAJOR };
		int32_t minor{ VERSION_MINOR };
		int32_t point{ VERSION_POINT };
	} version;

	// Initializes the CLGame.
	virtual void Initialize() = 0;

	// Shuts down the CLGame.
	virtual void Shutdown() = 0;
};

//---------------------------------------------------------------------
// MAIN interface to implement. It holds pointers to actual sub interfaces,
// which one of course has to implement as well.
//---------------------------------------------------------------------
class IClientGameExports {
public:
	// WID: TODO: Normally we'd use a Get, should we do that and make these private?
	// Perhaps not.
	IClientGameExportCore* core;
	
};

