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
void CM_TestCapsuleInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_Test_TraceSphere_In_Brush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_Test_TraceBox_In_Brush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );