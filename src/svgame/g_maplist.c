// LICENSE HERE...

//
// g_maplist.c
//
//
// Handles map switches accordingly using sv_maplist.
//
// (Influenced by certain cvars, whether to do a random map, or the map which
// which was voted for by the players.)
//
#include "g_local.h"

//
//===============
// SVG_NextMap
// 
// Switches over to the next map, 
//=============== 
//
void SVG_NextMap(void)
{
	edict_t		*ent;
	char *s, *t, *f;
	static const char *seps = " ,\n\r";

	// stay on same level flag
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		BeginIntermission (CreateTargetChangeLevel (level.mapname) );
		return;
	}

	//
	// N&C - Map 
	//
	// Only search for a next map to play in the maplist, if it's actually non empty.
	// (We "dereference", the string value pointer for this.)
	if (*sv_maplist->string) {
		// Create a duplicate of the string value which we can work with.
		s = strdup(sv_maplist->string);
		// Set our current "filename", to NULL, this will be used for searching maps.
		f = NULL;
		// Tokenize the string by the separators, see the seps variable to adjust these.
		t = strtok(s, seps);

		// As long as T isn't NULL, we'll keep on searching.
		while (t != NULL) {
			// If our current level is in the list, we start working from there.
			if (Q_stricmp(t, level.mapname) == 0) {
				// Try to find the next map that is in this list.
				t = strtok(NULL, seps);
 
				// if T == NULL, we've reached either the end of the list
				if (t == NULL) { // This means we've reached the end of the list.
					if (f == NULL) // There is no first level in the list, switch to the same map.
						BeginIntermission (CreateTargetChangeLevel (level.mapname) );
					else // Switch back to the first map.
						BeginIntermission (CreateTargetChangeLevel (f) );
				} else { // Switch to the next map.
					BeginIntermission (CreateTargetChangeLevel (t) );
				}
				free(s);
				return;
			}

			// If F is NULL, assign T to it.
			if (f == NULL)
				f = t;
			
			// Fetch the next string.
			t = strtok(NULL, seps);
		}

		// Free our duplicated string from memory.
		free(s);
	} else {
		// In case of not using sv_maplist, it is empty etc, we'll use the following code.
		if (level.nextmap[0]) // go to a specific map
			BeginIntermission (CreateTargetChangeLevel (level.nextmap) );
		else {	// search for a changelevel
			ent = G_Find (NULL, FOFS(classname), "target_changelevel");
			if (!ent)
			{	// the map designer didn't include a changelevel,
				// so create a fake ent that goes back to the same level
				BeginIntermission (CreateTargetChangeLevel (level.mapname) );
				return;
			}
			BeginIntermission (ent);
		}
	}
}