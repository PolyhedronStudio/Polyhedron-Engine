// License here.
// 
//
// ClientGameExportCore implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Core IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameExportCore : public IClientGameExportCore {
public:
	// Initializes the client game.
	void Initialize() final;

	// Shuts down the client game.
	void Shutdown() final;
};

