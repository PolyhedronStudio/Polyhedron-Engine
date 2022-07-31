/***
*
*	License here.
*
*	@file
*
*	Client Game Screen Interface Implementation.
* 
***/
// Client Game.
#include "../ClientGameLocals.h"

// Exports Interface Implementations.
#include "../ClientGameExports.h"
#include "Media.h"
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
        clge->media->LoadAndConfigureSky();
        return;
    }

    if (argc > 2) {
        rotate = std::atof(clgi.Cmd_Argv(2));
    } else {
        rotate = 0;
    }

    if (argc == 6) {
        axis[0] = std::atof(clgi.Cmd_Argv(3));
        axis[1] = std::atof(clgi.Cmd_Argv(4));
        axis[2] = std::atof(clgi.Cmd_Argv(5));
    } else {
        axis = vec3_t{0.f, 0.f, 1.f};
    }

    // Adjust sky.
    clgi.R_SetSky(name, rotate, axis);
}
void ClientGameScreen::Cmd_ClearChatHUD_f() {
    clge->screen->chatHUD.Clear();
}

/**
*   @brief  Callback for when the crosshair cvars have changed.
**/
void ClientGameScreen::CVarCrosshairChanged(cvar_t* cvar)
{
    // TODO: Clean up.
    char buffer[MAX_QPATH] = {};
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
    scr_scale               = clgi.Cvar_Get("scr_scale", nullptr, 0);
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
    //// First scale the hud to screen size. (Used for calculating things later on also.)
    screenData.hudSize = { cl->refdef.width, cl->refdef.height};

    // Calculate view rectangle.
    CalculateViewRectangle();
    
    // Calculate HUD size.
    screenData.hudSize *= vec2_t{ screenData.hudScale, screenData.hudScale };

    // Prepare for rendering the HUD.
    clgi.R_SetAlphaScale(screenData.hudAlpha);
    clgi.R_SetScale(screenData.hudScale);
    
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

    // Reset general alpha scale.
    clgi.R_SetAlphaScale(1.0f);
}

/**
*   @brief  Called when the screen mode has changed.
**/
void ClientGameScreen::ScreenModeChanged() {
    // Do special hud scale clamping. (Takes care of handling insane resolutions etc.)
    if (screenData.isInitialized) {
        screenData.hudScale = clgi.R_ClampScale(scr_scale);
    }

    // Reset alpha.
    screenData.hudAlpha = 1.f;
}

/**
*   @brief  Called when the client wants to render the loading screen.
**/
void ClientGameScreen::DrawLoadScreen() {
    // Set scale to hud scale.
    clgi.R_SetScale(screenData.hudScale);

	// Draw text.
	const vec2_t centerPosition = vec2_scale( screenData.hudSize, 0.5 );

	clgi.R_SetColor( U32Colors::Polyhedron );
	const std::string loadStr = "[Loading..]";
	DrawString( loadStr, centerPosition, UI_CENTER, loadStr.length() );
	clgi.R_ClearColor();
    // Reset scale.
    clgi.R_SetScale(1.0f);
}

/**
*   @brief  Called when the client wants to render the pause screen.
**/
void ClientGameScreen::DrawPauseScreen() {
    // Set scale to hud scale.
    clgi.R_SetScale(screenData.hudScale);
	
	// Draw text.
	const vec2_t centerPosition = vec2_scale( screenData.hudSize, 0.5 );

	clgi.R_SetColor( U32Colors::Polyhedron );
	const std::string pauseStr = "[Paused]";
	DrawString( pauseStr, centerPosition, UI_CENTER, pauseStr.length() );
	clgi.R_ClearColor();
    // Reset scale.
    clgi.R_SetScale(1.0f);
}



/***
*
*   Screen Functions.
* 
***/
/**
*   @brief  Draws a string to the screen at the given position, 
*           with an optionable max length.
*   @param  stringLength    Max amount of characters to draw. When 0, text.size() is used instead.
*   @return The advanced x coordinate.
**/
int32_t ClientGameScreen::DrawString(const std::string& text, const vec2_t& position, uint32_t flags, size_t stringLength) {
    // Acquire size.
    stringLength = (stringLength ? stringLength : text.size());

    // Ensure it doesn't exceed our limits.
    if (stringLength > MAX_STRING_CHARS) {
        stringLength = MAX_STRING_CHARS;
    }

    // Calculate position.
    vec2_t stringPosition = position;

    if ((flags & UI_CENTER) == UI_CENTER) {
        stringPosition.x -= stringLength * CHAR_WIDTH / 2;
    } else if (flags & UI_RIGHT) {
        stringPosition.x -= stringLength * CHAR_WIDTH;
    }

    // Draw string and return R_DrawString x advancement of characters.
    return clgi.R_DrawString(stringPosition.x, stringPosition.y, flags, stringLength, text.c_str(), screenData.fontHandle);
}

