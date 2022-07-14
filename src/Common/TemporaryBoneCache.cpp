/***
*
*	License here.
*
*	@file
*
*	Temporary Bone Cache Implementation.
*
***/
#include "Shared/Shared.h"
#include "Shared/Refresh.h"

#include "Common/TemporaryBoneCache.h"


//! The TOTAL maximum amount of temporary bones the cache can reserve during a frame.
//! Currently it is set to the same value as IQM_MAX_MATRICES (Look it up: /src/refresh/vkpt/shader/vertex_buffer.h)
static constexpr int32_t TBC_SIZE_MAX_CACHEBLOCK = 32768;

//! The TOTAL maximum size for each allocated pose block.
static constexpr int32_t TBC_SIZE_MAX_POSEBLOCK = IQM_MAX_JOINTS;

//! The size of each extra reserved bone cache block.
static constexpr int32_t TBC_SIZE_BLOCK_RESERVE = 8192;

//! The actual default size of our cache.
static constexpr int32_t TBC_SIZE_INITIAL = 8192; // Should allow for 32 distinct poses of size IQM_MAX_JOINTS(256).



//! Serves as a means of working with the std::vector::insert method.
static std::vector<EntitySkeletonBonePose> _cleanBonePoses(IQM_MAX_JOINTS);



/**
*	@brief	Clears the Temporary Bone Cache. Does NOT reset its size to defaults. Memory stays allocated as it was.
**/
void TBC_ClearCache( TemporaryBoneCache &cache ) {
	// Clear the cache.
	cache.cache.clear();
}

/**
*	@brief	Clears, AND resets the Temporary Bone Cache to its default size.
**/
void TBC_ResetCache( TemporaryBoneCache &cache ) {
	// Clear the cache.
	cache.cache.clear();

	// Reserve actual cache data.
	cache.cache.reserve( TBC_SIZE_INITIAL );

	// Ask kindly of this container will shrink to fit. (Implementation dependant.)
	cache.cache.shrink_to_fit();
}

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
EntitySkeletonBonePose *TBC_AcquireCachedMemoryBlock( TemporaryBoneCache &cache, uint32_t size ) {
	/**
	*	#0: Ensure we can properly allocate this block of memory.
	**/
	const size_t sizeDemand = cache.cache.size() + size - 1;
	const size_t currentCapacity = cache.cache.capacity();

	Com_Printf("--------------------\n");
	Com_Printf( "TBC_AcquireCachedMemoryBlock: sizeDemand=%i, currentCapacity=%i\n", sizeDemand, currentCapacity );
	// In case the size exceeds MAX_IQM_JOINTS, nullptr.
	if ( size > IQM_MAX_JOINTS ) {
		Com_Printf( "if ( size > IQM_MAX_JOINTS ) where size=%i\n", size );
		return nullptr;
	}

	// Reserve extra cache memory if needed.
	if ( sizeDemand > currentCapacity ) {
		// We still need to be careful to never exceed TBC_SIZE_MAX_POSEBLOCK here.
		// If it 
		if ( sizeDemand > TBC_SIZE_MAX_CACHEBLOCK ) {
			// TODO: Oughta warn here, or just bail out altogether.
			Com_Printf( "sizeDemand > TBC_SIZE_MAX_CACHEBLOCK where sizeDemand=%i\n", sizeDemand );
			return nullptr;
		}

		// It isn't exceeding TBC_SIZE_MAX_POSEBLOCK, so we can safely reserve another block.
		cache.cache.reserve( currentCapacity + TBC_SIZE_BLOCK_RESERVE );
	}

	/**
	*	#1: Insert (actually initialize and allocate memory if it has not done so before.), and return address.
	**/
	// Insert into our container to initialize the needed memory.
	return &(*cache.cache.insert(cache.cache.end(), _cleanBonePoses.begin(), _cleanBonePoses.begin() + size - 1));
}