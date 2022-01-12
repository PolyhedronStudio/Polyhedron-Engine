// License here.
// 
//
// ClientGameServerMessage implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game View IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameView : public IClientGameExportView {
public:
    // Called right after the engine clears the scene, and begins a new one.
    void PreRenderView() final;
    // Called whenever the engine wants to clear the scene.
    void ClearScene() final;
    // Called whenever the engine wants to render a valid frame.
    void RenderView() final;
    // Called right after the engine renders the scene, and prepares to
    // finish up its current frame loop iteration.
    void PostRenderView() final;
};
