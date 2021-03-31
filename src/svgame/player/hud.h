// LICENSE HERE.

//
// svgame/player/hud.h
//
// N&C SVGame: HUD Header
// 
//
#ifndef __SVGAME_PLAYER_HUD_H__
#define __SVGAME_PLAYER_HUD_H__

void HUD_MoveClientToIntermission(edict_t* client);
void HUD_SetClientStats(edict_t* ent);
void HUD_SetSpectatorStats(edict_t* ent);
void HUD_CheckChaseStats(edict_t* ent);
void HUD_ValidateSelectedItem(edict_t* ent);
void HUD_GenerateDMScoreboardLayout(edict_t* client, edict_t* killer);

#endif // __SVGAME_PLAYER_HUD_H__