// License here.
// 
//
// Interface that a client game dll his exports have to implement in order to
// be fully coherent with the actual client loading it in.
// 
// WID: Time to re-adjust here with new files. I agree, at last.
#pragma once

#include "shared/IClientGameExports.h"

//---------------------------------------------------------------------
// CORE implementation.
//---------------------------------------------------------------------
class ClientGameCore : public IClientGameCore {
	void Initialize();
	void Shutdown();
};

//---------------------------------------------------------------------
// COMMANDBUFFER implementation.
//---------------------------------------------------------------------
class ClientGameExportCommandBuffer : public IClientGameExportCommandBuffer {
	void AddText(const std::string& text);
	void InsertText(const std::string& text);
	void Execute();
	qboolean ForwardToServer();
};

//---------------------------------------------------------------------
// COLLISIONMODEL implementation.
//---------------------------------------------------------------------
class ClientGameExportCollisionModel : public IClientGameExportCollisionModel {
	mnode_t* HeadnodeForBox(const vec3_t& mins, const vec3_t& maxs);
	mmodel_t* (*CM_InlineModel) (cm_t* cm, const char* name);
	int32_t PointContents(const vec3_t& p, mnode_t* headNode);
	int32_t TransformedPointContents(const vec3_t& p, mnode_t* headNode, const vec3_t& origin, const vec3_t& angles);
	void BoxTrace(trace_t * trace, const vec3_t & start, const vec3_t & end, const vec3_t & mins, const vec3_t & maxs, mnode_t * headNode, int32_t brushmask);
	void TransformedBoxTrace(trace_t* trace, const vec3_t& start, const vec3_t& end, const vec3_t& mins, const vec3_t& maxs, mnode_t* headNode, int32_t brushmask, const vec3_t& origin, const vec3_t& angles);
	void ClipEntity(trace_t* dst, const trace_t* src, struct entity_s* ent);
};

//---------------------------------------------------------------------
// COMMAND implementation.
//---------------------------------------------------------------------
class ClientGameExportCommand : public IClientGameExportCommand {
	void AddCommand(const std::string& commandName, xcommand_t function);
	void RemoveCommand(const std::string& commandName);
	void Register(const cmdreg_t* reg);
	void Deregister(const cmdreg_t* reg);
	void AddMacro(const std::string& name, xmacro_t function);
	cmd_macro_t* FindMacro(const std::string& name);
	void Macro_g(genctx_t* context); // <-- Yeah.. lol
	qboolean Prompt_AddMatch(genctx_t* ctx, const std::string& str);
	void TokenizeString(const std::string& text, qboolean macroExpand);
	int32_t ArgumentCount();
	std::string ArgumentValue(int32_t argumentValue);
	std::string ArgumentString();
	void ExecuteTrigger(const std::string& str);
};

//---------------------------------------------------------------------
// COMMON implementation.
//---------------------------------------------------------------------
class ClientGameExportCommon : public IClientGameExports {
	void Error(ErrorType code, const char* fmt, ...);
	void Printf(PrintType type, const char* fmt, ...);
	const char* ErrorString) (qerror_t type);
	unsigned GetEventTime(void);
};

//---------------------------------------------------------------------
// CONSOLE implementation.
//---------------------------------------------------------------------
class ClientGameExportConsole : public IClientGameExportConsole {
	void ClearNotify();
	void SkipNotify(qboolean skip);
};

//---------------------------------------------------------------------
// CVAR implementation.
//---------------------------------------------------------------------
class IClientGameExportCVar : public IClientGameExportCVar {
	cvar_t* Get(const char* variableName, const char* value, int32_t flags);
	cvar_t* WeakGet(const char* variableName);
	qboolean Exists(const char* name, qboolean weak);
	float VariableValue(const char* var_name);
	int VariableInteger(const char* variableName);
	const char* VariableString(const char* variableName);
	cvar_t* Set(const char* variableName, const char* value);
	void SetValue(cvar_t* cvariable, float value, from_t from);
	void SetInteger(cvar_t* cvariable, int32_t value, from_t from);
	cvar_t* UserSet(const char* variableName, const char* value);
	void Reset(cvar_t* cvariable);
	int32_t ClampInteger(cvar_t* cvariable, int32_t minimal, int32_t maximal);
	float ClampValue(cvar_t* cvariable, float minimal, float maximal);
	virtual void GenerateVariable(genctx_t* context);
	virtual void DefaultVariable(genctx_t* context);
};

