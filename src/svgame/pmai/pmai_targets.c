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
qboolean PMAI_FindTarget(edict_t* self)
{
	edict_t* client = NULL;
	qboolean	heardit;
	edict_t* reflection;
	edict_t* self_reflection;
	int			r;

	if (self->monsterinfo.aiflags & (AI_CHASE_THING | AI_HINT_TEST))
		return false;

	if (self->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (self->goalentity && self->goalentity->inuse && self->goalentity->classname)
		{
			if (strcmp(self->goalentity->classname, "target_actor") == 0)
				return false;
		}

		// Lazarus: Look for monsters
		if (!self->enemy)
		{
			if (self->monsterinfo.aiflags & AI_FOLLOW_LEADER)
			{
				int		i;
				edict_t* e;
				edict_t* best = NULL;
				vec_t	dist, best_dist;

				best_dist = self->monsterinfo.max_range;
				for (i = game.maxclients + 1; i < globals.num_edicts; i++)
				{
					e = &g_edicts[i];
					if (!e->inuse)
						continue;
					if (!(e->svflags & SVF_MONSTER))
						continue;
					if (e->svflags & SVF_NOCLIENT)
						continue;
					if (e->solid == SOLID_NOT)
						continue;
					if (e->monsterinfo.aiflags & AI_GOOD_GUY)
						continue;
					if (!visible(self, e))
						continue;
					if (self->monsterinfo.aiflags & AI_BRUTAL)
					{
						if (e->health <= e->gib_health)
							continue;
					}
					else if (e->health <= 0)
						continue;
					dist = realrange(self, e);
					if (dist < best_dist)
					{
						best_dist = dist;
						best = e;
					}
				}
				if (best)
				{
					self->enemy = best;
					FoundTarget(self);
					return true;
				}
			}
			return false;
		}
		else if (level.time < self->monsterinfo.pausetime)
			return false;
		else
		{
			if (!visible(self, self->enemy))
				return false;
			else
			{
				FoundTarget(self);
				return true;
			}
		}
	}

	// if we're going to a combat point, just proceed
	if (self->monsterinfo.aiflags & AI_COMBAT_POINT)
		return false;

	// if the first spawnflag bit is set, the monster will only wake up on
	// really seeing the player, not another monster getting angry or hearing
	// something

	// revised behavior so they will wake up if they "see" a player make a noise
	// but not weapon impact/explosion noises

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

	// Lazarus: Force idle medics to look for dead monsters
	if (!self->enemy && !Q_stricmp(self->classname, "monster_medic"))
	{
		if (medic_FindDeadMonster(self))
			return true;
	}

	// in coop mode, ignore sounds if we're following a hint_path
	if ((coop && coop->value) && (self->monsterinfo.aiflags & AI_HINT_PATH))
		heardit = false;

	if (client->client)
	{
		if (client->flags & FL_NOTARGET)
			return false;
	}
	else if (client->svflags & SVF_MONSTER)
	{
		if (!client->enemy)
			return false;
		if (client->enemy->flags & FL_NOTARGET)
			return false;
	}
	else if (heardit)
	{
		if (client->owner && (client->owner->flags & FL_NOTARGET))
			return false;
	}
	else
		return false;

	reflection = NULL;
	self_reflection = NULL;
	if (level.num_reflectors)
	{
		int		i;
		edict_t* ref;

		for (i = 0; i < 6 && !reflection; i++)
		{
			ref = client->reflection[i];
			if (ref && visible(self, ref) && infront(self, ref))
			{
				reflection = ref;
				self_reflection = self->reflection[i];
			}
		}
	}

	if (!heardit)
	{
		r = range(self, client);

		if (r == RANGE_FAR)
			return false;

		// this is where we would check invisibility

				// is client in an spot too dark to be seen?
		if (client->light_level <= 5)
			return false;

		if (!visible(self, client))
		{
			vec3_t	temp;

			if (!reflection)
				return false;

			self->goalentity = self->movetarget = reflection;
			VectorSubtract(reflection->s.origin, self->s.origin, temp);
			self->ideal_yaw = vectoyaw(temp);
			M_ChangeYaw(self);
			// If MORON (=4) is set, then the reflection becomes the
			// enemy. Otherwise if DUMMY (=8) is set, reflection
			// becomes the enemy ONLY if the monster cannot see his
			// own reflection in the same mirror. And if neither situation
			// applies, then reflection is treated identically 
			// to a player noise.
			// Don't do the MORON/DUMMY bit if SF_MONSTER_KNOWS_MIRRORS
			// is set (set automatically for melee-only monsters, and
			// turned on once other monsters have figured out the truth)
			if (!(self->spawnflags & SF_MONSTER_KNOWS_MIRRORS))
			{
				if (reflection->activator->spawnflags & 4)
				{
					self->monsterinfo.attack_state = 0;
					self->enemy = reflection;
					goto got_one;
				}
				if (reflection->activator->spawnflags & 8)
				{
					if (!self_reflection || !visible(self, self_reflection))
					{
						self->monsterinfo.attack_state = 0;
						self->enemy = reflection;
						goto got_one;
					}
				}
			}
			self->monsterinfo.pausetime = 0;
			self->monsterinfo.aiflags &= ~AI_STAND_GROUND;
			self->monsterinfo.run(self);
			return false;
		}

		// Knightmare- commented this out because it causes a crash
		/*if (reflection && !(self->spawnflags & SF_MONSTER_KNOWS_MIRRORS) &&
			!infront(self,client))
		{
			// Client is visible but behind monster.
			// If MORON or DUMMY for the parent func_reflect is set,
			// attack the reflection (in the case of DUMMY, only
			// if monster doesn't see himself in the same mirror)
			if( (reflection->activator->spawnflags & 4) ||
				( (reflection->activator->spawnflags & 8) &&
				(!self_reflection || !visible(self,self_reflection)) ) ) // crashes here
			{
				vec3_t	temp;

				self->goalentity = self->movetarget = reflection;
				VectorSubtract(reflection->s.origin,self->s.origin,temp);
				self->ideal_yaw = vectoyaw(temp);
				M_ChangeYaw (self);
				self->enemy = reflection;
				goto got_one;
			}
		}*/

		if (!reflection)
		{
			if (r == RANGE_NEAR)
			{
				if (client->show_hostile < level.time && !infront(self, client))
					return false;
			}
			else if (r == RANGE_MID)
			{
				if (!infront(self, client))
					return false;
			}
		}

		self->enemy = client;

		if (strcmp(self->enemy->classname, "player_noise") != 0)
		{
			self->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

			if (!self->enemy->client)
			{
				self->enemy = self->enemy->enemy;
				if (!self->enemy->client)
				{
					self->enemy = NULL;
					return false;
				}
			}
		}
	}
	else	// heardit
	{
		vec3_t	temp;

		if (self->spawnflags & SF_MONSTER_SIGHT)
		{
			if (!visible(self, client))
				return false;
		}
		else if (!(client->flags & FL_REFLECT))
		{
			// Knightmare- exclude turret drivers from this check
			if (!gi.inPHS(self->s.origin, client->s.origin) && strcmp(self->classname, "turret_driver"))
				return false;
		}

		VectorSubtract(client->s.origin, self->s.origin, temp);

		if (VectorLength(temp) > 1000)	// too far to hear
		{
			return false;
		}

		// check area portals - if they are different and not connected then we can't hear it
		if (!(client->flags & FL_REFLECT))
		{
			if (client->areanum != self->areanum)
				if (!gi.AreasConnected(self->areanum, client->areanum))
					return false;
		}

		self->ideal_yaw = vectoyaw(temp);
		M_ChangeYaw(self);

		// hunt the sound for a bit; hopefully find the real player
		self->monsterinfo.aiflags |= AI_SOUND_TARGET;
		self->enemy = client;
	}

got_one:

	//
	// got one
	//
		// stop following hint_paths if we've found our enemy
	if (self->monsterinfo.aiflags & AI_HINT_PATH)
		hintpath_stop(self); // hintpath_stop calls foundtarget
	else if (self->monsterinfo.aiflags & AI_MEDIC_PATROL)
		medic_StopPatrolling(self);
	else
		FoundTarget(self);

	if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) && (self->monsterinfo.sight))
		self->monsterinfo.sight(self, self->enemy);

	return true;
}