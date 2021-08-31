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
// CORE interface.
//---------------------------------------------------------------------
class ClientGameCore : public IClientGameCore {
	void Initialize();
	void Shutdown();
};

//---------------------------------------------------------------------
// COMMANDBUFFER interface.
//---------------------------------------------------------------------
class ClientGameExportCommandBuffer : public IClientGameExportCommandBuffer {
	void AddText(const std::string& text);
	void InsertText(const std::string& text);
	void Execute();
	qboolean ForwardToServer();
};

//---------------------------------------------------------------------
// COLLISIONMODEL interface.
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
// COMMAND interface.
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
// COMMON interface.
//---------------------------------------------------------------------
class ClientGameExportCommon : public IClientGameExports {
	void Error(ErrorType code, const char* fmt, ...);
	void Printf(PrintType type, const char* fmt, ...);
	const char* ErrorString) (qerror_t type);
	unsigned GetEventTime(void);
};

//---------------------------------------------------------------------
// CONSOLE interface.
//---------------------------------------------------------------------
class ClientGameExportConsole : public IClientGameExportConsole {
	void ClearNotify();
	void SkipNotify(qboolean skip);
};

//---------------------------------------------------------------------
// CVAR interface.
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
// FILESYSTEM interface.
//---------------------------------------------------------------------
class ClientGameExportFileSystem : public IClientGameExportFileSystem {
	qerror_t RenameFile(const char* from, const char* to) = 0;
	qerror_t CreatePath(char* path) = 0;
	ssize_t OpenFile(const char* filename, qhandle_t* file, uint32_t mode) = 0;
	void CloseFile(qhandle_t file) = 0;
	qhandle_t EasyOpenFile(char* buffer, size_t size, uint32_t mode,
		const char* directory, const char* name, const char* extension) = 0;
	qhandle_t FileExists(const char* path) = 0;
	qhandle_t FileExistsEx(const char* path, uint32_t flags) = 0;
	qerror_t WriteFile(const char* path, const void* data, size_t length) = 0;
	qboolean EasyWriteFile(char* buffer, size_t size, unsigned mode,
		const char* directory, const char* name, const char* extension,
		const void* data, size_t length) = 0;
	ssize_t Read(void* buffer, size_t length, qhandle_t file) = 0;
	ssize_t Write(const void* buffer, size_t len, qhandle_t f) = 0;
	ssize_t Printf(qhandle_t file, const char* format, ...) = 0;
	ssize_t ReadLine(qhandle_t file, char* buffer, size_t size) = 0;
	void Flush(qhandle_t file) = 0;
	ssize_t Tell(qhandle_t file) = 0;
	qerror_t Seek(qhandle_t file, off_t offset) = 0;
	ssize_t Length(qhandle_t file) = 0;
	qboolean WildCardCompare(const char* filter, const char* string) = 0;
	qboolean ExtensionCompare(const char* extension, const char* string) = 0;
	qerror_t LastModified(char const* file, uint64_t* lastTimeModified) = 0;
	void** ListFiles(const char* path, const char* filter, uint32_t flags, int32_t* count_p) = 0;
	void** CopyList(void** list, int count) = 0;
	file_info_t* CopyInfo(const char* name, size_t size, time_t ctime, time_t mtime) = 0;
	void FreeList(void** list) = 0;
	size_t NormalizePath(char* out, const char* in) = 0;
	size_t NormalizePathBuffer(char* out, const char* in, size_t size) = 0;
	int32_t ValidatePath(const char* s) = 0;
	void SanitizeFilenameVariable(cvar_t* var) = 0;
};

//---------------------------------------------------------------------
// KEYBOARD interface.
//---------------------------------------------------------------------
class IClientGameExportKeyboard {
	// Returns whether in overstrike mode.
	virtual qboolean GetOverstrikeMode(void) = 0;
	// Sets key in overstrike mode.
	virtual void SetOverstrikeMode(qboolean overStrike) = 0;
	// Returns the current client state key destination.
	virtual keydest_t GetDest(void) = 0;
	// Sets the key destination.
	virtual void SetDest(keydest_t dest) = 0;
	// Returns key down status: if > 1, it is auto-repeating
	virtual int32_t IsDown(int32_t key) = 0;
	// Returns total number of keys down.
	virtual int32_t AnyKeyDown(void) = 0;
	// Returns a key number to be used to index keybindings[] by looking at
	// the given string.  Single ascii characters return themselves, while
	// the K_* names are matched up.
	virtual int32_t StringToKeynum(const char* str) = 0;
	// Returns a string (either a single ascii char, or a K_* name) for the
	// given keynum.
	virtual const char* KeynumToString(int32_t keyNumber) = 0;
	//Returns the name of the first key found.
	virtual const char* GetBinding(const char* binding) = 0;
	// Returns the command bound to a given key.
	virtual const char* GetBindingForKey(int32_tkeynum) = 0;
	// Fills the binding string with the name of the binding matching to the key.
	// Returns -1 in case nothing was found.
	virtual int EnumBindings(int32_t key, const char* binding) = 0;
	// Sets keybinding for the given keynum.
	virtual void SetBinding(int32_t keyNumber, const char* binding) = 0;
};