//---------------------------------------------------------------------
// FILESYSTEM implementation.
//---------------------------------------------------------------------
class ClientGameExportFileSystem : public IClientGameExportFileSystem {
	qerror_t RenameFile(const char* from, const char* to);
	qerror_t CreatePath(char* path);
	ssize_t OpenFile(const char* filename, qhandle_t* file, uint32_t mode);
	void CloseFile(qhandle_t file);
	qhandle_t EasyOpenFile(char* buffer, size_t size, uint32_t mode,
		const char* directory, const char* name, const char* extension);
	qhandle_t FileExists(const char* path);
	qhandle_t FileExistsEx(const char* path, uint32_t flags);
	qerror_t WriteFile(const char* path, const void* data, size_t length);
	qboolean EasyWriteFile(char* buffer, size_t size, unsigned mode,
		const char* directory, const char* name, const char* extension,
		const void* data, size_t length);
	ssize_t Read(void* buffer, size_t length, qhandle_t file);
	ssize_t Write(const void* buffer, size_t len, qhandle_t f);
	ssize_t Printf(qhandle_t file, const char* format, ...);
	ssize_t ReadLine(qhandle_t file, char* buffer, size_t size);
	void Flush(qhandle_t file);
	ssize_t Tell(qhandle_t file);
	qerror_t Seek(qhandle_t file, off_t offset);
	ssize_t Length(qhandle_t file);
	qboolean WildCardCompare(const char* filter, const char* string);
	qboolean ExtensionCompare(const char* extension, const char* string);
	qerror_t LastModified(char const* file, uint64_t* lastTimeModified);
	void** ListFiles(const char* path, const char* filter, uint32_t flags, int32_t* count_p);
	void** CopyList(void** list, int count);
	file_info_t* CopyInfo(const char* name, size_t size, time_t ctime, time_t mtime);
	void FreeList(void** list);
	size_t NormalizePath(char* out, const char* in);
	size_t NormalizePathBuffer(char* out, const char* in, size_t size);
	int32_t ValidatePath(const char* s);
	void SanitizeFilenameVariable(cvar_t* var);
};

//---------------------------------------------------------------------
// KEYBOARD implementation.
//---------------------------------------------------------------------
class ClientGameExportKeyboard : public IClientGameExportKeyboard {
	qboolean GetOverstrikeMode(void);
	void SetOverstrikeMode(qboolean overStrike);
	keydest_t GetDest(void);
	void SetDest(keydest_t dest);
	int32_t IsDown(int32_t key);
	int32_t AnyKeyDown(void);
	int32_t StringToKeynum(const char* str);
	const char* KeynumToString(int32_t keyNumber);
	const char* GetBinding(const char* binding);
	const char* GetBindingForKey(int32_tkeynum);
	int EnumBindings(int32_t key, const char* binding);
	void SetBinding(int32_t keyNumber, const char* binding);
};

//---------------------------------------------------------------------
// MEDIA implementation.
//---------------------------------------------------------------------
class ClientGameExportMedia : public IClientGameExportMedia{
	std::string GetLoadStateName(LoadState loadState);
	void Initialize();
};

//---------------------------------------------------------------------
// MEMORY implementation.
//---------------------------------------------------------------------
class ClientGameExportMouse : public IClientGameExportMouse {
	void* ZoneTagMalloc(size_t size, memtag_t memoryTag);
	void* ZoneTagMallocz(size_t size, memtag_t memoryTag);
	void ZoneTagReserve(size_t size, memtag_t memoryTag);
	char* ZoneTagCopyString(const char* in, memtag_t memoryTag);
	void ZoneFree(void* ptr);
};

//---------------------------------------------------------------------
// MOVEMENT implementation.
//---------------------------------------------------------------------
class ClientGameExportMovement : public IClientGameExportMovement {
	void BuildFrameMovementCommand(int32_t msec);
	void FinalizeFrameMovementCommand();
};

//---------------------------------------------------------------------
// MOUSE implementation.
//---------------------------------------------------------------------
class ClientGameExportMouse : public IClientGameExportMouse {
	void GetMotion(int32_t deltaX, int32_t deltaY);
};

//---------------------------------------------------------------------
// MESSAGE implementation.
//---------------------------------------------------------------------
class ClientGameExportMessage : public IClientGameExportMessage {
	int32_t	ReadChar();
	int32_t ReadByte();
	int32_t ReadShort();
	int32_t ReadWord();
	int32_t ReadLong();
	size_t ReadString(char* destination, size_t size);
	vec3_t ReadVector3();
	int32_t WriteChar(int32_t characterValue);
	int32_t WriteByte(int32_t byteValue);
	int32_t WriteShort(int32_t shortValue);
	int32_t WriteLong(int32_t longValue);
	int32_t WriteString(const char* stringValue);
	int32_t WriteVector3(const vec3_t& vectorValue);
	int32_t FlushTo(sizebuf_t* buffer);
};

//---------------------------------------------------------------------
// REGISTER implementation.
//---------------------------------------------------------------------
class ClientGameExportRegister : public IClientGameExportRegister {
	qhandle_t Model(const char* name);
	qhandle_t Image(const char* name, imagetype_t type,
		imageflags_t flags, qerror_t* err_p);
	qhandle_t RawImage(const char* name, int32_t width, int32_t height, byte* pic, imagetype_t type,
		imageflags_t flags);
	void UnregisterImage(qhandle_t handle);
	qhandle_t Picture(const char* name);
	qhandle_t PermanentPicture(const char* name);
	qhandle_t Font(const char* name);
	qhandle_t Skin(const char* name);
	model_t* GetModelHandlePointer(qhandle_t handle);
};

