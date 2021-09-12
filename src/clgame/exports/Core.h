// License here.
// 
//
// ClientGameExportCore implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// MAIN interface to implement. It holds pointers to actual sub interfaces,
// which one of course has to implement as well.
//---------------------------------------------------------------------
class ClientGameExportCore : public IClientGameExportCore {
public:
	// Initializes the client game.
	void Initialize() final;

	// Shuts down the client game.
	void Shutdown() final;
};

