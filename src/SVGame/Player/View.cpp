/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// Core.
#include "../ServerGameLocal.h"

// Entities.
#include "../Entities.h"
#include "../Entities/Base/PlayerClient.h"

// Player Hud & Animations Header.
#include "Hud.h"
#include "Animations.h"

//
//===============
// SV_AddBlend
// 
//===============
//
static void SV_AddBlend(float r, float g, float b, float a, float *v_blend)
{
    float   a2, a3;

    if (a <= 0)
        return;
    a2 = v_blend[3] + (1 - v_blend[3]) * a; // new total alpha
    a3 = v_blend[3] / a2;   // fraction of color from old

    v_blend[0] = v_blend[0] * a3 + r * (1 - a3);
    v_blend[1] = v_blend[1] * a3 + g * (1 - a3);
    v_blend[2] = v_blend[2] * a3 + b * (1 - a3);
    v_blend[3] = a2;
}

//
//===============
// SVG_CalculateBlend
// 
//===============
//
void SVG_Client_CalculateBlend(PlayerClient *ent)
{
    // Check whether ent is valid, and a PlayerClient hooked up 
    // to a valid client.
    ServerClient* client = nullptr;

    if (!ent || !(client = ent->GetClient()) ||
        !ent->IsSubclassOf<PlayerClient>()) {
        return;
    }

    // Clear blend values.
    client->playerState.blend[0] = client->playerState.blend[1] =
        client->playerState.blend[2] = client->playerState.blend[3] = 0;

    // Calculate view origin to use for PointContents.
    vec3_t viewOrigin = ent->GetOrigin() + client->playerState.pmove.viewOffset;
    int32_t contents = gi.PointContents(viewOrigin);

	if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
        client->playerState.rdflags |= RDF_UNDERWATER;
    else
        client->playerState.rdflags &= ~RDF_UNDERWATER;

    if (contents & (CONTENTS_SOLID | CONTENTS_LAVA))
        SV_AddBlend(1.0, 0.3, 0.0, 0.6, client->playerState.blend);
    else if (contents & CONTENTS_SLIME)
        SV_AddBlend(0.0, 0.1, 0.05, 0.6, client->playerState.blend);
    else if (contents & CONTENTS_WATER)
        SV_AddBlend(0.5, 0.3, 0.2, 0.4, client->playerState.blend);

    // add for damage
    if (client->damageAlpha > 0)
        SV_AddBlend(client->damageBlend[0], client->damageBlend[1]
                    , client->damageBlend[2], client->damageAlpha, client->playerState.blend);

    if (client->bonusAlpha > 0)
        SV_AddBlend(0.85, 0.7, 0.3, client->bonusAlpha, client->playerState.blend);

    // drop the damage value
    client->damageAlpha -= 0.06;
    if (client->damageAlpha < 0)
        client->damageAlpha = 0;

    // drop the bonus value
    client->bonusAlpha -= 0.1;
    if (client->bonusAlpha < 0)
        client->bonusAlpha = 0;
}