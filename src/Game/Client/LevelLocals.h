/***
*
*	License here.
*
*	@file
* 
*   Client Entity utility functions.
*
***/
#pragma once

#include "ClientGameLocals.h"



/**
*   @brief  This one is here temporarily, it's currently the way how things still operate in the ServerGame
*           module. Eventually we got to aim for a more streamlined design. So for now it resides here as an
*           ugly darn copy of..
*/
struct LevelLocals  {
    //! Current sum of total frame time taken.
    GameTime time = GameTime::zero();
};