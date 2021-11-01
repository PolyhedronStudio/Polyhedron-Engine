// License here.
// 
//
// Interface that a client game dll his imports have to implement in order to
// be fully coherent with the actual client loading it in.
// 
// WID: Time to re-adjust here with new files. I agree, at last.
#pragma once

//---------------------------------------------------------------------
// CORE interface.
//---------------------------------------------------------------------
class IClientGameImportCore {
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
// COMMANDBUFFER interface.
//---------------------------------------------------------------------
class IClientGameImportCommandBuffer {
	// Adds text at the end of command buffer.
	virtual void AddText(const std::string &text) = 0;

	// Inserts text at the beginning of the command buffer.
	virtual void InsertText(const std::string& text) = 0;

	// Executes the current command buffer.
	virtual void Execute() = 0;

	// Adds the current command line text as a clc_stringcmd to the client 
	// message. Things like godmode, noclip, etc, are commands directed to 
	// the server, so when they are typed in at the console, they will 
	// need to be forwarded.
	virtual qboolean ForwardToServer() = 0;
};

//---------------------------------------------------------------------
// COLLISIONMODEL interface.
//---------------------------------------------------------------------
class IClientGameImportCollisionModel {
	// Creates a clipping hull for an arbitrary box.
	virtual mnode_t* HeadnodeForBox(const vec3_t& mins, const vec3_t& maxs) = 0;
	// We need a way to share these values to cgame dll.
	virtual mmodel_t* InlineModel(cm_t* cm, const char* name) = 0;
	// TODO: Document.
	virtual int32_t PointContents(const vec3_t& p, mnode_t* headNode) = 0;
	virtual int32_t TransformedPointContents(const vec3_t& p, mnode_t* headNode, const vec3_t& origin, const vec3_t& angles) = 0;
	virtual void BoxTrace(trace_t* trace, const vec3_t& start, const vec3_t& end, const vec3_t& mins, const vec3_t& maxs, mnode_t* headNode, int32_t brushmask) = 0;
	virtual void TransformedBoxTrace(trace_t* trace, const vec3_t& start, const vec3_t& end, const vec3_t& mins, const vec3_t& maxs, mnode_t* headNode, int32_t brushmask, const vec3_t& origin, const vec3_t& angles) = 0;
	virtual void ClipEntity(trace_t* dst, const trace_t* src, struct entity_s* ent) = 0;
};

//---------------------------------------------------------------------
// COMMAND interface.
//---------------------------------------------------------------------
class IClientGameImportCommand {
	// WID: TODO: These xcommand_t etc things need more work.
	// Adds the function with the command name to the list of client commands.
	virtual void AddCommand(const std::string &commandName, xcommand_t function) = 0;
	// Removes the command name from the client command list.
	virtual void RemoveCommand(const std::string& commandName) = 0;

	// Registers the list of commands to the client command list.
	virtual void Register(const cmdreg_t* reg) = 0;
	// Deregisters the list of commands to the client command list.
	virtual void Deregister(const cmdreg_t* reg) = 0;

	// Adds a macro command to the list of client macros.
	virtual void AddMacro(const std::string& name, xmacro_t function) = 0;
	// Finds the macro matching the name, and returns a pointer to it.
	virtual cmd_macro_t* FindMacro(const std::string& name) = 0;
	// TODO: Document. (MacroGenerate? Hehe)
	virtual void Macro_g(genctx_t* context) = 0; // <-- Yeah.. lol
	// virtual void GenerateMacro(genctx_t *context) = 0;
	// 
	// Adds a match for generating in the cmd prompt.
	virtual qboolean Prompt_AddMatch(genctx_t* ctx, const std::string& str) = 0;

	// Takes a null terminated string.  Does not need to be \n terminated.
	// breaks the string up into arg tokens.
	virtual void TokenizeString(const std::string& text, qboolean macroExpand) = 0;
	// Returns the amount of arguments fed to the current command.
	virtual int32_t ArgumentCount() = 0;
	// Returns the value of the argument number fed to the current command.
	virtual std::string ArgumentValue(int32_t argumentValue) = 0;
	// Returns the original argument string fed to the current command.
	virtual std::string ArgumentString() = 0;
	// Executes matching cmd triggers.
	virtual void ExecuteTrigger(const std::string &str) = 0;
};

//---------------------------------------------------------------------
// COMMON interface.
//---------------------------------------------------------------------
class IClientGameImportCommon {
	// WID: TODO: These are interesting, because templates or this... hmm haha.
	// I'd suggest actually wrapping them up, as per usual.
	//
	// Will discuss this later with you, Mr Fox ;-)
	virtual void Error(ErrorType code, const char* fmt, ...) = 0;
	virtual void Printf(PrintType type, const char* fmt, ...) = 0;

