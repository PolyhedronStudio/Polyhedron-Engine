// License here.
// 
//
// ClientGameServerMessage implementation.
#pragma once

// Client Game Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"

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

private:
    // Finalizes the view values, aka render first or third person specific view data.
    void FinalizeViewValues();
    // Sets up a firstperson view mode.
    void SetupFirstpersonView();
    // Sets up a thirdperson view mode.
    void SetupThirdpersonView();

    // TODO: If this feature gets to stay-alive, it should be moved over to the server.
    // Technically this is rather impossible to do given that we have no pre-baked data anymore.
    // And even if we do, it won't align to the real time RTX data that is made use of.
    void SetLightLevel();
};
