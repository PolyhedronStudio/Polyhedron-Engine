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
void CM_TraceCapsuleThroughCapsule( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief 
**/
void CM_TraceBoxThroughCapsule( TraceContext &traceContext, mleaf_t *leaf );
/**
*   @brief 
**/
void CM_Trace_TraceBox_Through_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf );

/**
*   @brief 
**/
void CM_Trace_TraceSphere_Through_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf );


/**
*	@brief
**/
void CM_Trace_TraceBox_ThroughLeaf( TraceContext &traceContext, mleaf_t *leaf );
/**
*	@brief
**/
void CM_Trace_TraceSphere_ThroughLeaf( TraceContext &traceContext, mleaf_t *leaf );

/**
*	@brief
**/
//void CM_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf );