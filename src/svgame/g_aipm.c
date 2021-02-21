// LICENSE HERE.

//
// g_aipm.c
//
//
// Works the Player Move AI system logic. Does the thinking for them ;-)
//
#include "g_local.h"
#include "g_aipm.h"

//
//==========================================================================
//
// TARGET SEEKING ETC.
//
//==========================================================================
//
qboolean AIPM_FindTarget(edict_t* self) {
	heardit = false;
	if ((level.sight_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & SF_MONSTER_SIGHT))
	{
		client = level.sight_entity;
		if (client->enemy == self->enemy)
		{
			return false;
		}
	}
	else if (level.disguise_violation_framenum > level.framenum)
	{
		client = level.disguise_violator;
	}
	else if (level.sound_entity_framenum >= (level.framenum - 1))
	{
		client = level.sound_entity;
		heardit = true;
	}
	else if (!(self->enemy) && (level.sound2_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & SF_MONSTER_SIGHT))
	{
		client = level.sound2_entity;
		heardit = true;
	}
	else
	{
		client = level.sight_client;
		if (!client)
			return false;	// no clients to get mad at
	}

	// if the entity went away, forget it
	if (!client || !client->inuse)
	return false;

	// Lazarus
	if (client->client && client->client->camplayer)
	client = client->client->camplayer;

	if (client == self->enemy)
		return true;	// JDC false;
	}

	return false;
}