void ClientGameScreen::DrawMultilineString(const std::string &text, const vec2_t &position, uint32_t flags, size_t maxLength) {
    //char    *p;
    //size_t  len;

    size_t lineLength = text.find_first_of('\n');

    // Current Position of line rendering.
    vec2_t currentPosition = position;

    // Loop till we can't find any newline feeds.
    std::string lineText = text;
    while (lineLength != std::string::npos) {
        lineText = text.substr(lineLength);
        // Render our string.
        DrawString(text.substr(lineLength), position, flags, maxLength);

        // Find next newline feed.
        lineLength = lineText.find('\n', 0);
    }

    // Draw last/first line, depending on whether we ever found a newline feed or not.
    DrawString(lineText, position, flags, maxLength);
}

/**
*   @brief  Utility functions for calculating the alpa fade value based on time.
**/
float ClientGameScreen::FadeAlpha(uint32_t startTime, uint32_t visTime, uint32_t fadeTime) {
    uint32_t deltaTime = clgi.GetRealTime() - startTime;

    if (deltaTime >= visTime) {
        return 0.f;
    }

    if (fadeTime > visTime) {
        fadeTime = visTime;
    }

    float alpha = 1.f;
    uint32_t timeLeft = visTime - deltaTime;
    if (timeLeft < fadeTime) {
        alpha = (float)timeLeft / fadeTime;
    }

    return alpha;
}

/**
*   @brief  Adds('prints'), a line of text to the chat hud.
**/
void ClientGameScreen::ChatPrint(const std::string& text) {
    chatHUD.AddText(text);
}

/**
*   @brief  Adds('prints'), another line of text to the centerprint and resets its timestamp.
**/
void ClientGameScreen::CenterPrint(const std::string& text) {
    // Store timestamp.
    screenData.centerStringTimeStamp = clgi.GetRealTime();

    // Don't process any further, no text to process.
    if (text.empty()) {
        //Com_DPrint("WOOOWWW NO CENTERPRINT TEXTS DAWG!?\n");
        return;
    }

    // Copy text and assign as screenData centerstring.
    screenData.centerString = text;

    // Count the number of lines for centering.
    size_t newlineFeedPos = 0;
    while (newlineFeedPos != std::string::npos) {
        screenData.centerStringLines++;
        newlineFeedPos = text.find('\n', newlineFeedPos + 1);
    }

    // Echo text to console.
    Com_LPrintf(PrintType::Regular, "%s\n", screenData.centerString.c_str());

    // Clear notify.
    clgi.Con_ClearNotify();
}

/**
*   @brief  Register screen media.
**/
void ClientGameScreen::RegisterMedia() {
    //// Pause & Load screen pics.
    //screenData.pausePic     = clgi.R_RegisterPic("pause.png");
    //screenData.loadPic      = clgi.R_RegisterPic("loading.png");

    //// TODO: Adjust R_ functions to use vec2_ts.
    //int32_t x = 0;
    //int32_t y = 0;
    //clgi.R_GetPicSize(&x, &y, screenData.pausePic);
    //screenData.pausePicSize = vec2_t{x, y};

    //clgi.R_GetPicSize(&x, &y, screenData.loadPic);
    //screenData.loadPicSize = vec2_t{x, y};

    // Acquire handle to the font (already loaded by the client itself.)
    screenData.fontHandle = clgi.R_RegisterFont(scr_font->string);

    // Load up number pics, the '-' minus pic and the '/' divisor pic.
    screenData.numberPics[0] = clgi.R_RegisterPic("/pics/hud/num_0.tga");
    screenData.numberPics[1] = clgi.R_RegisterPic("/pics/hud/num_1.tga");
    screenData.numberPics[2] = clgi.R_RegisterPic("/pics/hud/num_2.tga");
    screenData.numberPics[3] = clgi.R_RegisterPic("/pics/hud/num_3.tga");
    screenData.numberPics[4] = clgi.R_RegisterPic("/pics/hud/num_4.tga");
    screenData.numberPics[5] = clgi.R_RegisterPic("/pics/hud/num_5.tga");
    screenData.numberPics[6] = clgi.R_RegisterPic("/pics/hud/num_6.tga");
    screenData.numberPics[7] = clgi.R_RegisterPic("/pics/hud/num_7.tga");
    screenData.numberPics[8] = clgi.R_RegisterPic("/pics/hud/num_8.tga");
    screenData.numberPics[9] = clgi.R_RegisterPic("/pics/hud/num_9.tga");
    screenData.numberDivisorPic = clgi.R_RegisterPic("/pics/hud/num_div.tga");
    screenData.numberMinusPic   = clgi.R_RegisterPic("/pics/hud/num_min.tga");

    // Ensure crosshair gets loaded.
    CVarCrosshairChanged(scr_crosshair);
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
    // Return if crosshair rendering has been disabled, or we got no valid pic handle.
    if (!scr_crosshair->integer || !screenData.crosshairPic) {
        return;
    }

    // Calculate cross hair position. (Center of screen.)
    vec2_t crosshairPosition = vec2_scale(screenData.hudSize - screenData.crosshairSize, 0.5f);

    // Set crosshair color.
    clgi.R_SetScale(screenData.hudScale);
    clgi.R_SetColor(screenData.crosshairColor.u32);

    // Render it.
    clgi.R_DrawStretchPic(crosshairPosition.x + ch_x->integer, 
        crosshairPosition.y + ch_y->integer,
        screenData.crosshairSize.x,
        screenData.crosshairSize.y,
        screenData.crosshairPic);

    // Reset scale.
    clgi.R_SetScale(1.0f);
}

