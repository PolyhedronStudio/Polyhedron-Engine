/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Box Tracing API.
*
***/
#pragma once


/**
*	Trace Configuration:
**/
//! Use FLT_EPSILON for FRAC_EPSILON
#define USE_FLT_EPSILON_AS_FRAC_EPSILON
//! Fraction Epsilon
#ifdef USE_FLT_EPSILON_AS_FRAC_EPSILON
static constexpr float FRAC_EPSILON = FLT_EPSILON; //1.0f / 1024.0f;
#else
static constexpr float FRAC_EPSILON = 1.0f / 1024.0f;
#endif

//! Use a smaller DIST_EPSILON ( 0.03125 instead of 0.125 )
#define USE_SMALLER_DIST_EPSILON
//! 1/32 Epsilon to keep floating point happy, yay, happy floating point be happy * cough *
#ifdef USE_SMALLER_DIST_EPSILON
static constexpr float DIST_EPSILON = 1.0 / 32.0; // = 0.03125
#else
static constexpr float DIST_EPSILON = 0.125;
#endif

//! Enable optimized point trace support. (Seems broken for capsules atm)
//#define TRACE_ENABLE_BOUNDS_POINT_CASE


//! 1.0 epsilon for cylinder and sphere radius offsets.
static constexpr float CM_RAD_EPSILON = 1.0;




/**
*	End of Trace Configuration.
**/



/**
*	@brief	Determines the traceType for the traceContext to perform,
*			and also is used to describe what headNode type we are dealing
*			with.
**/
struct CMHullType {
	// No tracing type.
	static constexpr int32_t World = 0;

	// AABB BoundingBox type.
	static constexpr int32_t Box = 1;
	// AABB OctagonHull type.
	static constexpr int32_t Octagon = 2;

	// Sphere Hull type.
	static constexpr int32_t Sphere	= 3;
	// Cylinder Hull type.
	static constexpr int32_t Cylinder = 4;
	// Capsule Hull type.
	static constexpr int32_t Capsule = 5;

	// BiSphere type.
	static constexpr int32_t BiSphere = 6;
};
//// Sphere containing start and end radius.
//struct bisphere_t {
//	float startRadius = 0.f;
//	float endRadius = 0.f;
//};
//// The actual sphere, used for oriented capsule collision detection.
//struct sphere_t {
//	float radius = 0.f;
//	float halfHeight = 0.f;
//	vec3_t offset = vec3_zero();
//};
// TODO: Move into separate header.
#include "Common/CollisionModel/SphereHull.h"
#include "Common/CollisionModel/CapsuleHull.h"

/**
*	@brief	Stores the actual data of a trace.
**/
struct TraceContext {
	/**
	*	User Input context values.
	**/
	//! The actual collision model it operates on (either server's, or client's.)
	cm_t *collisionModel = nullptr;

	//! The node to trace from.
	mnode_t *headNode = nullptr;
	//! The leaf node of one of our specific 'Shape Hull' types.
	//! When null ?
	mleaf_t *headNodeLeaf = nullptr;
	//! The contents mask to collide with.
	int32_t contentsMask = 0;

	//! The transformation matrix for plane collisions.
	glm::mat4 matTransform;//glm::identity< glm::mat4 >();
	//! The transformation matrix for user input starts/ends/bounds.
	glm::mat4 matInvTransform;//glm::identity< glm::mat4 >();

	//! Start and end points in 'World Space' of this trace.
	vec3_t start = vec3_zero();
	vec3_t end = vec3_zero();

	//! The trace box its bounds ( mins/maxs ).
	bbox3_t bounds = bbox3_infinity();
	//! The trace box with expanded epsilon bounds offset.
	bbox3_t boundsEpsilonOffset = bbox3_infinity();
	//! The transformed trace box its bounds. (If the trace is transformed, otherwise, equals bounds itself.)
	bbox3_t transformedBounds = bbox3_infinity();

