/***
*
*	License here.
*
*	@file
*
*	Temporary Bone Cache Declarations:
*
*	When we work with the Entity Skeletons we need a place to store all the bone cache for all the
*	processed frame Transforms. The bone cache clears itself at the beginning of each frame in order
*	to allow reuse of the same allocated memory.
*
*	At each new start of a map the cache is cleared. If during gameplay the cache grows it only allocates
*	this memory once, and keeps it allocated until the map ends.
*
***/
#pragma once


// We need to know about our EntitySkeletonBonePose type.
#include "Shared/EntitySKeleton.h"

/**
*	@brief	Currently only contains our vector cache container.
**/
struct TemporaryBoneCache {
	//! This is the actual cache container to which each frame the Game Modules can request 
	//! a block of memory from.
	std::vector<EntitySkeletonBonePose> cache;

	//! Nothing here yet.
};

/**
*	@brief	Clears the Temporary Bone Cache. Does NOT reset its size to defaults. Memory stays allocated as it was.
**/
void TBC_ClearCache( TemporaryBoneCache &cache );

/**
*	@brief	Clears, AND resets the Temporary Bone Cache to its default size.
**/
void TBC_ResetCache( TemporaryBoneCache &cache );

/**
*	@brief	See @return for description. The maximum size of an allocated block is hard limited to TBC_SIZE_MAX_POSEBLOCK.
*
*	@return	A pointer to a prepared block of memory in the bone cache. If needed, the bone cache resizes to meet
*			demands. Note that if it returns a nullptr it means that the actual limit of TBC_SIZE_MAX_POSEBLOCK has
*			been reached. It should always be the same value of IQM_MAX_MATRICES, which is defined in:
*			/src/refresh/vkpt/shader/vertex_buffer.h
*			
*			If you need a larger temporary bone cache than both IQM_MAX_MATRICES as well as TBC_SIZE_MAX_POSEBLOCK
*			need to be increased in a higher, and equally same number.
**/
EntitySkeletonBonePose *TBC_AcquireCachedMemoryBlock( TemporaryBoneCache &cache, uint32_t size = IQM_MAX_JOINTS );