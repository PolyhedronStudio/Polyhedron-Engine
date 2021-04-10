// LICENSE HERE.

//
// src/clgame/IEAPI/ClientGameExports.hpp
//
// Export implementation class of the Import/Export(IE) API,
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
#ifndef __INC_SHARED_CLIENT_CLIENTGAMEEXPORTS_HPP__
#define __INC_SHARED_CLIENT_CLIENTGAMEEXPORTS_HPP__

    // Structure containing all the client dll game function pointers for the engine to work with.
    class ClientGameExports : public IClientGameExports {
        //---------------------------------------------------------------------
        // Important VIRTUAL Destructor
        //---------------------------------------------------------------------
        ClientGameExports();
        virtual ~ClientGameExports();


        //-----------------------------------------------------s----------------
        // Pointers to CG Module.
        //---------------------------------------------------------------------
        // Shared player move parameters.
        // N&C: This has moved over to the client game exports.
        // It has been changed in to a pointer. By doing so we can prevent
        // this structure from turning inconsistent if the game decides to
        // add extra parameters to this structure.
        pmoveParams_t* pmoveParams; // PMOVE: Remove once the game modules init pmove themselves using CLG_ParseServerData.


        //---------------------------------------------------------------------
        // Core.
        //---------------------------------------------------------------------
        virtual void Init (void);
        virtual void Shutdown (void); 

        virtual float CalculateFOV (float fov_x, float width, float height);
        virtual void CalculateViewValues (void);

        virtual void ClearState (void);
        virtual void DemoSeek (void);

        virtual void ClientBegin (void);
        virtual void ClientDeltaFrame (void);
        virtual void ClientFrame (void);
        virtual void ClientDisconnect (void);

        virtual void UpdateUserinfo (cvar_t* var, from_t from);


        //---------------------------------------------------------------------
        // Entities.
        //---------------------------------------------------------------------
        virtual void EntityEvent(int number);


        //---------------------------------------------------------------------
        // Media.
        //---------------------------------------------------------------------
        virtual char *GetMediaLoadStateName (load_state_t state);
        virtual void InitMedia (void);
        virtual void LoadScreenMedia (void);
        virtual void LoadWorldMedia (void);
        virtual void ShutdownMedia (void);

        // Called by the client to initialize PMove.
        virtual void PMoveInit (pmoveParams_t* pmp);
        // Called by the client when the enable QW movement is toggled.
        virtual void PMoveEnableQW (pmoveParams_t* pmp);


        //---------------------------------------------------------------------
        // Predict Movement (Client Side)
        //---------------------------------------------------------------------
        virtual void CheckPredictionError (int frame, unsigned int cmd);
        virtual void PredictAngles (void);
        virtual void PredictMovement (unsigned int ack, unsigned int current);


        //---------------------------------------------------------------------
        // ServerMessage Parsing.
        //---------------------------------------------------------------------
        virtual qboolean UpdateConfigString (int index, const char* str);

        virtual void StartServerMessage (void);
        virtual qboolean ParseServerMessage (int serverCommand);
        virtual void EndServerMessage (int realTime);

        virtual qboolean SeekDemoMessage (int demoCommand);


        //---------------------------------------------------------------------
        // Screen
        //---------------------------------------------------------------------
        virtual void ScreenModeChanged (void);
        
        virtual void RenderScreen (void);

        virtual void DrawLoadScreen (void);
        virtual void DrawPauseScreen (void);
        

        //---------------------------------------------------------------------
        // View
        //---------------------------------------------------------------------
        virtual void PreRenderView (void);
        virtual void ClearScene (void);
        virtual void RenderView (void);
        virtual void PostRenderView (void);
    };

#endif