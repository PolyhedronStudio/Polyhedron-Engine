// LICENSE HERE.

//
// g_pmai.c
//
//
// Works the Player Move AI system logic. Does the thinking for them ;-)
//
#include "g_local.h"
#include "g_pmai.h"

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
		return gi.trace(start, mins, maxs, end, pmai_passent, MASK_MONSTERSOLID);
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
// Returns true if an entity is visible, even if it is NOT IN FRONT of the
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
//===============
// PMAI_BrushInFront
//
// Will return true in case a brush has been traced from the given viewheight.
// This can be used to check for jumping, or crouching.
//===============
//
int PMAI_BrushInFront(edict_t* self, float viewheight)
{
	// Vectors.
	vec3_t dir;				// Direction.
	vec3_t forward, right;	// Forward, and right vector.
	vec3_t start, end;		// Start and end point of our traces.
	vec3_t offset;			// The offset to start tracing from, IN that direction.

	vec3_t mins;			// The actual PMove mins.
	vec3_t maxs;			// The actual PMove maxs.

	// The actual trace results.
	trace_t trace_forward;

	// Copy pmove mins and max.
	VectorCopy(self->pmai.pmove.mins, mins);
	VectorCopy(self->pmai.pmove.maxs, maxs);

	// Anglevectors for our direction that we're moving in.
	VectorCopy(self->s.angles, dir);
	AngleVectors(dir, forward, right, NULL);

	// Start calculating the start and offset endfor forward tracing.
	VectorSet(offset, 0, 0, 0);
	G_ProjectSource(self->s.origin, offset, forward, right, start);
	offset[0] += 18;
	G_ProjectSource(self->s.origin, offset, forward, right, end);

	// Execute our forward trace.
	trace_forward = gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);
	//gi.dprintf("-------------\nstart = %s			end = %s\n",
	//	vtos(start),
	//	vtos(end)
	//);
	//gi.dprintf("-------------\nfraction = %f			allsolid = %s\n",
	//	trace_forward.fraction,
	//	(trace_forward.allsolid == true ? "solid" : "nonsolid")
	//);

	if (trace_forward.fraction < 1.0)	{
		trace_t trace_crouch;
		trace_t trace_jump;
		vec3_t top;

		// Check for crouching
		start[2] -= 14;
		end[2] -= 14;

		// Set up for crouching check
		VectorCopy(self->maxs, top);
		top[2] = 0.0; // crouching height
		trace_crouch = gi.trace(start, self->mins, top, end, self, MASK_PLAYERSOLID);

		//gi.dprintf("-------------\nstart = %s\nend = %s\n",
		//	vtos(start),
		//	vtos(end)
		//);
		//gi.dprintf("-------------\nfraction = %f\nallsolid = %s\n",
		//	trace_crouch.fraction,
		//	(trace_crouch.allsolid == true ? "solid" : "nonsolid")
		//);

		// Crouch
		if (!trace_crouch.allsolid)
		{
			//gi.dprintf("brushAction = crouch\n");
			return 1;
		}

		// Check for jump
		start[2] += 32;
		end[2] += 32;
		trace_jump = gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

		if (!trace_jump.allsolid)
		{
			//gi.dprintf("brushAction = jump\n");
			return 2;
		}
	}

	return -1; // We did not resolve a move here
}

//
//===============
// PMAI_CheckEyes
//
// Check for obstructions in front of the AI. If the move is resolved here
// we return true.
//===============
//
qboolean PMAI_CheckEyes(edict_t* self, usercmd_t* ucmd)
{
	vec3_t  forward, right;
	vec3_t  leftstart, rightstart, focalpoint;
	vec3_t  upstart, upend;
	vec3_t  dir, offset;

	trace_t traceRight, traceLeft, traceUp, traceFront; // for eyesight

	// Get current angle and set up "eyes"
	VectorCopy(self->s.angles, dir);
	AngleVectors(dir, forward, right, NULL);

	// Let them move to targets by walls
	if (!self->movetarget)
		VectorSet(offset, 200, 0, 4); // focalpoint 
	else
		VectorSet(offset, 36, 0, 4); // focalpoint 

	G_ProjectSource(self->s.origin, offset, forward, right, focalpoint);

	// Check from self to focalpoint
	// Ladder code
	VectorSet(offset, 36, 0, 0); // set as high as possible
	G_ProjectSource(self->s.origin, offset, forward, right, upend);
	traceFront = gi.trace(self->s.origin, self->mins, self->maxs, upend, self, MASK_OPAQUE);

	if (traceFront.contents & 0x8000000) // using detail brush here cuz sometimes it does not pick up ladders...??
	{
		ucmd->upmove = 400;
		ucmd->forwardmove = 240;
		return true;
	}

	// If this check fails we need to continue on with more detailed checks
	if (traceFront.fraction == 1)
	{
		ucmd->forwardmove = 240;
		return true;
	}

	VectorSet(offset, 0, 18, 4);
	G_ProjectSource(self->s.origin, offset, forward, right, leftstart);

	offset[1] -= 36; // want to make sure this is correct
	//VectorSet(offset, 0, -18, 4);
	G_ProjectSource(self->s.origin, offset, forward, right, rightstart);

	traceRight = gi.trace(rightstart, NULL, NULL, focalpoint, self, MASK_OPAQUE);
	traceLeft = gi.trace(leftstart, NULL, NULL, focalpoint, self, MASK_OPAQUE);

	// Wall checking code, this will degenerate progressivly so the least cost 
	// check will be done first.

	// If open space move ok
	if (traceRight.fraction != 1 || traceLeft.fraction != 1 || strcmp(traceLeft.ent->classname, "func_door") != 0)
	{
		// Special uppoint logic to check for slopes/stairs/jumping etc.
		VectorSet(offset, 0, 18, 24);
		G_ProjectSource(self->s.origin, offset, forward, right, upstart);

		VectorSet(offset, 0, 0, 200); // scan for height above head
		G_ProjectSource(self->s.origin, offset, forward, right, upend);
		traceUp = gi.trace(upstart, NULL, NULL, upend, self, MASK_OPAQUE);

		VectorSet(offset, 200, 0, 200 * traceUp.fraction - 5); // set as high as possible
		G_ProjectSource(self->s.origin, offset, forward, right, upend);
		traceUp = gi.trace(upstart, NULL, NULL, upend, self, MASK_OPAQUE);

		// If the upper trace is not open, we need to turn.
		if (traceUp.fraction != 1)
		{
			//if (traceRight.fraction > traceLeft.fraction)
			//	self->s.angles[YAW] += (1.0 - traceLeft.fraction) * 45.0;
			//else
			//	self->s.angles[YAW] += -(1.0 - traceRight.fraction) * 45.0;

			ucmd->forwardmove = 240;
			return true;
		}
	}

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
	self->pmai.settings.ranges.melee = 80.f;
	self->pmai.settings.ranges.hostility = 500.f;

	self->pmai.settings.ranges.max_hearing = 1024.f;
	self->pmai.settings.ranges.max_sight = 1024.f;

	self->pmai.settings.ranges.min_dot = 0.3f;

	// Setup the pmove trace and point contents function pointers.
	self->pmai.pmove.trace = PMAI_Trace;				// Adds default parms
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