	// Returns a string description value of the given qerror_t type.
	virtual const char* ErrorString (qerror_t type) = 0;
	// Returns the event time between "common event frames" engine internal.
	virtual unsigned GetEventTime(void) = 0;
};

//---------------------------------------------------------------------
// CONSOLE interface.
//---------------------------------------------------------------------
class IClientGameImportConsole {
	virtual void ClearNotify() = 0;
	virtual void SkipNotify(qboolean skip) = 0;
};

//---------------------------------------------------------------------
// CVAR interface.
//---------------------------------------------------------------------
class IClientGameImportCVar {
	// Creates the variable if it doesn't exist, or returns the existing one
	// if it exists, the value will not be changed, but flags will be ORed in
	// that allows variables to be unarchived without needing bitflags.
	virtual cvar_t* Get(const char* variableName, const char* value, int32_t flags) = 0;
	// Creates weak variable without value.
	virtual cvar_t* WeakGet(const char* variableName) = 0;
	// Returns true if the cvar exists, false otherwise.
	// If weak is true, it'll also return true for weak cvars.
	virtual qboolean Exists(const char* name, qboolean weak) = 0;

	// Returns the float value of the cvar.
	virtual float VariableValue(const char* var_name) = 0;
	// Returns the integer value of the cvar.
	// NOTE: returns 0 if not defined or non numeric.
	virtual int VariableInteger(const char* variableName) = 0;
	// Returns the string value of the cvar.
	virtual const char* VariableString(const char* variableName) = 0;

	// Will create the variable if it doesn't exist.
	// Sets the string value.
	virtual cvar_t* Set(const char* variableName, const char* value) = 0;
	// Sets a floating point value on the cvar.
	virtual void SetValue(cvar_t* cvariable, float value, from_t from) = 0;
	// Sets an integer value on the cvar.
	virtual void SetInteger(cvar_t* cvariable, int32_t value, from_t from) = 0;
	// Will set the variable even if NOSET or LATCH.
	virtual cvar_t* UserSet(const char* variableName, const char* value) = 0;
	// Resets the cvar to its default value.
	virtual void Reset(cvar_t* cvariable) = 0;

	// Clamps the cvar in the integer range min and max.
	virtual int32_t ClampInteger(cvar_t* cvariable, int32_t minimal, int32_t maximal) = 0;
	// Clamps the cvar in the float range min and max.
	virtual float ClampValue(cvar_t* cvariable, float minimal, float maximal) = 0;

	// TODO: Document.
	// WID: Old names, suppose and never checked been long ago "GENERATE" aka _g...
	//virtual void Variable_g) (genctx_t* ctx);
	//virtual void Default_g) (genctx_t* ctx);
	virtual void GenerateVariable(genctx_t* context) = 0;
	virtual void DefaultVariable(genctx_t* context) = 0;
};

//---------------------------------------------------------------------
// FILESYSTEM interface.
//---------------------------------------------------------------------
class IClientGameImportFileSystem {
	// Renames the file contained in string from, to the one in string to.
	virtual qerror_t RenameFile(const char* from, const char* to) = 0;
	// Creates the directories of the given path.
	virtual qerror_t CreatePath(char* path) = 0;

	// Opens the given filename and stores it in the passed to handle.
	// Returns 0 in case the file could not open.
	virtual ssize_t OpenFile(const char* filename, qhandle_t* file, uint32_t mode) = 0;
	// Closes the given file handle.
	virtual void CloseFile(qhandle_t file) = 0;
	// A wrapper function for opening a file in a given directory,
	// with the given name, and extension.
	virtual qhandle_t EasyOpenFile(char* buffer, size_t size, uint32_t mode,
		const char* directory, const char* name, const char* extension) = 0;

