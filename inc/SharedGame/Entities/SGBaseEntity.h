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

//
// Placeholder.
//
class SGBaseEntity {
public:
    SGBaseEntity() = default;
    ~SGBaseEntity() = default;

    /**
	*	@return A pointer to the server entity.
	**/
    virtual Entity* GetServerEntity() const { return serverEntity; }

private:
    // Actual pointer to the server entity.
    Entity* serverEntity = nullptr;
};