#include "../ClientGameLocal.h"

#include "../Effects.h"
#include "../Entities.h"
#include "../Input.h"
#include "../Main.h"
#include "../Media.h"
#include "../Parse.h"
#include "../Predict.h"
#include "../Screen.h"
#include "../TemporaryEntities.h"
#include "../View.h"

#include "shared/interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Screen.h"

extern cvar_t* scr_alpha;
extern cvar_t* scr_scale;

extern RenderScreenData scr;

//---------------
// ClientGameScreen::CheckPredictionError
//
//---------------
void ClientGameScreen::RenderScreen() {
    // start with full screen HUD
    scr.hud_width = cl->refdef.width;
    scr.hud_height = cl->refdef.height;

    // Calculate view rectangle.
    SCR_CalcVRect();

    clgi.R_SetAlphaScale(scr.hud_alpha);

    clgi.R_SetScale(scr.hud_scale);

    scr.hud_height *= scr.hud_scale;
    scr.hud_width *= scr.hud_scale;

    // Render the crosshair.
    SCR_DrawCrosshair();

    // the rest of 2D elements share common alpha
    clgi.R_ClearColor();
    clgi.R_SetAlpha(clgi.Cvar_ClampValue(scr_alpha, 0, 1));

    // Draw status.
    SCR_DrawStats();

    // Draw layout string.
    SCR_DrawLayout();

    // Draw inventory.
    SCR_DrawInventory();

    // Draw center screen print
    SCR_DrawCenterString();

    // Draw objects.
    SCR_DrawObjects();

    // Draw FPS.
    SCR_DrawFPS();

    // Draw Chat Hud.
    SCR_DrawChatHUD();
}

//---------------
// ClientGameScreen::ScreenModeChanged
//
//---------------
void ClientGameScreen::ScreenModeChanged() {
    if (scr.initialized)
        scr.hud_scale = clgi.R_ClampScale(scr_scale);

    scr.hud_alpha = 1.f;
}

//---------------
// ClientGameScreen::DrawLoadScreen
//
//---------------
void ClientGameScreen::DrawLoadScreen() {
    clgi.R_SetScale(scr.hud_scale);

    int32_t x = (cl->refdef.width * scr.hud_scale - scr.loading_width) / 2; //x = (r_config.width * scr.hud_scale - scr.loading_width) / 2;
    int32_t y = (cl->refdef.height * scr.hud_scale - scr.loading_height) / 2; //y = (r_config.height * scr.hud_scale - scr.loading_height) / 2;

    clgi.R_DrawPic(x, y, scr.loading_pic);
    clgi.R_SetScale(1.0f);
}

//---------------
// ClientGameScreen::DrawPauseScreen
//
//---------------
void ClientGameScreen::DrawPauseScreen() {
    int32_t x = (scr.hud_width - scr.pause_width) / 2;
    int32_t y = (scr.hud_height - scr.pause_height) / 2;

    clgi.R_DrawPic(x, y, scr.pause_pic);
}