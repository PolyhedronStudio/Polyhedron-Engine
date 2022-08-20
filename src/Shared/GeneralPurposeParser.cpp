/***
*
*	License here.
*
*	@file
*
*	Lightweight General Purpose Parser
*
***/
#pragma once

// Shared header.
#include "Shared.h"
#include "GeneralPurposeParser.h"
#include <numeric>

/**
*	@brief	Gets us the current source location information of our parsing state.
**/
static GPPSourceToken::GPPSourceLocation GPP_GetCurrentSourceLocation( const GPPState& state ) {
	//Let's do the heavy lifting of finding the source location inside a buffer WHEN we actually need it. Not before.
	size_t lastNewlinePos = state.mBuffer.substr(0, state.currentPos).find_last_of('\n');
	if (lastNewlinePos == std::string::npos)
	{
		// No newlines found, this is our first line.
		return {
			1,
			state.currentPos
		};
	}

	// If we got here we count the amount of newlines between 'start' and 'currentPos'
	size_t newlineCount = std::accumulate(
		state.mBuffer.begin(), state.mBuffer.begin() + lastNewlinePos, 0, 
			[]( size_t prev, char c ) {
				return c != '\n' ? prev : prev + 1; 
			}
	);

	// Return current source location of our parse state.
	return {
		newlineCount + 1,
		state.currentPos - lastNewlinePos
	};
}

/**
*	@brief	Gets us the next token in our parsing state.
**/
static std::string GPP_NextToken( GPPState& state ) {
	// These delimiters should take care of tabs and new lines altogether, including spaces.
	static std::string delimiters = " \t\r\n";

	// Store current position as old position.
	size_t oldPos = state.currentPos;
	// Find ourselves the relative offset from currentPos to 'first of any delimiter'.
	size_t relativeOffset = state.mBuffer.substr(state.currentPos).find_first_of(delimiters);
	// If we were unable to find one then move to the end of our buffer.
	if(relativeOffset == std::string::npos)	{
		state.currentPos = state.mBuffer.size();
	}
	// If we hit a delimiter immediately process for another token.
    else if (relativeOffset == 0)
    {
        state.currentPos++;
        return GPP_NextToken(state);
    }
	// Otherwise move relativeOffset,
	else
	{
		state.currentPos += relativeOffset;
	}
	// And return a substring containing our new found token.
	return state.mBuffer.substr(oldPos, relativeOffset);
}

/**
*	@brief	Parses the SKC data from the string buffer and places the resulting tokens into the GPPState.
**/
const bool GPP_ParseTokens( GPPState &gppState, const std::vector< std::string > &commandIdentifiers, const bool allowUnknownIdentifiers ) {
	// Keep looping until we're done.
	for(;;)
	{
		/**
		* Fetch ourselves our first token, if it is empty and we're over our buffer size, break out! 
		* otherwise continue for the next iteration.
		**/
		const std::string token = GPP_NextToken(gppState);
		if(token.empty())
		{
            if(gppState.currentPos >= gppState.mBuffer.size())
            {
    			break;
            }
            continue;
		}

		/**
		* Time to identify(categorize type) of our found token.
		**/
		const bool isQuotedValue = token.size() > 1 && token[0] == '"';
		const bool isNumeric = token[0] >= '0' && token[0] <= '9' || token.size() > 1 && token[0] == '-' && token[1] >= '0' && token[1] <= '9';
		const bool isFloat = isNumeric && token.find_first_of('.') != std::string::npos;
		const bool isInteger = isNumeric && !isFloat;

		// Quoted String Token.
		if(isQuotedValue) {
			// We know it is quoted, get ourselves an unquoted copy of its value.
			std::string unQuotedValue = token.substr(1, token.size() - 2);
			
			// Add to our parsed tokens.
			gppState.parsedTokens.push_back({
				.type = GPPSourceToken::Type::QuotedString,
				.sourceLocation = GPP_GetCurrentSourceLocation( gppState ),
				.value = {
					.str = unQuotedValue
				}
			});

		// Floating Numerical Token.
		} else if(isFloat) {
			float number = std::atof(token.c_str());

			// Add to our parsed tokens.
			gppState.parsedTokens.push_back({
				.type = GPPSourceToken::Type::FloatNumber,
				.sourceLocation = GPP_GetCurrentSourceLocation( gppState ),
				.value = {
					.floatNumber = number
				}
			});

		// Integral Numerical Token.
		} else if(isInteger) {
			int32_t number = std::atoi(token.c_str());

			// Add to our parsed tokens.
			gppState.parsedTokens.push_back({
				.type = GPPSourceToken::Type::IntegralNumber,
				.sourceLocation = GPP_GetCurrentSourceLocation( gppState ),
				.value = {
					.integralNumber = number
				}
			});

		// CommandIdentifier/UnknownIdentifier Token.
		} else {
			// See if the token exists in our command list.
			const bool isCommandIdentifier = std::any_of(commandIdentifiers.begin(), commandIdentifiers.begin() + commandIdentifiers.size(), 
				[&token]( const std::string &commandIdentifier ) {
					return commandIdentifier == token; 
				} 
			);

			// If we do NOT allow UnknownIdentifiers, error out if it is not a CommandIdentifier.
			if ( !allowUnknownIdentifiers && !isCommandIdentifier ) {
				// TODO: Error somehow.
				return false;
			}

			// Otherwise we add it just like any other.
			gppState.parsedTokens.push_back({
				.type = (isCommandIdentifier ? GPPSourceToken::Type::CommandIdentifier : GPPSourceToken::Type::UnknownIdentifier ),
				.sourceLocation = GPP_GetCurrentSourceLocation( gppState ),
				.value = {
					.str = token
				}
			});
		}
	}

	// Return our resulting parsed tokens.
	return true;
}