	// Checks if the given file exists.
	virtual qhandle_t FileExists(const char* path) = 0;
	// Check if the given file exists, matching the given flags.
	virtual qhandle_t FileExistsEx(const char* path, uint32_t flags) = 0;

	// Writes data to of a given length to the file(path).
	virtual qerror_t WriteFile(const char* path, const void* data, size_t length) = 0;
	// Another easy function, should be understandable now.
	virtual qboolean EasyWriteFile(char* buffer, size_t size, unsigned mode,
		const char* directory, const char* name, const char* extension,
		const void* data, size_t length) = 0;
	// Read the given length from the file handle into the given buffer.
	virtual ssize_t Read(void* buffer, size_t length, qhandle_t file) = 0;
	// Write from the given buffer into the given file with given length.
	virtual ssize_t Write(const void* buffer, size_t len, qhandle_t f) = 0;
	// properly handles partial reads
	// File printf.
	virtual ssize_t Printf(qhandle_t file, const char* format, ...) = 0;
	// Read file line into buffer.
	virtual ssize_t ReadLine(qhandle_t file, char* buffer, size_t size) = 0;
	// Flushes file handle.
	virtual void Flush(qhandle_t file) = 0;
	// Tell where file handle is.
	virtual ssize_t Tell(qhandle_t file) = 0;
	// Seek in file handle to given offset.
	virtual qerror_t Seek(qhandle_t file, off_t offset) = 0;
	// Retrieve given file handle length.
	virtual ssize_t Length(qhandle_t file) = 0;

	// Wildcard comparison.
	virtual qboolean WildCardCompare(const char* filter, const char* string) = 0;
	// Extension comparison.
	virtual qboolean ExtensionCompare(const char* extension, const char* string) = 0;
	// Last modified.
	virtual qerror_t LastModified(char const* file, uint64_t* lastTimeModified) = 0;

	// File list functionality.
	virtual void** ListFiles(const char* path, const char* filter, uint32_t flags, int32_t* count_p) = 0;
	virtual void** CopyList(void** list, int count) = 0;
	virtual file_info_t* CopyInfo(const char* name, size_t size, time_t ctime, time_t mtime) = 0;
	virtual void FreeList(void** list) = 0;
	// Normalize path.
	virtual size_t NormalizePath(char* out, const char* in) = 0;
	virtual size_t NormalizePathBuffer(char* out, const char* in, size_t size) = 0;
	// Validate path.
	virtual int32_t ValidatePath(const char* s) = 0;
	// Sanitize cvar filename variable, so it is safe to use.
	virtual void SanitizeFilenameVariable(cvar_t* var) = 0;
};

//---------------------------------------------------------------------
// KEYBOARD interface.
//---------------------------------------------------------------------
class IClientGameImportKeyboard {
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
	virtual const char* GetBindingForKey(int32_t keynum) = 0;
	// Fills the binding string with the name of the binding matching to the key.
	// Returns -1 in case nothing was found.
	virtual int EnumBindings(int32_t key, const char* binding) = 0;
	// Sets keybinding for the given keynum.
	virtual void SetBinding(int32_t keyNumber, const char* binding) = 0;
};

//---------------------------------------------------------------------
// MEMORY interface.
//---------------------------------------------------------------------
class IClientGameImportMemory {
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
class IClientGameImportMovement {
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
class IClientGameImportMouse {
	virtual void GetMotion(int32_t deltaX, int32_t deltaY) = 0;
};

//---------------------------------------------------------------------
// MESSAGE interface.
//---------------------------------------------------------------------
class IClientGameImportMessage {
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
	virtual int32_t FlushTo(SizeBuffer* buffer) = 0;
};

//---------------------------------------------------------------------
// REGISTER interface.
//---------------------------------------------------------------------
class IClientGameImportRegister {
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
class IClientGameImportRenderer {
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
	virtual qboolean GetPictureSize(int32_t* width, int32_t* height, qhandle_t picture) = 0;   // returns transparency bit
	virtual void DrawPicture(int32_t x, int32_t y, qhandle_t picture) = 0;
	virtual void DrawStretchedPicture(int32_t x, int32_t y, int32_t width, int32_t height, qhandle_t picture) = 0;
	virtual void TileClear(int32_t x, int32_t y, int32_t width, int32_t height, qhandle_t picture) = 0;
	virtual void DrawFill8(int32_t x, int32_t y, int32_t width, int32_t height, int32_t character) = 0;
	virtual void DrawFill32(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) = 0;
};

//---------------------------------------------------------------------
// SCREEN interface.
//---------------------------------------------------------------------
class IClientGameImportScreen {
	//---------------------------------------------------------------------
	// Screen.
	//---------------------------------------------------------------------
	// This is called every frame by the client itself. However in the case
	// of this CG Module, it can also be called explicitly to flush text to
	// the screen.
	virtual void UpdateScreen() = 0;
};

//---------------------------------------------------------------------
// Sound interface.
//---------------------------------------------------------------------
class IClientGameImportSound {
	// Begins the sound registration process.
	virtual void BeginRegistration(void) = 0;
	// Precaches a sound with the given filename. (Can be a path.)
	virtual qhandle_t RegisterSound(const char* name) = 0;
	// Ends the sound registration process.
	virtual void EndRegistration(void) = 0;

