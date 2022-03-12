// License here.
// 
//
// ClientGameCore implementation.
#pragma once

#include "Shared/Interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Core IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameCore : public IClientGameExportCore {
public:
	/**
	*	@brief	Initializes the client game.
	**/
	void Initialize() final;

	/**
	*	@brief	Shuts down the client game.
	**/
	void Shutdown() final;

private:

};

