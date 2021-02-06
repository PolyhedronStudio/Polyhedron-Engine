// LICENSE.

//
// clgame.c
//
//
// Takes care of importing the client game dll function pointer struct,
// and exports the engine function pointer struct. 
//
// Also handles wrapping up certain functions which are usually defined
// while they shouldn't be. But that's the old ways of C, so we'll just
// workaround that for now.
//
// Also contains the function definitions for all the game client module 
// interactionfunctionality.
//
// TODO: On a sunny day, take some rest. BUT On a rainy sunday, try and 
// replace all the defined function bodies with an actual function. 
// This way, we can remove the _wrp_ functions from this file and have 
// the function pointers point to the actual functions.
//
#include "client.h"
#include "server/server.h"
#include "client/gamemodule.h"   // TODO: How come it can find client.h??
#include "shared/cl_types.h"
#include "shared/cl_game.h"

// Contains the function s being exported to client game dll.
static clg_export_t *cge;

// Operating System handle to the cgame library.
static void *cgame_library;

//
//=============================================================================
//
//	WRAPPER CODE FOR CERTAIN FUNCTIONALITIES SUCH AS COMMAND BUFFERS ETC.
//
//=============================================================================
// CBUF_
void _wrp_Cbuf_AddText(char *text) {
	Cbuf_AddText(&cmd_buffer, text);
}
void _wrp_Cbuf_Execute() {
	Cbuf_Execute(&cmd_buffer);
}
void _wrp_Cbuf_InsertText(char *text) {
	Cbuf_InsertText(&cmd_buffer, text);
}

// CVAR_
void _wrp_Cvar_Reset(cvar_t *var) {
	Cvar_Reset(var);
}

// CMODEL
mmodel_t *_wrp_CM_InlineModel(cm_t *cm, const char *name) {
    return CM_InlineModel(cm, name);
}

// FILES
qhandle_t _wrp_FS_FileExists(const char *path) {
    return FS_FileExists(path);
}
qhandle_t _wrp_FS_FileExistsEx(const char *path, unsigned flags) {
    return FS_FileExistsEx(path, flags);
}
ssize_t _wrp_FS_FPrintf(qhandle_t f, const char *format, ...) {
    ssize_t ret;
    va_list args;
    va_start (args, format);
    ret = FS_FPrintf(f, format, args);
    va_end (args);
    return ret;
}

// NETWORKING
void _trp_MSG_FlushTo(sizebuf_t *buf) {
    MSG_FlushTo(buf);
}

// REFRESH - These are wrapped, since some of the actual function pointers 
// get loaded in later in at boot time. 
qhandle_t _wrp_R_RegisterPic(const char *name) {
    return R_RegisterPic(name);
}
qhandle_t _wrp_R_RegisterPic2(const char *name) {
    return R_RegisterPic2(name);
}
qhandle_t _wrp_R_RegisterFont(const char *name) {
    return R_RegisterFont(name);
}
qhandle_t _wrp_R_RegisterSkin(const char *name) {
    return R_RegisterSkin(name);
}

qhandle_t _wrp_R_RegisterModel(const char *name) {
    return R_RegisterModel(name);
}
qhandle_t _wrp_R_RegisterImage(const char *name, imagetype_t type,
                                        imageflags_t flags, qerror_t *err_p) {
    return R_RegisterImage(name, type, flags, err_p);
}
qhandle_t _wrp_R_RegisterRawImage(const char *name, int width, int height, byte* pic, imagetype_t type,
                                        imageflags_t flags) {
    return R_RegisterRawImage(name, width, height, pic, type, flags);
}
void _wrp_R_UnregisterImage(const char *name) {
    R_UnregisterImage(name);
}
 
void _wrp_R_LightPoint(vec3_t origin, vec3_t light) {
    if (R_LightPoint)
        R_LightPoint(origin, light);
    else
        Com_EPrintf("%s - Invalid R_LightPoint func_ptr\n");
}

