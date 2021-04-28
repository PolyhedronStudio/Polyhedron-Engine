// LICENSE HERE.

//
// svgame/player/client.h
//
// N&C SVGame: Client Header
// 
//
#ifndef __SVGAME_PLAYER_CLIENT_H__
#define __SVGAME_PLAYER_CLIENT_H__

qboolean ClientConnect(entity_t* ent, char* userinfo);
void ClientDisconnect(entity_t* ent);

void PutClientInServer(entity_t* ent);
void InitClientPersistant(gclient_t* client);
void InitClientResp(gclient_t* client);
void HUD_BeginIntermission(entity_t* targ);

// Respawns the actual client.
void RespawnClient(entity_t* ent);

// Tosses the client weapon.
void TossClientWeapon(entity_t* self);

// Updates the client obituary.
void ClientUpdateObituary(entity_t* self, entity_t* inflictor, entity_t* attacker);
void TossClientWeapon(entity_t* self);

void ClientBegin(entity_t* ent);
void ClientCommand(entity_t* ent);
void ClientUserinfoChanged(entity_t* ent, char* userinfo);

void ClientBeginServerFrame(entity_t* ent);
void ClientThink(entity_t* ent, cl_cmd_t* cmd);

#endif // __SVGAME_PLAYER_CLIENT_H__