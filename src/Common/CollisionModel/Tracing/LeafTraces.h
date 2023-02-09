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
void CM_TraceBoxThroughSphere( TraceContext &traceContext, mleaf_t *leaf );

/**
*   @brief 
**/
void CM_TraceSphereThroughSphere( TraceContext &traceContext, mleaf_t *leaf );


/**
*	@brief
**/
void CM_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf );