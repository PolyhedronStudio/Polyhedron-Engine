// LICENSE HERE.

//
// g_pmai.c
//
//
// Works the Player Move AI system logic. Does the thinking for them ;-)
//
#include "g_local.h"
#include "g_pmai.h"

// The actual client that is being taken care of in this current frame of iterating AI logic.
// Each game frame another client is picked to scan for, this goes on and on.
static edict_t* ai_client;

// This is set to true in case the AI has heard the client.
static qboolean ai_heardit;


//
//==========================================================================
//
// UTILITY FUNCTIONS
//
//==========================================================================
//

//
// This is a straight copy from PM_Trace.
// Eventually it needs to be moved into its own file obviously.
//
edict_t* pmai_passent;
trace_t	PMAI_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pmai_passent->health > 0)
		return gi.trace(start, mins, maxs, end, pmai_passent, MASK_PLAYERSOLID);
	else
		return gi.trace(start, mins, maxs, end, pmai_passent, MASK_DEADSOLID);
}

//
//===============
// PMAI_SetSightClient
// 
// Called once each frame to set level.sight_client to one of the clients
// that is in sight.
// 
// If all clients are either dead or in notarget, sight_client
// will be null.
// 
// In coop games, sight_client will cycle between the clients.
//===============
//
void PMAI_SetSightClient(void)
{
	// Stores the actual entity that's found.
	edict_t* ent;

	// Counter variables.
	int	start, index;

	// Start all over if we don't have any client in sight.
	if (level.sight_client == NULL)
		start = 1;
	else
		start = level.sight_client - g_edicts;

	// Start a loop 
	index = start;
	while (1)
	{
		// Increment.
		index++;

		// If we've exceeded maxclients, reset the check counter.
		if (index > game.maxclients)
			index = 1;

		// Fetch the entity.
		ent = &g_edicts[index];

		// Ensure it is in use, has health, and isn't a no target entity or
		// a disguised entity.
		if (ent->inuse
			&& ent->health > 0
			&& !(ent->flags & (FL_NOTARGET | FL_DISGUISED)))
		{
			// If player is using func_monitor, make
			// the sight_client = the fake player at the
			// monitor currently taking the player's place.
			// Do NOT do this for players using a
			// target_monitor, though... in this case
			// both player and fake player are ignored.
			if (ent->client && ent->client->camplayer)
			{
				if (ent->client->spycam)
				{
					level.sight_client = ent->client->camplayer;
					return;
				}
			}
			else
			{
				// We've found an entity, store it.
				level.sight_client = ent;
				return;
			}
		}

		// We've checked each client for this frame. Time to move on since
		// none is in sight.
		if (index == start)
		{
			level.sight_client = NULL;
			return;		// nobody to see
		}
	}
}

//
//===============
// PMAI_EntityRange
// 
// Returns the range(distance in units) between the two entities.
//===============
//
float PMAI_EntityRange(edict_t* self, edict_t* other)
{
	vec3_t	v;
	float	len;

	VectorSubtract(self->s.origin, other->s.origin, v);
	len = VectorLength(v);

	return len;
}

//
//===============
// PMAI_EntityVisible
// 
// Returns true if an entity is visible, even if it is not IN FRONT of the
// other entity.
//===============
//
qboolean PMAI_EntityIsVisible(edict_t* self, edict_t* other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	// Ensure we aren't having NULL pointers here.
	if (!self || !other)
		return false;

	// Calculate the starting vector for tracing. Starting at 'self' its
	// origin + viewheight.
	VectorCopy(self->s.origin, spot1);
	spot1[2] += self->pmai.settings.view.height;

	// Calculate the end vector. Take note that this should trace to the 
	// origin + viewheight of the 'other' AI entity.
	VectorCopy(other->s.origin, spot2);
	spot2[2] += other->pmai.settings.view.height;
	
	// Execute the trace.
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);

	// Ensure the trace was valid.
	if ((trace.fraction == 1.0) || (trace.ent == other)) {
		return true;
	}

	// If we've reached this point, we return false because no entity was found
	return false;
}

