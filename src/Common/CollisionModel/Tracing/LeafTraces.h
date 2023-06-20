/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Contains all 'Tracing' through 'Leafs' related work.
*
***/
#pragma once



/**
*   @brief 
**/
void CM_TraceCapsule_ThroughCapsuleLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief 
**/
void CM_TraceBox_ThroughCapsuleLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief 
**/
void CM_TraceBox_TraceThroughSphereLeaf( TraceContext &traceContext, mleaf_t *leaf );

/**
*   @brief 
**/
void CM_TraceSphere_TraceThroughSphereLeaf( TraceContext &traceContext, mleaf_t *leaf );


/**
*	@brief
**/
void CM_TraceBox_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*	@brief
**/
void CM_TraceSphere_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf );

/**
*	@brief
**/
//void CM_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf );