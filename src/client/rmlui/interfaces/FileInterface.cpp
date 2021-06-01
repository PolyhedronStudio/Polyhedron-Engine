// LICENSE HERE.

//
// client/rmlui/interfaces/FileInterface.cpp
//
//
// RmlUI N&C File Interface implementation.
//
// RmlUi Includes.
#include <RmlUi/Core.h>
#include <RmlUi/Core/URL.h>
#include <RmlUi/Debugger.h>

// Client includes.
#include "../../client.h"
#include "client/sound/vorbis.h"
#include "client/gamemodule.h"

// RmlUi Client includes.
#include "../rmlui.h"
#include "FileInterface.h"

//
//=============================================================================
// Open
//
// Wraps up FS_FOpenFile for RmlUI.
//=============================================================================
//
Rml::FileHandle RmlUIFileInterface::Open(const Rml::String& path) {
	// Check path.
	Rml::URL url(path);
	std::string protocol = url.GetProtocol();

	// TODO: Possible protocol check?

	// Quake FS handle.
	qhandle_t qhFile;

	// Try and open the file using our FileSystem.
	ssize_t retVal = FS_FOpenFile(url.GetPathedFileName().c_str(), &qhFile, FS_MODE_READ );

	if (retVal <= 0)
		return 0;

	// Store file size.
	fileSizeMap[qhFile] = retVal;

	return qhFile;
}

//
//=============================================================================
// Read
//
// Wraps up FS_FCloseFile for RmlUI.
//=============================================================================
//
void RmlUIFileInterface::Close(Rml::FileHandle file) {
	// We'll assume all is fine and well.
	if (file != 0) {
		fileSizeMap.erase((qhandle_t)file);
		FS_FCloseFile((qhandle_t)file);
	}
}

//
//=============================================================================
// Read
//
// Wraps up FS_Read for RmlUI.
//=============================================================================
//
size_t RmlUIFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file) {
	// Read file using our FS.
	size_t retVal = FS_Read(buffer, size, file);
	return retVal;
}

//
//=============================================================================
// Seek
//
// Wraps up FS_Seek for RmlUI.
//=============================================================================
//
bool RmlUIFileInterface::Seek(Rml::FileHandle file, long offset, int origin) {
	int method = FS_SEEK_SET;
	if (origin == SEEK_SET) {
		method = FS_SEEK_SET;
	}
	else if (origin == SEEK_END) {
		method = FS_SEEK_END;
	}
	else if (origin == SEEK_CUR) {
		method = FS_SEEK_CUR;
	}
	else {
		return false;
	}

	// Seek returns Q_ERR_SUCCESS when succeeded. Which is 0.
	// In such a case, return true, seeking succeeded.
	if (FS_SeekEx(file, origin, method) == Q_ERR_SUCCESS)
		return true;
	else
		return false;
}

//
//=============================================================================
// Tell
//
// Wraps up FS_Tell for RmlUI.
//=============================================================================
//
size_t RmlUIFileInterface::Tell(Rml::FileHandle file) {
	return FS_Tell(file);
}

//
//=============================================================================
// Length
//
// Returns the cached length belonging to the file handle.
//=============================================================================
//
size_t RmlUIFileInterface::Length(Rml::FileHandle file) {
	qhandle_t qhFile = (qhandle_t)file;
	fileSizeMap_t::iterator it = fileSizeMap.find(qhFile);

	// This means that the file was never opened at all.
	if (it == fileSizeMap.end()) {
		return 0;
	}
	return fileSizeMap[qhFile];
}