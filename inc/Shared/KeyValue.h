/***
*
*	License here.
*
*	@file
*
*	Key/Value parsing utilities.
*
***/
#pragma once

//! Maximum info key string length.
static constexpr uint32_t MAX_INFO_KEY = 64;
//! Maximum info value string length.
static constexpr uint32_t MAX_INFO_VALUE = 64;
//! Maximum entity info string length.
static constexpr uint32_t MAX_INFO_STRING = 512;

/**
*	@brief	Parses the value for 'key' in the entity info string.
*	@param[in]	*s A char* pointer to an entity info string.
*	@param[in]	*key A char* pointer to the key.
*	@return If found, the value for 'key' in the entity info string.
**/
char*	 Info_ValueForKey(const char* s, const char* key);

/**
*	@brief	Removes a key from the entity info string.
*	@param[out/in]	*s A char* pointer to an entity info string.
*	@param[in]	*key A char* pointer to the key.
**/
void	 Info_RemoveKey(char* s, const char* key);

/**
*	@brief	Sets the value for a key in the given entity info string.
*	@param[out/in]	*s A char* pointer to an entity info string.
*	@param[in]	*key A char* pointer to the key.
*	@return	Returns true on success, false if failed. (Key might not have been found etc.)
**/
qboolean Info_SetValueForKey(char* s, const char* key, const char* value);

/**
*	@brief	Validates the entity info string.
*	@param[in]	*s A char* pointer to an entity info string.
*	@return	Returns true if the entity info string was validated accordingly.
**/
qboolean Info_Validate(const char* s);

/**
*	@brief	Validates a sub part of an entity info string.
*	@param[in]	*s A char* pointer to a sub part of an entity info string.
*	@return	Returns true if the entity info string was validated accordingly.
**/

size_t	 Info_SubValidate(const char* s);

/**
*	@brief	Progresses on to the next pair key/value.
*	@param[in]	**s A char** pointer to an entity info string.
*	@param[out]	*key A char* pointer to store the key in.
* 	@param[out]	*value A char* pointer to store the value in.
**/
void	 Info_NextPair(const char** string, char* key, char* value);

/**
*	@brief	Prints the entity info string.
*	@param[in]	*s A char** pointer to an entity info string.
**/
void	 Info_Print(const char* infostring);
