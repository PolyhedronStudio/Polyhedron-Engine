// LICENSE HERE.

//
// pmai_targets.c
//
//
// Target finding implementation for PMAI.
//
#include "../g_local.h"
#include "../g_pmai.h"

// TODO: Move these elsewhere...
extern edict_t* pmai_passent;
extern trace_t	PMAI_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);

//
//==========================================================================
//
// MOVEMENT.
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
void PMAI_ProcessMovement(edict_t* self) {
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
