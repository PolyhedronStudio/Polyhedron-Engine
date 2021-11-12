// License here.
// 
//
// Interface that a client game dll his exports have to implement in order to
// be fully coherent with the actual client loading it in.
// 
// WID: Time to re-adjust here with new files. I agree, at last.
#pragma once

//---------------------------------------------------------------------
// CORE Export Interface.
//---------------------------------------------------------------------
class IClientGameExportCore {
public:
	//---------------------------------------------------------------------
	// API Version.
	// 
	// The version numbers will always be equal to those that were set in 
	// CMake at the time of building the engine/game(dll/so) binaries.
	// 
	// In an ideal world, we comply to proper version releasing rules.
	// For Polyhedron FPS, the general following rules apply:
	// --------------------------------------------------------------------
	// MAJOR: Ground breaking new features, you can expect anything to be 
	// incompatible at that.
	// 
	// MINOR : Everytime we have added a new feature, or if the API between
	// the Client / Server and belonging game counter-parts has actually 
	// changed.
	// 
	// POINT : Whenever changes have been made, and the above condition 
	// is not met.
	//---------------------------------------------------------------------
	struct APIVersion {
		int32_t major{ VERSION_MAJOR };
		int32_t minor{ VERSION_MINOR };
		int32_t point{ VERSION_POINT };
	} version;

	// Initializes the CLGame.
	virtual void Initialize() = 0;

	// Shuts down the CLGame.
	virtual void Shutdown() = 0;
};

//---------------------------------------------------------------------
// ENTITY interface to implement. 
//---------------------------------------------------------------------
class IClientGameExportEntities {
public:
    // Executed whenever an entity event is receieved.
    virtual void Event(int32_t number) = 0;
};

//---------------------------------------------------------------------
// MEDIA interface.
//---------------------------------------------------------------------
class IClientGameExportMedia {
public:
    // Called when the client wants to know the name of a custom load state.
    virtual std::string GetLoadStateName(LoadState loadState) = 0;

    // This is called when the client starts, but also when the renderer has had
    // modified settings.
    //
    // It should register the basic screen media, 2D icons etc.
    virtual void LoadScreen() = 0;

    // This is called when the client spawns into a server,
    //
    // It should register world related media here, such as particles that are
    // used in-game, or view models, or sounds, etc.
    virtual void LoadWorld() = 0;

    // This is called upon every time the renderer initializes, or does a total
    // hard restart.
    //
    // Use this to load in persistent data, such as 2D pics. Or use it to
    // register certain CVars related to.
    virtual void Initialize() = 0;

    // This is called when the client stops the renderer.
    // Use this to unload remaining data.
    virtual void Shutdown() = 0;
};

//---------------------------------------------------------------------
// MOVEMENT interface.
//---------------------------------------------------------------------
class IClientGameExportMovement {
public:
    // Called when the movement command needs to be build for the given
    // client networking frame.
    virtual void BuildFrameMovementCommand(int32_t miliseconds) = 0;
    // Finished off building the actual movement vector before sending it
    // to server.
    virtual void FinalizeFrameMovementCommand() = 0;
};

//---------------------------------------------------------------------
// Predict Movement (Client Side)
//---------------------------------------------------------------------
class IClientGameExportPrediction {
public:
    virtual void CheckPredictionError(ClientMoveCommand* moveCommand) = 0;
    virtual void PredictAngles() = 0;
    virtual void PredictMovement(uint32_t acknowledgedCommandIndex, uint32_t currentCommandIndex) = 0;

    virtual void UpdateClientSoundSpecialEffects(PlayerMove* pm) = 0;
};

//---------------------------------------------------------------------
// Screen
//---------------------------------------------------------------------
class IClientGameExportScreen {
public:
    // Called when the engine decides to render the 2D display.
    virtual void RenderScreen() = 0;
    // Called when the screen mode has changed.
    virtual void ScreenModeChanged() = 0;
    // Called when the client wants to render the loading screen.
    virtual void DrawLoadScreen() = 0;
    // Called when the client wants to render the pause screen.
    virtual void DrawPauseScreen() = 0;
};

//---------------------------------------------------------------------
// ServerMessage Parsing.
//---------------------------------------------------------------------
class IClientGameExportServerMessage {
public:
    // Breaks up playerskin into name(optional), modeland skin components.
    // If model or skin are found to be invalid, replaces them with sane defaults.
    virtual qboolean ParsePlayerSkin(char* name, char* model, char* skin, const char* str) = 0;
    // Called when a configstring update has been parsed and still left
    // unhandled by the client.
    virtual qboolean UpdateConfigString(int32_t index, const char* str) = 0;

    // Called at the start of receiving a server message.
    virtual void Start() = 0;
    // Actually parses the server message, and handles it accordingly.
    // Returns false in case the message was unkown, or corrupted, etc.
    virtual qboolean ParseMessage(int32_t serverCommand) = 0;
    // Handles the demo message during playback.
    // Returns false in case the message was unknown, or corrupted, etc.
    virtual qboolean SeekDemoMessage(int32_t demoCommand) = 0;
    // Called when we're done receiving a server message.
    virtual void End(int32_t realTime) = 0;
};

//---------------------------------------------------------------------
// View
//---------------------------------------------------------------------
class IClientGameExportView {
public:
    // Called right after the engine clears the scene, and begins a new one.
    virtual void PreRenderView() = 0;
    // Called whenever the engine wants to clear the scene.
    virtual void ClearScene() = 0;
    // Called whenever the engine wants to render a valid frame.
    virtual void RenderView() = 0;
    // Called right after the engine renders the scene, and prepares to
    // finish up its current frame loop iteration.
    virtual void PostRenderView() = 0;
};

//---------------------------------------------------------------------
// MAIN interface to implement. It holds pointers to actual sub interfaces,
// which one of course has to implement as well.
//---------------------------------------------------------------------
class IClientGameExports {
public:
	// WID: TODO: Normally we'd use a Get, should we do that and make these private?
	// Perhaps not.
	IClientGameExportCore* core;
    IClientGameExportEntities* entities;
    IClientGameExportMedia* media;
    IClientGameExportMovement* movement;
    IClientGameExportPrediction* prediction;
    IClientGameExportScreen* screen;
    IClientGameExportServerMessage* serverMessage;
    IClientGameExportView* view;

    //---------------------------------------------------------------------
    // General.
    //---------------------------------------------------------------------
    // Calculates the FOV the client is running. (Important to have in order.)
    virtual float ClientCalculateFieldOfView(float fieldOfViewX, float width, float height) = 0;

    // Called upon whenever a client disconnects, for whichever reason.
    // Could be him quiting, or pinging out etc.
    virtual void ClientClearState() = 0;

    // Called when a demo is being seeked through.
    virtual void DemoSeek() = 0;

    // Called after all downloads are done. (Aka, a map has started.)
    // Not used for demos.
    virtual void ClientBegin() = 0;
    // Called each VALID client frame. Handle per VALID frame basis 
    // things here.
    virtual void ClientDeltaFrame() = 0;
    // Called each client frame. Handle per frame basis things here.
    virtual void ClientFrame() = 0;
    // Called when a disconnect even occures. Including those for Com_Error
    virtual void ClientDisconnect() = 0;

    // Updates the origin. (Used by the engine for determining current audio position too.)
    virtual void ClientUpdateOrigin() = 0;

    // Called when there is a needed retransmit of user info variables.
    virtual void ClientUpdateUserinfo(cvar_t* var, from_t from) = 0;
};

