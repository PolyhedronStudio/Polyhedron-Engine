// License here.
// 
//
// ClientGameServerMessage implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Server Message IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameServerMessage : public IClientGameExportServerMessage {
public:
    // Called when a configstring update has been parsed and still left
    // unhandled by the client.
    qboolean UpdateConfigString(int32_t index, const char* str) final;
    // Called at the start of receiving a server message.
    void Start() final;
    // Actually parses the server message, and handles it accordingly.
    // Returns false in case the message was unkown, or corrupted, etc.
    qboolean Parse(int32_t serverCommand) final;
    // Handles the demo message during playback.
    // Returns false in case the message was unknown, or corrupted, etc.
    qboolean SeekDemoMessage(int32_t demoCommand) final;
    // Called when we're done receiving a server message.
    void End(int32_t realTime) final;
};

