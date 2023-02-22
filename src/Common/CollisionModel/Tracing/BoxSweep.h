/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Box Sweep API.
*
***/
#pragma once


/**
*   @brief	Performs a recursive 'Trace Bounds Sweep' through the 'BSP World Tree Node', when successfully landing in a leaf node, perform its corresponding node trace.
**/
void CM_RecursiveBoxTraceThroughTree( TraceContext &traceContext, mnode_t *node, const float p1f, const float p2f, const vec3_t &p1, const vec3_t &p2 );