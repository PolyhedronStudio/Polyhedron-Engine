// LICENSE HERE.

//
// client/rmlui/rmlui.cpp
//
//
// RmlUI Wrapper API that is client game friendly if needed.
//

// Client includes.
#include "../Client.h"
#include "Client/Sound/Vorbis.h"
#include "Client/GameModule.h"

// RmlUI includes.
#include "RmlUI.h"

//
//=============================================================================
// RMLUI_Init
// 
// Initializes the RMLUI library.
//=============================================================================
//
void RMLUI_Init(void) {

}

//
//=============================================================================
// RMLUI_ProcessKeyDown
// 
// True if the event was not consumed, false if it was.
//=============================================================================
//
bool RMLUI_ProcessKeyDown(SDL_Keycode key) {
	return true;
}

//
//=============================================================================
// RMLUI_ProcessKeyUp
// 
// True if the event was not consumed, false if it was.
//=============================================================================
//
bool RMLUI_ProcessKeyUp(SDL_Keycode key) {
	return true;
}

//
//=============================================================================
// RMLUI_ProcessTextInput
// 
// 
//=============================================================================
//
bool RMLUI_ProcessTextInput(const char* text) {
	return true;
}

//
//=============================================================================
// RMLUI_ProcessMouseMove
// 
// True if the mouse is not interacting with any elements in the context 
// (see 'IsMouseInteracting'), otherwise false.
//=============================================================================
//
bool RMLUI_ProcessMouseMove(int x, int y) {
	return true;
}

//
//=============================================================================
// RMLUI_ProcessMouseWheel
// 
// True if the event was not consumed (ie, was prevented from propagating by an element), false if it was.
//=============================================================================
//
bool RMLUI_ProcessMouseWheel(float delta) {
	return true;
}

//
//=============================================================================
// RMLUI_ProcessMouseButtonUp
// 
// True if the mouse is not interacting with any elements in the context 
// (see 'IsMouseInteracting'), otherwise false.
//=============================================================================
//
bool RMLUI_ProcessMouseButtonUp(int button) {
	return true;
}

//
//=============================================================================
// RMLUI_ProcessMouseButtonDown
// 
// True if the mouse is not interacting with any elements in the context 
// (see 'IsMouseInteracting'), otherwise false.
//=============================================================================
//
bool RMLUI_ProcessMouseButtonDown(int button) {
	return true;
}

// Render RMLUI
void RMLUI_UpdateFrame(void) {

}

// Render RMLUI
void RMLUI_RenderFrame(void) {

}

// Shutdowns RMLUI.
void RMLUI_Shutdown(void) {

}