/***
*
*	License here.
*
*	@file
*
*	Contains the implementations for debug rendering client information.
*
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! ClientGame Local headers.
#include "Game/Client/ClientGameLocals.h"

// Used for debug rendering.
#include "Game/Client/Effects/Particles.h"



#ifdef _DEBUG
// Static array which should be large enough to contain at least all

// Default color used for drawing debug lines.
//static const vec4_t debugLineColor = { 23.f, 233.f, 164.f, 255.f };

// Draws a debug line.
void CLG_DrawDebugLine(const vec3_t& start, const vec3_t& end, const vec4_t& color) {
    cparticle_t* p = nullptr;

	vec3_t move = start;
    
	float length = 0.f;
	vec3_t vec = end - start;
    vec = vec3_normalize_length(vec, length);

	vec3_t right, up;
    MakeNormalVectors(vec, right, up);
	
    int32_t dec = 8;   
	vec = vec3_scale(vec, dec);


    while (length > 0) {
	    length -= dec;

		p = Particles::GetFreeParticle();
		if (!p)
			return;

		p->time = cl->time;
		p->acceleration = vec3_zero();
		p->vel = vec3_zero();
		p->alpha = 0.85;
		p->alphavel = -1.85f;
		p->brightness = 2.f;
		p->color = -1;
		p->rgba.u32 = MakeColor((uint32_t)color.x, (uint32_t)color.y, (uint32_t)color.z, (uint32_t)color.w);
		p->org = move;

		move += vec;
    }
}

// Performs the actual drawing.
void CLG_DrawDebugBoundingBox( const vec3_t &absoluteMins, const vec3_t &absoluteMaxs, const vec4_t &color ) {
	//
	// Horizontal lines. (From mins z to maxs z.)
	//
	// Line 1.
	vec3_t start = absoluteMins;
	vec3_t end = vec3_t { absoluteMins.x, absoluteMins.y, absoluteMaxs.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 2.
	start = vec3_t { absoluteMaxs.x, absoluteMins.y, absoluteMins.z };
	end = vec3_t { absoluteMaxs.x, absoluteMins.y, absoluteMaxs.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 3.
	start = vec3_t { absoluteMins.x, absoluteMaxs.y, absoluteMins.z };
	end = vec3_t { absoluteMins.x, absoluteMaxs.y, absoluteMaxs.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 4.
	start = vec3_t { absoluteMaxs.x, absoluteMaxs.y, absoluteMins.z };
	end = vec3_t { absoluteMaxs.x, absoluteMaxs.y, absoluteMaxs.z };
	CLG_DrawDebugLine(start, end, color);

	//
	// Vertical lines. (From mins z to maxs z.)
	//
	start = absoluteMins;
	end = vec3_t { absoluteMins.x, absoluteMaxs.y, absoluteMins.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 2.
	start = vec3_t { absoluteMaxs.x, absoluteMins.y, absoluteMins.z };
	end = vec3_t { absoluteMaxs.x, absoluteMaxs.y, absoluteMins.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 3.
	start = vec3_t { absoluteMins.x, absoluteMins.y, absoluteMaxs.z };
	end = vec3_t { absoluteMins.x, absoluteMaxs.y, absoluteMaxs.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 4.
	start = vec3_t { absoluteMaxs.x, absoluteMins.y, absoluteMaxs.z };
	end = vec3_t { absoluteMaxs.x, absoluteMaxs.y, absoluteMaxs.z };
	CLG_DrawDebugLine(start, end, color);

	//
	// Depth lines.
	//
	start = absoluteMins;
	end = vec3_t { absoluteMaxs.x, absoluteMins.y, absoluteMins.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 2.
	start = vec3_t { absoluteMins.x, absoluteMins.y, absoluteMaxs.z };
	end = vec3_t { absoluteMaxs.x, absoluteMins.y, absoluteMaxs.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 3.
	start = vec3_t {  absoluteMins.x, absoluteMaxs.y, absoluteMins.z};
	end = vec3_t { absoluteMaxs.x, absoluteMaxs.y, absoluteMins.z };
	CLG_DrawDebugLine(start, end, color);

	// Line 2.
	start = vec3_t { absoluteMins.x, absoluteMaxs.y, absoluteMaxs.z };
	end = vec3_t { absoluteMaxs.x, absoluteMaxs.y, absoluteMaxs.z };
	CLG_DrawDebugLine(start, end, color);
}
// Draws a debug bounding box.
void CLG_DrawDebugBoundingBox(const vec3_t& origin, const vec3_t& mins, const vec3_t& maxs, const vec4_t& color) {
	// Add origin to the mins and maxs.
	const vec3_t absoluteMins = origin + mins;
	const vec3_t absoluteMaxs = origin + maxs;
	
	// Draw
	CLG_DrawDebugBoundingBox( absoluteMins, absoluteMaxs, color );
}

#else
// Empty place holders since _DEBUG isn't defined.
void CLG_DrawDebugLine(const vec3_t& start, const vec3_t& end, const vec4_t& color) { }
void CLG_DrawDebugBoundingBox(const vec3_t& origin, const vec3_t& mins, const vec3_t& end, const vec4_t& color) { }

#endif