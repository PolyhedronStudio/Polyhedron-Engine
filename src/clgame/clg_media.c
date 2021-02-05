// LICENSE HERE.

//
// clg_media.c
//
//
// Media load handling, usually happens when the renderer initializes, or
// restarts (think about changing screen mode, or other settings).
//
#include "clg_local.h"

//
//=============================================================================
//
// CLIENT MODULE MEDIA ENTRY FUNCTIONS.
//
//=============================================================================

//
//===============
// CLG_InitMedia
// 
// This is called upon every time the renderer initializes, or does a total
// hard restart.
//
// Use this to load in persistent data, such as 2D pics. Or use it to
// register certain CVars related to.
//===============
//
void CLG_InitMedia(void)
{
    Com_DPrint("[CG Module Callback] - %s\n", __func__);
    
    // Initialize View Data.
    V_Init();
}

//
//===============
// CLG_RegisterMedia
// 
// This is called when the client starts, but also when the renderer has had
// modified settings.
//
// It should register the basic screen media, 2D icons etc.
//===============
//
void CLG_RegisterMedia(void)
{
    Com_DPrint("[CG Module Callback] - %s\n", __func__);
}

//
//===============
// CLG_ShutdownMedia
// 
// This is called when the client stops the renderer.
// Use this to unload remaining data.
//===============
//
void CLG_ShutdownMedia (void) {
    Com_DPrint("[CG Module Callback] - %s\n", __func__);

    // Shutdown View Data.
    V_Shutdown();
}