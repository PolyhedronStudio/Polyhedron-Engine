// LICENSE HERE.

//
// svgame/player/client.h
//
// Polyhedron: Client Header
// 
//
#ifndef __SVGAME_PLAYER_CLIENT_H__
#define __SVGAME_PLAYER_CLIENT_H__

class SVGBasePlayer;

qboolean SVG_ClientConnect(Entity* ent, char* userinfo);
void SVG_ClientDisconnect(Entity* ent);

void SVG_HUD_BeginIntermission(Entity* targ);

// Tosses the client weapon.
void SVG_TossClientWeapon(SVGBasePlayer* playerClient);

void SVG_ClientBegin(Entity* ent);
void SVG_ClientCommand(Entity* ent);
void SVG_ClientUserinfoChanged(Entity* ent, char* userinfo);

//void SVG_ClientBeginServerFrame(SVGBaseEntity* ent); // WID: Moved to gamemodes.
void SVG_ClientThink(Entity* svEntity, ClientMoveCommand* cmd);

void SVG_ClientEndServerFrames(void);

#endif // __SVGAME_PLAYER_CLIENT_H__