//
//=============================================================================
//
//	LIBRARY LOADING.
//
//=============================================================================
//
// Utility function.
static void *_CL_LoadGameLibrary(const char *path)
{
    void *entry;

    entry = Sys_LoadLibrary(path, "GetClientGameAPI", &cgame_library);
    if (!entry)
        Com_EPrintf("Failed to load Client Game library: %s\n", Com_GetLastError());
    else
        Com_Printf("Loaded Client Game library from %s\n", path);

    return entry;
}

//
// Utility function.
static void *CL_LoadGameLibrary(const char *game, const char *prefix)
{
    char path[MAX_OSPATH];
    size_t len;

    len = Q_concat(path, sizeof(path), sys_libdir->string,
                   PATH_SEP_STRING, game, PATH_SEP_STRING,
                   prefix, "clgame" LIBSUFFIX, NULL);
    if (len >= sizeof(path)) {
        Com_EPrintf("Client Game library path length exceeded\n");
        return NULL;
    }

    if (os_access(path, F_OK)) {
        if (!*prefix)
            Com_Printf("Can't access %s: %s\n", path, strerror(errno));
        return NULL;
    }

    return _CL_LoadGameLibrary(path);
}

//
//===============
// CL_ShutdownGameProgs
// 
// Called when either the entire client is being killed, or
// it is changing to a different game directory.
// ===============
//
void CL_ShutdownGameProgs(void)
{
    if (cge) {
        cge = NULL;
    }

    if (cgame_library) {
        Sys_FreeLibrary(cgame_library);
        cgame_library = NULL;
    }
    Cvar_Set("CL_GM_features", "0");
}

