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
class ClientGameEntities : public IClientGameExportEntities {
public:
	// Executed whenever an entity event is receieved.
	void Event(int32_t number) final;
};

