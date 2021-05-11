// LICENSE HERE.

//
// svgame/player/client.h
//
// N&C SVGame: Client Header
// 
//
#ifndef __SVGAME_PLAYER_CLIENT_H__
#define __SVGAME_PLAYER_CLIENT_H__

qboolean ClientConnect(Entity* ent, char* userinfo);
void ClientDisconnect(Entity* ent);

void PutClientInServer(Entity* ent);
void InitClientPersistant(GameClient* client);
void InitClientResp(GameClient* client);
void HUD_BeginIntermission(Entity* targ);

// Respawns the actual client.
void RespawnClient(Entity* ent);

// Tosses the client weapon.
void TossClientWeapon(Entity* self);

// Updates the client obituary.
void ClientUpdateObituary(Entity* self, Entity* inflictor, Entity* attacker);
void TossClientWeapon(Entity* self);

void ClientBegin(Entity* ent);
void ClientCommand(Entity* ent);
void ClientUserinfoChanged(Entity* ent, char* userinfo);

void ClientBeginServerFrame(Entity* ent);
void ClientThink(Entity* ent, ClientUserCommand* cmd);

#endif // __SVGAME_PLAYER_CLIENT_H__