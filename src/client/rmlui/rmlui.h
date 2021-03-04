// LICENSE HERE.

//
// client/rmlui/rmlui.h
//
//
// RmlUI implementation.
//
#ifndef __CLIENT_RMLUI_RMLUI_H__
#define __CLIENT_RMLUI_RMLUI_H__

// Initializes RMLUI.
void RMLUI_Init(void);

// Process mouse move events
bool RMLUI_ProcessMouseMove(int x, int y);

// Update RMLUI
void RMLUI_UpdateFrame(void);

// Render RMLUI
void RMLUI_RenderFrame(void);

// Shutdowns RMLUI.
void RMLUI_Shutdown(void);

#endif // __CLIENT_RMLUI_RMLUI_H__