//
//===============
// CL_InitGameProgs
//
// Init the game client modulke.
//===============
//
void CL_InitGameProgs(void)
{
    clg_import_t   import;
    clg_export_t   *(*entry)(clg_import_t *) = NULL;

    // unload anything we have now
    CL_ShutdownGameProgs();

    // for debugging or `proxy' mods
    if (sys_forcecgamelib->string[0])
        entry = _CL_LoadGameLibrary(sys_forcecgamelib->string);

    // try game first
    if (!entry && fs_game->string[0]) {
        entry = CL_LoadGameLibrary(fs_game->string, "");
    }

    // then try basenac
    if (!entry) {
        entry = CL_LoadGameLibrary(BASEGAME, "");
    }

    // all paths failed
    if (!entry)
        Com_Error(ERR_DROP, "Failed to load Client Game library");

	//
    // Setup the function pointers for the cgame dll.
	//
    import.cl                           = &cl;

	// Command Buffer.
	import.Cbuf_AddText					= _wrp_Cbuf_AddText;
	import.Cbuf_InsertText				= _wrp_Cbuf_InsertText;
	import.Cbuf_Execute					= _wrp_Cbuf_Execute;
	import.CL_ForwardToServer			= CL_ForwardToServer;

    // Collision Model.
    import.CM_HeadnodeForBox            = CM_HeadnodeForBox;
    import.CM_InlineModel               = _wrp_CM_InlineModel;
    import.CM_PointContents             = CM_PointContents;
    import.CM_TransformedPointContents  = CM_TransformedPointContents;
    import.CM_BoxTrace                  = CM_BoxTrace;
    import.CM_TransformedBoxTrace       = CM_TransformedBoxTrace;

	// Command.
	import.Cmd_AddCommand				= Cmd_AddCommand;
	import.Cmd_RemoveCommand			= Cmd_RemoveCommand;
	import.Cmd_TokenizeString			= Cmd_TokenizeString;
	import.Cmd_Argc						= Cmd_Argc;
	import.Cmd_Argv						= Cmd_Argv;
	import.Cmd_Args						= Cmd_Args;

	// Common.
    import.Com_Error 					= Com_Error;
    import.Com_LPrintf 					= Com_LPrintf;

    import.Com_ErrorString              = Q_ErrorString;

    import.Com_SetClientLoadState       = CL_SetLoadState;
    import.Com_GetClientState           = CL_GetState;
    import.Com_SetClientState           = CL_SetState;

    import.Com_GetServerState           = SV_GetState;
    import.Com_SetServerState           = SV_SetState;

	// Cvar.
	import.Cvar_Get						= Cvar_Get;
	import.Cvar_WeakGet					= Cvar_WeakGet;
	import.Cvar_Exists					= Cvar_Exists;
	import.Cvar_VariableValue			= Cvar_VariableValue;
	import.Cvar_VariableInteger			= Cvar_VariableInteger;
	import.Cvar_VariableString			= Cvar_VariableString;
	import.Cvar_Set						= Cvar_Set;
	import.Cvar_SetValue				= Cvar_SetValue;
	import.Cvar_SetInteger				= Cvar_SetInteger;
	import.Cvar_UserSet					= Cvar_UserSet;
	import.Cvar_Reset					= _wrp_Cvar_Reset;
	import.Cvar_ClampInteger			= Cvar_ClampInteger;
	import.Cvar_ClampValue				= Cvar_ClampValue;

    // Files.
    import.FS_RenameFile                = FS_RenameFile;
    import.FS_CreatePath                = FS_CreatePath;
    import.FS_FOpenFile                 = FS_FOpenFile;
    import.FS_FCloseFile                = FS_FCloseFile;
    import.FS_EasyOpenFile              = FS_EasyOpenFile;
    import.FS_FileExists                = _wrp_FS_FileExists;
    import.FS_FileExistsEx              = _wrp_FS_FileExistsEx;
    import.FS_WriteFile                 = FS_WriteFile;
    import.FS_EasyWriteFile             = FS_EasyWriteFile;
    import.FS_Read                      = FS_Read;
    import.FS_Write                     = FS_Write;
    import.FS_FPrintf                   = _wrp_FS_FPrintf;
    import.FS_ReadLine                  = FS_ReadLine;
    import.FS_Flush                     = FS_Flush;
    import.FS_Tell                      = FS_Tell;
    import.FS_Seek                      = FS_Seek;
    import.FS_Length                    = FS_Length;
    import.FS_WildCmp                   = FS_WildCmp;
    import.FS_ExtCmp                    = FS_ExtCmp;
    import.FS_LastModified              = FS_LastModified;
    import.FS_ListFiles                 = FS_ListFiles;
    import.FS_CopyList                  = FS_CopyList;
    import.FS_CopyInfo                  = FS_CopyInfo;
    import.FS_FreeList                  = FS_FreeList;
    import.FS_NormalizePath             = FS_NormalizePath;
    import.FS_NormalizePathBuffer       = FS_NormalizePathBuffer;
    import.FS_ValidatePath              = FS_ValidatePath;
    import.FS_SanitizeFilenameVariable  = FS_SanitizeFilenameVariable;

    // Networking.
    import.MSG_ReadChar                 = MSG_ReadChar;
    import.MSG_ReadByte                 = MSG_ReadByte;
    import.MSG_ReadShort                = MSG_ReadShort;
    import.MSG_ReadWord                 = MSG_ReadWord;
    import.MSG_ReadLong                 = MSG_ReadLong;
    import.MSG_ReadString               = MSG_ReadString;
    import.MSG_ReadDir                  = MSG_ReadDir;
    import.MSG_ReadPos                  = MSG_ReadPos;
    import.MSG_WriteChar                = MSG_WriteChar;
    import.MSG_WriteByte                = MSG_WriteByte;
    import.MSG_WriteShort               = MSG_WriteShort;
    import.MSG_WriteLong                = MSG_WriteLong;
    import.MSG_WriteString              = MSG_WriteString;
    import.MSG_WritePos                 = MSG_WritePos;
    import.MSG_WriteAngle               = MSG_WriteAngle;
    
    // Register.
    import.R_RegisterModel              = _wrp_R_RegisterModel;
    import.R_RegisterImage              = _wrp_R_RegisterImage;
    import.R_RegisterRawImage           = _wrp_R_RegisterRawImage;
    import.R_UnregisterImage            = _wrp_R_UnregisterImage;

    import.R_RegisterPic                = _wrp_R_RegisterPic;
    import.R_RegisterPic2               = _wrp_R_RegisterPic2;
    import.R_RegisterFont               = _wrp_R_RegisterFont;
    import.R_RegisterSkin               = _wrp_R_RegisterSkin;

    // Rendering
    import.R_LightPoint                 = _wrp_R_LightPoint;

    // Sound.
    import.S_BeginRegistration          = S_BeginRegistration;
    import.S_RegisterSound              = S_RegisterSound;
    import.S_EndRegistration            = S_EndRegistration;

    import.S_StartSound                 = S_StartSound;
    import.S_StartLocalSound            = S_StartLocalSound;

	// Load up the cgame dll.
    cge = entry(&import);
    if (!cge) {
        Com_Error(ERR_DROP, "Client Game DLL returned NULL exports");
    }

    if (cge->apiversion != CGAME_API_VERSION) {
        Com_Error(ERR_DROP, "Client Game DLL is version %d, expected %d",
                  cge->apiversion, CGAME_API_VERSION);
    }

    // We will not be calling the init function here, since we want to actually initialize later.
}