//---------------------------------------------------------------------
// MEDIA interface.
//---------------------------------------------------------------------
class IClientGameExportMedia {
	// Called when the client wants to know the name of a custom load state.
	virtual std::string GetLoadStateName(LoadState loadState) = 0;

	// Called upon initialization of the renderer.
	virtual void Initialize() = 0;
};

//---------------------------------------------------------------------
// MEMORY interface.
//---------------------------------------------------------------------
class IClientGameExportMouse {
	// TODO: Document.
	virtual void* ZoneTagMalloc(size_t size, memtag_t memoryTag) = 0;
	// TODO: Document.
	virtual void* ZoneTagMallocz(size_t size, memtag_t memoryTag) = 0;
	// TODO: Document.
	virtual void ZoneTagReserve(size_t size, memtag_t memoryTag) = 0;
	// TODO: Document.
	virtual char* ZoneTagCopyString(const char* in, memtag_t memoryTag) = 0;
	// TODO: Document.
	virtual void ZoneFree(void* ptr) = 0;
};

//---------------------------------------------------------------------
// MOVEMENT interface.
//---------------------------------------------------------------------
class IClientGameExportMovement {
	// Called when the movement command needs to be build for the given
	// client networking frame.
	virtual void BuildFrameMovementCommand(int32_t msec) = 0;
	// Finished off building the actual movement vector before sending it
	// to server.
	virtual void FinalizeFrameMovementCommand() = 0;
};

//---------------------------------------------------------------------
// MOUSE interface.
//---------------------------------------------------------------------
class IClientGameExportMouse {
	virtual void GetMotion(int32_t deltaX, int32_t deltaY) = 0;
};

//---------------------------------------------------------------------
// MESSAGE interface.
//---------------------------------------------------------------------
class IClientGameExportMessage {
	// Reads a character from the network.
	virtual int32_t	ReadChar() = 0;
	// Reads a byte from the network.
	virtual int32_t ReadByte() = 0;
	// Reads a short from the network.
	virtual int32_t ReadShort() = 0;
	// Reads a word from the network.
	virtual int32_t ReadWord() = 0;
	// Reads a long from the network.
	virtual int32_t ReadLong() = 0;
	// Reads a string from the network.
	virtual size_t ReadString(char* destination, size_t size) = 0;
	// Reads a vector3 from the network.
	virtual vec3_t ReadVector3() = 0;

	// Writes a character over the network.
	virtual int32_t WriteChar(int32_t characterValue) = 0;
	// Writes a byte over the network.
	virtual int32_t WriteByte(int32_t byteValue) = 0;
	// Writes a short over the network.
	virtual int32_t WriteShort(int32_t shortValue) = 0;
	// Writes a long over the network.
	virtual int32_t WriteLong(int32_t longValue) = 0;
	// Writes a string over the network.
	virtual int32_t WriteString(const char* stringValue) = 0;
	// Writes a position over the network.
	virtual int32_t WriteVector3(const vec3_t& vectorValue) = 0;
	// Flushes message.
	virtual int32_t FlushTo(sizebuf_t* buffer) = 0;
};

//---------------------------------------------------------------------
// REGISTER interface.
//---------------------------------------------------------------------
class IClientGameExportRegister {
	// Precaches a model, and returns its handle.
	virtual qhandle_t Model(const char* name) = 0;
	// Precaches an image(3d use, filters etc), and returns its handle.
	virtual qhandle_t Image(const char* name, imagetype_t type,
		imageflags_t flags, qerror_t* err_p) = 0;
	// More advanced version of R_RegisterImage.
	virtual qhandle_t RawImage(const char* name, int32_t width, int32_t height, byte* pic, imagetype_t type,
		imageflags_t flags) = 0;
	// Uncaches the image matching the given handle.
	virtual void UnregisterImage(qhandle_t handle) = 0;

	// Precaches a 2D image, and returns its handle.
	virtual qhandle_t Picture(const char* name) = 0;
	// Precaches a 2D image permanently and returns its handle.
	virtual qhandle_t PermanentPicture(const char* name) = 0;
	// Precache a font, returns its handle. These are permanent by default.
	virtual qhandle_t Font(const char* name) = 0;
	// Precaches a skin, and returns its handle.
	virtual qhandle_t Skin(const char* name) = 0;

	// Returns a pointer to the model based on the given handle.
	virtual model_t* GetModelHandlePointer(qhandle_t handle) = 0;
};