//
//===============
// PMAI_EntityIsInFront
// 
// Returns true in case the other entity is in front of self, and within
// range of the calculated dot product.
//
// Note that it does not mean that the AI can actually see the entity, it
// might be hidden behind a brush etc. Use PMAI_EntityIsVisible to test if
// they can actually see each other.
//===============
//
qboolean PMAI_EntityIsInFront(edict_t* self, edict_t* other, float min_dot)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	// Ensure we aren't having NULL pointers here.
	if (!self || !other)
		return false;

	// Calculate the forward vector based on the angles of 'self'.
	AngleVectors(self->s.angles, forward, NULL, NULL);

	// Subtract the other origin, from self, and normalize the resulting vec.
	VectorSubtract(other->s.origin, self->s.origin, vec);
	VectorNormalize(vec);

	// Do a dot product on vec and forward to calculate whether it is actually
	// visible or not.
	dot = DotProduct(vec, forward);

	// In case the dot product is higher than the min_dot, it is visible.
	if (dot > min_dot)
		return true;

	// Nothing visible if we've reached this point.
	return false;
}


//
//==========================================================================
//
// SETTINGS
//
//==========================================================================
//

//
//===============
// PMAI_ProcessMovement
// 
// 
//===============
//
void PMAI_ProcessMovement(edict_t *self) {
	// Ensure it is valid.
	if (!self)
		return;

	// Execute the player movement using the given "AI Player Input"
	pmai_passent = self;								// Store self in pm_passent
	self->pmai.pmove.cmd = self->pmai.movement.cmd;		// Copy over ai movement cmd.

	// Execute!
	Pmove(&self->pmai.pmove, &self->pmai.pmp);

	// Unlink the entity, copy origin, relink it.
	gi.unlinkentity(self);
	VectorCopy(self->pmai.pmove.s.origin, self->s.origin);
	gi.linkentity(self);
}


//
//==========================================================================
//
// SETTINGS
//
//==========================================================================
//

//
//===============
// PMAI_Initialize
// 
// Initializes an entity with the default AI settings. Call this before
// tweaking them on your own.
//===============
//
void PMAI_Initialize(edict_t* self) {
	// Make sure it's valid.
	if (!self)
		return;

	// Setup the player move parameters.
	PmoveInit(&self->pmai.pmp);

	// Setup view.
	self->pmai.settings.view.height = 25;

	// Setup ranges.
	self->pmai.settings.ranges.melee = 80;
	self->pmai.settings.ranges.hostility = 500;
	
	self->pmai.settings.ranges.max_hearing = 1024;
	self->pmai.settings.ranges.max_sight = 1024;

	self->pmai.settings.ranges.min_dot = 0.3;

	// Setup the pmove trace and point contents function pointers.
	self->pmai.pmove.trace = PMAI_Trace;	// adds default parms
	self->pmai.pmove.pointcontents = gi.pointcontents;

	// Setup the pmove bounding box.
	VectorSet(self->pmai.pmove.mins, -16, -16, -24);
	VectorSet(self->pmai.pmove.maxs, 16, 16, 32);

	// Setup the pmove state flags.
	self->pmai.pmove.s.pm_flags &= ~PMF_NO_PREDICTION;	// We don't want it to use prediction, there is no client.
	self->pmai.pmove.s.gravity = sv_gravity->value;		// Default gravity.
	self->pmai.pmove.s.pm_type = PM_NORMAL;				// Defualt Player Movement.
	self->pmai.pmove.s.pm_time = 1;						// 1ms = 8 units

	// Copy over the entities origin into the player move for its spawn point.
	VectorCopy(self->s.origin, self->pmai.pmove.s.origin);

}

//
//==========================================================================
//
// TARGET SEEKING ETC.
//
//==========================================================================
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
	} else {
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