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



//---------------------------------------------------------
#define ICON_WIDTH  24
#define ICON_HEIGHT 24
#define DIGIT_WIDTH 16
#define ICON_SPACE  8

static void HUD_DrawNumber(int x, int y, int color, int width, int value)
{
    char    num[16], * ptr;
    int     l;
    int     frame;

    if (width < 1)
        return;

    // draw number string
    if (width > 5)
        width = 5;

    color &= 1;

    l = Q_scnprintf(num, sizeof(num), "%i", value);
    if (l > width)
        l = width;
    x += 2 + DIGIT_WIDTH * (width - l);

    ptr = num;
    while (*ptr && l) {
        if (*ptr == '-')
            frame = STAT_MINUS;
        else
            frame = *ptr - '0';

        clgi.R_DrawPic(x, y, scr.sb_pics[color][frame]);
        x += DIGIT_WIDTH;
        ptr++;
        l--;
    }
}

static void SRC_DrawNewHud_PrimaryAmmo() {
    uint32_t x = scr.hud_width - 288;
    uint32_t y = scr.hud_height - 160;

    uint32_t width = 3;
    int32_t value = cl->frame.playerState.stats[PlayerStats::PrimaryAmmo];
    int32_t color = 0;
    if (value > 5)
        color = 0;  // green
    else if (value >= 0)
        color = ((cl->frame.number / CLG_FRAMEDIV) >> 2) & 1;     // flash
    else
        return;

    if (cl->frame.playerState.stats[PlayerStats::Flashes] & 4)
        clgi.R_DrawPic(x, y, scr.field_pic);

    HUD_DrawNumber(x, y, color, width, value);
}

void SRC_DrawNewHud() {
        clgi.R_SetScale(scr.hud_scale);
    clgi.R_DrawPic(scr.hud_width - 128, scr.hud_height - 32, clgi.R_RegisterPic("num_rightslash"));
    HUD_DrawNumber(scr.hud_width - 96, scr.hud_height - 32, 0, 3, cl->frame.playerState.stats[PlayerStats::PrimaryAmmo]);
    HUD_DrawNumber(scr.hud_width - 128, scr.hud_height - 32, 0, 3, cl->frame.playerState.stats[PlayerStats::PrimaryAmmo]);
}
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

    // Draw new stats lmao, we got a lot of cleaning up to do for the time being :-P
    SRC_DrawNewHud();

    // Draw status.
    //SCR_DrawStats();

    // Draw layout string.
    //SCR_DrawLayout();

    // Draw inventory.
    //SCR_DrawInventory();

    // Draw center screen print
    SCR_DrawCenterString();

    // Draw objects.
    //SCR_DrawObjects();

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
    if (scr.initialized) {
        scr.hud_scale = clgi.R_ClampScale(scr_scale);
    }

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