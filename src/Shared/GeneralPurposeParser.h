/**
*
*
*	Lightweight General Purpose Parser
*
*
**/
#pragma once 



/**
*	@brief	Final resulting Token, stores its source location also.
**/
struct GPPSourceToken {
	//! Token types:
	struct Type {
		//! When this type is set it means this token had trouble parsing and was never initialized properly.
		static constexpr int32_t Uninitialized = 1 << 0;
		//! Quoted string token types indicating we refer to something instead of signifying a command.
		static constexpr int32_t QuotedString = 1 << 1;
		//! Integral number token type.
		static constexpr int32_t IntegralNumber = 1 << 2;
		//! Type for floating point number tokens( 1337.012345 )
		static constexpr int32_t FloatNumber = 1 << 3;
		//! Type for commands that exist in our allowed command list.
		static constexpr int32_t CommandIdentifier = 1 << 4;
		//! String token UnknownIdentifier, only set if allowed, otherwise these tokens will result in an error instead.
		static constexpr int32_t UnknownIdentifier = 1 << 5;
	};
	//! Determines on how to treat the final resulting token based on its type.
	int32_t type = Type::Uninitialized;

	//! Source location this token was found at.
	struct GPPSourceLocation {
		//! The line at which the token resides.
		size_t line = 0;
		//! The column at which the token resides at.
		size_t column = 0;
	} sourceLocation;

	//! Stores the actual values of said token, use type to determine
	//! where to read its value from.
	struct GPPSourceTokenValue {
		//! Used for IntegralNumber types.
		int32_t integralNumber = 0;
		//! Used for FloatNumber types.
		float floatNumber = 0.f;
		//! Used for QuotedString, CommandIdentifier AND identifier.
		std::string str = "";
	} value;
};

/**
*	@brief
**/
class GPPState {
public:
	GPPState(const std::string &buffer) : mBuffer(buffer) {};
	virtual ~GPPState() = default;

	//! Parse State actual string buffer.
	const std::string &mBuffer;

	//! Current position in our buffer.
	size_t currentPos = 0;

	//! All parsed tokens.
	std::vector< GPPSourceToken > parsedTokens;
};

///**
//*	@brief	Gets us the current source location information of our parsing state.
//**/
//static GPPSourceLocation GPP_GetCurrentSourceLocation( const GPPState& state );
///**
//*	@brief	Gets us the next token in our parsing state.
//**/
//std::string GPP_NextToken( GPPState& state );
/**
*	@brief	Parses the SKC data from the string buffer.
**/
const bool GPP_ParseTokens( GPPState &gppState, const std::vector< std::string > &commandIdentifiers, const bool allowUnknownIdentifiers = false );