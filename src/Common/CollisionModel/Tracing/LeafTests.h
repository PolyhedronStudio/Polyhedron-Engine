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
void CM_TraceBox_TestInLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief	
**/
void CM_TraceSphere_TestInLeaf( TraceContext &traceContext, mleaf_t *leaf );

/**
*
*	In LeafBrush Tests
*
**/
/**
*   @brief	Tests whether the traceContext traceType capsule against a temporary leaf capsule.
**/
void CM_TraceCapsule_TestInCapsuleLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief	Exchanges the tracing type with 'Capsule' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_TraceBox_TestInCapsuleLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief	Exchanges the tracing type with 'Sphere' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_TraceBox_TestInSphereLeaf( TraceContext &traceContext, mleaf_t *leaf );

/**
*   @brief	Performs a 'Sphere' trace test on the 'Sphere' leaf node.
**/
void CM_TraceSphere_TestInSphereLeaf( TraceContext &traceContext, mleaf_t *leaf );


/**
*
*	In LeafBrush Tests
*
**/
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
