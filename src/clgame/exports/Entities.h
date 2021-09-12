// License here.
// 
//
// ClientGameEntities implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Entities IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameEntities : public IClientGameExportEntities {
public:
	// Executed whenever an entity event is receieved.
	void Event(int32_t number) final;
};