	// Plays a sound at the given origin. If origin is NULL, the sound will
	// be dynamically sourced from the entity.
	virtual void StartSound(const vec3_t* origin, int32_t entityNumber, int32_t entityChannel,
		qhandle_t sfx, float fvol, float attenuation, float timeOffset);
	// Plays a local 2D sound on entchannel 0.                               
	virtual void StartLocalSound(const char* s) = 0;
	// Plays a local 2D sound on entchannel 256.                               
	virtual void StartLocalSound_(const char* s) = 0;

	// Enables under water special audio effect.
	virtual void EnableUnderWater() = 0;
	// Disables under water special audio effect.
	virtual void DisableUnderWater() = 0;
};

//---------------------------------------------------------------------
// SYSTEM interface.
//---------------------------------------------------------------------
class IClientGameImportSystem {
	virtual uint32_t Milliseconds() = 0;
};

//---------------------------------------------------------------------
// ENTITY interface.
//---------------------------------------------------------------------
class IClientGameImportEntities {
	// Executed whenever an entity event is receieved.
	virtual void Event(int32_t number) = 0;
};

//---------------------------------------------------------------------
// MAIN interface to implement. It holds pointers to actual sub interfaces,
// which one of course has to implement as well.
//---------------------------------------------------------------------
class IClientGameImports {
public:
	// WID: TODO: Normally we'd use a Get, should we do that and make these private?
	// Perhaps not.
	IClientGameImportCore* core;
	IClientGameImportCollisionModel* collisionModel;
	IClientGameImportCommand* command;
	IClientGameImportCommandBuffer* commandBuffer;
	IClientGameImportCommon* common;
	IClientGameImportConsole* console;
	IClientGameImportCVar* cvar;
	IClientGameImportEntities* entities;
	IClientGameImportFileSystem* filesystem;
	IClientGameImportMessage* message;
	IClientGameImportMouse* mouse;
	IClientGameImportMovement* movement;
	IClientGameImportRegister* registry;
	IClientGameImportRenderer* renderer;
	IClientGameImportScreen* screen;
	IClientGameImportSound* sound;
	IClientGameImportSystem* system;


	///
	/// Belong to exports.
	///
	//// Calculates the FOV the client is running. (Important to have in order.)
	//virtual float ClientCalculateFieldOfView(float x, float width, float height) = 0;

	//// Called upon whenever a client disconnects, for whichever reason.
	//// Could be him quiting, or pinging out etc.
	//virtual void ClientClearState() = 0;

	//// Updates the origin. (Used by the engine for determining current audio position too.)
	//virtual void UpdateClientOrigin() = 0;

	//// Called when a demo is being seeked through.
	//virtual void DemoSeek() = 0;

	//// Called after all downloads are done. (Aka, a map has started.)
	//// Not used for demos.
	//virtual void ClientBegin() = 0;
	//// Called each VALID client frame. Handle per VALID frame basis 
	//// things here.
	//virtual void ClientDeltaFrame() = 0;
	//// Called each client frame. Handle per frame basis things here.
	//virtual void ClientFrame() = 0;
	//// Called when a disconnect even occures. Including those for Com_Error
	//virtual void ClientDisconnect() = 0;

	//// Called when there is a needed retransmit of user info variables.
	//virtual void ClientUpdateUserinfo(cvar_t* var, from_t from) = 0;
};

