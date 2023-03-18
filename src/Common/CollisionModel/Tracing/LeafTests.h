/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Contains all 'Testing' in 'Leafs' related work.
*
***/
#pragma once



/**
*   @brief	
**/
//void CM_TestInLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief	
**/
void CM_Test_TraceBox_InLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief	
**/
void CM_Test_TraceSphere_InLeaf( TraceContext &traceContext, mleaf_t *leaf );

/**
*
*	In LeafBrush Tests
*
**/
/**
*   @brief	Tests whether the traceContext traceType capsule against a temporary leaf capsule.
**/
void CM_TestCapsuleLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief	Exchanges the tracing type with 'Capsule' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_TestBoxLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief	Exchanges the tracing type with 'Sphere' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_Test_TraceBox_In_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf );

/**
*   @brief	Performs a 'Sphere' trace test on the 'Sphere' leaf node.
**/
void CM_Test_TraceSphere_In_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf );


/**
*
*	In LeafBrush Tests
*
**/
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
