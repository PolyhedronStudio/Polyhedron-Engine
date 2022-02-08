/***
*
*	License here.
*
*	@file
*
*	EntityBridge implementation.
*
***/
#pragma once

// Forward declare.
class SVGBaseEntity;
class SVGEntityHandle;

/**
*	Helper function to cast from an entity handle pointer to a SVGBaseEntity pointer.
**/
template<typename T> static inline T CastBridge(SVGEntityHandle& bridge) { return static_cast<T>( static_cast<SVGBaseEntity*>(bridge) ); }
template<typename T> static inline T CastBridge(const SVGEntityHandle& bridge) { return static_cast<T>(static_cast<SVGBaseEntity*>(bridge)); }

/**
*	@brief A bridge between class entity and server entity.
* 
*	@details An Entity "Bridge" is used to link a "Classentity" to a "ServerEntity".
*	It is used as a handle to acquire the class, and server entity pointers with.
*
*	By storing the entity number and a pointer to a classentity it is capable of
*	acting like a safe handle. If the entity number does not match with the one
*	of the currently pointing to classentity, the * operator will kindly return
*	us a nullptr.
* 
*	It can be constructed in the following manners:
*		- By passing a SVGBaseEntity pointer.
*		- By passing a const reference to another handle.
*		- By passing a server entity, and an entity number.
**/
class SVGEntityHandle {
public:
    /**
	*	@brief Simple constructor of an entity bridge that will accept a
	*	class entity.
	**/
    SVGEntityHandle(SVGBaseEntity* baseEntity);

    /**
	*	@brief Simple constructor that will accept an other bridge entity.
	**/
    SVGEntityHandle(const SVGEntityHandle& other);

    /**
	*	@brief Simple constructor that will accept a server entity and an entity number.
	**/
    SVGEntityHandle(Entity* entity, const uint32_t number);

    /**
	*	@brief Default destructor
	**/
    ~SVGEntityHandle() = default;

public:
    /**
	*	@brief	Acquire a pointer to the server entity.
    *
	*	@return	If the entityIDs match, a pointer to the server entity. 
	*			Nullptr otherwise.
	**/
    Entity* Get() const;

    /**
	*	@brief	Sets the server entity pointer and assigns its entity ID to this
	*			handle in case of a non nullptr.
	*
	*	@return	The pointer passed to the function name.
	**/
    Entity* Set(Entity* entity);

    /**
	*	@return	The entityID stored in this "bridge" handle.
	**/
    const uint32_t ID();

    /**
	*	@brief * operator implementations.
	**/
    SVGBaseEntity*	 operator*();
    const SVGBaseEntity* operator*() const;

    /**
	*	@brief	Assigns the SVGBaseEntity to this handle if it has a valid server entity.
	*			If no valid SVGBaseEntity and server entity pointer are passed it unsets
	*			this current handle to nullptr and entityID = 0.
	**/
    SVGBaseEntity* operator=(SVGBaseEntity* baseEntity);
    SVGBaseEntity* operator->() const;

    bool operator==(const SVGBaseEntity*);

    operator bool() { return serverEntity != 0 ; }

private:
    //! Actual pointer referring to the server entity.
    Entity* serverEntity = nullptr;

    //! Stores the server entity's number(ID). Used to determine whether the
    //! currently stored classentity pointer is still pointing to a valid entity.
    uint32_t entityID = 0;
};