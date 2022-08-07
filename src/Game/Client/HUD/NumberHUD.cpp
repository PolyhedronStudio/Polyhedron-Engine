/***
*
*	License here.
*
*	@file
*
*	Client Game NumberHUD display implementation.
* 
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"
// ChatHUD.
#include "Game/Client/HUD/NumberHUD.h"
// Exports Interface Implementations.
#include "Game/Client/Exports/Screen.h"



/***
*
*   
* 
***/
//! Constructor.
NumberHUD::NumberHUD(ClientGameScreen* clgScreen) : screen(clgScreen) {

}

/**
*   @brief  Draws the ChatHUD at given position.
**/
int32_t NumberHUD::Draw(const vec2_t &position) {
	// Keep score of the amount of pixels covered from right to left.
	int32_t finalWidth = 0;
	
	// NOTE: We draw from right to left, so yes indeed, the secondary number  gets drawn first.
	//if (enabledElement & ELEMENT_SECONDARY)
	clgi.R_SetColor(displayColors[SecondaryNumber].u32);
	finalWidth += DrawStringDigits(position, numericals[SecondaryNumber]);
	//endif

	// Draw the Divisor.
	//if (enabledElement & ELEMENT_DIVISOR)
	// We know its image width = 96, but the divisor symbol is about 50 to 60. 
	// To prevent it from looking odd with spacing, we hard code a 80 * scale to add to finalWidth.
	// yet render it at finalWidth - 16. I suck at explaining this thing, got work to do :P
	DrawNumberDivisor(position - vec2_t{finalWidth - (12 * scale), 0.f});
	finalWidth += 72 * scale;
	//endif

	// Draw the primary number.
	////if (enabledElement & ELEMENT_PRIMARY)
	clgi.R_SetColor(displayColors[PrimaryNumber].u32);
	finalWidth += DrawStringDigits(position - vec2_t{finalWidth, 0}, numericals[PrimaryNumber]);
	//end

	// We're done rendering with this HUD element, reset colors.
	clgi.R_ClearColor();

	// Return the final width amount of pixels consumed.
	return finalWidth;
}

/**
*   @brief  Sets the primary number that is always displayed.
**/
void NumberHUD::SetPrimaryNumber(int32_t value) {
	numericals[PrimaryNumber] = value;
}

/**
*   @brief  Sets the secondary number that is optionally displayed.
**/
void NumberHUD::SetSecondaryNumber(int32_t value) {
	numericals[SecondaryNumber] = value;
}

/**
*   @brief  Sets a color for the designated number, or their divisor.
**/
void NumberHUD::SetColor(const color_t& color, uint32_t element) {
	// Ensure we're within bounds.
	if (element >= TotalElements) {
		CLG_Print( PrintType::Warning, "Trying to set an invalid element index in NumberHUD\n");
		return;
	}

	// Set color.
	displayColors[element] = color;
}

/**
*   @brief  Draws the numerical digits of 'value'.
**/
int32_t NumberHUD::DrawStringDigits(const vec2_t& position, int32_t value) {
	// Default pic size.
	const vec2_t picSize = { 96, 128 };

	// Decremental x value.
	int32_t digitX = position.x;
	int32_t digitY = position.y;

	// Returns as being the final sum of pixels traveled (offset from right to left).
	int32_t offsetX = 0;
	
	// Convert value to string.
	std::string digitString = std::to_string(value);

	// Set overall alpha scale.
	clgi.R_SetAlphaScale(1.0f);

	// Apply hud scale & alpha.
	clgi.R_SetScale(screen->screenData.hudScale);
	clgi.R_SetAlpha(screen->screenData.hudAlpha);

	// Loop through numerical string from right to left.
	for (auto it = digitString.crbegin(); it != digitString.crend(); it++) {
		// Ensure it is a digit regardless of what one may expect.
		if (PH_IsDigit(*it)) {
			clgi.R_DrawStretchPic(digitX - (offsetX), digitY, picSize.x * scale, picSize.y * scale, screen->screenData.numberPics[*it - '0']);
		// Assume it is the '-' minus sign.
		} else if (*it == '-') {
			clgi.R_DrawStretchPic(digitX - (offsetX), digitY, picSize.x * scale, picSize.y * scale, screen->screenData.numberMinusPic);
		}

		// Apply scale to the next step in offsetting, and add it on to offsetX.
		offsetX += picSize.x * scale;
	}

	// Reset scale.
	clgi.R_SetScale(1.0f);
	clgi.R_SetAlpha(1.0f);

	// Return the amount of offset we've rendered these numbers at with scale applied, of course.
	return offsetX;
}

/**
*   @brief  Draws the divisor.
**/
int32_t NumberHUD::DrawNumberDivisor(const vec2_t& position) {
	// Default pic size.
	const vec2_t picSize = { 96, 128 };

	// Draw pic.
	clgi.R_DrawStretchPic(position.x, position.y, picSize.x * scale, picSize.y * scale, screen->screenData.numberDivisorPic);

	// Return scaled amount of pixels covered.
	return picSize.x * scale;
}