//---------------------------------------------------------------------
// RENDERER implementation.
//---------------------------------------------------------------------
class ClientGameExportRenderer : public IClientGameExportRenderer {
	void AddDecal(decal_t* d);
	void LightPoint(const vec3_t& origin, vec3_t& light);
	void SetSky(const char* name, float rotate, vec3_t& axis);

	void ClearColor(void);
	void SetAlpha(float clpha);
	void SetAlphaScale(float alpha);
	void SetColor(uint32_t color);
	void SetClipRect(const clipRect_t* clip);
	float ClampScale(cvar_t* var);
	void SetScale(float scale);
	void DrawCharacater(int32_t x, int32_t y, int32_t flags, int32_t character, qhandle_t font);
	int DrawString(int32_t x, int32_t y, int32_t flags, size_t maxChars,
		const char* string, qhandle_t font);
	qboolean GetPictureSize) (int32_t* width, int32_t* height, qhandle_t picture);   // returns transparency bit
	void DrawPicture(int32_t x, int32_t y, qhandle_t picture);
	void DrawStretchedPicture(int32_t x, int32_t y, int32_t width, int32_t height, qhandle_t picture);
	void TileClear(int32_t x, int32_t y, int32_t width, int32_t height, qhandle_t picture);
	void DrawFill8(int32_t x, int32_t y, int32_t width, int32_t height, int32_t character);
	void DrawFill32(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color);
};

//---------------------------------------------------------------------
// SCREEN implementation.
//---------------------------------------------------------------------
class ClientGameExportScreen : public IClientGameExportScreen {
	void UpdateScreen();
};

//---------------------------------------------------------------------
// Sound implementation.
//---------------------------------------------------------------------
class IClientGameExportSound {
	// Begins the sound registration process.
	void BeginRegistration(void);
	// Precaches a sound with the given filename. (Can be a path.)
	qhandle_t RegisterSound(const char* name);
	// Ends the sound registration process.
	void EndRegistration(void);

	// Plays a sound at the given origin. If origin is NULL, the sound will
	// be dynamically sourced from the entity.
	void StartSound(const vec3_t* origin, int32_t entityNumber, int32_t entityChannel,
		qhandle_t sfx, float fvol, float attenuation, float timeOffset);
	// Plays a local 2D sound on entchannel 0.                               
	void StartLocalSound(const char* s);
	// Plays a local 2D sound on entchannel 256.                               
	void StartLocalSound(const char* s);

	// Enables under water special audio effect.
	void EnableUnderWater();
	// Disables under water special audio effect.
	void DisableUnderWater();
};

//---------------------------------------------------------------------
// SYSTEM implementation.
//---------------------------------------------------------------------
class IClientGameExportSystem {
	virtual uint32_t Milliseconds() = 0;
};

//---------------------------------------------------------------------
// ENTITY implementation.
//---------------------------------------------------------------------
class IClientGameExportEntities {
	// Executed whenever an entity event is receieved.
	virtual void Event(int32_t number) = 0;
};


//---------------------------------------------------------------------
// MAIN interface implementation.
//---------------------------------------------------------------------
class ClientGameExports : public IClientGameExports {
public:
	// WID: TODO: Normally we'd use a Get, should we do that and make these private?
	// Perhaps not.
	IClientGameCore* core;
	IClientGameExportCollisionModel* collisionModel;
	IClientGameExportCommand* command;
	IClientGameExportCommandBuffer* commandBuffer;
	IClientGameExportCommon* common;
	IClientGameExportConsole* console;
	IClientGameExportCVar* cvar;
	IClientGameExportEntities* entities;
	IClientGameExportFileSystem* filesystem;
	IClientGameExportMedia* media;
	IClientGameExportMessage* message;
	IClientGameExportMouse* mouse;
	IClientGameExportMovement* movement;
	IClientGameExportRegister* registry;
	IClientGameExportRenderer* renderer;
	IClientGameExportScreen* screen;
	IClientGameExportSound* sound;
	IClientGameExportSystem* system;

	float CalculateClientFieldOfView(float x, float width, float height);
	void ClearClientState();
	void UpdateClientOrigin();
	void DemoSeek();
	void ClientBegin();
	void ClientDeltaFrame();
	void ClientFrame();
	void ClientDisconnect();
	void ClientUpdateUserinfo(cvar_t* var, from_t from) = 0;
	void SetClientLoadState(LoadState loadState);
	uint32_t GetClientState();
	qboolean CheckForIgnore(const std::string& str);
	void CheckForIP(const std::string& str);
};

