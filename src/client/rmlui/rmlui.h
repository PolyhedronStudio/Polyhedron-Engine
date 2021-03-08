// LICENSE HERE.

//
// client/rmlui/rmlui.h
//
//
// RmlUI implementation.
//
#ifndef __CLIENT_RMLUI_RMLUI_H__
#define __CLIENT_RMLUI_RMLUI_H__

// Required include.
#include <SDL.h>

// Include shared headers.
#include "shared/shared.h"

// Initializes RMLUI.
void RMLUI_Init(void);

// Process keyboard events.
bool RMLUI_ProcessKeyDown(SDL_Keycode key);
bool RMLUI_ProcessKeyUp(SDL_Keycode key);

// Process text events.
bool RMLUI_ProcessTextInput(const char* text);

// Process mouse events
bool RMLUI_ProcessMouseMove(int x, int y);
bool RMLUI_ProcessMouseWheel(float delta);
bool RMLUI_ProcessMouseButtonUp(int button);
bool RMLUI_ProcessMouseButtonDown(int button);

// Update RMLUI
void RMLUI_UpdateFrame(void);

// Render RMLUI
void RMLUI_RenderFrame(void);

// Shutdowns RMLUI.
void RMLUI_Shutdown(void);
#endif // __CLIENT_RMLUI_RMLUI_H__