//---------------------------------------------------------------------
// RENDERER interface.
//---------------------------------------------------------------------
class IClientGameExportRenderer {
	virtual void AddDecal(decal_t* d) = 0;
	virtual void LightPoint(const vec3_t& origin, vec3_t& light) = 0;
	virtual void SetSky(const char* name, float rotate, vec3_t& axis) = 0;

	virtual void ClearColor(void) = 0;
	virtual void SetAlpha(float clpha) = 0;
	virtual void SetAlphaScale(float alpha) = 0;
	virtual void SetColor(uint32_t color) = 0;
	virtual void SetClipRect(const clipRect_t* clip) = 0;
	virtual float ClampScale(cvar_t* var) = 0;
	virtual void SetScale(float scale) = 0;
	virtual void DrawCharacater(int32_t x, int32_t y, int32_t flags, int32_t character, qhandle_t font) = 0;
	virtual int DrawString(int32_t x, int32_t y, int32_t flags, size_t maxChars,
		const char* string, qhandle_t font) = 0;  // returns advanced x coord
	virtual qboolean GetPictureSize) (int32_t* width, int32_t* height, qhandle_t picture) = 0;   // returns transparency bit
	virtual void DrawPicture(int32_t x, int32_t y, qhandle_t picture) = 0;
	virtual void DrawStretchedPicture(int32_t x, int32_t y, int32_t width, int32_t height, qhandle_t picture) = 0;
	virtual void TileClear(int32_t x, int32_t y, int32_t width, int32_t height, qhandle_t picture) = 0;
	virtual void DrawFill8(int32_t x, int32_t y, int32_t width, int32_t height, int32_t character) = 0;
	virtual void DrawFill32(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) = 0;
};

//---------------------------------------------------------------------
// SCREEN interface.
//---------------------------------------------------------------------
class IClientGameExportScreen {
	//---------------------------------------------------------------------
	// Screen.
	//---------------------------------------------------------------------
	// This is called every frame by the client itself. However in the case
	// of this CG Module, it can also be called explicitly to flush text to
	// the screen.
	virtual void UpdateScreen() = 0;
};

//---------------------------------------------------------------------
// Sound.
//---------------------------------------------------------------------
class IClientGameExportSound {
	// Begins the sound registration process.
	virtual void BeginRegistration) (void);
	// Precaches a sound with the given filename. (Can be a path.)
	virtual qhandle_t RegisterSound) (const char* name);
	// Ends the sound registration process.
	virtual void EndRegistration) (void);

	// Plays a sound at the given origin. If origin is NULL, the sound will
	// be dynamically sourced from the entity.
	virtual void StartSound(const vec3_t* origin, int32_t entityNumber, int32_t entityChannel,
		qhandle_t sfx, float fvol, float attenuation, float timeOffset);
	// Plays a local 2D sound on entchannel 0.                               
	virtual void StartLocalSound(const char* s) = 0;
	// Plays a local 2D sound on entchannel 256.                               
	virtual void StartLocalSound(const char* s) = 0;

	// Enables under water special audio effect.
	virtual void EnableUnderWater() = 0;
	// Disables under water special audio effect.
	virtual void DisableUnderWater() = 0;
};

//---------------------------------------------------------------------
// SYSTEM interface.
//---------------------------------------------------------------------
class IClientGameExportSystem {
	virtual uint32_t Milliseconds() = 0;
};

//---------------------------------------------------------------------
// ENTITY interface.
//---------------------------------------------------------------------
class IClientGameExportEntities {
	// Executed whenever an entity event is receieved.
	virtual void Event(int32_t number) = 0;
};


//---------------------------------------------------------------------
// MAIN interface to implement. It holds pointers to actual sub interfaces,
// which one of course has to implement as well.
//---------------------------------------------------------------------
class IClientGameExports {
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

	// Calculates the FOV the client is running. (Important to have in order.)
	virtual float CalculateClientFieldOfView(float x, float width, float height) = 0;

	// Called upon whenever a client disconnects, for whichever reason.
	// Could be him quiting, or pinging out etc.
	virtual void ClearClientState() = 0;

	// Updates the origin. (Used by the engine for determining current audio position too.)
	virtual void UpdateClientOrigin() = 0;

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

	// Called when there is a needed retransmit of user info variables.
	virtual void ClientUpdateUserinfo(cvar_t* var, from_t from) = 0;

	// Sets the client load state.
	virtual void SetClientLoadState(LoadState loadState);

	// Returns the current state of the client.
	virtual uint32_t GetClientState() = 0;

	// Checks if the name of the player is on the client's ignore list.
	virtual qboolean CheckForIgnore(const std::string& str) = 0;

	// Add scanned out IP address to circular array of recent addresses.
	virtual void CheckForIP(const std::string& str) = 0;
};

