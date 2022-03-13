// Client Game.
#include "../ClientGameLocal.h"
#include "../Main.h"
#include "../Media.h"

// Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"

// Exports Interface Implementations.
#include "../ClientGameExports.h"
#include "Screen.h"

// HUD Elements.
#include "../HUD/ChatHUD.h"

/***
*
*   Cmd/CVar callbacks.
* 
***/
void ClientGameScreen::Cmd_Sky_f() {
    const char* name; // C++20: STRING: Added const to char*
    float   rotate;
    vec3_t  axis;
    int     argc = clgi.Cmd_Argc();

    if (argc < 2) { 
        Com_Print("Usage: sky <basename> [rotate] [axis x y z]\n");
        return;
    }

    if (clgi.GetClienState() != ClientConnectionState::Active) {
        Com_Print("No map loaded.\n");
        return;
    }

    name = clgi.Cmd_Argv(1);
    if (!*name) {
        CLG_SetSky();
        return;
    }

    if (argc > 2)
        rotate = std::atof(clgi.Cmd_Argv(2));
    else
        rotate = 0;

    if (argc == 6) {
        axis[0] = std::atof(clgi.Cmd_Argv(3));
        axis[1] = std::atof(clgi.Cmd_Argv(4));
        axis[2] = std::atof(clgi.Cmd_Argv(5));
    } else {
        axis = vec3_t{0.f, 0.f, 1.f};
    }

    clgi.R_SetSky(name, rotate, axis);
}
void ClientGameScreen::Cmd_ClearChatHUD_f() {

}

