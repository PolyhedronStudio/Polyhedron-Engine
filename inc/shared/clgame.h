/*
// LICENSE HERE.

//
// inc/shared/clgame.h
//
// Contains the client game related code that is shared with and by the engine.
//
*/
#ifndef __INC_SHARED_CLGAME_H__
#define __INC_SHARED_CLGAME_H__

#define CGAME_API_VERSION_MAJOR VERSION_MAJOR
#define CGAME_API_VERSION_MINOR VERSION_MINOR
#define CGAME_API_VERSION_POINT VERSION_POINT

//
//=============================================================================
//
//	CLIENT GAME IMPORT AND EXPORT STRUCTURES.
//
//=============================================================================
//
#include "shared/interfaces/IClientGameExports.h"
#include "shared/interfaces/IClientGameImports.h"

#ifdef __cplusplus
extern "C" {
#endif
    //// Structure containing all the client dll game function pointers for the engine to work with.
    //typedef struct clg_export_s {
    //    //---------------------------------------------------------------------
    //    // API Version.
    //    // 
    //    // The version numbers will always be equal to those that were set in 
    //    // CMake at the time of building the engine/game(dll/so) binaries.
    //    // 
    //    // In an ideal world, we comply to proper version releasing rules.
    //    // For Polyhedron FPS, the general following rules apply:
    //    // --------------------------------------------------------------------
    //    // MAJOR: Ground breaking new features, you can expect anything to be 
    //    // incompatible at that.
    //    // 
    //    // MINOR : Everytime we have added a new feature, or if the API between
    //    // the Client / Server and belonging game counter-parts has actually 
    //    // changed.
    //    // 
    //    // POINT : Whenever changes have been made, and the above condition 
    //    // is not met.
    //    //---------------------------------------------------------------------
    //    struct {
    //        int32_t major;         
    //        int32_t minor;
    //        int32_t point;
    //    } apiversion;

    //    //---------------------------------------------------------------------
    //    // Core.
    //    //---------------------------------------------------------------------
    //    // Initializes the client game dll.
    //    void		(*Init) ();
    //    // Shuts down the client game dll.
    //    void		(*Shutdown) (void); 

    //    // Can be called by the engine too.
    //    float       (*CalcFOV) (float fov_x, float width, float height);
    //    // Called when the client (and/is) disconnected for whichever reasons.
    //    void        (*ClearState) (void);
    //    // Can be called by the engine too for updating audio positioning.
    //    void        (*UpdateOrigin) (void);
    //    // Called by the engine when a demo is being seeked.
    //    void        (*DemoSeek) (void);
    //    
    //    // Called after all downloads are done. (Aka, a map has started.)
    //    // Not used for demos.
    //    void        (*ClientBegin) (void);
    //    // Called each VALID client frame. Handle per VALID frame basis 
    //    // things here.
    //    void        (*ClientDeltaFrame) (void);
    //    // Called each client frame. Handle per frame basis things here.
    //    void        (*ClientFrame) (void);
    //    // Called when a disconnect even occures. Including those for Com_Error
    //    void        (*ClientDisconnect) (void);

    //    // Called when there is a needed retransmit of user info variables.
    //    void        (*UpdateUserinfo) (cvar_t* var, from_t from);

    //    //---------------------------------------------------------------------
    //    // Entities.
    //    //---------------------------------------------------------------------
    //    void        (*EntityEvent) (int number);

    //    //---------------------------------------------------------------------
    //    // 
    //    //---------------------------------------------------------------------
    //    // Called when the movement command needs to be build for the given
    //    // client networking frame.
    //    void        (*BuildFrameMoveCommand) (int msec);
    //    // Finished off building the actual movement vector before sending it
    //    // to server.
    //    void        (*FinalizeFrameMoveCommand) (void);

    //    //---------------------------------------------------------------------
    //    // Media.
    //    //---------------------------------------------------------------------
    //    // Called when the client wants to know the name of a custom load stat.
    //    char        *(*GetMediaLoadStateName) (LoadState state);
    //    // Called when the renderer initializes.
    //    void        (*InitMedia) (void);
    //    // Called whenever the screen media has te reinitialize.
    //    // Load all HUD/Menu related media here. 2D Images, sounds
    //    // for HUD, etc.
    //    void        (*LoadScreenMedia) (void);
    //    // Called during map load, register world data here such as particles,
    //    // view models, sounds for entities, etc.
    //    void        (*LoadWorldMedia) (void);
    //    // Called when the renderer shutsdown. Should unload all media.
    //    void        (*ShutdownMedia) (void);

    //    //---------------------------------------------------------------------
    //    // Predict Movement (Client Side)
    //    //---------------------------------------------------------------------
    //    void		(*CheckPredictionError) (ClientMoveCommand* moveCommand);
    //    void		(*PredictAngles) (void);
    //    void        (*PredictMovement) (unsigned int acknowledgedCommandIndex, unsigned int currentCommandIndex);

    //    //---------------------------------------------------------------------
    //    // ServerMessage Parsing.
    //    //---------------------------------------------------------------------
    //    // Called when a configstring update has been parsed and still left
    //    // unhandled by the client.
    //    qboolean    (*UpdateConfigString) (int index, const char* str);

    //    // Called at the start of receiving a server message.
    //    void        (*StartServerMessage) (void);
    //    // Actually parses the server message, and handles it accordingly.
    //    // Returns false in case the message was unkown, or corrupted, etc.
    //    qboolean    (*ParseServerMessage) (int serverCommand);
    //    // Handles the demo message during playback.
    //    // Returns false in case the message was unknown, or corrupted, etc.
    //    qboolean   (*SeekDemoMessage) (int demoCommand);
    //    // Called when we're done receiving a server message.
    //    void        (*EndServerMessage) (int realTime);

    //    //---------------------------------------------------------------------
    //    // Screen
    //    //---------------------------------------------------------------------
    //    // Called when the engine decides to render the 2D display.
    //    void        (*RenderScreen) (void);
    //    // Called when the screen mode has changed.
    //    void        (*ScreenModeChanged) (void);
    //    // Called when the client wants to render the loading screen.
    //    void        (*DrawLoadScreen) (void);
    //    // Called when the client wants to render the pause screen.
    //    void        (*DrawPauseScreen) (void);
    //    

    //    //---------------------------------------------------------------------
    //    // View
    //    //---------------------------------------------------------------------
    //    // Called right after the engine clears the scene, and begins a new one.
    //    void        (*PreRenderView) (void);
    //    // Called whenever the engine wants to clear the scene.
    //    void        (*ClearScene) (void);
    //    // Called whenever the engine wants to render a valid frame.
    //    void        (*RenderView) (void);
    //    // Called right after the engine renders the scene, and prepares to
    //    // finish up its current frame loop iteration.
    //    void        (*PostRenderView) (void);
    //} ClientGameExport;

    // Structure containing all the engine function pointers for the client dll to work with.
    typedef struct clg_import_s {
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
        struct {
            int32_t major;
            int32_t minor;
            int32_t point;
        } apiversion;

        //---------------------------------------------------------------------
        // Client.
        //---------------------------------------------------------------------
        // Returns the current amount of seconds since the last frame.
        float           (*GetFrameTime) (void);
        // Returns the amount of miliseconds since the start of client.
        unsigned        (*GetRealTime) (void);

        int             (*GetFramesPerSecond) (void);
        int             (*GetResolutionScale) (void);

        // Returns the server state.
        int             (*GetServerState) (void);
        // Returns the protocol type version.
        int             (*GetServerProtocol) (void);
        // Returns the protocol minor version.
        int             (*GetProtocolVersion) (void);

        // Returns true if we're in demo playback, false otherwise.
        qboolean        (*IsDemoPlayback) (void);
        
        // Updates the client's audio position values to the current
        // cl.refdef.vieworg, cl->v_forward, cl->v_right and cl->v_up
        // values.
        void            (*UpdateListenerOrigin) (void);

        //
        // Client state.
        // Sets the client load state.
        void            (*SetClientLoadState) (LoadState state);
        // Returns the current state of the client.
        uint32_t        (*GetClienState) (void);

        // Checks if the name of the player is on the client's ignore list.
        qboolean        (*CheckForIgnore) (const char *s);
        // Add scanned out IP address to circular array of recent addresses.
        void            (*CheckForIP) (const char* s);

        //---------------------------------------------------------------------
        // Command Buffer.
        //---------------------------------------------------------------------
        // Adds text at the end of command buffer.
        void        (*Cbuf_AddText) (char *text);
        // Inserts text at the beginning of the command buffer.
        void        (*Cbuf_InsertText) (char *text);
        // Executes the current command buffer.
        void        (*Cbuf_Execute) ();
        // Adds the current command line text as a clc_stringcmd to the client 
        // message. Things like godmode, noclip, etc, are commands directed to 
        // the server, so when they are typed in at the console, they will 
        // need to be forwarded.
        qboolean    (*CL_ForwardToServer) ();
                 
        //---------------------------------------------------------------------
        // Collision Model.
        //---------------------------------------------------------------------
        // Creates a clipping hull for an arbitrary box.
        mnode_t     *(*CM_HeadnodeForBox) (const vec3_t &mins, const vec3_t &maxs);
        // We need a way to share these values to cgame dll.
        mmodel_t    *(*CM_InlineModel) (cm_t *cm, const char *name);
        // TODO: Document.
        int         (*CM_PointContents) (const vec3_t &p, mnode_t *headNode);
        int         (*CM_TransformedPointContents) (const vec3_t &p, mnode_t *headNode,
                                                    const vec3_t &origin, const vec3_t &angles);
        void        (*CM_BoxTrace)(trace_t *trace, const vec3_t &start, const vec3_t &end,
                                    const vec3_t &mins, const vec3_t &maxs,
                                    mnode_t *headNode, int brushmask);
        void        (*CM_TransformedBoxTrace) (trace_t *trace, const vec3_t &start, const vec3_t &end,
                                                const vec3_t &mins, const vec3_t &maxs,
                                                mnode_t * headNode, int brushmask,
                                                const vec3_t &origin, const vec3_t &angles);
        void        (*CM_ClipEntity) (trace_t* dst, const trace_t* src, struct entity_s* ent);

        //---------------------------------------------------------------------
        // Command
        //---------------------------------------------------------------------
        // Adds the function with the command name to the list of client commands.
        void        (*Cmd_AddCommand) (const char *cmd_name, xcommand_t function);
        // Removes the command name from the client command list.
        void        (*Cmd_RemoveCommand) (const char *cmd_name);

        // Registers the list of commands to the client command list.
        void        (*Cmd_Register) (const cmdreg_t* reg);
        // Deregisters the list of commands to the client command list.
        void        (*Cmd_Unregister) (const cmdreg_t* reg);

        // Adds a macro command to the list of client macros.
        void            (*Cmd_AddMacro) (const char* name, xmacro_t function);
        // Finds the macro matching the name, and returns a pointer to it.
        cmd_macro_t*    (*Cmd_FindMacro) (const char* name);
        // TODO: Document.
        void            (*Cmd_Macro_g) (genctx_t* ctx);

        // Adds a match for generating in the cmd prompt.
        qboolean        (*Prompt_AddMatch) (genctx_t* ctx, const char* s);

        // Takes a null terminated string.  Does not need to be \n terminated.
        // breaks the string up into arg tokens.
        void        (*Cmd_TokenizeString) (const char *text, qboolean macroExpand);
        // Returns the amount of arguments fed to the current command.
        int         (*Cmd_Argc) (void);
        // Returns the value of the argument number fed to the current command.
        const char  *(*Cmd_Argv) (int arg);
        // Returns the original argument string fed to the current command.
        const char  *(*Cmd_Args) (void);
        // Executes matching cmd triggers.
        void        (*Cmd_ExecTrigger) (const char* string);


        //---------------------------------------------------------------------
        // Common.
        // 
        // NOTE: Look at clg_local.h to see documentation for the following:
        // Com_Error, Com_LPrintf
        //---------------------------------------------------------------------
        void		(*Com_Error) (ErrorType code, const char *fmt, ...);
	    void		(*Com_LPrintf) (PrintType type, const char *fmt, ...);

        // Returns a string description value of the given qerror_t type.
        const char  *(*Com_ErrorString) (qerror_t type);
        // Returns the event time between "common event frames" engine internal.
        unsigned    (*Com_GetEventTime) (void);

        //---------------------------------------------------------------------
        // Console.
        //---------------------------------------------------------------------
        void        (*Con_ClearNotify) (void);
        void        (*Con_SkipNotify) (qboolean skip);

        //---------------------------------------------------------------------
        // CVar.
        //---------------------------------------------------------------------
        // Creates the variable if it doesn't exist, or returns the existing one
        // if it exists, the value will not be changed, but flags will be ORed in
        // that allows variables to be unarchived without needing bitflags.
        cvar_t      *(*Cvar_Get) (const char *var_name, const char *value, int flags);
        // Creates weak variable without value.
        cvar_t      *(*Cvar_WeakGet) (const char *var_name);
        // Returns true if the cvar exists, false otherwise.
        // If weak is true, it'll also return true for weak cvars.
        qboolean    (*Cvar_Exists) (const char *name, qboolean weak);

        // Returns the float value of the cvar.
        float       (*Cvar_VariableValue) (const char *var_name);
        // Returns the integer value of the cvar.
        // NOTE: returns 0 if not defined or non numeric.
        int         (*Cvar_VariableInteger) (const char *var_name);
        // Returns the string value of the cvar.
        const char  *(*Cvar_VariableString) (const char *var_name);

        // Will create the variable if it doesn't exist.
        // Sets the string value.
        cvar_t      *(*Cvar_Set) (const char *var_name, const char *value);
        // Sets a floating point value on the cvar.
        void        (*Cvar_SetValue) (cvar_t *var, float value, from_t from);
        // Sets an integer value on the cvar.
        void        (*Cvar_SetInteger) (cvar_t *var, int value, from_t from);
        // Will set the variable even if NOSET or LATCH.
        cvar_t      *(*Cvar_UserSet) (const char *var_name, const char *value);
        // Resets the cvar to its default value.
        void        (*Cvar_Reset) (cvar_t *var);

        // Clamps the cvar in the integer range min and max.
        int         (*Cvar_ClampInteger) (cvar_t *var, int min, int max);
        // Clamps the cvar in the float range min and max.
        float       (*Cvar_ClampValue) (cvar_t *var, float min, float max);

        // TODO: Document.
        void        (*Cvar_Variable_g) (genctx_t* ctx);
        void        (*Cvar_Default_g) (genctx_t* ctx);

        //---------------------------------------------------------------------
        // Filesystem.
        //---------------------------------------------------------------------
        // Renames the file contained in string from, to the one in string to.
        qerror_t    (*FS_RenameFile) (const char *from, const char *to);
        // Creates the directories of the given path.
        qerror_t    (*FS_CreatePath) (char *path);
        
        // Opens the given filename and stores it in the passed to handle.
        // Returns 0 in case the file could not open.
        ssize_t     (*FS_FOpenFile) (const char *filename, qhandle_t *f, unsigned mode);
        // Closes the given file handle.
        void        (*FS_FCloseFile) (qhandle_t f);
        // A wrapper function for opening a file in a given directory,
        // with the given name, and extension.
        qhandle_t   (*FS_EasyOpenFile) (char *buf, size_t size, unsigned mode,
                                const char *dir, const char *name, const char *ext);
        
        // Checks if the given file exists.
        qhandle_t   (*FS_FileExists) (const char *path);
        // Check if the given file exists, matching the given flags.
        qhandle_t   (*FS_FileExistsEx) (const char *path, unsigned flags);
        
        // Writes data to of a given length to the file(path).
        qerror_t    (*FS_WriteFile) (const char *path, const void *data, size_t len);
        // Another easy function, should be understandable now.
        qboolean    (*FS_EasyWriteFile) (char *buf, size_t size, unsigned mode,
                                 const char *dir, const char *name, const char *ext,
                                 const void *data, size_t len);
        // Read the given length from the file handle into the given buffer.
        ssize_t     (*FS_Read) (void *buffer, size_t len, qhandle_t f);
        // Write from the given buffer into the given file with given length.
        ssize_t     (*FS_Write) (const void *buffer, size_t len, qhandle_t f);
        // properly handles partial reads
        // File printf.
        ssize_t     (*FS_FPrintf) (qhandle_t f, const char *format, ...);
        // Read file line into buffer.
        ssize_t     (*FS_ReadLine) (qhandle_t f, char *buffer, size_t size);
        // Flushes file handle.
        void        (*FS_Flush) (qhandle_t f);
        // Tell where file handle is.
        ssize_t     (*FS_Tell) (qhandle_t f);
        // Seek in file handle to given offset.
        qerror_t    (*FS_Seek) (qhandle_t f, off_t offset);
        // Retrieve given file handle length.
        ssize_t     (*FS_Length) (qhandle_t f);
        
        // Wildcard comparison.
        qboolean    (*FS_WildCmp) (const char *filter, const char *string);
        // Extension comparison.
        qboolean    (*FS_ExtCmp) (const char *extension, const char *string);
        // Last modified.
        qerror_t    (*FS_LastModified) (char const * file, uint64_t * last_modified);
        
        // File list functionality.
        void        **(*FS_ListFiles) (const char *path, const char *filter, unsigned flags, int *count_p);
        void        **(*FS_CopyList) (void **list, int count);
        file_info_t *(*FS_CopyInfo) (const char *name, size_t size, time_t ctime, time_t mtime);
        void        (*FS_FreeList) (void **list);
        // Normalize path.
        size_t      (*FS_NormalizePath) (char *out, const char *in);
        size_t      (*FS_NormalizePathBuffer) (char *out, const char *in, size_t size);
        // Validate path.
        int         (*FS_ValidatePath) (const char *s);
        // Sanitize cvar filename variable, so it is safe to use.
        void        (*FS_SanitizeFilenameVariable) (cvar_t *var);


        //---------------------------------------------------------------------
        // Key/User input.
        //---------------------------------------------------------------------
        //// Returns whether a key is down or not.
        //int         (*Key_IsDown) (int key);
        //// Returns total number of keys down.
        //int         (*Key_AnyKeyDown) (void);
        //// Returns the ASCII value of the key belonging to the binding.
        //const char  *(*Key_GetBinding) (const char* binding);
        // Returns whether in overstrike mode.
        qboolean    (*Key_GetOverstrikeMode) (void);
        // Sets key in overstrike mode.
        void        (*Key_SetOverstrikeMode) (qboolean overstrike);
        // Returns the current client state key destination.
        keydest_t(*Key_GetDest) (void);
        // Sets the key destination.
        void        (*Key_SetDest) (keydest_t dest);
        // Returns key down status: if > 1, it is auto-repeating
        int         (*Key_IsDown) (int key);
        // Returns total number of keys down.
        int         (*Key_AnyKeyDown) (void);
        // Returns a key number to be used to index keybindings[] by looking at
        // the given string.  Single ascii characters return themselves, while
        // the K_* names are matched up.
        int         (*Key_StringToKeynum) (const char* str);
        // Returns a string (either a single ascii char, or a K_* name) for the
        // given keynum.
        const char* (*Key_KeynumToString) (int keynum);
        //Returns the name of the first key found.
        const char* (*Key_GetBinding) (const char* binding);
        // Returns the command bound to a given key.
        const char* (*Key_GetBindingForKey) (int keynum);
        // Fills the binding string with the name of the binding matching to the key.
        // Returns -1 in case nothing was found.
        int         (*Key_EnumBindings) (int key, const char* binding);
        // Sets keybinding for the given keynum.
        void        (*Key_SetBinding) (int keynum, const char* binding);

        //---------------------------------------------------------------------
        // Mouse.
        //---------------------------------------------------------------------
        qboolean        (*Mouse_GetMotion) (int *deltaX, int *deltaY);

        //---------------------------------------------------------------------
        // Memory.
        //---------------------------------------------------------------------
        // TODO: Document.
        void        *(*Z_TagMalloc) (size_t size, memtag_t tag);
        // TODO: Document.
        void        *(*Z_TagMallocz) (size_t size, memtag_t tag);
        // TODO: Document.
        void        (*Z_TagReserve) (size_t size, memtag_t tag);
        // TODO: Document.
        char        *(*Z_TagCopyString) (const char* in, memtag_t tag);
        // TODO: Document.
        void        (*Z_Free) (void* ptr);


        //---------------------------------------------------------------------
        // Messaging.
        //---------------------------------------------------------------------
        // TODO.


        //---------------------------------------------------------------------
        // Network.
        //---------------------------------------------------------------------
        // Reads a character from the network.
        int			(*MSG_ReadChar) (void);
        // Reads a byte from the network.
        int			(*MSG_ReadByte) (void);
        // Reads a short from the network.
        int			(*MSG_ReadShort) (void);
        // Reads a word from the network.
        int 		(*MSG_ReadWord) (void);
        // Reads a long from the network.
        int			(*MSG_ReadLong) (void);
        // Reads a string from the network.
        size_t		(*MSG_ReadString) (char *dest, size_t size);
        // Reads a vector3 from the network.
        vec3_t		(*MSG_ReadVector3) (void);

        // Writes a character over the network.
        void        (*MSG_WriteChar) (int c);
        // Writes a byte over the network.
        void        (*MSG_WriteByte) (int c);
        // Writes a short over the network.
        void        (*MSG_WriteShort) (int c);
        // Writes a long over the network.
        void        (*MSG_WriteLong) (int c);
        // Writes a string over the network.
        void        (*MSG_WriteString) (const char *s);
        // Writes a position over the network.
        void        (*MSG_WriteVector3) (const vec3_t &pos);
        // Flushes message.
        void        (*MSG_FlushTo) (SizeBuffer *buf);
        
        //---------------------------------------------------------------------
        // Registering.
        //---------------------------------------------------------------------
        // Precaches a model, and returns its handle.
        qhandle_t       (*R_RegisterModel) (const char *name);
        // Precaches an image(3d use, filters etc), and returns its handle.
        qhandle_t       (*R_RegisterImage) (const char *name, imagetype_t type,
                                        imageflags_t flags, qerror_t *err_p);
        // More advanced version of R_RegisterImage.
        qhandle_t       (*R_RegisterRawImage) (const char *name, int width, int height, byte* pic, imagetype_t type,
                                        imageflags_t flags);
        // Uncaches the image matching the given handle.
        void            (*R_UnregisterImage) (qhandle_t handle);

        // Precaches a 2D image, and returns its handle.
        qhandle_t       (*R_RegisterPic) (const char *name);
        // Precaches a 2D image permanently and returns its handle.
        qhandle_t       (*R_RegisterPic2) (const char *name);
        // Precache a font, returns its handle. These are permanent by default.
        qhandle_t       (*R_RegisterFont) (const char *name);
        // Precaches a skin, and returns its handle.
        qhandle_t       (*R_RegisterSkin) (const char *name);

        // Returns a pointer to the model based on the given handle.
        model_t         *(*MOD_ForHandle) (qhandle_t h);

        //---------------------------------------------------------------------
        // Rendering.
        //---------------------------------------------------------------------
        void        (*R_AddDecal) (decal_t* d);
        void        (*R_LightPoint) (const vec3_t &origin, vec3_t &light);
        void        (*R_SetSky)(const char* name, float rotate, vec3_t &axis);

        void        (*R_ClearColor) (void);
        void        (*R_SetAlpha) (float clpha);
        void        (*R_SetAlphaScale) (float alpha);
        void        (*R_SetColor) (uint32_t color);
        void        (*R_SetClipRect) (const clipRect_t* clip);
        float       (*R_ClampScale) (cvar_t* var);
        void        (*R_SetScale) (float scale);
        void        (*R_DrawChar) (int x, int y, int flags, int ch, qhandle_t font);
        int         (*R_DrawString) (int x, int y, int flags, size_t maxChars,
                                    const char* string, qhandle_t font);  // returns advanced x coord
        qboolean    (*R_GetPicSize) (int* w, int* h, qhandle_t pic);   // returns transparency bit
        void        (*R_DrawPic) (int x, int y, qhandle_t pic);
        void        (*R_DrawStretchPic) (int x, int y, int w, int h, qhandle_t pic);
        void        (*R_TileClear) (int x, int y, int w, int h, qhandle_t pic);
        void        (*R_DrawFill8) (int x, int y, int w, int h, int c);
        void        (*R_DrawFill32) (int x, int y, int w, int h, uint32_t color);

        //---------------------------------------------------------------------
        // 2D Rendering.
        //---------------------------------------------------------------------
        // TODO:

        //---------------------------------------------------------------------
        // Screen.
        //---------------------------------------------------------------------
        // This is called every frame by the client itself. However in the case
        // of this CG Module, it can also be called explicitly to flush text to
        // the screen.
        void            (*SCR_UpdateScreen) (void);

        //---------------------------------------------------------------------
        // Sound.
        //---------------------------------------------------------------------
        // Begins the sound registration process.
        void            (*S_BeginRegistration) (void);
        // Precaches a sound with the given filename. (Can be a path.)
        qhandle_t       (*S_RegisterSound) (const char *name);
        // Ends the sound registration process.
        void            (*S_EndRegistration) (void);

        // Plays a sound at the given origin. If origin is NULL, the sound will
        // be dynamically sourced from the entity.
        void            (*S_StartSound)(const vec3_t *origin, int entnum, int entchannel,
                                       qhandle_t sfx, float fvol, float attenuation, float timeofs);
        // Plays a local 2D sound on entchannel 0.                               
        void            (*S_StartLocalSound) (const char *s);
        // Plays a local 2D sound on entchannel 256.                               
        void            (*S_StartLocalSound_) (const char* s);

        // Enables under water special audio effect.
        void            (*SFX_Underwater_Enable) (void);
        // Disables under water special audio effect.
        void            (*SFX_Underwater_Disable) (void);

        //---------------------------------------------------------------------
        // System.
        //---------------------------------------------------------------------
        unsigned        (*Sys_Milliseconds) (void);

        //
        // Pointers to actual client data.
        //
        // Client State.
        ClientState *cl;
        // Client Shared.
        ClientShared* cs;
    } ClientGameImport;

    // Function pointer type for handling the actual import function.
    typedef IClientGameExports (*GetClientGameAPI_t) (ClientGameImport*);
#ifdef __cplusplus
};  // Extern C.
#endif

#endif // __INC_SHARED_CL_GAME_H__