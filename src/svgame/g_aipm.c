// LICENSE HERE.

//
// g_aipm.c
//
//
// Works the Player Move AI system logic. Does the thinking for them ;-)
//
#include "g_local.h"
#include "g_aipm.h"

// The actual client that is being taken care of in this current frame of iterating AI logic.
// Each game frame another client is picked to scan for, this goes on and on.
static edict_t* ai_client;

// This is set to true in case the AI has heard the client.
static qboolean ai_heardit;

//
//==========================================================================
//
// TARGET SEEKING ETC.
//
//==========================================================================
//
edict_t *AIPM_FindTarget(edict_t* self) {
	heardit = false;
	// if the first spawnflag bit is set, the monster will only wake up on
	// really seeing the player, not another monster getting angry or hearing
	// something
	if ((level.sight_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & SF_MONSTER_SIGHT))
	{
		ai_client = level.sight_entity;
		if (ai_client->enemy == self->enemy)
		{
			return NULL;
		}
	}
	else if (level.disguise_violation_framenum > level.framenum)
	{
		ai_client = level.disguise_violator;
	}
	else if (level.sound_entity_framenum >= (level.framenum - 1))
	{
		ai_client = level.sound_entity;
		heardit = true;
	}
	else if (!(self->enemy) && (level.sound2_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & SF_MONSTER_SIGHT))
	{
		ai_client = level.sound2_entity;
		heardit = true;
	}
	else
	{
		ai_client = level.sight_client;
		if (!ai_client)
			return NULL;	// no ai_clients to get mad at
	}

	// if the entity went away, forget it
	if (!ai_client || !ai_client->inuse)
		return NULL;

	return ai_client;
}