//
//=============================================================================
//
//	Client Game Module GLUE Functions.
//
//=============================================================================
//

//
//===============
// CL_GM_Init
// 
// Called by the client when it is ready initializing.
//===============
//
void CL_GM_Init (void) {
    if (cge)
        cge->Init();
}

//
//===============
// CL_GM_Shutdown
//
// Called by the client right before shutting down.
//===============
//
void CL_GM_Shutdown (void) {
    if (cge)
        cge->Shutdown();
}

//
//===============
// CL_GM_StartServerMessage
// 
// Called by the client BEFORE all server messages have been parsed
//===============
//
void CL_GM_StartServerMessage (void) {
    if (cge)
        cge->StartServerMessage();
}

//
//===============
// CL_GM_ParseServerMessage
// 
// Parses command operations known to the game dll
// Returns true if the message was parsed
//===============
//
qboolean CL_GM_ParseServerMessage (int serverCommand) {
    if (cge)
        return cge->ParseServerMessage(serverCommand);

    return qfalse;
}

//
//===============
// CL_GM_ParseServerMessage
// 
// Parses command operations known to the game dll, but for
// demo playback only. This means certain commands such as
// svc_centerprint can be skipped.
//===============
//
qboolean CL_GM_ParseDemoMessage (int serverCommand) {
    // TODO: IMPLEMENT...
    return qtrue;
}

//
//===============
// CL_GM_EndServerMessage
// 
// Called by the client AFTER all server messages have been parsed
//===============
//
void CL_GM_EndServerMessage () {
    if (cge)
        cge->EndServerMessage(cls.realtime);
}

//
//===============
// CL_GM_InitMedia
// 
// Call into the CG Module for notifying about "Init Media"
//===============
//
void CL_GM_InitMedia(void)
{
    if (cge)
        cge->InitMedia();
}

//
//==============
// GetMediaLoadStateName
//
// Call into the CG Module for notifying about "Media Load State Name"
//===============
//
const char *CL_GM_GetMediaLoadStateName(load_state_t state)
{
    if (cge)
        return cge->GetMediaLoadStateName(state);
    else
        return "";
}

//
//===============
// CL_GM_LoadScreenMedia
// 
// Call into the CG Module for notifying about "Register Screen Media"
//===============
//
void CL_GM_LoadScreenMedia(void)
{
    if (cge)
        cge->LoadScreenMedia();
}

//
//===============
// CL_GM_LoadWorldMedia
// 
// Call into the CG Module for notifying about "Register Screen Media"
//===============
//
void CL_GM_LoadWorldMedia(void)
{
    if (cge)
        cge->LoadWorldMedia();
}


//
//===============
// CL_GM_ShutdownMedia
// 
// Call into the CG Module for notifying about "Shutdown Media"
//===============
//
void CL_GM_ShutdownMedia (void) {
    if (cge)
        cge->ShutdownMedia();
}

//
//===============
// CL_GM_PreRenderView
// 
// Call into the CG Module for notifying about "Pre Render View"
//===============
//
void CL_GM_PreRenderView () {
    if (cge)
        cge->PreRenderView();
}

//
//===============
// CL_GM_RenderView
// 
// Call into the CG Module for notifying about "Render View"
//===============
//
void CL_GM_RenderView () {
    if (cge)
        cge->RenderView();
}

//
//===============
// CL_GM_PostRenderView
// 
// Call into the CG Module for notifying about "Post Render"
//===============
//
void CL_GM_PostRenderView () {
    if (cge)
        cge->PostRenderView();
}