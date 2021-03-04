// LICENSE HERE.

//
// client/rmlui/interfaces/SystemInterface.cpp
//
//
// RmlUI N&C System Interface implementation.
//
// Declare static library linkage.
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

// Client includes.
#include "../../client.h"
#include "client/sound/vorbis.h"
#include "client/gamemodule.h"

#include "../rmlui.h"
#include "SystemInterface.h"

/// Get the number of seconds elapsed since the start of the application
/// @returns Seconds elapsed
double RmlUISystemInterface::GetElapsedTime() {
	// Cheap hack for now.
	static double elapsedTime = 0.0f;
	if (elapsedTime < cls.frametime)
		elapsedTime += cls.frametime;

	return elapsedTime;
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

//// Translate the input string into the translated string.
//int RmlUISystemInterface::TranslateString(Rml::String& translated, const Rml::String& input) {
//	return 0;
//}
//
//The LogMessage() function is called when RmlUi generates a message.
// Here, type is one of Rml::Log::ERROR for error messages, 
// Rml::Log::ASSERT for failed internal assertions(debug library only), 
// Rml::Log::WARNING for non - fatal warnings, or 
// Rml::Log::INFO for generic information messages.
// The message parameter is the actual message itself. The function should return true if program execution should continue, 
// or false to generate an interrupt to break execution.
// This can be useful if you are running inside a debugger to see exactly what an application is doing to trigger a certain message.
bool RmlUISystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message) {
	// Convert output to Com_Print
	switch (type) {
	case Rml::Log::Type::LT_ALWAYS:
		Com_Printf("[RmlUI]: %s\n", message.c_str());
		break;
	case Rml::Log::Type::LT_ERROR:
		Com_EPrintf("[RmlUI - ERROR]: %s\n", message.c_str());
		break;
	case Rml::Log::Type::LT_ASSERT:
		Com_EPrintf("[RmlUI - ASSERT]: %s\n", message.c_str());
		break;
	case Rml::Log::Type::LT_WARNING:
		Com_WPrintf("[RmlUI - WARNING]: %s\n", message.c_str());
		break;
	case Rml::Log::Type::LT_INFO:
		Com_Printf("[RmlUI - INFO]: %s\n", message.c_str());
		break;
	case Rml::Log::Type::LT_DEBUG:
		Com_DPrintf("[RmlUI - DEBUG]: %s\n", message.c_str());
		break;
	}

	return true;
}