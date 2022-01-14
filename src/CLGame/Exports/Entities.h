// License here.
// 
//
// ClientGameEntities implementation.
#pragma once

// Client Game Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Entities IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameEntities : public IClientGameExportEntities {
public:
	// Executed whenever an entity event is receieved.
	void Event(int32_t number) final;

	// Parse the server frame for server entities to add to our client view.
	// Also applies special rendering effects to them where desired.
	void AddPacketEntities() final;

private:
	// Gives the opportunity to adjust render effects where desired.
	int32_t ApplyRenderEffects(int32_t renderEffects);
};