/**
*   @brief  Callback for when the crosshair cvars have changed.
**/
void ClientGameScreen::CVarCrosshairChanged(cvar_t* cvar)
{
    // TODO: Clean up.
    char buffer[16] = {};
    int32_t w, h;
    float scale = 0.f;

    // Get pointer to ClientGameScreen
    ClientGameScreen *screen = clge->screen;

    if (cvar->integer > 0) {
        // TODO: Let this actually... load a crosshair
        Q_snprintf(buffer, sizeof(buffer), "/pics/crosshairs/%i.tga", cvar->integer);
        screen->screenData.crosshairPic = clgi.R_RegisterPic(buffer);
        if (!screen->screenData.crosshairPic) {
            screen->screenData.crosshairPic = clgi.R_RegisterPic("/pics/crosshairs/0.tga");
        }
        clgi.R_GetPicSize(&w, &h, screen->screenData.crosshairPic);

        // prescale
        scale = clgi.Cvar_ClampValue(clgi.Cvar_Get("ch_scale", nullptr, 0), 0.1f, 9.0f);
        screen->screenData.crosshairSize.x = w * scale;
        screen->screenData.crosshairSize.y = h * scale;

        if (screen->screenData.crosshairSize.x < 1) {
            screen->screenData.crosshairSize.x = 1;
        }
        if (screen->screenData.crosshairSize.y < 1) {
            screen->screenData.crosshairSize.y = 1;
        }

        // Convert and assign cvar crosshair color values.
        cvar_t *ch_red = clgi.Cvar_Get("ch_red", nullptr, 0);
        if (ch_red) {
            screen->screenData.crosshairColor.u8[0] = (byte)(ch_red->value * 255);
        }
        cvar_t *ch_green = clgi.Cvar_Get("ch_green", nullptr, 0);
        if (ch_green) {
            screen->screenData.crosshairColor.u8[1] = (byte)(ch_green->value * 255);
        }
        cvar_t *ch_blue = clgi.Cvar_Get("ch_blue", nullptr, 0);
        if (ch_blue) {
            screen->screenData.crosshairColor.u8[2] = (byte)(ch_blue->value * 255);
        }
        cvar_t *ch_alpha = clgi.Cvar_Get("ch_alpha", nullptr, 0);
        if (ch_alpha) {
            screen->screenData.crosshairColor.u8[3] = (byte)(ch_alpha->value * 255);
        }
    } else {
        screen->screenData.crosshairPic = 0;
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
    scr_viewsize    = clgi.Cvar_Get("viewsize", nullptr, 0);
    scr_draw2d      = clgi.Cvar_Get("scr_draw2d", nullptr, 0);
    scr_alpha       = clgi.Cvar_Get("scr_alpha", nullptr, 0);
    scr_font        = clgi.Cvar_Get("scr_font", nullptr, 0);
    scr_fps         = clgi.Cvar_Get("scr_fps", nullptr, 0);

    // Create CVars.
    scr_scale               = clgi.Cvar_Get("scr_scale", "1", 0);
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

    // Allocate our HUD objects.
    chatHUD = new ChatHUD(this);

    // We've initialized the screen.
    screenData.isInitialized = true;
}

void ClientGameScreen::Shutdown() {
    // Unregister screen console commands.
    clgi.Cmd_Unregister(screenCommands);

    // Delete HUD elements.
    delete chatHUD;
    chatHUD = nullptr;

    // Unset isInitialized.
    screenData.isInitialized = false;
}

/**
*   @brief  Called when the engine needs to render the 2D display.
**/
void ClientGameScreen::RenderScreen() {
    //// First scale the hud to screen size. (Used for calculating things later on also.)
    //screenData.hudSize = { cl->refdef.width, cl->refdef.height};

    //// Calculate view rectangle.
    //CalculateViewRectangle();

    //// Prepare for rendering the HUD.
    //clgi.R_SetAlphaScale(screenData.hudAlpha);
    //clgi.R_SetScale(screenData.hudScale);
    //
    //// Calculate HUD size.
    //screenData.hudSize *= vec2_t{ screenData.hudScale, screenData.hudScale };

    //// Draw the crosshair.
    //DrawCrosshair();

    //// The rest of 2D elements share the common scr_alpha cvar value.
    //clgi.R_ClearColor();
    //clgi.R_SetAlpha(clgi.Cvar_ClampValue(scr_alpha, 0, 1));
    //
    //// Draw the HUD.
    //DrawPlayerHUD();

    //// Draw center screen print
    //DrawCenterString();

    //// Draw FPS.
    //DrawFPS();

    //// Draw Chat Hud.
    //DrawChatHUD();
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
    clgi.R_SetScale(screenData.hudScale);

    vec2_t loadPicPosition = vec2_scale(
        {
            cl->refdef.width * screenData.hudScale - screenData.loadPicSize.x,
            cl->refdef.height * screenData.hudScale - screenData.loadPicSize.y
        },
        0.5f
    );
    if (screenData.loadPic) {
        clgi.R_DrawPic(loadPicPosition.x, loadPicPosition.y, screenData.loadPic);
    }
    clgi.R_SetScale(1.0f);
}

/**
*   @brief  Called when the client wants to render the pause screen.
**/
void ClientGameScreen::DrawPauseScreen() {
    clgi.R_SetScale(screenData.hudScale);
    vec2_t pausePicPosition = vec2_scale(screenData.hudSize - screenData.pausePicSize, 0.5f);

    if (screenData.pausePic) {
        clgi.R_DrawPic(pausePicPosition.x, pausePicPosition.y, screenData.pausePic);
    }
    clgi.R_SetScale(1.0f);
}



/***
*
*   Screen Functions.
* 
***/
/**
*   @brief  Draws a string to the screen at the given position.
*   @return The advanced x coordinate.
**/
int32_t ClientGameScreen::DrawString(const std::string& text, const vec2_t& position, uint32_t flags) {
    // Acquire size.
    size_t stringLength = text.size();

    // Ensure it doesn't exceed our limits.
    if (stringLength > MAX_STRING_CHARS) {
        stringLength = MAX_STRING_CHARS;
    }

    // Calculate position.
    vec2_t stringPosition = position;

    if ((flags & UI_CENTER) == UI_CENTER) {
        stringPosition.x -= stringLength * CHAR_WIDTH / 2;
    }
    else if (flags & UI_RIGHT) {
        stringPosition.x -= stringLength * CHAR_WIDTH;
    }

    return clgi.R_DrawString(stringPosition.x, stringPosition.y, flags, stringLength, text.c_str(), screenData.fontHandle);
}

/**
*   @brief  Register screen media.
**/
void ClientGameScreen::RegisterMedia() {
    // Load in default crosshair picture.
    screenData.crosshairPic = clgi.R_RegisterPic("/pics/crosshairs/0.tga");
    int32_t x = 0; 
    int32_t y = 0;
    clgi.R_GetPicSize(&x, &y, screenData.crosshairPic);
    screenData.crosshairSize = vec2_t{x, y};

    // Pause & Load screen pics.
    screenData.pausePic     = clgi.R_RegisterPic("pause.png");
    screenData.loadPic      = clgi.R_RegisterPic("loading.png");

    // TODO: Adjust R_ functions to use vec2_ts.
    clgi.R_GetPicSize(&x, &y, screenData.pausePic);
    screenData.pausePicSize = vec2_t{x, y};

    clgi.R_GetPicSize(&x, &y, screenData.loadPic);
    screenData.loadPicSize = vec2_t{x, y};

    // Acquire handle to the font (already loaded by the client itself.)
    screenData.fontHandle = clgi.R_RegisterFont("conchars");
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

    // Set crosshair color.
    clgi.R_SetScale(1.f);
    clgi.R_SetColor(screenData.crosshairColor.u32);

    // Render it.
    clgi.R_DrawStretchPic(crosshairPosition.x + ch_x->integer, 
        crosshairPosition.y + ch_y->integer,
        32,
        32,
        clgi.R_RegisterPic("/pics/crosshairs/0.tga"));
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