/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Contains all 'Test' in 'LeafBrushes' related work.
*
***/
#pragma once



/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TraceCapsule_TestInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TraceSphere_TestInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TraceBox_TestInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );