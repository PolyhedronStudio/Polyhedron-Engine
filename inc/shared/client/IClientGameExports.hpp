// LICENSE HERE.

//
// inc/shared/client/IGameExports.hpp
//
// Interface that needs to be implemented by a Client Game(CG) module in order for
// it to be compatible with our game client.
//
// --------------------------------------------------------------------
// API Version:
// 
// The version numbers will always be equal to those that were set in 
// CMake at the time of building the engine/game(dll/so) binaries.
// 
// In an ideal world, we comply to proper version releasing rules.
// For Nail & Crescent, the general following rules apply:
//
// MAJOR: Ground breaking new features, you can expect anything to be 
// incompatible at that.
// 
// MINOR : Everytime we have added a new feature, or if the API between
// the Client / Server and belonging game counter-parts has actually 
// changed.
// 
// POINT : Whenever changes have been made, and the above condition 
// is not met.
// --------------------------------------------------------------------
#ifndef __INC_SHARED_CLIENT_ICLIENTGAMEEXPORTS_HPP__
#define __INC_SHARED_CLIENT_ICLIENTGAMEEXPORTS_HPP__

    // Structure containing all the client dll game function pointers for the engine to work with.
    class IClientGameExports {
        //---------------------------------------------------------------------
        // Important VIRTUAL Destructor
        //---------------------------------------------------------------------
        virtual ~IClientGameExports() {};

        //---------------------------------------------------------------------
        // API Version.
        //---------------------------------------------------------------------
        struct {
            int32_t major;         
            int32_t minor;
            int32_t point;
        } apiversion;

        //---------------------------------------------------------------------
        // Core.
        //---------------------------------------------------------------------
        // Initializes the CG module.
        virtual void Init (void) = 0;
        // Shuts down the CG module.
        virtual void Shutdown (void) = 0; 

        // Can be called by the engine too.
        virtual float CalcFOV (float fov_x, float width, float height) = 0;
        // Called when the client (and/is) disconnected for whichever reasons.
        virtual void ClearState (void) = 0;
        // Can be called by the engine too for updating audio positioning.
        virtual void CalcViewValues (void) = 0;
        // Called by the engine when a demo is being seeked.
        virtual void DemoSeek (void) = 0;
        
        // Called after all downloads are done. (Aka, a map has started.)
        // Not used for demos.
        virtual void ClientBegin (void) = 0;
        // Called each VALID client frame. Handle per VALID frame basis 
        // things here.
        virtual void ClientDeltaFrame (void) = 0;
        // Called each client frame. Handle per frame basis things here.
        virtual void ClientFrame (void) = 0;
        // Called when a disconnect even occures. Including those for Com_Error
        virtual void ClientDisconnect (void) = 0;
        // Called when there is a needed retransmit of user info variables.
        virtual void UpdateUserinfo (cvar_t* var, from_t from) = 0;

        //---------------------------------------------------------------------
        // Entities.
        //---------------------------------------------------------------------
        virtual void EntityEvent(int number) = 0;

        //---------------------------------------------------------------------
        // Media.
        //---------------------------------------------------------------------
        // Called when the client wants to know the name of a custom load stat.
        virtual char *GetMediaLoadStateName (load_state_t state) = 0;
        // Called when the renderer initializes.
        virtual void InitMedia (void) = 0;
        // Called whenever the screen media has te reinitialize.
        // Load all HUD/Menu related media here. 2D Images, sounds
        // for HUD, etc.
        virtual void LoadScreenMedia (void) = 0;
        // Called during map load, register world data here such as particles,
        // view models, sounds for entities, etc.
        virtual void LoadWorldMedia (void) = 0;
        // Called when the renderer shutsdown. Should unload all media.
        virtual void ShutdownMedia (void) = 0;

        // Called by the client to initialize PMove.
        virtual void PMoveInit (pmoveParams_t* pmp) = 0;
        // Called by the client when the enable QW movement is toggled.
        virtual void PMoveEnableQW (pmoveParams_t* pmp) = 0;

        //---------------------------------------------------------------------
        // Predict Movement (Client Side)
        //---------------------------------------------------------------------
        virtual void CheckPredictionError (int frame, unsigned int cmd) = 0;
        virtual void PredictAngles (void) = 0;
        virtual void PredictMovement (unsigned int ack, unsigned int current) = 0;

        //---------------------------------------------------------------------
        // ServerMessage Parsing.
        //---------------------------------------------------------------------
        // Called when a configstring update has been parsed and still left
        // unhandled by the client.
        virtual qboolean UpdateConfigString (int index, const char* str) = 0;

        // Called at the start of receiving a server message.
        virtual void StartServerMessage (void) = 0;
        // Actually parses the server message, and handles it accordingly.
        // Returns false in case the message was unkown, or corrupted, etc.
        virtual qboolean ParseServerMessage (int serverCommand) = 0;
        // Handles the demo message during playback.
        // Returns false in case the message was unknown, or corrupted, etc.
        virtual qboolean SeekDemoMessage (int demoCommand) = 0;
        // Called when we're done receiving a server message.
        virtual void EndServerMessage (int realTime) = 0;

        //---------------------------------------------------------------------
        // Screen
        //---------------------------------------------------------------------
        // Called when the engine decides to render the 2D display.
        virtual void RenderScreen (void) = 0;
        // Called when the screen mode has changed.
        virtual void ScreenModeChanged (void) = 0;
        // Called when the client wants to render the loading screen.
        virtual void DrawLoadScreen (void) = 0;
        // Called when the client wants to render the pause screen.
        virtual void DrawPauseScreen (void) = 0;
        

        //---------------------------------------------------------------------
        // View
        //---------------------------------------------------------------------
        // Called right after the engine clears the scene, and begins a new one.
        virtual void PreRenderView (void) = 0;
        // Called whenever the engine wants to clear the scene.
        virtual void ClearScene (void) = 0;
        // Called whenever the engine wants to render a valid frame.
        virtual void RenderView (void) = 0;
        // Called right after the engine renders the scene, and prepares to
        // finish up its current frame loop iteration.
        virtual void PostRenderView (void) = 0;

    };

#endif