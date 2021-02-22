// LICENSE HERE.

//
// pmai_targets.c
//
//
// Target finding implementation for PMAI.
//
#include "../g_local.h"
#include "../g_pmai.h"


//
//===============
// PMAI_FindTarget
// 
// Self is currently not attacking anything, so try to find a target
// 
// Returns TRUE if an enemy was sighted
// 
// When a player fires a missile, the point of impact becomes a fakeplayer so
// that monsters that see the impact will respond as if they had seen the
// player.
// 
// To avoid spending too much time, only a single client (or fakeclient) is
// checked each frame.  This means multi player games will have slightly
// slower noticing monsters.
//===============
//
//
//===============
// PMAI_FindTarget
// 
// 
//===============
//
qboolean PMAI_FindEnemyTarget(edict_t* self) {
	// Is non NULL if we found a client target.
	edict_t* target = NULL;
	// This is currently for client targets only, but they can make noise, so..
	qboolean heardit = false;
	// The actual range between self and its target.
	float range = 0.0f;

	// Check whether we've found a client.
	if (level.sound_entity_framenum >= (level.framenum - 1))
	{
		// Someone jumped, or shot.
		target = level.sound_entity;
		heardit = true;
	}
	else if (!(self->pmai.targets.enemy.entity) && (level.sound2_entity_framenum >= (level.framenum - 1)) && !(self->spawnflags & SF_MONSTER_SIGHT))
	{
		// Someone caused an impact (explosion etc).
		target = level.sound2_entity;
		heardit = true;
	}
	else
	{
		// Someone was in sight.
		target = level.sight_client;

		// If we still haven't found a client target, return.
		if (!target)
			return false;	// no clients to get mad at
	}

	// If the entity is away, out of use, we can't work with it.
	if (!target || !target->inuse)
		return false;

	// Cam player.
	if (target->client && target->client->camplayer)
		target = target->client->camplayer;

	// If target equals the AI we were already angry at, we've found a match.
	if (target == self->pmai.targets.enemy.entity)
		return true;	// JDC false;

	// in coop mode, ignore sounds if we're following a hint_path
	//if ((coop && coop->value) && (self->monsterinfo.aiflags & AI_HINT_PATH))
	//	heardit = false;

	// Don't return true in case the target is marked as 
	if (target->client) {
		if (target->flags & FL_NOTARGET)
			return false;
	}
	// Checks for when the entity that was found is a monster.
	else if (target->svflags & SVF_MONSTER) {
		// If it has no enemy, return false.
		if (!target->enemy)
			return false;
		// If the enemy has no target set as flags, return false.
		if (target->enemy->flags & FL_NOTARGET)
			return false;
	}
	// Ensure the owner of the sound, isn't having a NOTARGET flag.
	else if (heardit) {
		if (target->owner && (target->owner->flags & FL_NOTARGET))
			return false;
	}
	else {
		return false;
	}

	// Two paths for going further:
	// 1: We've not heard a sound, but seen a client.
	// 2: We've heard a sound. Treat things differently.
	if (!heardit) {
		range = PMAI_EntityRange(self, target);

		// If the range exceeds the AI settings max_sight, return false.
		if (range >= self->pmai.settings.ranges.max_sight)
			return false;

		// If the client is in a too dark spot, we can't see it, return false.
		if (target->light_level <= 5)
			return false;

		// Return false if an enemy is in range of melee combat but is not in front of.
		if (range >= target->pmai.settings.ranges.melee) {
			if (target->show_hostile < level.time && !PMAI_EntityIsInFront(self, target, target->pmai.settings.ranges.min_dot))
				return false;
		}
		// Return false if an enemy isn't in front FOV range of the AI for combat.
		else if (range >= target->pmai.settings.ranges.hostility) {
			if (!PMAI_EntityIsInFront(self, target, target->pmai.settings.ranges.min_dot))
				return false;
		}

		// If we've come this far, we can safely assign it as our enemy.
		self->pmai.targets.enemy.entity = target;

		// If the classname of the entity was player_noise.
		if (strcmp(self->pmai.targets.enemy.entity->classname, "player_noise") != 0)
		{
			// Remove target heard flag.
			self->pmai.targets.enemy.flags &= ~FL_AI_TARGET_HEARD;

			// If the target isn't a client, do some further processing.
			if (!self->pmai.targets.enemy.entity->client)
			{
				edict_t* ent_a = self->pmai.targets.enemy.entity;
				edict_t* ent_b = ent_a->pmai.targets.enemy.entity;

				// Set the new enemy to be the target's enemy, if it is still no
				// client, return false and unset the entity.
				if (!ent_b->client)
				{
					self->pmai.targets.enemy.entity = NULL;
					return false;
				}
			}
		}
	}
	else	// heardit
	{
		// Ensure the target isn't too far off.
		range = PMAI_EntityRange(self, target);

		if (range >= self->pmai.settings.ranges.max_hearing) {
			return false;
		}

		// Check if the areas between the target and the AI are connected.
		// If they aren't connected, then the audio could've been from another
		// area that sits right next to it within distance.
		if (target->areanum != self->areanum)
			if (!gi.AreasConnected(self->areanum, target->areanum))
				return false;

		// Update the enemy target.
		self->pmai.targets.enemy.flags |= AI_SOUND_TARGET;
		self->pmai.targets.enemy.entity = target;
	}

	//foundtarget:
		// TODO: Remove, didn't wanna bother testing if it'd work with no code here
		// although I assume the compiler will rid it since it's useless anyway.


		//
		// got one
		//
		// WatIsDeze - TODO: Perform certain callbacks here for when we've found an
		// enemy target.
	return true;
}