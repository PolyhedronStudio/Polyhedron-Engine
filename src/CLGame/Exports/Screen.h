// License here.
// 
//
// ClientGameScreen implementation.
#pragma once

#include "Shared/Interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Screen IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameScreen : public IClientGameExportScreen {
public:
    // Called when the engine decides to render the 2D display.
    void RenderScreen() final;
    // Called when the screen mode has changed.
    void ScreenModeChanged() final;
    // Called when the client wants to render the loading screen.
    void DrawLoadScreen() final;
    // Called when the client wants to render the pause screen.
    void DrawPauseScreen() final;
};

