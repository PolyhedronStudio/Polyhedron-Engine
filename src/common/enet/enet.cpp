
#define ENET_IMPLEMENTATION
#include <enet.h>

#include "shared/shared.h"
#include "common/common.h"
#include "common/cvar.h"
#include "common/fifo.h"
#ifdef _DEBUG
#include "common/files.h"
#endif
#include "common/msg.h"
#include "common/enet/enet.h"
#include "common/net/net.h"
#include "common/protocol.h"
#include "common/zone.h"
#include "client/client.h"
#include "server/server.h"
#include "system/system.h"
#include "common/enet/enet.h"

void ENET_Init(void) {
    // Initialize enet, obviously.
    if (enet_initialize() != 0) {
        Com_Error(ERR_FATAL, "An error occurred while initializing ENet:'%d'.\n",
            enet_linked_version());
    }

    // DEV: Inform devs about it.
    Com_DPrintf("ENet:'%d' succesfully initialized\n");
}

void ENET_Shutdown(void) {
    // Deinitialize enet, obviously.
    enet_deinitialize();
}