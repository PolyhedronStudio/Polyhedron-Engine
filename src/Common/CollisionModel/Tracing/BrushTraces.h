/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Contains all 'Tracing' through 'LeafBrushes' related work.
*
***/
#pragma once



/**
*   @brief Performs a 'Capsule Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceCapsule_TraceThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
/**
*   @brief Performs a 'Sphere Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceSphere_TraceThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
/**
*   @brief Performs a 'BoundingBox Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceBox_TraceThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
