// LICENSE HERE.

//
// client/rmlui/rmlui.cpp
//
//
// RmlUI implementation.
//

// Declare static library linkage.
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

// Client includes.
#include "../client.h"
#include "client/sound/vorbis.h"
#include "client/gamemodule.h"

// RmlUI includes.
#include "rmlui.h"

#include "interfaces/FileInterface.h"
#include "interfaces/RenderInterface.h"
#include "interfaces/SystemInterface.h"


//////////////////////
static Rml::Context* context = NULL;
static Rml::ElementDocument* document = NULL;

static RmlUiRenderInterface rmlRenderInterface;
static RmlUISystemInterface rmlSystemInterface;
static RmlUIFileInterface rmlFileInterface;

// Initializes RMLUI.
void RMLUI_Init(void) {

	// Begin by installing the custom interfaces.
	Rml::SetRenderInterface(&rmlRenderInterface);
	Rml::SetSystemInterface(&rmlSystemInterface);
	Rml::SetFileInterface(&rmlFileInterface);

	// Now we can initialize RmlUi.
	Rml::Initialise();

	// Load FontFaces.
	struct FontFace {
		Rml::String filename;
		bool fallback_face;
	};
	FontFace font_faces[] = {
		{ "LatoLatin-Regular.ttf",    false },
		{ "LatoLatin-Italic.ttf",     false },
		{ "LatoLatin-Bold.ttf",       false },
		{ "LatoLatin-BoldItalic.ttf", false },
		{ "NotoEmoji-Regular.ttf",    true  },
	};

	for (const FontFace& face : font_faces)
	{
		Rml::LoadFontFace("fonts/" + face.filename, face.fallback_face);
	}

	// Create a context next.
	context = Rml::CreateContext("main", Rml::Vector2i( 1280, 720 ));
	if (!context)
	{
		Rml::Shutdown();
		return;
	}

	// If you want to use the debugger, initialize it now.
	Rml::Debugger::Initialise(context);

	// Now we are ready to load our document.
	document = context->LoadDocument("fonts/demo.rml");
	if (!document)
	{
		Rml::Shutdown();
		return;
	}

	document->Show();
}

// Render RMLUI
void RMLUI_UpdateFrame(void) {
	if (!context)
		return;
	if (!(Key_GetDest() & KEY_MENU)) {
		return;
	}
	context->Update();
}

// Render RMLUI
void RMLUI_RenderFrame(void) {
	if (!context)
		return;
	if (!(Key_GetDest() & KEY_MENU)) {
		return;
	}

	context->Render();
}

// Shutdowns RMLUI.
void RMLUI_Shutdown(void) {
	// Shutting down RmlUi releases all its resources, including elements, documents, and contexts.
	Rml::Shutdown();
}