	//! Contains the trace its absolute bounds, from start to end.
	bbox3_t absoluteBounds = bbox3_infinity();


	/**
	*	Trace Processing context values.
	**/
	//! Determines what internal trace type to process with.
	int32_t traceType = CMHullType::World;
	//! Determines what headNode hull type we got. In case of None, it assumes World Brushes.
	int32_t headNodeType = CMHullType::World;


	//! If true then we're dealing with a trace that has non identity matrices going for it.
	bool isTransformedTrace = false;
	//! Point or Box Trace.
	bool isPoint = false; // Optimized case.


	//! Corners of the trace its bounds. Enables fast plane-side testing.
	vec3_t offsets[8] = {vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero() };

	//! The size of the trace bounds.
	vec3_t size = vec3_zero();
	//! The centered to 0,0,0 and symmetrical trace bounds for use with rotations.
	vec3_t extents = vec3_zero();

	//! Spheric data for Capsule trace types.
	sphere_t traceSphere;
	//! BiSpheric data for Capsule trace types.
	bisphere_t traceBiSphere;

	//! Contains the actual final trace results that'll be returned to the caller.
	TraceResult traceResult;

	//! Real Fraction used for testing.
	float realFraction = 0.f;
	//! Trace checkCount.
	int32_t checkCount = 0;
};


/**
*	@return	The entire absolute trace bounds in world space.
**/
const bbox3_t CM_CalculateBoxTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds );
/**
*	@return	The entire absolute 'capsule' trace bounds in world space.
**/
const bbox3_t CM_CalculateCapsuleTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius );

/**
*	@return	True if the bounds intersected.
**/
const bool CM_TraceIntersectBounds( TraceContext &traceContext, const bbox3_t &testBounds );
/**
*	@return	True if the sphere and the trace bounds intersect.
**/
const bool CM_TraceIntersectSphere( TraceContext &traceContext, const sphere_t &sphere, const float radiusDistEpsilon = 0.f );
/**
*	@return	True if the 2D Cylinder and the trace bounds intersect.
**/
const bool CM_TraceIntersect2DCylinder( TraceContext &traceContext, const sphere_t &sphere, const float radiusDistEpsilon = 0.f );

/**
*   @brief  Clips the source trace result against given entity.
**/
void CM_ClipEntity( TraceResult *dst, const TraceResult *src, struct PODEntity *ent );


/**
*   @brief	Operates as the main 'Box' shape trace function, used by other trace functions after changing model frame of references
*			or other necessaties.
**/
const TraceResult _CM_Trace( TraceContext &traceContext );
/**
*   @brief  General box tracing routine.
**/
const TraceResult CM_BoxTrace( cm_t *cm, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, mnode_t *headNode, int32_t brushMask );
/**
*   @brief  Same as CM_BoxTrace but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
const TraceResult CM_TransformedBoxTrace( cm_t *cm, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, mnode_t *headNode, int32_t brushMask, const glm::mat4 &entityMatrix, const glm::mat4 &invEntityMatrix );


/**
*   @brief	Operates as the main 'Sphere' shape trace function, used by other trace functions after changing model frame of references
*			or other necessaties.
**/
const TraceResult _CM_Sphere_Trace( TraceContext &traceContext );
/**
*   @brief  General 'Sphere' shape tracing routine.
**/
const TraceResult CM_SphereTrace( cm_t *cm, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, mnode_t *headNode, int32_t brushMask );
/**
*   @brief  Same as CM_SphereTrace but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
const TraceResult CM_TransformedSphereTrace( cm_t *cm, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, mnode_t *headNode, int32_t brushMask, const glm::mat4 &entityMatrix, const glm::mat4 &invEntityMatrix );


/**
*   @brief  Same as PointContents but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
int32_t CM_PointContents( cm_t *cm, const vec3_t &p, mnode_t *headNode, const glm::mat4 &matInvTransform  );
