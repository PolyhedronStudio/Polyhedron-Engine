//-----------------------------------------------------------------------------
// License here.
//-----------------------------------------------------------------------------
// Debug.h
//-----------------------------------------------------------------------------
//
// Header for debug rendering client game data.
//
//-----------------------------------------------------------------------------
#pragma once

// Default color used for drawing debug lines.
static const vec4_t debugLineColor = vec4_t { 23.f, 233.f, 164.f, 255.f };

// Draws a debug line.
void CLG_DrawDebugLine(const vec3_t &start, const vec3_t &end, const vec4_t &color = debugLineColor );

// Draws a debug bounding box.
void CLG_DrawDebugBoundingBox( const vec3_t &absoluteMins, const vec3_t &absoluteMaxs, const vec4_t &color = debugLineColor );
void CLG_DrawDebugBoundingBox(const vec3_t& origin, const vec3_t& mins, const vec3_t& maxs, const vec4_t& color = debugLineColor );
