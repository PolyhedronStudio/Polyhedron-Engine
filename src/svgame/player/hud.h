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
void HUD_SetClientStats(Entity* ent);
void HUD_SetSpectatorStats(Entity* ent);
void HUD_CheckChaseStats(Entity* ent);
void HUD_ValidateSelectedItem(Entity* ent);
void HUD_GenerateDMScoreboardLayout(Entity* client, Entity* killer);

#endif // __SVGAME_PLAYER_HUD_H__