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
	for(int32_t i = 0; i < MaxLines; i++) {
		chatLines[i] = {};
	}
	chatHeadline = 0;
}

/**
*   @brief  Draws the ChatHUD at given position.
**/
void ChatHUD::Draw() {
	// Return if the ChatHUD has been disabled.
	if (screen->scr_chathud->integer == 0) {
		return;
	}

    int32_t x = screen->scr_chathud_x->integer;
    int32_t y = screen->scr_chathud_y->integer;

	int32_t flags = 0;
    if (screen->scr_chathud->integer == 2) {
        flags = UI_ALTCOLOR;
	}

    if (x < 0) {
        x += screen->screenData.hudSize.x + 1;
        flags |= UI_RIGHT;
    } else {
        flags |= UI_LEFT;
    }

	int32_t step = 0;
    if (y < 0) {
        y += screen->screenData.hudSize.y - CHAR_HEIGHT + 1;
        step = -CHAR_HEIGHT;
    } else {
        step = CHAR_HEIGHT;
    }

    uint32_t lines = screen->scr_chathud_lines->integer;
    if (lines > chatHeadline) {
        lines = chatHeadline;
	}

    uint32_t time = screen->scr_chathud_time->value * 1000;

	for (uint32_t i = 0; i < lines; i++) {
		ChatLine &line = chatLines[(chatHeadline - i - 1) & ChatLineMask];

		if (time) {
			float alpha = screen->FadeAlpha(line.timeStamp, time, 1000);
			if (!alpha) {
				break;
			}

			if (screen->scr_chathud_time->integer) {
		        clgi.R_SetAlpha(alpha * screen->scr_alpha->value);
				screen->DrawString(line.text, vec2_t{x, y}, flags);
		        clgi.R_SetAlpha(screen->scr_alpha->value);
		    } else {
		        screen->DrawString(line.text, vec2_t{x, y}, flags);
		    }

		    y += step;
		}
	}
}

/**
*   @brief  Adds a new line of text to the queue.
**/
void ChatHUD::AddText(const std::string& text) {
	// Check whether there are any new lines in this text.
	size_t newLinePosition = text.find_first_of('\n');

	// Ensure newLinePosition does not >= 150
	newLinePosition = (newLinePosition >= 150 || newLinePosition == std::string::npos ? 150 : newLinePosition);

	// In case of newLinePosition being std::string::npos, substr will just copy over the whole string.
	std::string lineText = text.substr(0, newLinePosition);

	// Fetch the next chat headline.
	ChatLine &line = chatLines[chatHeadline++ & ChatLineMask];

	// Configure it with the text and current timeStamp.
	line.text = lineText;
	line.timeStamp = clgi.GetRealTime();
}


