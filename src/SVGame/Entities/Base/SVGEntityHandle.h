/***
*
*	License here.
*
*	@file
*
*	EntityHandle declaration.
*
***/
#pragma once

// Forward declare.
class SVGBaseEntity;
class SVGEntityHandle;

/**
*	Helper function to cast from an entity handle pointer to a SVGBaseEntity pointer.
**/
template<typename T> static inline T CastHandle(SVGEntityHandle& bridge) { return static_cast<T>( static_cast<SVGBaseEntity*>(bridge) ); }
template<typename T> static inline T CastHandle(const SVGEntityHandle& bridge) { return static_cast<T>(static_cast<SVGBaseEntity*>(bridge)); }

/**
*	@brief A bridge between class entity and server entity.
* 
*	@details An entity handle is used to link a "Classentity" to a "ServerEntity".
*	The handle stores a pointer to the server entity and its index number.
*
*	By storing the entity index number and a pointer to the server entity it is 
*   capable of acting like a safe handle. The -> operator returns a pointer to an
*   SVGBaseEntity if, and only if, the server entity isn't 0 and the entity index
*   number is equal to the one of the server entity's classentity pointer. In order
*   to assign the pointer to an SVGBaseEntity use the * operator. Using the * operator
*   also allows for us to do boolean checks such as:
*   if (*someEntity->GetGroundEntity()) { ... } Where GetGroundEntity returns an 
*   EntityHandle.
* 
*   
* 
*	It can be constructed in the following manners:
*		- By passing a SVGBaseEntity pointer.
*		- By passing a const reference to another handle.
*		- By passing a server entity, and an entity number.
**/
class SVGEntityHandle {
public:
    /**
    *   @brief Empty handle constructor.
    **/
    SVGEntityHandle() = default;

    /**
	*	@brief Simple constructor of an entity handle that will accept a
	*	class entity.
	**/
    SVGEntityHandle(SVGBaseEntity* classEntity);

    /**
    *	@brief Simple constructor that will accept a reference to another handle entity.
    **/
    SVGEntityHandle(SVGEntityHandle& other);

    /**
    *	@brief Simple constructor that will accept a const reference to another handle entity.
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
	*	@return	The entityID stored in this handle.
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
    SVGBaseEntity* operator=(SVGBaseEntity* classEntity);

    /**
    *	@brief	Used to access the class entity its methods.
    **/
    SVGBaseEntity* operator->() const;

    /**
    *   @brief  Comparison check for whether this handle points to the same entity as 
    *           the SVGBaseEntity pointer does.
    * 
    *   @return Returns true if SVGBaseEntity* != nullptr, its serverEntity pointer 
    *           != nullptr, and their entity index number matches.
    **/
    bool operator==(const SVGBaseEntity*);

    /**
    *   @brief Used to check whether this entity handle has a valid server entity.
    **/
    operator bool();

private:
    //! Actual pointer referring to the server entity.
    Entity* serverEntity = nullptr;

    //! Stores the server entity's number(ID). Used to determine whether the
    //! currently stored classentity pointer is still pointing to a valid entity.
    uint32_t entityID = 0;
};