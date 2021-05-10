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
// interaction functionalities.
//
// TODO: On a sunny day, take some rest. BUT On a rainy sunday, try and 
// replace all the defined function bodies with an actual function. 
// This way, we can remove the _wrp_ functions from this file and have 
// the function pointers point to the actual functions.
//
#include "client.h"
#include "server/server.h"
#include "refresh/models.h"

// (Client/Game) related.
#include "client/gamemodule.h"   // TODO: How come it can find client.h??
#include "shared/cltypes.h"
#include "shared/clgame.h"

// Contains the functions being exported to client game dll.
ClientGameExport *cge;

// Operating System handle to the cgame library.
static void *cgame_library;

// Cheesy externs..
extern int CL_GetFps(void);
extern int CL_GetResolutionScale(void);

//
//=============================================================================
//
//	WRAPPER CODE FOR CERTAIN FUNCTIONALITIES SUCH AS COMMAND BUFFERS ETC.
//
//=============================================================================
// CLIENT.
int _wrp_GetServerProtocol(void) {
    return cls.serverProtocol;
}
int _wrp_GetProtocolVersion(void) {
    return cls.protocolVersion;
}
int _wrp_GetServerState(void) {
    return cl.serverState;
}

unsigned _wrp_GetRealTime(void) {
    return cls.realtime;
}
float _wrp_GetFrameTime(void) {
    return cls.frameTime;
}
qboolean _wrp_IsDemoPlayback(void) {
    return cls.demo.playback;
}

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

