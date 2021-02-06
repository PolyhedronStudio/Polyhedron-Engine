/*
// LICENSE HERE.

//
// clg_game.h
//
// Contains the client game related code that is shared with and by the engine.
//
*/
#ifndef __CLGAME_CGAME_H__
#define __CLGAME_CGAME_H__

#define CGAME_API_VERSION 1

//
//=============================================================================
//
// SHARED ENGINE/CLIENT DEFINITIONS.
//
// These should never be tampered with, unless you intend to recompile the
// whole engine afterwards, with the risk of possibly breaking any 
// compatibility with current mods.
//=============================================================================
//
// TODO: Luckily there are none here yet! ;-)

//
//=============================================================================
//
//	CLIENT GAME IMPORT AND EXPORT STRUCTURES.
//
//=============================================================================
//
#ifdef __cplusplus
extern "C" {
#endif
    // Structure containing all the client dll game function pointers for the engine to work with.
    typedef struct clg_export_s {
        //---------------------------------------------------------------------
        // API Version.
        //---------------------------------------------------------------------
        // Should always be the same as the import's struct api_version.
        int apiversion;


        //---------------------------------------------------------------------
        // Core.
        //---------------------------------------------------------------------
        // Initializes the client game dll.
        void		(*Init) ();
        // Shuts down the client game dll.
        void		(*Shutdown) (void); 

        //---------------------------------------------------------------------
        // Core.
        //---------------------------------------------------------------------
        // Called when the client wants to know the name of a custom load stat.
        const char  *(*GetMediaLoadStateName) (load_state_t state);
        // Called when the renderer initializes.
        void        (*InitMedia) (void);
        // Called whenever the screen media has te reinitialize.
        // Load all HUD/Menu related media here. 2D Images, sounds
        // for HUD, etc.
        void        (*LoadScreenMedia) (void);
        // Called during map load, register world data here such as particles,
        // view models, sounds for entities, etc.
        void        (*LoadWorldMedia) (void);
        // Called when the renderer shutsdown. Should unload all media.
        void        (*ShutdownMedia) (void);

        //---------------------------------------------------------------------
        // ServerMessage Parsing.
        //---------------------------------------------------------------------
        // Called at the start of receiving a server message.
        void        (*StartServerMessage) (void);
        // Actually parses the server message, and handles it accordingly.
        qboolean    (*ParseServerMessage) (int serverCommand);
        // Called when we're done receiving a server message.
        void        (*EndServerMessage) (int realTime);

        //---------------------------------------------------------------------
        // View
        //---------------------------------------------------------------------
        // Called right after the engine clears the scene, and begins a new one.
        void        (*PreRenderView) (void);
        // Called whenever the engine wants to render a valid frame.
        void        (*RenderView) (void);
        // Called right after the engine renders the scene, and prepares to
        // finish up its current frame loop iteration.
        void        (*PostRenderView) (void);

    } clgame_export_t;

    // Structure containing all the engine function pointers for the client dll to work with.
    typedef struct clg_import_s {
        //---------------------------------------------------------------------
        // API Version.
        //---------------------------------------------------------------------
        int apiversion;                // Should always be the same as the extport's struct api_version.

        //---------------------------------------------------------------------
        // Command Buffer.
        //---------------------------------------------------------------------
        // Adds text at the end of command buffer.
        void        (*Cbuf_AddText) (char *text);
        // Inserts text at the beginning of the command buffer.
        void        (*Cbuf_InsertText) (char *text);
        // Executes the current command buffer.
        void        (*Cbuf_Execute) ();
        // Forwards current command buffer to the server for processing (skips client processing).
        qboolean    (*CL_ForwardToServer) ();


        //---------------------------------------------------------------------
        // Collision Model.
        //---------------------------------------------------------------------
        // Creates a clipping hull for an arbitrary box.
        struct mnode_t     *(*CM_HeadnodeForBox) (vec3_t mins, vec3_t maxs);
        // We need a way to share these values to cgame dll.
        mmodel_t    *(*CM_InlineModel) (cm_t *cm, const char *name);
        // TODO: Document.
        int         (*CM_PointContents) (vec3_t p, mnode_t *headnode);
        int         (*CM_TransformedPointContents) (vec3_t p, mnode_t *headnode,
                                                    vec3_t origin, vec3_t angles);
        void        (*CM_BoxTrace)(trace_t *trace, vec3_t start, vec3_t end,
                                    vec3_t mins, vec3_t maxs,
                                    mnode_t *headnode, int brushmask);
        void        (*CM_TransformedBoxTrace) (trace_t *trace, vec3_t start, vec3_t end,
                                                vec3_t mins, vec3_t maxs,
                                                mnode_t * headnode, int brushmask,
                                                vec3_t origin, vec3_t angles);

        //---------------------------------------------------------------------
        // Command
        //---------------------------------------------------------------------
        // Adds the function with the command name to the list of client commands.
        void        (*Cmd_AddCommand) (const char *cmd_name, xcommand_t function);
        // Removes the command name from the client command list.
        void        (*Cmd_RemoveCommand) (const char *cmd_name);

        // Takes a null terminated string.  Does not need to be \n terminated.
        // breaks the string up into arg tokens.
        void        (*Cmd_TokenizeString) (char *text, qboolean macroExpand);
        // Returns the amount of arguments fed to the current command.
        int         (*Cmd_Argc) (void);
        // Returns the value of the argument number fed to the current command.
        char        *(*Cmd_Argv) (int arg);
        // Returns the original argument string fed to the current command.
        char        *(*Cmd_Args) (void);


        //---------------------------------------------------------------------
        // Common.
        // 
        // NOTE: Look at clg_local.h to see documentation for the following:
        // Com_Error, Com_LPrintf
        //---------------------------------------------------------------------
        void		(*Com_Error) (error_type_t code, char *fmt, ...);
	    void		(*Com_LPrintf) (print_type_t type, char *fmt, ...);

        // Returns a string description value of the given qerror_t type.
        const char  *(*Com_ErrorString) (qerror_t type);

        // Client state management is used for managing precaching.
        // Sets the client load state.
        void            (*Com_SetClientLoadState) (load_state_t state);
        // Returns the current state of the client.
        connstate_t     (*Com_GetClientState) (void);
        // Sets the current state of the client.
        void            (*Com_SetClientState) (connstate_t state);
        
        // Returns the current state of the server.
        server_state_t	(*Com_GetServerState) (void);
        // Sets the current state of the client.
        void        	(*Com_SetServerState) (server_state_t state);


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
        char        *(*Cvar_VariableString) (const char *var_name);

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
        // TODO.


        //---------------------------------------------------------------------
        // Memory.
        //---------------------------------------------------------------------
        // TODO.


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
        float		(*MSG_ReadWord) (void);
        // Reads a long from the network.
        int			(*MSG_ReadLong) (void);
        // Reads a string from the network.
        size_t		(*MSG_ReadString) (char *dest, size_t size);
        // Reads a direction from the network.
        void		(*MSG_ReadDir) (vec3_t dir);
        // Reads a position from the network.
        void		(*MSG_ReadPos) (vec3_t pos);

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
        void        (*MSG_WritePos) (const vec3_t pos);
        // Writes an angle over the network.
        void        (*MSG_WriteAngle) (float f);
        // Flushes message.
        void        (*MSG_FlushTo) (sizebuf_t *buf);
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

        //---------------------------------------------------------------------
        // Rendering.
        //---------------------------------------------------------------------
        void            (*R_LightPoint) (vec3_t origin, vec3_t light);

        //---------------------------------------------------------------------
        // 2D Rendering.
        //---------------------------------------------------------------------
        // TODO:

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
        void            (*S_StartSound)(const vec3_t origin, int entnum, int entchannel,
                                       qhandle_t sfx, float fvol, float attenuation, float timeofs);
        // Plays a local 2D sound.                               
        void            (*S_StartLocalSound) (const char *s);

        //---------------------------------------------------------------------
        // System.
        //---------------------------------------------------------------------
        // TODO.

                //
        // Pointers to actual client data.
        //
        client_state_t *cl;
        client_test_t *ct;
    } clgame_import_t;

    // Function pointer type for handling the actual import function.
    typedef clgame_export_t (*GetClientGameAPI_t) (clgame_import_t);
#ifdef __cplusplus
};  // Extern C.
#endif

#endif