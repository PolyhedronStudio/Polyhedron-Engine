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

//
//=============================================================================
// SetMouseCursor
//
// Assigns the text to VID_SetClipboardData
//=============================================================================
//
void RmlUISystemInterface::SetMouseCursor(const Rml::String& cursor_name) {
	
}

//
//=============================================================================
// SetClipboardText
//
// Assigns the text to VID_SetClipboardData
//=============================================================================
//
void RmlUISystemInterface::SetClipboardText(const Rml::String& text) {
	// Set clipboard data.
	VID_SetClipboardData(text.c_str());
}

//
//=============================================================================
// GetClipboardText
//
// Captures the VID_GetClipboardData and assigns it to text.
//=============================================================================
//
void RmlUISystemInterface::GetClipboardText(Rml::String& text) {
	char *data = VID_GetClipboardData();
	if (data)
		text = data;
}

//
//=============================================================================
// LogMessage
//
// Type is one of the following:
// Rml::Log::ERROR for error messages
// Rml::Log::ASSERT for failed internal assertions(debug library only)
// Rml::Log::WARNING for non - fatal warnings
// Rml::Log::INFO for generic information messages
// 
// The message parameter is the actual message itself. 
// 
// The function should return true if program execution should continue, 
// or false to generate an interrupt to break execution. This can be useful if 
// you are running inside a debugger to see exactly what an application is 
// doing to trigger a certain message.
//=============================================================================
//
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