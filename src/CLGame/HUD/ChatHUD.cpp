// Client Game.
#include "../ClientGameLocal.h"
#include "../Main.h"
#include "../Media.h"

// ChatHUD.
#include "ChatHUD.h"

// Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"

// Exports Interface Implementations.
#include "../ClientGameExports.h"
#include "../Exports/Screen.h"



/***
*
*   
* 
***/
//! Constructor.
ChatHUD::ChatHUD(ClientGameScreen* clgScreen) : screen(clgScreen) {

}

/**
*   @brief  Clears the ChatHUD.
**/
void ChatHUD::Clear() {

}

/**
*   @brief  Draws the ChatHUD at given position.
**/
void ChatHUD::Draw(const vec2_t &position) {
	// Return if the ChatHUD has been disabled.
	if (screen->scr_chathud->integer == 0) {
		return;
	}


}

/**
*   @brief  Adds a new line of text to the queue.
**/
void ChatHUD::AddText(const std::string& text) {
	// Check whether there are any new lines in this text.
	size_t newLinePosition = text.find_first_of('\n');

	// Ensure newLinePosition does not >= 150
	newLinePosition = (newLinePosition >= 150 ? 150 : newLinePosition);

	// In case of newLinePosition being std::string::npos, substr will just copy over the whole string.
	std::string lineText = text.substr(0, newLinePosition);

	// Manage our queue.
	if (chatLineQueue.size() >= MaxLines) {

	}

	// Push our ChatLine to queue.
	chatLineQueue.push({
		.text = lineText,
		.timeStamp = clgi.GetRealTime()
	});
}


