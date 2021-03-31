// LICENSE HERE.

//
// svgame/player/hud.h
//
// N&C SVGame: HUD Header
// 
//
#ifndef __SVGAME_PLAYER_HUD_H__
#define __SVGAME_PLAYER_HUD_H__

void MoveClientToIntermission(edict_t* client);
void G_SetClientStats(edict_t* ent);
void G_SetSpectatorStats(edict_t* ent);
void G_CheckChaseStats(edict_t* ent);
void ValidateSelectedItem(edict_t* ent);
void DeathmatchScoreboardMessage(edict_t* client, edict_t* killer);

#endif // __SVGAME_PLAYER_HUD_H__