// COMMON
unsigned _prt_Com_GetEventTime(void) {
    return com_eventTime;
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

// REGISTER
qhandle_t _wrp_R_RegisterPic(const char *name) {
    // CPP: Explicit cast required.
    return (qhandle_t)R_RegisterPic(name);
}
qhandle_t _wrp_R_RegisterPic2(const char *name) {
    // CPP: Explicit cast required.
    return (qhandle_t)R_RegisterPic2(name);
}
qhandle_t _wrp_R_RegisterFont(const char *name) {
    // CPP: Explicit cast required.
    return (qhandle_t)R_RegisterFont(name);
}
qhandle_t _wrp_R_RegisterSkin(const char *name) {
    // CPP: Explicit cast required.
    return (qhandle_t)R_RegisterSkin(name);
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
void _wrp_R_UnregisterImage(qhandle_t image) {
    R_UnregisterImage(image);
}

// REFRESH - These are wrapped, since some of the actual function pointers 
// get loaded in later in at boot time. 
void _wrp_R_AddDecal(decal_t* d) {
    if (R_AddDecal)
        R_AddDecal(d);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}

void _wrp_R_LightPoint(const vec3_t &origin, vec3_t &light) {
    if (R_LightPoint)
        R_LightPoint(origin, light);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}
void _wrp_R_SetSky(const char* name, float rotate, vec3_t &axis) {
    if (R_SetSky)
        R_SetSky(name, rotate, axis);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}

void _wrp_R_ClearColor(void) {
    if (R_ClearColor)
        R_ClearColor();
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}
void _wrp_R_SetAlpha(float alpha) {
    if (R_SetAlpha)
        R_SetAlpha(alpha);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}
void _wrp_R_SetAlphaScale(float scale) {
    if (R_SetAlphaScale)
        R_SetAlphaScale(scale);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}
void _wrp_R_SetColor(uint32_t color) {
    if (R_SetColor)
        R_SetColor(color);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}
void _wrp_R_SetClipRect(const clipRect_t* clip) {
    if (R_SetClipRect)
        R_SetClipRect(clip);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}
float _wrp_R_ClampScale(cvar_t *var) {
    // CPP: This was a bug I guess in the C code, there is no funcptr check.
    //if (R_ClampScale) {
        return R_ClampScale(var);
    //} else {
    //    Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
    //    return 0.0f;
    //}
}
void _wrp_R_SetScale(float scale) {
    if (R_SetScale)
        R_SetScale(scale);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}
void _wrp_R_DrawChar(int x, int y, int flags, int ch, qhandle_t font) {
    if (R_DrawChar)
        R_DrawChar(x, y, flags, ch, font);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
}
int _wrp_R_DrawString(int x, int y, int flags, size_t maxChars,
                                    const char* string, qhandle_t font) {
    if (R_DrawString)
        return R_DrawString(x, y, flags, maxChars, string, font);
    else
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);

    return 0;
}
qboolean _wrp_R_GetPicSize(int* w, int* h, qhandle_t pic) {
    // CPP: Bug in the old code I guess.
//    if (R_GetPicSize) {
        return R_GetPicSize(w, h, pic);
    //}
    //else {
    //    Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
    //    return false;
    //}
}
void _wrp_R_DrawPic(int x, int y, qhandle_t pic) {
    if (R_DrawPic) {
        R_DrawPic(x, y, pic);
    } else {
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
    }
}
void _wrp_R_DrawStretchPic(int x, int y, int w, int h, qhandle_t pic) {
    if (R_DrawStretchPic) {
        R_DrawStretchPic(x, y, w, h, pic);
    }
    else {
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
    }
}
void _wrp_R_TileClear(int x, int y, int w, int h, qhandle_t pic) {
    if (R_TileClear) {
        R_TileClear(x, y, w, h, pic);
    }
    else {
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
    }
}
void _wrp_R_DrawFill8(int x, int y, int w, int h, int c) {
    if (R_DrawFill8) {
        R_DrawFill8(x, y, w, h, c);
    }
    else {
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
    }
}
void _wrp_R_DrawFill32(int x, int y, int w, int h, uint32_t color) {
    if (R_DrawFill32) {
        R_DrawFill32(x, y, w, h, color);
    }
    else {
        Com_EPrintf("%s - Contains access to an invalid func_ptr\n", __func__);
    }
}
// SOUND
extern void AL_SpecialEffect_Underwater_Disable();
void _wrp_SFX_Underwater_Disable(void) {
    AL_SpecialEffect_Underwater_Disable();
}
extern void AL_SpecialEffect_Underwater_Enable();
void _wrp_SFX_Underwater_Enable(void) {
    AL_SpecialEffect_Underwater_Enable();
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
    ClientGameImport   importAPI;
    ClientGameExport* (*entry)(ClientGameImport*) = NULL;

    // unload anything we have now
    CL_ShutdownGameProgs();

    // for debugging or `proxy' mods
    if (sys_forcecgamelib->string[0])
        entry = (ClientGameExport * (*)(ClientGameImport*))_CL_LoadGameLibrary(sys_forcecgamelib->string); // CPP: WARNING: IMPORTANT: Is this cast valid? lol.

    // try game first
    if (!entry && fs_game->string[0]) {
        entry = (ClientGameExport * (*)(ClientGameImport*))CL_LoadGameLibrary(fs_game->string, ""); // CPP: WARNING: IMPORTANT: Is this cast valid? lol.
    }

    // then try basenac
    if (!entry) {
        entry = (ClientGameExport * (*)(ClientGameImport*))CL_LoadGameLibrary(BASEGAME, ""); // CPP: WARNING: IMPORTANT: Is this cast valid? lol.
    }

    // all paths failed
    if (!entry)
        Com_Error(ERR_DROP, "Failed to load Client Game library");

    // API Version.
    importAPI.apiversion = {
        CGAME_API_VERSION_MAJOR,
        CGAME_API_VERSION_MINOR,
        CGAME_API_VERSION_POINT,
    };

    //
    // Setup the function pointers for the cgame dll.
    //
    importAPI.cl = &cl;
    importAPI.cs = &cs;

    // Client.
    importAPI.GetFrameTime = _wrp_GetFrameTime;
    importAPI.GetRealTime = _wrp_GetRealTime;

    importAPI.GetFramesPerSecond = CL_GetFps;
    importAPI.GetResolutionScale = CL_GetResolutionScale;

    importAPI.GetServerState = _wrp_GetServerState;
    importAPI.GetServerProtocol = _wrp_GetServerProtocol;
    importAPI.GetProtocolVersion = _wrp_GetProtocolVersion;

    importAPI.IsDemoPlayback = _wrp_IsDemoPlayback;

    importAPI.UpdateListenerOrigin = CL_UpdateListenerOrigin;

    importAPI.SetClientLoadState = CL_SetLoadState;
    importAPI.GetClienState = CL_GetConnectionState;

    importAPI.CheckForIgnore = CL_CheckForIgnore;
    importAPI.CheckForIP = CL_CheckForIP;

    // Command Buffer.
    importAPI.Cbuf_AddText = _wrp_Cbuf_AddText;
    importAPI.Cbuf_InsertText = _wrp_Cbuf_InsertText;
    importAPI.Cbuf_Execute = _wrp_Cbuf_Execute;
    importAPI.CL_ForwardToServer = CL_ForwardToServer;

    // Collision Model.
    importAPI.CM_HeadnodeForBox = CM_HeadnodeForBox;
    importAPI.CM_InlineModel = _wrp_CM_InlineModel;
    importAPI.CM_PointContents = CM_PointContents;
    importAPI.CM_TransformedPointContents = CM_TransformedPointContents;
    importAPI.CM_BoxTrace = CM_BoxTrace;
    importAPI.CM_TransformedBoxTrace = CM_TransformedBoxTrace;
    importAPI.CM_ClipEntity = CM_ClipEntity;

    // Command.
    importAPI.Cmd_AddCommand = Cmd_AddCommand;
    importAPI.Cmd_RemoveCommand = Cmd_RemoveCommand;

    importAPI.Cmd_Register = Cmd_Register;
    importAPI.Cmd_Deregister = Cmd_Deregister;

    importAPI.Cmd_AddMacro = Cmd_AddMacro;
    importAPI.Cmd_FindMacro = Cmd_FindMacro;
    importAPI.Cmd_Macro_g = Cmd_Macro_g;

    importAPI.Prompt_AddMatch = Prompt_AddMatch;

    importAPI.Cmd_TokenizeString = Cmd_TokenizeString;
    importAPI.Cmd_Argc = Cmd_Argc;
    importAPI.Cmd_Argv = Cmd_Argv;
    importAPI.Cmd_Args = Cmd_Args;

    importAPI.Cmd_ExecTrigger = Cmd_ExecTrigger;

    // Common.
    importAPI.Com_Error = Com_Error;
    importAPI.Com_LPrintf = Com_LPrintf;

    importAPI.Com_ErrorString = Q_ErrorString;
    importAPI.Com_GetEventTime = _prt_Com_GetEventTime;

    // Console.
    importAPI.Con_ClearNotify = Con_ClearNotify_f;
    importAPI.Con_SkipNotify = Con_SkipNotify;

    // Cvar.
    importAPI.Cvar_Get = Cvar_Get;
    importAPI.Cvar_WeakGet = Cvar_WeakGet;
    importAPI.Cvar_Exists = Cvar_Exists;
    importAPI.Cvar_VariableValue = Cvar_VariableValue;
    importAPI.Cvar_VariableInteger = Cvar_VariableInteger;
    importAPI.Cvar_VariableString = Cvar_VariableString;
    importAPI.Cvar_Set = Cvar_Set;
    importAPI.Cvar_SetValue = Cvar_SetValue;
    importAPI.Cvar_SetInteger = Cvar_SetInteger;
    importAPI.Cvar_UserSet = Cvar_UserSet;
    importAPI.Cvar_Reset = _wrp_Cvar_Reset;
    importAPI.Cvar_ClampInteger = Cvar_ClampInteger;
    importAPI.Cvar_ClampValue = Cvar_ClampValue;

    importAPI.Cvar_Variable_g = Cvar_Variable_g;
    importAPI.Cvar_Default_g = Cvar_Default_g;

    // Files.
    importAPI.FS_RenameFile = FS_RenameFile;
    importAPI.FS_CreatePath = FS_CreatePath;
    importAPI.FS_FOpenFile = FS_FOpenFile;
    importAPI.FS_FCloseFile = FS_FCloseFile;
    importAPI.FS_EasyOpenFile = FS_EasyOpenFile;
    importAPI.FS_FileExists = _wrp_FS_FileExists;
    importAPI.FS_FileExistsEx = _wrp_FS_FileExistsEx;
    importAPI.FS_WriteFile = FS_WriteFile;
    importAPI.FS_EasyWriteFile = FS_EasyWriteFile;
    importAPI.FS_Read = FS_Read;
    importAPI.FS_Write = FS_Write;
    importAPI.FS_FPrintf = _wrp_FS_FPrintf;
    importAPI.FS_ReadLine = FS_ReadLine;
    importAPI.FS_Flush = FS_Flush;
    importAPI.FS_Tell = FS_Tell;
    importAPI.FS_Seek = FS_Seek;
    importAPI.FS_Length = FS_Length;
    importAPI.FS_WildCmp = FS_WildCmp;
    importAPI.FS_ExtCmp = FS_ExtCmp;
    importAPI.FS_LastModified = FS_LastModified;
    importAPI.FS_ListFiles = FS_ListFiles;
    importAPI.FS_CopyList = FS_CopyList;
    importAPI.FS_CopyInfo = FS_CopyInfo;
    importAPI.FS_FreeList = FS_FreeList;
    importAPI.FS_NormalizePath = FS_NormalizePath;
    importAPI.FS_NormalizePathBuffer = FS_NormalizePathBuffer;
    importAPI.FS_ValidatePath = FS_ValidatePath;
    importAPI.FS_SanitizeFilenameVariable = FS_SanitizeFilenameVariable;

    // Keys.
    importAPI.Key_GetOverstrikeMode = Key_GetOverstrikeMode;
    importAPI.Key_SetOverstrikeMode = Key_SetOverstrikeMode;
    importAPI.Key_GetDest = Key_GetDest;
    importAPI.Key_SetDest = Key_SetDest;
    importAPI.Key_IsDown = Key_IsDown;
    importAPI.Key_AnyKeyDown = Key_AnyKeyDown;
    importAPI.Key_StringToKeynum = Key_StringToKeynum;
    importAPI.Key_KeynumToString = Key_KeynumToString;
    importAPI.Key_GetBinding = Key_GetBinding;
    importAPI.Key_GetBindingForKey = Key_GetBindingForKey;
    importAPI.Key_EnumBindings = Key_EnumBindings;
    importAPI.Key_SetBinding = Key_SetBinding;

    // Mouse.
    importAPI.Mouse_GetMotion = CL_GetMouseMotion;

    // Memory.
    importAPI.Z_TagMalloc = Z_TagMalloc;
    importAPI.Z_TagMallocz = Z_TagMallocz;
    importAPI.Z_TagReserve = Z_TagReserve;
    importAPI.Z_TagCopyString = Z_TagCopyString;
    importAPI.Z_Free = Z_Free;

    // Networking.
    importAPI.MSG_ReadChar = MSG_ReadChar;
    importAPI.MSG_ReadByte = MSG_ReadByte;
    importAPI.MSG_ReadShort = MSG_ReadShort;
    importAPI.MSG_ReadWord = MSG_ReadWord;
    importAPI.MSG_ReadLong = MSG_ReadLong;
    importAPI.MSG_ReadString = MSG_ReadString;
    importAPI.MSG_ReadDirection = MSG_ReadDirection;
    importAPI.MSG_ReadPosition = MSG_ReadPosition;
    importAPI.MSG_WriteChar = MSG_WriteChar;
    importAPI.MSG_WriteByte = MSG_WriteByte;
    importAPI.MSG_WriteShort = MSG_WriteShort;
    importAPI.MSG_WriteLong = MSG_WriteLong;
    importAPI.MSG_WriteString = MSG_WriteString;
    importAPI.MSG_WritePosition = MSG_WritePosition;

    importAPI.MSG_FlushTo = _trp_MSG_FlushTo;

    // Register.
    importAPI.R_RegisterModel = _wrp_R_RegisterModel;
    importAPI.R_RegisterImage = _wrp_R_RegisterImage;
    importAPI.R_RegisterRawImage = _wrp_R_RegisterRawImage;
    importAPI.R_UnregisterImage = _wrp_R_UnregisterImage;

    importAPI.R_RegisterPic = _wrp_R_RegisterPic;
    importAPI.R_RegisterPic2 = _wrp_R_RegisterPic2;
    importAPI.R_RegisterFont = _wrp_R_RegisterFont;
    importAPI.R_RegisterSkin = _wrp_R_RegisterSkin;

    importAPI.MOD_ForHandle = MOD_ForHandle;

    // Rendering
    importAPI.R_AddDecal = _wrp_R_AddDecal;
    importAPI.R_LightPoint = _wrp_R_LightPoint;
    importAPI.R_SetSky = _wrp_R_SetSky;

    importAPI.R_ClearColor = _wrp_R_ClearColor;
    importAPI.R_SetAlpha = _wrp_R_SetAlpha;
    importAPI.R_SetAlphaScale = _wrp_R_SetAlphaScale;
    importAPI.R_SetColor = _wrp_R_SetColor;
    importAPI.R_SetClipRect = _wrp_R_SetClipRect;
    importAPI.R_ClampScale = _wrp_R_ClampScale;
    importAPI.R_SetScale = _wrp_R_SetScale;
    importAPI.R_DrawChar = _wrp_R_DrawChar;
    importAPI.R_DrawString = _wrp_R_DrawString;
    importAPI.R_GetPicSize = _wrp_R_GetPicSize;
    importAPI.R_DrawPic = _wrp_R_DrawPic;
    importAPI.R_DrawStretchPic = _wrp_R_DrawStretchPic;
    importAPI.R_TileClear = _wrp_R_TileClear;
    importAPI.R_DrawFill8 = _wrp_R_DrawFill8;
    importAPI.R_DrawFill32 = _wrp_R_DrawFill32;

    // Screen.
    importAPI.SCR_UpdateScreen = SCR_UpdateScreen;

    // System.
    importAPI.Sys_Milliseconds = Sys_Milliseconds;

    // Sound.
    importAPI.S_BeginRegistration = S_BeginRegistration;
    importAPI.S_RegisterSound = S_RegisterSound;
    importAPI.S_EndRegistration = S_EndRegistration;

    importAPI.S_StartSound = S_StartSound;
    importAPI.S_StartLocalSound = S_StartLocalSound;
    importAPI.S_StartLocalSound_ = S_StartLocalSound_;

    importAPI.SFX_Underwater_Disable = _wrp_SFX_Underwater_Enable;
    importAPI.SFX_Underwater_Enable = _wrp_SFX_Underwater_Enable;

    // Load up the cgame dll.
    cge = entry(&importAPI);

    if (!cge) {
        Com_Error(ERR_DROP, "Client Game DLL returned NULL exports");
        return;
    }

    if (cge->apiversion.major != CGAME_API_VERSION_MAJOR ||
        cge->apiversion.minor != CGAME_API_VERSION_MINOR) {
        Com_Error(ERR_DROP, "Client Game DLL is version %i.%i.%i, expected %i.%i.%i",
            cge->apiversion, cge->apiversion.major, cge->apiversion.minor, cge->apiversion.point, CGAME_API_VERSION_MAJOR, CGAME_API_VERSION_MINOR, CGAME_API_VERSION_POINT);
        return;
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
// CL_GM_DemoSeek
// 
// Called when the client is seeking in a demo playback.
//===============
//
void CL_GM_EntityEvent(int number) {
    if (cge)
        cge->EntityEvent(number);
}

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
// CL_GM_CalcFOV
//
// Called whenever the FOV has to be calculated.
//===============
//
float CL_GM_CalcFOV(float fov_x, float width, float height) {
    if (cge)
        return cge->CalcFOV(fov_x, width, height);
    else
        return 0.f;
}

//
//===============
// CL_GM_UpdateOrigin
// 
// Called by the client in case it wants to update audio positioning.
//===============
//
void CL_GM_UpdateOrigin(void) {
    if (cge)
        cge->UpdateOrigin();
}

//
//===============
// CL_GM_ClientBegin
// 
// Called after finishing in CL_Begin (aka after map load etc)
//===============
//
void CL_GM_ClientBegin(void) {
    if (cge)
        cge->ClientBegin();
}

//
//===============
// CL_GM_ClientDeltaFrame
// 
// Called each time the client has parsed a valid frame. 
// Handle per VALID frame basis things here.
//===============
//
void CL_GM_ClientDeltaFrame(void) {
    if (cge)
        cge->ClientDeltaFrame();
}

//
//===============
// CL_GM_ClientFrame
// 
// Called each client frame. Handle per frame basis things here.
//===============
//
void CL_GM_ClientFrame(void) {
    if (cge)
        cge->ClientFrame();
}

//
//===============
// CL_GM_ClientDisconnect
// 
// Called when the client disconnects, including by Com_Error etc.
//===============
//
void CL_GM_ClientDisconnect(void) {
    if (cge)
        cge->ClientDisconnect();
}

//
//===============
// CL_GM_ClearState
// 
// Called when the client (and/is) disconnected for whichever reasons.
//===============
//
void CL_GM_ClearState(void) {
    if (cge)
        cge->ClearState();
}

//
//===============
// CL_GM_DemoSeek
// 
// Called when the client is seeking in a demo playback.
//===============
//
void CL_GM_DemoSeek(void) {
    if (cge)
        cge->DemoSeek();
}

//
//===============
// CL_GM_UpdateUserInfo
// 
// Called when the client has changed user info.
// Here we can fix up the gender for example before all data gets applied and
// send to the other clients.
//===============
//
void CL_GM_UpdateUserInfo(cvar_t* var, from_t from) {
    if (cge)
        cge->UpdateUserinfo(var, from);
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
// CL_GM_UpdateConfigString
// 
// Called when the engine is about to parse a configstring, we will
// give the CG Module access to do with it as it pleases.
//===============
//
qboolean CL_GM_UpdateConfigString (int index, const char *str) {
    if (cge)
        return cge->UpdateConfigString(index, str);

    return false;
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

    return false;
}

//
//===============
// CL_GM_SeekDemoMessage
// 
// Parses command operations known to the game dll, but for
// demo playback only. This means certain commands such as
// svc_centerprint can be skipped.
//===============
//
qboolean CL_GM_SeekDemoMessage (int demoCommand) {
    if (cge)
        return cge->SeekDemoMessage(demoCommand);

    return false;
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
// CL_GM_CheckPredictionError
// 
// Called by the client to check for prediction errors.
//===============
//
void CL_GM_CheckPredictionError(ClientUserCommand *clientUserCommand) {
    if (cge)
        cge->CheckPredictionError(clientUserCommand);
}

//
//===============
// CL_GM_BuildFrameMoveCommand
// 
// Called by the client to build up the movement command for
// the current frame.
//===============
//
void CL_GM_BuildFrameMoveCommand(int msec) {
    if (cge)
        cge->BuildFrameMoveCommand(msec);
}

//
//===============
// CL_GM_FinalizeFrameMoveCommand
// 
// Called by the client to finalize the move command for this current
// frame.
//===============
//
void CL_GM_FinalizeFrameMoveCommand(void) {
    if (cge)
        cge->FinalizeFrameMoveCommand();
}

//
//===============
// CL_GM_PredictAngles
// 
// Called by the client to set prediction angles.
//===============
//
void CL_GM_PredictAngles(void) {
    if (cge)
        cge->PredictAngles();
}

//
//===============
// CL_GM_PredictMovement
// 
// Called by the client to predict the actual movement.
//===============
//
void CL_GM_PredictMovement(unsigned int ack, unsigned int current) {
    if (cge)
        cge->PredictMovement(ack, current);
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
const char *CL_GM_GetMediaLoadStateName(LoadState state)
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
// CL_GM_RenderScreen
// 
// Call into the CG Module for rendering the screen's 2D elements.
//===============
//
void CL_GM_RenderScreen(void) {
    if (cge)
        cge->RenderScreen();
}

//
//===============
// CL_GM_ScreenModeChanged
// 
// Call into the CG Module for notifying the screen mode changed.
//===============
//
void CL_GM_ScreenModeChanged(void) {
    if (cge)
        cge->ScreenModeChanged();
}

//
//===============
// CL_GM_DrawLoadScreen
// 
// Call into the CG Module for notifying the screen mode changed.
//===============
//
void CL_GM_DrawLoadScreen(void) {
    if (cge)
        cge->DrawLoadScreen();
}

//
//===============
// CL_GM_DrawPauseScreen
// 
// Call into the CG Module for notifying the screen mode changed.
//===============
//
void CL_GM_DrawPauseScreen(void) {
    if (cge)
        cge->DrawPauseScreen();
}

//
//===============
// CL_GM_RenderView
// 
// Call into the CG Module for notifying about "Clear Scene"
//===============
//
void CL_GM_ClearScene() {
    if (cge)
        cge->ClearScene();
}

//
//===============
// CL_GM_PreRenderView
// 
// Call into the CG Module for notifying about "Pre Render View"
//===============
//
void CL_GM_PreRenderView() {
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