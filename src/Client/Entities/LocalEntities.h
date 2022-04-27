/***
*
*	License here.
*
*	@file
*
*	'Non-Wired' Local Entities Frame Processing.
* 
***/
#pragma once

/**
*   @brief  Updates the entity belonging to the entity state. If it doesn't
*           exist yet, it'll create it.
**/
void LocalEntity_Update(const EntityState &state);

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
void LocalEntity_FireEvent(int32_t number);