/**
*   @brief  Draws the 'center strings' on display.
**/
void ClientGameScreen::DrawCenterString() {
    clgi.Cvar_ClampValue(scr_centertime, 0.3f, 10.0f);

    float alpha = FadeAlpha(screenData.centerStringTimeStamp, scr_centertime->value * 1000, 300);
    if (!alpha) {
        return;
    }
    
    // Set scale to hud scale.
    clgi.R_SetAlpha(alpha * scr_alpha->value);

    float y = screenData.hudSize.y / 4 - screenData.centerStringLines * 8 / 2;

    DrawMultilineString(screenData.centerString, {screenData.hudSize.x / 2, y}, UI_CENTER);

    clgi.R_SetAlpha(scr_alpha->value);
}

//                // ammo number
//            int     color;
//#include "clg_local.h"
//
//#include "clg_main.h"
//#include "clg_media.h"
//#include "clg_screen.h"
//            width = 3;
//            value = cl->frame.playerState.stats[STAT_AMMO];
//            if (value > 5)
//                color = 0;  // green
//            else if (value >= 0)
//                color = ((cl->frame.number / CL_FRAMEDIV) >> 2) & 1;     // flash
//            else
//                continue;   // negative number = don't show
//
//            if (cl->frame.playerState.stats[STAT_FLASHES] & 4)
//                clgi.R_DrawPic(x, y, scr.field_pic);
//
//            HUD_DrawNumber(x, y, color, width, value);
//            continue;

/**
*   @brief  Draws the player HUD. (Ammo, Health, etc.)
**/
void ClientGameScreen::DrawPlayerHUD() {
    // Offset for primary ammo display. (We render from right to left.)
    vec2_t primaryOffset = { -112.f * primaryAmmo.GetScale(), -144* primaryAmmo.GetScale()};

    // Set alpha.
    clgi.R_SetScale(screenData.hudScale); // divide by 2?? hmm...
    clgi.R_SetAlpha(screenData.hudAlpha);

    /**
    *   Clip Ammo & Primary Ammo Display.
    **/
    primaryAmmo.SetColor(NumberHUD::DefaultColor, NumberHUD::PrimaryNumber);
    primaryAmmo.SetColor(NumberHUD::DefaultColor, NumberHUD::SecondaryNumber);

    // Set values.
    primaryAmmo.SetPrimaryNumber(cl->frame.playerState.stats[PlayerStats::ClipAmmo]);
    primaryAmmo.SetSecondaryNumber(cl->frame.playerState.stats[PlayerStats::PrimaryAmmo]);

    // Draw our primary ammo inventory display to screen.
    primaryAmmo.Draw(screenData.hudSize + primaryOffset);
    
    // Reset to defaults.
    clgi.R_SetScale(1.0f);
    clgi.R_SetAlpha(1.0f);
}

/**
*   @brief  Draws the FPS value to display.
**/
void ClientGameScreen::DrawFPS() {
    if (!scr_fps->integer) {
        return;
    }
    // Set scale to hud scale.
    clgi.R_SetScale(screenData.hudScale);

    int32_t fps = clgi.GetFramesPerSecond();
    int32_t scale = clgi.GetResolutionScale();

    char buffer[MAX_QPATH];
    if (scr_fps->integer == 2 && vid_rtx->integer) {
        Q_snprintf(buffer, MAX_QPATH, "%d FPS at %3d%%", fps, scale);
    } else {
        Q_snprintf(buffer, MAX_QPATH, "%d FPS", fps);
    }

    //clgi.R_SetColor(~0u);
	if ( fps < 30 ) {
		clgi.R_SetColor( U32Colors::Red );
	} else if (fps < 60) {
		clgi.R_SetColor( U32Colors::Orange );
	} else if (fps < 120) {
		clgi.R_SetColor( U32Colors::Yellow );
	} else {
		clgi.R_SetColor( U32Colors::Polyhedron );
	}
    DrawString(buffer, {screenData.hudSize.x - 2.f, 1.f}, UI_RIGHT);

    clgi.R_SetScale(1.f);
}

/**
*   @brief  Draws the chat HUD.
**/
void ClientGameScreen::DrawChatHUD() {
    chatHUD.Draw();
}