/***
*
*	License here.
*
*	@file
*
*	Client Game Screen Interface Implementation.
* 
***/
#pragma once

// Game Screen HUD Utility Objects.
#include "../HUD/ChatHUD.h"
#include "../HUD/NumberHUD.h"

/**
*   @brief  Client Game Module implementation of the Screen interface.
* 
*           Used for drawing 2D elements such as the player stats and the
*           chat HUD.
**/
class ClientGameScreen : public IClientGameExportScreen {
public:
    friend class ChatHUD;

    //! Destructor.
    virtual ~ClientGameScreen() = default;

    /***
    *
    * 
    *   Interface Implementation.
    * 
    * 
    ***/
    /**
    *   @brief  Initialize screen related matter.
    **/
    void Initialize() final;
    /**
    *   @brief  Called when the client starts or when the renderer demands so
    *           after having modified its settings. Used to register basic 
    *           screen media, 2D icons etc.
    **/
    void RegisterMedia() final;
    /**
    *   @brief  Take care of undo-ing all screen related matter.
    **/
    void Shutdown() final;

    /**
    *   @brief  Called when the engine needs to render the 2D display.
    **/
    void RenderScreen() final;
    /**
    *   @brief  Called when the screen mode has changed.
    **/
    void ScreenModeChanged() final;
    /**
    *   @brief  Called when the client wants to render the loading screen.
    **/
    void DrawLoadScreen() final;
    /**
    *   @brief  Called when the client wants to render the pause screen.
    **/
    void DrawPauseScreen() final;

public:
    /***
    *
    *   Screen Functionality.
    * 
    ***/
    /**
    *   @brief  Contains all related screen data.
    **/
    struct ScreenData {
        //! If initialized, we're ready to draw.
        qboolean isInitialized = false;
        
        //! View Rectangle.
        vec4_t  viewRectangle = { 0, 0, 0, 0 };

        // ! Size of the HUD.
        vec2_t  hudSize     = { 0,0 };
        float   hudScale    = 0.f;
        float   hudAlpha    = 0.f;

        //! Handle to the font used for rendering 2D text elements. (Already loaded by the client, just need its handle.)
        qhandle_t   fontHandle      = 0;

        //! Handle to the image used for pause display.
        qhandle_t   pausePic        = 0;
        //! Size of pause display image.
        vec2_t      pausePicSize    = { 0, 0 };

        //! Handle to the image used for pause display.
        qhandle_t   loadPic     = 0;
        //! Size of pause display image.
        vec2_t      loadPicSize = { 0, 0 };

        //! Handle to the crosshair picture in use.
        qhandle_t   crosshairPic    = 0;
        vec2_t      crosshairSize   = { 0, 0 };
        color_t     crosshairColor  = { 
            .u32 = MakeColor(255, 255, 255, 255) // White's default.
        };

        //! Center String management.
        std::string centerString = "";
        uint32_t centerStringTimeStamp = 0;
        uint32_t centerStringLines = 0;

        //! Handles to number pics for NumberHUD elements.
        qhandle_t numberPics[10] = {};
        qhandle_t numberMinusPic = 0;   //! Handle to the '-' minus pic.
        qhandle_t numberDivisorPic = 0; //! Handle to the '/' divisor pic.
    } screenData;

    /**
    *   @brief  Draws a string to the screen at the given position, 
    *           with an optionable max length.
    *   @param  stringLength    Max amount of characters to draw. When 0, text.size() is used instead.
    *   @return The advanced x coordinate.
    **/
    int32_t DrawString(const std::string &text, const vec2_t &position, uint32_t flags = 0, size_t stringLength = 0);

    /**
    *   @brief  Draws a string supporting '\n' multi-line characters to the screen 
    *           at given position.
    **/
    void DrawMultilineString(const std::string &text, const vec2_t &position, uint32_t flags = 0, size_t maxLength = 0);

    /**
    *   @brief  Utility functions for calculating the alpa fade value based on time.
    **/
    float FadeAlpha(uint32_t  startTime, uint32_t visTime, uint32_t fadeTime);

    /**
    *   @brief  Adds('prints'), a line of text to the chat hud.
    **/
    void ChatPrint(const std::string &text);

    /**
    *   @brief  Adds('prints'), another line of text to the centerprint and resets its timestamp.
    **/
    void CenterPrint(const std::string &text);

private:
    /**
    *   @brief  Calculate the screen's view rectangle.
    **/
    void CalculateViewRectangle();

    /**
    *   @brief  Draws the crosshair.
    **/
    void DrawCrosshair();

    /**
    *   @brief  Draws the 'center strings' on display.
    **/
    void DrawCenterString();

    /**
    *   @brief  Draws the player HUD. (Ammo, Health, etc.)
    **/
    void DrawPlayerHUD();

    /**
    *   @brief  Draws the FPS value to display.
    **/
    void DrawFPS();

    /**
    *   @brief  Draws the chat HUD.
    **/
    void DrawChatHUD();


public:
    /***
    *
    *   Cmd/CVar callbacks.
    * 
    ***/
    //! Console command implementations.
    static void Cmd_Sky_f();
    static void Cmd_ClearChatHUD_f();

    //! Console commands.
    const cmdreg_t screenCommands[5] = {
        //{ "timerefresh", SCR_TimeRefresh_f },
        //{ "sizeup", SCR_SizeUp_f },
        //{ "sizedown", SCR_SizeDown_f },
        { "sky", Cmd_Sky_f },
        { "clearchathud", Cmd_ClearChatHUD_f },
        { NULL }
    };

private:
    //! Stack based HUD objects. (Small.)
    ChatHUD chatHUD{ this }; //! Handles the chat display.

    NumberHUD clipAmmo{ this }; //! For displaying the amount of the weapon's clip ammo.
    NumberHUD primaryAmmo{ this }; //! For displaying the inventarized amount of the weapon's primary ammo type.
    NumberHUD secondaryAmmo{ this }; //! For displaying the inventarized amount of the weapon's secondary ammo type.

    //! Set on several cvars related to the crosshair display.
    static void CVarCrosshairChanged(cvar_t *cvar);
    //! Set on scr_scale cvar.
    static void CVarScreenScaleChanged(cvar_t* cvar);


    /***
    *
    *   CVars.
    * 
    ***/
    // Screen Settings.
    cvar_t* scr_viewsize;       //! Determines the views scale. Probably a bad name though.
    cvar_t* scr_fps;            //! Enables display for the FPS.
    cvar_t* scr_showitemname;   //! Enables showing an item name or not.
    cvar_t* scr_draw2d;         //! Enables 2D Screen rendering. (Heads up display, etc.)
    cvar_t* scr_alpha;          //! Alpha value to render 2D screen elements with.
    cvar_t* scr_font;           //! Determines the font to use for rendering 2D texts.
    cvar_t* scr_scale;          //! Determines the scale for the 2D screen elements.
    cvar_t* scr_centertime;     //! Controls for how long a center print stays on screen.
    cvar_t* scr_crosshair;      //! Enables the crosshair.

    // Chat hud display settings.
    cvar_t* scr_chathud;
    cvar_t* scr_chathud_lines;
    cvar_t* scr_chathud_time;
    cvar_t* scr_chathud_x;
    cvar_t* scr_chathud_y;

    // Crosshair display settings.
    cvar_t* ch_scale;
    cvar_t* ch_x;
    cvar_t* ch_y;
    cvar_t* ch_red;
    cvar_t* ch_green;
    cvar_t* ch_blue;
    cvar_t* ch_alpha;
};

