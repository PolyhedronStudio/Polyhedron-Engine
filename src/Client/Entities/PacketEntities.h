/***
*
*	License here.
*
*	@file
*
*	'Wired' Packet Entities Frame Parsing and Processing.
* 
***/
#pragma once

/**
*   @brief  Updates the entity belonging to the entity state. If it doesn't
*           exist yet, it'll create it.
**/
void PacketEntity_UpdateState(const EntityState &state);

/**
*   @brief  Notifies the client game about an entity event to execute.
**/
void PacketEntity_FireEvent(int32_t number);
