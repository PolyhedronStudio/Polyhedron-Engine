// LICENSE HERE.

//
// ClientGameExports.cpp
//
// Local includes (shared, & other game defines.)
#include "../clg_local.h"

// The actual Implementation.
#include "ClientGameExports.hpp"

//
//===============
// ClientGameExports
//===============
//
ClientGameExports::ClientGameExports() {
    // Setup the API version.
    apiversion = {
        CGAME_API_VERSION_MAJOR,
        CGAME_API_VERSION_MINOR,
        CGAME_API_VERSION_POINT,
    };
}

//
//===============
// ~ClientGameExports
//===============
//
ClientGameExports::~ClientGameExports() {

}

