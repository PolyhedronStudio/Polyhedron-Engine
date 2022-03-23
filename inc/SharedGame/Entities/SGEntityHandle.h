/***
*
*	License here.
*
*	@file
*
*	SGEntityHandle declaration.
*
***/
#pragma once



// Predeclare.
class SGEntityHandle;

/**
*	Helper function to cast from an entity handle pointer to a ClassEntity pointer.
**/
template<typename T> static inline T CastHandle(SGEntityHandle& bridge) { return static_cast<T>( static_cast<ClassEntity*>(bridge) ); }
template<typename T> static inline T CastHandle(const SGEntityHandle& bridge) { return static_cast<T>(static_cast<ClassEntity*>(bridge)); }

/**
*	@brief A bridge between class entity and server entity.
* 
*	@details An entity handle is used to link a "Classentity" to a "POD Entity".
*	The handle stores a pointer to the POD Entity and its index number.
*
*	By storing the entity index number and a pointer to the POD Entity it is 
*   capable of acting like a safe handle. The -> operator returns a pointer to a
*   ClassEntity if, and only if, the POD Entity isn't 0 and the entity index
*   number is equal to the one of the POD Entity's classentity pointer. In order
*   to assign the pointer to an SVG/CLG -BaseEntity use the * operator. 
*
*   Using the * operator also allows for executing logical boolean checks such as:
*   if (*someEntity->GetGroundEntity()) { ... } 
*   Where GetGroundEntity returns an EntityHandle.
*   
* 
*	It can be constructed in the following manners:
*		- By passing a SVGBaseEntity pointer.
*		- By passing a const reference to another handle.
*		- By passing a server entity, and an entity number.
**/
class SGEntityHandle {
public:
    /**
    *   @brief Empty handle constructor.
    **/
    SGEntityHandle() = default;

    /**
	*	@brief Simple constructor of an entity handle that will accept a
	*	class entity.
	**/
    SGEntityHandle(ClassEntity* classEntity);

    /**
    *	@brief Simple constructor that will accept a reference to another handle entity.
    **/
    SGEntityHandle(SGEntityHandle& other);

    /**
    *	@brief Simple constructor that will accept a const reference to another handle entity.
    **/
    SGEntityHandle(const SGEntityHandle& other);

    /**
	*	@brief Simple constructor that will accept a server entity.
	**/
    SGEntityHandle(PODEntity* podEntity);

    /**
	*	@brief Simple constructor that will accept a server entity and an entity number.
	**/
    SGEntityHandle(PODEntity* podEntity, const uint32_t number);

    /**
	*	@brief Default destructor
	**/
    ~SGEntityHandle() = default;

public:
    /**
	*	@brief	Acquire a pointer to the POD Entity.
    *
	*	@return	If the entityIDs match, a pointer to the POD Entity. 
	*			Nullptr otherwise.
	**/
    PODEntity* Get() const;

    /**
	*	@brief	Sets the POD Entity pointer and assigns its entity ID to this
	*			handle in case of a non nullptr.
	*
	*	@return	The pointer passed to the function name.
	**/
    PODEntity* Set(PODEntity* podEntity);

    /**
	*	@return	The entityID stored in this handle.
	**/
    const uint32_t ID();

    /**
	*	@brief * operator implementations.
	**/
    ClassEntity*        operator*();
    const ClassEntity*  operator*() const;

    /**
	*	@brief	Assigns the ClassEntity to this handle if it has a valid POD Entity.
	*			If no valid ClassEntity and POD Entity pointer are passed it unsets
	*			the current handle to nullptr and entityID = 0.
	**/
    ClassEntity* operator=(ClassEntity* classEntity);

    /**
    *	@brief	Used to access the class entity its methods.
    **/
    ClassEntity* operator->() const;

    /**
    *   @brief  Comparison check for whether this handle points to the same entity as 
    *           the ClassEntity pointer does.
    * 
    *   @return Returns true if ClassEntity* != nullptr, its POD Entity pointer != nullptr, 
    *           and their entity index number matches.
    **/
    bool operator==(const ClassEntity*);

    /**
    *   @brief Used to check whether this entity handle has a valid server entity.
    **/
    operator bool();

private:
    //! Actual pointer referring to the POD Entity.
    PODEntity* podEntity = nullptr;

    //! Stores the server entity's number(ID). Used to determine whether the
    //! currently stored classentity pointer is still pointing to a valid entity.
    uint32_t entityID = 0;
};