// LICENSE HERE.

//
// client/rmlui/interfaces/SystemInterface.cpp
//
//
// RmlUI N&C System Interface implementation.
//
#include "../rmlui.h"
#include "SystemInterface.h"

/// Get the number of seconds elapsed since the start of the application
/// @returns Seconds elapsed
double RmlUISystemInterface::GetElapsedTime() {
	return 0.f;
}

/// Set mouse cursor.
/// @param[in] cursor_name Cursor name to activate.
void RmlUISystemInterface::SetMouseCursor(const Rml::String& cursor_name) {

}

/// Set clipboard text.
/// @param[in] text Text to apply to clipboard.
void RmlUISystemInterface::SetClipboardText(const Rml::String& text) {

}

/// Get clipboard text.
/// @param[out] text Retrieved text from clipboard.
void RmlUISystemInterface::GetClipboardText(Rml::String& text) {

}