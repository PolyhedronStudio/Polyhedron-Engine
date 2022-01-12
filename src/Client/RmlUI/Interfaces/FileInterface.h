// LICENSE HERE.

//
// client/rmlui/interfaces/FileInterface.h
//
//
// RmlUI Polyhedron File Interface implementation.
//
#ifndef __CLIENT_RMLUI_INTERFACES_FILEINTERFACE_H__
#define __CLIENT_RMLUI_INTERFACES_FILEINTERFACE_H__

// Required include here to prevent collisions.
#include <map>
#include "Shared/Shared.h"

//
// Simple Polyhedron System Interface to Rml
//
class RmlUIFileInterface : public Rml::FileInterface
{
public:
	// File
	typedef std::map<qhandle_t, size_t> fileSizeMap_t;
private:
	// Contains the file size, based on file ID in the map.
	fileSizeMap_t fileSizeMap;

public:
	// Opens a file.
	virtual Rml::FileHandle Open(const Rml::String& path);

	// Closes a previously opened file.
	virtual void Close(Rml::FileHandle file);

	// Reads data from a previously opened file.
	virtual size_t Read(void* buffer, size_t size, Rml::FileHandle file);

	// Seeks to a point in a previously opened file.
	virtual bool Seek(Rml::FileHandle file, long offset, int origin);

	// Returns the current position of the file pointer.
	virtual size_t Tell(Rml::FileHandle file);

	/// Returns the length of the file.
	/// The default implementation uses Seek & Tell.
	/// @param file The handle of the file to be queried.
	/// @return The length of the file in bytes.
	virtual size_t Length(Rml::FileHandle file);
};

#endif // __CLIENT_RMLUI_INTERFACES_FILEINTERFACE_H__
