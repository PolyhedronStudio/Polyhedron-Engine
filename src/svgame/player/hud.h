// LICENSE HERE.

//
// svgame/player/hud.h
//
// N&C SVGame: HUD Header
// 
//
#ifndef __SVGAME_PLAYER_HUD_H__
#define __SVGAME_PLAYER_HUD_H__

void HUD_MoveClientToIntermission(entity_t* client);
void HUD_SetClientStats(entity_t* ent);
void HUD_SetSpectatorStats(entity_t* ent);
void HUD_CheckChaseStats(entity_t* ent);
void HUD_ValidateSelectedItem(entity_t* ent);
void HUD_GenerateDMScoreboardLayout(entity_t* client, entity_t* killer);

#endif // __SVGAME_PLAYER_HUD_H__