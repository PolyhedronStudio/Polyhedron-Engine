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

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Screen.h"

extern cvar_t* scr_alpha;
extern cvar_t* scr_scale;

extern RenderScreenData scr;

/***
*
*   Cmd/CVar callbacks.
* 
***/
void ClientGameScreen::Cmd_Sky_f() {

}
void ClientGameScreen::Cmd_Draw_f() {

}
void ClientGameScreen::Cmd_Draw_c(genctx_t* ctx, int argnum) {

}
void ClientGameScreen::Cmd_UnDraw_f() {

}
void ClientGameScreen::Cmd_UnDraw_c(genctx_t* ctx, int argnum) {

}
void ClientGameScreen::Cmd_ClearChatHUD_f() {

}

/**
*   @brief  Callback for when the crosshair scale cvar changed.
**/
void ClientGameScreen::CVarCrosshairChanged(cvar_t* cvar)
{
    char buffer[16];
    int w, h;
    float scale;

    if (cvar->integer > 0) {
        Q_snprintf(buffer, sizeof(buffer), "ch%i", cvar->integer);
        scr.crosshair_pic = clgi.R_RegisterPic(buffer);
        clgi.R_GetPicSize(&w, &h, scr.crosshair_pic);

        // prescale
        scale = clgi.Cvar_ClampValue(clgi.Cvar_Get("ch_scale", nullptr, 0), 0.1f, 9.0f);
        scr.crosshair_width = w * scale;
        scr.crosshair_height = h * scale;

        if (scr.crosshair_width < 1) {
            scr.crosshair_width = 1;
        }
        if (scr.crosshair_height < 1) {
            scr.crosshair_height = 1;
        }

        // Convert and assign cvar crosshair color values.
        cvar_t *ch_red = clgi.Cvar_Get("ch_red", nullptr, 0);
        if (ch_red) {
            scr.crosshair_color.u8[0] = (byte)(ch_red->value * 255);
        }
        cvar_t *ch_green = clgi.Cvar_Get("ch_green", nullptr, 0);
        if (ch_green) {
            scr.crosshair_color.u8[1] = (byte)(ch_green->value * 255);
        }
        cvar_t *ch_blue = clgi.Cvar_Get("ch_blue", nullptr, 0);
        if (ch_blue) {
            scr.crosshair_color.u8[2] = (byte)(ch_blue->value * 255);
        }
        cvar_t *ch_alpha = clgi.Cvar_Get("ch_alpha", nullptr, 0);
        if (ch_alpha) {
            scr.crosshair_color.u8[3] = (byte)(ch_alpha->value * 255);
        }
    }
    else {
        scr.crosshair_pic = 0;
    }
}

/**
*   @brief  Callback for when the screen scale cvar changed.
**/
void ClientGameScreen::CVarScreenScaleChanged(cvar_t* cvar) {
    clge->screen->screenData.hudScale = clgi.R_ClampScale(cvar);
}



/***
*
*   Interface Implementation.
* 
***/
void ClientGameScreen::Initialize() {
    // Fetch CVars from client.
    scr_viewsize    = clgi.Cvar_Get("viewsize", NULL, 0);
    scr_draw2d      = clgi.Cvar_Get("scr_draw2d", NULL, 0);
    scr_alpha       = clgi.Cvar_Get("scr_alpha", NULL, 0);
    scr_font        = clgi.Cvar_Get("scr_font", NULL, 0);
    scr_fps         = clgi.Cvar_Get("scr_fps", NULL, 0);

    // Create CVars.
    scr_scale               = clgi.Cvar_Get("scr_scale", "2", 0);
    scr_scale->changed      = CVarScreenScaleChanged;

    scr_showitemname        = clgi.Cvar_Get("scr_showitemname", "1", CVAR_ARCHIVE);

    scr_centertime          = clgi.Cvar_Get("scr_centertime", "2.5", 0);

    scr_crosshair           = clgi.Cvar_Get("crosshair", "0", CVAR_ARCHIVE);
    scr_crosshair->changed  = CVarCrosshairChanged;

    ch_scale            = clgi.Cvar_Get("ch_scale", "1", 0);
    ch_scale->changed   = CVarCrosshairChanged;
    ch_x                = clgi.Cvar_Get("ch_x", "0", 0);
    ch_y                = clgi.Cvar_Get("ch_y", "0", 0);

    scr_chathud         = clgi.Cvar_Get("scr_chathud", "0", 0);
    scr_chathud_lines   = clgi.Cvar_Get("scr_chathud_lines", "4", 0);
    scr_chathud_time    = clgi.Cvar_Get("scr_chathud_time", "0", 0);
    scr_chathud_x       = clgi.Cvar_Get("scr_chathud_x", "8", 0);
    scr_chathud_y       = clgi.Cvar_Get("scr_chathud_y", "-64", 0);

    ch_red              = clgi.Cvar_Get("ch_red", "1", 0);
    ch_red->changed     = CVarCrosshairChanged;
    ch_green            = clgi.Cvar_Get("ch_green", "1", 0);
    ch_green->changed   = CVarCrosshairChanged;
    ch_blue             = clgi.Cvar_Get("ch_blue", "1", 0);
    ch_blue->changed    = CVarCrosshairChanged;
    ch_alpha            = clgi.Cvar_Get("ch_alpha", "1", 0);
    ch_alpha->changed   = CVarCrosshairChanged;

    // Register commands.
    clgi.Cmd_Register(screenCommands);

    // Call upon the callback ourselves here.
    CVarScreenScaleChanged(scr_scale);

    // We've initialized the screen.
    screenData.isInitialized = true;
}

