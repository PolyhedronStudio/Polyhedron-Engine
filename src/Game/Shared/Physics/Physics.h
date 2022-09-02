/***
*
*	License here.
*
*	@file
*
*	Both the ClientGame and the ServerGame modules share the same general Physics code.
* 
***/
#pragma once



/**
*
*
*	Physics:
*
*
**/
/**
*	@brief	Called when two entities have touched so we can safely call their touch callback functions.
**/
void SG_Impact( GameEntity *entityA, const SGTraceResult &trace );
/**
*	@brief	Processes active game and physics logic of this entity for the current time/frame.
**/
void SG_RunEntity( SGEntityHandle &entityHandle );
/**
*	@brief	Gives the entity a chance to process 'Think' callback logic if the
*			time is there for it to do so.
*	@return	True if it failed. Yeah, odd, I know, it was that way, it stays that way for now.
**/
const bool SG_RunThink( GameEntity *geThinker );



/**
*
*
*	Velocity:
*
*
**/
/**
*	@brief	Bounce Velocity.
**/
const vec3_t SG_BounceVelocity( const vec3_t &in, const vec3_t &normal, float overbounce );
/**
*	@brief	Clip Velocity.
**/
const vec3_t SG_ClipVelocity( const vec3_t &velocity, const vec3_t &normal );
/**
*	@brief	Keep entity velocity within bounds.
**/
void SG_BoundVelocity( GameEntity *geCheck );


/**
*
*
*	Gravity/Friction:
*
*
**/
/**
*	@brief	Applies 'downward' gravity forces to the entity.
**/
void SG_AddGravity( GameEntity *sharedGameEntity );
/**
*	@brief	The rotational friction is NOT SET to geRotateFriction!
*	@return	The angular velocity of geRotateFriction after applying rotational friction.
**/
const vec3_t SG_CalculateRotationalFriction( GameEntity *geRotateFriction );
/**
*	@brief	Apply ground friction forces to entity.
**/
void SG_AddGroundFriction( GameEntity *geGroundFriction, const float friction, const float stopSpeed );



/**
*
*
*	Utilities:
*
*
**/
/**
*	@brief	Pushes the entity. Does not change the entities velocity at all
**/
SGTraceResult SG_PushEntity( GameEntity *gePushEntity, const vec3_t &pushOffset );
/**
*	@return	The proper Solid mask to use for the passed game entity.
**/
int32_t SG_SolidMaskForGameEntity( GameEntity *gameEntity );
/**
*	@brief	Checks if this entity should have a groundEntity set or not.
**/
void SG_CheckGround( GameEntity *geCheck );
/**
*	@brief	Utility function that determines whether a plane is too steep to walk on or not.
**/
static inline bool IsWalkablePlane(const CollisionPlane& plane) {
	return plane.normal.z >= 0.7f ? true : false;
}



/**
*
*	MoveType Logic. The engine operates by letting entities Think during and/or
*	have the Think implement, the physics processing. 
*
*	There are various move types, the information below might be outdated but is
*	enough to give you an idea of how Solids and MoveTypes perform magic to create
*	various types of entities.
*	
*	Push: Push objects do not obey gravity, and do not interact with each other, 
*	or trigger fields. They block any other normal movement and result in the touching
*	object being pushed away.
*	
*	OnGround: is set for toss objects when they come to a complete rest.  it is set for steping or walking objects
*	
*	Doors, plats, etc: Are Solid::BSP, and MoveType::Push.
*	Items: Are Solid::Trigger touch, and MoveType::Toss.
*	Corpses Are Solid::Not and MoveType::Toss.
*	Crates: Are Solid::BoundingBox and MoveType::TossSlide
*	Flying/Swimming/Walking: Are Solid::Octagon and MoveType::RootMotionMove.
*	
**/
/**
*	@brief Logic for MoveType::(None, PlayerMove): Gives the entity a chance to 'Think', does not execute any physics logic.
**/
void SG_Physics_None(SGEntityHandle& entityHandle);
/**
*	@brief Logic for MoveType::(NoClip): Moves the entity based on angular- and regular- velocity. Does not clip to world or entities.
**/
void SG_Physics_NoClip(SGEntityHandle &entityHandle);
/**
*	@brief	Performs a velocity based 'Root Motion' movement for general NPC's.
*			If EntityFlags::Swim or EntityFlags::Fly are not set, it'll be affected
*			by gravity and perform continious ground and step checks in order
*			to navigate the terrain properly.
**/
void SG_Physics_RootMotionMove(SGEntityHandle &entityHandle);
/**
*	@brief Logic for MoveType::(Toss, TossSlide, Bounce, Fly and FlyMissile)
**/
void SG_Physics_Toss(SGEntityHandle& entityHandle);
/**
*	@brief Logic for MoveType::(Push, Stop): Pushes all objects except for brush models. 
**/
void SG_Physics_Pusher( SGEntityHandle &gePusherHandle );



/**
*	@brief	Performs a SlideBox move that expects the entity to have been 'tossed' by setting a 
*			velocity and angular velocity into a random direction. Does not perform 'Stepping' up
*			stairs. When touching a 'walkable' surface it'll gradually slow down by friction and
*			stop rotating.
**/
struct SlideBoxMove;
const int32_t SG_Physics_TossSlideBox( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction, SlideBoxMove *slideBoxMove );