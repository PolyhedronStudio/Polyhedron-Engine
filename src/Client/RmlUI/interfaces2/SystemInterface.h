// LICENSE HERE.

//
// client/rmlui/interfaces/SystemInterface.h
//
//
// RmlUI Polyhedron System Interface implementation.
//
#ifndef __CLIENT_RMLUI_INTERFACES_SYSTEMINTERFACE_H__
#define __CLIENT_RMLUI_INTERFACES_SYSTEMINTERFACE_H__

// Required include.
#include <SDL.h>

//
// Simple Polyhedron System Interface to Rml
//
class RmlUISystemInterface : public Rml::SystemInterface
{
public:
	/// Get the number of seconds elapsed since the start of the application
	/// @returns Seconds elapsed
	double GetElapsedTime() override;

	/// Set mouse cursor.
	void SetMouseCursor(const Rml::String& cursor_name) override;

	/// Set clipboard text.
	void SetClipboardText(const Rml::String& text) override;

	/// Get clipboard text.
	void GetClipboardText(Rml::String& text) override;

	//// Translate the input string into the translated string.
	//virtual int TranslateString(Rml::String& translated, const Rml::String& input);

	//// Log the specified message.
	virtual bool LogMessage(Rml::Log::Type type, const Rml::String& message);

	// Translate key.
	Rml::Input::KeyIdentifier TranslateKey(SDL_Keycode sdlkey);

	// Translate mouse button.
	int TranslateMouseButton(uint8_t button);

	// Get key modifiers.
	static int GetKeyModifiers();
};

#endif // __CLIENT_RMLUI_INTERFACES_SHELLSYSTEMINTERFACE_H__
