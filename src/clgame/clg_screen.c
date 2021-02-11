// LICENSE HERE.

//
// clg_screen.c
//
//
// Client Screen Implementation.
//
#include "clg_local.h"

//
//=============================================================================
//
// CLIENT MODULE SCREEN COMMAND FUNCTIONS.
//
//=============================================================================
// 
/*
=================
SCR_Sky_f

Set a specific sky and rotation speed. If empty sky name is provided, falls
back to server defaults.
=================
*/
static void SCR_Sky_f(void)
{
    char* name;
    float   rotate;
    vec3_t  axis;
    int     argc = clgi.Cmd_Argc();

    if (argc < 2) { 
        Com_Print("Usage: sky <basename> [rotate] [axis x y z]\n");
        return;
    }

    if (clgi.GetClienState() != ca_active) {
        Com_Print("No map loaded.\n");
        return;
    }

    name = clgi.Cmd_Argv(1);
    if (!*name) {
        CLG_SetSky();
        return;
    }

    if (argc > 2)
        rotate = atof(clgi.Cmd_Argv(2));
    else
        rotate = 0;

    if (argc == 6) {
        axis[0] = atof(clgi.Cmd_Argv(3));
        axis[1] = atof(clgi.Cmd_Argv(4));
        axis[2] = atof(clgi.Cmd_Argv(5));
    }
    else
        VectorSet(axis, 0, 0, 1);

    clgi.R_SetSky(name, rotate, axis);
}

//
//=============================================================================
//
// CLIENT MODULE SCREEN ENTRY FUNCTIONS.
//
//=============================================================================
//
static const cmdreg_t scr_cmds[] = {
    //{ "timerefresh", SCR_TimeRefresh_f },
    //{ "sizeup", SCR_SizeUp_f },
    //{ "sizedown", SCR_SizeDown_f },
    { "sky", SCR_Sky_f },
    //{ "draw", SCR_Draw_f, SCR_Draw_c },
    //{ "undraw", SCR_UnDraw_f, SCR_UnDraw_c },
    //{ "clearchathud", SCR_ClearChatHUD_f },
    { NULL }
};

//
//===============
// CLG_SCR_Init
// 
// 
//================
//
void SCR_Init(void)
{
//    scr_viewsize = Cvar_Get("viewsize", "100", CVAR_ARCHIVE);
//    scr_showpause = Cvar_Get("scr_showpause", "1", 0);
//    scr_centertime = Cvar_Get("scr_centertime", "2.5", 0);
//#ifdef _DEBUG
//    scr_netgraph = Cvar_Get("netgraph", "0", 0);
//    scr_timegraph = Cvar_Get("timegraph", "0", 0);
//    scr_debuggraph = Cvar_Get("debuggraph", "0", 0);
//    scr_graphheight = Cvar_Get("graphheight", "32", 0);
//    scr_graphscale = Cvar_Get("graphscale", "1", 0);
//    scr_graphshift = Cvar_Get("graphshift", "0", 0);
//#endif
//    scr_demobar = Cvar_Get("scr_demobar", "1", 0);
//    scr_font = Cvar_Get("scr_font", "conchars", 0);
//    scr_font->changed = scr_font_changed;
//    scr_scale = Cvar_Get("scr_scale", "2", 0);
//    scr_scale->changed = scr_scale_changed;
//    scr_crosshair = Cvar_Get("crosshair", "0", CVAR_ARCHIVE);
//    scr_crosshair->changed = scr_crosshair_changed;
//
//    scr_chathud = Cvar_Get("scr_chathud", "0", 0);
//    scr_chathud_lines = Cvar_Get("scr_chathud_lines", "4", 0);
//    scr_chathud_time = Cvar_Get("scr_chathud_time", "0", 0);
//    scr_chathud_x = Cvar_Get("scr_chathud_x", "8", 0);
//    scr_chathud_y = Cvar_Get("scr_chathud_y", "-64", 0);
//
//    ch_health = Cvar_Get("ch_health", "0", 0);
//    ch_health->changed = scr_crosshair_changed;
//    ch_red = Cvar_Get("ch_red", "1", 0);
//    ch_red->changed = scr_crosshair_changed;
//    ch_green = Cvar_Get("ch_green", "1", 0);
//    ch_green->changed = scr_crosshair_changed;
//    ch_blue = Cvar_Get("ch_blue", "1", 0);
//    ch_blue->changed = scr_crosshair_changed;
//    ch_alpha = Cvar_Get("ch_alpha", "1", 0);
//    ch_alpha->changed = scr_crosshair_changed;
//
//    ch_scale = Cvar_Get("ch_scale", "1", 0);
//    ch_scale->changed = scr_crosshair_changed;
//    ch_x = Cvar_Get("ch_x", "0", 0);
//    ch_y = Cvar_Get("ch_y", "0", 0);
//
//    scr_draw2d = Cvar_Get("scr_draw2d", "2", 0);
//    scr_showturtle = Cvar_Get("scr_showturtle", "1", 0);
//    scr_showitemname = Cvar_Get("scr_showitemname", "1", CVAR_ARCHIVE);
//    scr_lag_x = Cvar_Get("scr_lag_x", "-1", 0);
//    scr_lag_y = Cvar_Get("scr_lag_y", "-1", 0);
//    scr_lag_draw = Cvar_Get("scr_lag_draw", "0", 0);
//    scr_lag_min = Cvar_Get("scr_lag_min", "0", 0);
//    scr_lag_max = Cvar_Get("scr_lag_max", "200", 0);
//    scr_alpha = Cvar_Get("scr_alpha", "1", 0);
//    scr_fps = Cvar_Get("scr_fps", "0", CVAR_ARCHIVE);
//#ifdef _DEBUG
//    scr_showstats = Cvar_Get("scr_showstats", "0", 0);
//    scr_showpmove = Cvar_Get("scr_showpmove", "0", 0);
//#endif

    clgi.Cmd_Register(scr_cmds);

    //scr_scale_changed(scr_scale);

    //scr.initialized = qtrue;
}

//
//===============
// CLG_SCR_Init
// 
// 
//================
//
void SCR_Shutdown(void)
{
    clgi.Cmd_Deregister(scr_cmds);
//    scr.initialized = qfalse;
}
