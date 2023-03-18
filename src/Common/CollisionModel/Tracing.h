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
static constexpr float CM_RAD_EPSILON = 1.0; // = 1.0; // Used to be 1.0.




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
	*
	*	User Input context values.
	*
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

	/**
	*	AABB Trace
	**/
	struct AABBTrace {
		//! The trace box its bounds ( mins/maxs ).
		bbox3_t bounds = bbox3_infinity();
		//! The trace box with expanded epsilon bounds offset.
		bbox3_t boundsEpsilonOffset = bbox3_infinity();
		//! The transformed trace box its bounds. (If the trace is transformed, otherwise, equals bounds itself.)
		bbox3_t transformedBounds = bbox3_infinity();

		//! Corners of AABB. Enables fast plane-side testing.
		vec3_t offsets[8] = {vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero() };

		//! Corners of Transformed AABB. Enables fast plane-side testing.
		vec3_t transformedOffsets[8] = {vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero(), vec3_zero() };

		//! The size of the AABB.
		vec3_t size = vec3_zero();
		//! Symmetrical AABB extents.
		vec3_t extents = vec3_zero();

		//! The AABB trace its absolute bounds, from start to end.
		//bbox3_t absoluteBounds = bbox3_infinity();
	} aabbTrace;
	/**
	*	Sphere Trace: Calculated based on the AABB.
	**/
	struct SphereTrace {
		//! The original source bounds that were used to calculate this sphere trace with.
		bbox3_t sourceBounds = bbox3_infinity();

		//! Non-Transformed trace bound sphere. The sphere is generated based on the user input 'bounds'.
		sphere_t sphere;
		//! Transformed trace bound sphere. The sphere is generated based on the user input 'bounds'.
		sphere_t transformedSphere;

		//! The Sphere trace its absolute bounds, from start to end.
		//bbox3_t absoluteBounds = bbox3_infinity();
	} sphereTrace;

	//! The actual in-use bounds, identical to the absoluteBounds of our current trace shape.
	bbox3_t absoluteBounds = bbox3_infinity();


	/**
	*
	*	Trace Processing context values.
	*
	**/
	//! The 'Trace Shape' to test and trace our nodes with.
	int32_t traceShape = TraceShape::AABB;

	//! Determines what internal trace type to process with.
	int32_t traceType = CMHullType::World;
	//! Determines what headNode hull type we got. In case of None, it assumes World Brushes.
	int32_t headNodeType = CMHullType::World;


	//! If true then we're dealing with a trace that has non identity matrices going for it.
	bool isTransformedTrace = false;
	//! Point or Box Trace.
	bool isPoint = false; // Optimized case.

	//! Contains the actual final trace results that'll be returned to the caller.
	TraceResult traceResult;

	//! Real Fraction used for testing.
	float realFraction = 0.f;
	//! Trace checkCount.
	int32_t checkCount = 0;
};


/**
*	@return	The entire absolute 'AABB' trace bounds in world space.
**/
const bbox3_t CM_AABB_CalculateTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds );
/**
*	@return	The entire absolute 'Sphere' trace bounds in world space.
**/
const bbox3_t CM_Sphere_CalculateTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius, const float sphereRadiusEpsilon = CM_RAD_EPSILON );
/**
*	@return	The entire absolute 'Capsule' trace bounds in world space.
**/
const bbox3_t CM_CalculateCapsuleTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius );

/**
*	@return	True if the bounds intersected.
**/
const bool CM_TraceIntersectBounds( TraceContext &traceContext, const bbox3_t &testBounds );
/**
*	@return	True if the sphere and the trace bounds intersect.
**/
const bool CM_TraceIntersectSphere( TraceContext &traceContext, const sphere_t &sphere, const int32_t testType, const float radiusDistEpsilon, const bool useOriginOffset = true );
/**
*	@return	True if the 2D Cylinder and the trace bounds intersect.
**/
const bool CM_TraceIntersect2DCylinder( TraceContext &traceContext, const sphere_t &sphere, const int32_t testType, const float radiusDistEpsilon );

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
const TraceResult CM_SphereTrace( cm_t *cm, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const sphere_t &sphere, mnode_t *headNode, int32_t brushMask );
/**
*   @brief  Same as CM_SphereTrace but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
const TraceResult CM_TransformedSphereTrace( cm_t *cm, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const sphere_t &sphere, mnode_t *headNode, int32_t brushMask, const glm::mat4 &entityMatrix, const glm::mat4 &invEntityMatrix );


/**
*   @brief  Same as PointContents but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
int32_t CM_PointContents( cm_t *cm, const vec3_t &p, mnode_t *headNode, const glm::mat4 &matInvTransform  );
