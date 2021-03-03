// LICENSE HERE.

//
// client/rmlui/interfaces/SystemInterface.h
//
//
// RmlUI N&C System Interface implementation.
//
#ifndef __CLIENT_RMLUI_INTERFACES_SYSTEMINTERFACE_H__
#define __CLIENT_RMLUI_INTERFACES_SYSTEMINTERFACE_H__

#include <RmlUi/Core/SystemInterface.h>

//
// Simple Nac System Interface to Rml
//
class RmlUISystemInterface : public Rml::SystemInterface
{
public:
	/// Get the number of seconds elapsed since the start of the application
	/// @returns Seconds elapsed
	double GetElapsedTime() override;

	/// Set mouse cursor.
	/// @param[in] cursor_name Cursor name to activate.
	void SetMouseCursor(const Rml::String& cursor_name) override;

	/// Set clipboard text.
	/// @param[in] text Text to apply to clipboard.
	void SetClipboardText(const Rml::String& text) override;

	/// Get clipboard text.
	/// @param[out] text Retrieved text from clipboard.
	void GetClipboardText(Rml::String& text) override;
};

#endif // __CLIENT_RMLUI_INTERFACES_SHELLSYSTEMINTERFACE_H__
