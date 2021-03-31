// LICENSE HERE.

//
// svgame/player/client.h
//
// N&C SVGame: Client Header
// 
//
#ifndef __SVGAME_PLAYER_CLIENT_H__
#define __SVGAME_PLAYER_CLIENT_H__

qboolean ClientConnect(edict_t* ent, char* userinfo);
void ClientDisconnect(edict_t* ent);

void PutClientInServer(edict_t* ent);
void InitClientPersistant(gclient_t* client);
void InitClientResp(gclient_t* client);
void BeginIntermission(edict_t* targ);

void InitBodyQue(void);
void RespawnClient(edict_t* ent);
void TossClientWeapon(edict_t* self);

void ClientUpdateObituary(edict_t* self, edict_t* inflictor, edict_t* attacker);
void TossClientWeapon(edict_t* self);


void ClientBegin(edict_t* ent);
void ClientCommand(edict_t* ent);
void ClientUserinfoChanged(edict_t* ent, char* userinfo);

void ClientBeginServerFrame(edict_t* ent);
void ClientThink(edict_t* ent, usercmd_t* cmd);

#endif // __SVGAME_PLAYER_CLIENT_H__