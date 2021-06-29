// LICENSE HERE.

//
// svgame/player/hud.h
//
// N&C SVGame: HUD Header
// 
//
#ifndef __SVGAME_PLAYER_HUD_H__
#define __SVGAME_PLAYER_HUD_H__

void HUD_MoveClientToIntermission(Entity* client);
void SVG_HUD_SetClientStats(Entity* ent);
void SVG_HUD_SetSpectatorStats(Entity* ent);
void SVG_HUD_CheckChaseStats(Entity* ent);
void HUD_ValidateSelectedItem(PlayerClient* ent);
void SVG_HUD_GenerateDMScoreboardLayout(SVGBaseEntity* client, SVGBaseEntity* killer);

#endif // __SVGAME_PLAYER_HUD_H__