void ClientGameScreen::Shutdown() {
    // Unregister screen console commands.
    clgi.Cmd_Unregister(screenCommands);

    // Unset isInitialized.
    screenData.isInitialized = false;
}

/**
*   @brief  Called when the engine needs to render the 2D display.
**/
void ClientGameScreen::RenderScreen() {
    // First scale the hud to screen size. (Used for calculating things later on also.)
    screenData.hudSize = { cl->refdef.width, cl->refdef.height};

    // Calculate view rectangle.
    CalculateViewRectangle();

    // Prepare for rendering the HUD.
    clgi.R_SetAlphaScale(scr.hud_alpha);
    clgi.R_SetScale(scr.hud_scale);

    // Calculate HUD size.
    screenData.hudSize *= vec2_t{ screenData.hudScale, screenData.hudScale };

    // Draw the crosshair.
    DrawCrosshair();

    // Draw the HUD.
    DrawPlayerHUD();

    // The rest of 2D elements share the common scr_alpha cvar value.
    clgi.R_ClearColor();
    clgi.R_SetAlpha(clgi.Cvar_ClampValue(scr_alpha, 0, 1));

    // Draw center screen print
    DrawCenterString();

    // Draw FPS.
    DrawFPS();

    // Draw Chat Hud.
    DrawChatHUD();
}

/**
*   @brief  Called when the screen mode has changed.
**/
void ClientGameScreen::ScreenModeChanged() {
    if (screenData.isInitialized) {
        screenData.hudScale = clgi.R_ClampScale(scr_scale);
    }

    screenData.hudAlpha = 1.f;
}

/**
*   @brief  Called when the client wants to render the loading screen.
**/
void ClientGameScreen::DrawLoadScreen() {
    clgi.R_SetScale(scr.hud_scale);

    int32_t x = (cl->refdef.width * scr.hud_scale - scr.loading_width) / 2; //x = (r_config.width * scr.hud_scale - scr.loading_width) / 2;
    int32_t y = (cl->refdef.height * scr.hud_scale - scr.loading_height) / 2; //y = (r_config.height * scr.hud_scale - scr.loading_height) / 2;

    clgi.R_DrawPic(x, y, scr.loading_pic);
    clgi.R_SetScale(1.0f);
}

/**
*   @brief  Called when the client wants to render the pause screen.
**/
void ClientGameScreen::DrawPauseScreen() {
    int32_t x = (scr.hud_width - scr.pause_width) / 2;
    int32_t y = (scr.hud_height - scr.pause_height) / 2;

    clgi.R_DrawPic(x, y, scr.pause_pic);
}



/***
*
*   Screen Functions.
* 
***/
/**
*   @brief  Register screen media.
**/
void ClientGameScreen::RegisterMedia() {

}

/**
*   @brief  Calculate the screen's view rectangle.
**/
void ClientGameScreen::CalculateViewRectangle() {
    screenData.viewRectangle = vec4_t { 0, 0, screenData.hudSize.x, screenData.hudSize.y};
}

/**
*   @brief  Register screen media.
**/
void ClientGameScreen::DrawCrosshair() {
    // Return if crosshair rendering has been disabled.
    if (!scr_crosshair->integer) {
        return;
    }

    // Calculate cross hair position. (Center of screen.)
    vec2_t crosshairPosition = vec2_scale(screenData.hudSize - screenData.crosshairSize, 0.5f);


    clgi.R_SetColor(screenData.crosshairColor.u32);


    clgi.R_DrawStretchPic(crosshairPosition.x + ch_x->integer, 
        crosshairPosition.y + ch_y->integer,
        screenData.crosshairSize.x,
        screenData.crosshairSize.y,
        clgi.R_RegisterPic("crosshairthing"));
}

/**
*   @brief  Draws the 'center strings' on display.
**/
void ClientGameScreen::DrawCenterString() {

}

/**
*   @brief  Draws the player HUD. (Ammo, Health, etc.)
**/
void ClientGameScreen::DrawPlayerHUD() {

}

/**
*   @brief  Draws the FPS value to display.
**/
void ClientGameScreen::DrawFPS() {

}

/**
*   @brief  Draws the chat HUD.
**/
void ClientGameScreen::DrawChatHUD() {

}