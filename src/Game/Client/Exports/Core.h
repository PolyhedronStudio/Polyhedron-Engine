/***
*
*	License here.
*
*	@file
*
*	Client Game Core Interface Implementation.
* 
***/
#pragma once

//---------------------------------------------------------------------
// Client Game Core IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameCore : public IClientGameExportCore {
public:
	//! Destructor.
    virtual ~ClientGameCore() = default;

	/**
	*	@brief	Initializes the client game.
	**/
	void Initialize() final;

	/**
	*	@brief	Shuts down the client game.
	**/
	void Shutdown() final;

private:

};

